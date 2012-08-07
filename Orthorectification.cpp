/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataElement.h"
#include "DesktopServices.h"
#include "GcpLayer.h"
#include "GcpList.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "Orthorectification.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "ProgressResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "PlugInResource.h"
#include "switchOnEncoding.h"

#include <complex>
#include <cmath>
#include <iostream>
#include <fstream>

double bilinear_height(DataAccessor pSrcAcc, double I, double J)
   {

	   double z1=0,z2=0,z3=0,z4=0;
       pSrcAcc->toPixel(int(J),int(I));
	   VERIFY(pSrcAcc.isValid());
	   z1 = pSrcAcc->getColumnAsDouble();

       pSrcAcc->toPixel(int(J)+1,int(I));
	   VERIFY(pSrcAcc.isValid());
	   z2 = pSrcAcc->getColumnAsDouble();

       pSrcAcc->toPixel(int(J),int(I)+1);
	   VERIFY(pSrcAcc.isValid());
	   z3 = pSrcAcc->getColumnAsDouble();

       pSrcAcc->toPixel(int(J)+1,int(I)+1);
	   VERIFY(pSrcAcc.isValid());
	   z4 = pSrcAcc->getColumnAsDouble();

       double a=0,b=0,c=0,d=0;    

	   d = z1;
	   a = (z2-z1);
	   b = (z3-z1);
	   c = (z4+z1-z2-z3);
	   
	   double H = a*(J-int(J))+b*(I-int(I))+c*(I-int(I))*(J-int(J))+d; 

	   return H;
   }

namespace
{  
   template<typename T> 
   void copypixel3(T* pData, DataAccessor pSrcAcc, int I, int J, int boxsize)
   {
	  double pixel=0;
	  
	  for (int n=-1;n<2;n++) for(int m=-boxsize;m<boxsize+1;m++)
	  {
		  pSrcAcc->toPixel(J+n,I+m);
          VERIFYNRV(pSrcAcc.isValid());
          pixel += pSrcAcc->getColumnAsDouble();
	  }

      *pData = static_cast<T>((pixel/((2*boxsize+1)*3.0))); 
   }
};

Orthorectification::Orthorectification(RasterElement *inputRaster, SAR_Model *inputModel,GRID inputGrid,double inputHeight)//,GRID *inputGrid,float *inputModel)
{
	Image = inputRaster;

	OrthoGrid = inputGrid;

	Model = inputModel;

	FlatHeight = inputHeight;

	setAbortSupported(true);
}

bool Orthorectification::getInputSpecification(PlugInArgList*& pInArgList)
{
	return true;
}

bool Orthorectification::getOutputSpecification(PlugInArgList*& pOutArgList)
{
    return true;
}

bool Orthorectification::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
	return true;
}

bool Orthorectification::execute(int type)
{
	StepResource pStep("Orthorectification Process", "app", "B4D426EC-E06D-11E1-83C8-42E56088709B");
	pStep->addStep("Start","app", "B4D426EC-E06D-11E1-83C8-42E56088709B");

	boxsize=0;
	res_type = type;
	
	if (res_type == 0) 
	{
		boxsize=0;	
	}
	else if (res_type == 1)
	{
		boxsize=1;		
	}
	else if (res_type == 2)
	{
		boxsize=2;		
	}
	else if (res_type == 3)
	{
		boxsize=3;	
	}

    ProgressResource pResource("ProgressBar");

	Progress *pProgress=pResource.get(); 

	pProgress->setSettingAutoClose(false);

	RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(Image->getDataDescriptor());
    
    FactoryResource<DataRequest> pRequest;
    DataAccessor pSrcAcc = Image->getDataAccessor(pRequest.release());
 
	unsigned int N_Row = int(OrthoGrid.Y_Dim)+1;
	unsigned int N_Col = int(OrthoGrid.X_Dim)+1;

    // Check name of raster element //

	Service<ModelServices> pModel;
    vector<string> mCubeNames = pModel->getElementNames("RasterElement");

	int NameIndex = 0, control=0;
	stringstream out;
	string OutputName=Image->getName();

	string OutputName1 = OutputName.substr(0,OutputName.find_last_of("."));
	while (control == 0)
	{
		control = 1;
		OutputName = OutputName1+"_ortho_";
		out << NameIndex;
		OutputName.append(out.str()+".tiff");		
		
		for (unsigned int k=0; k<mCubeNames.size(); k++)
		{
		if (OutputName.compare(mCubeNames[k]) == 0) control = 0;
		}

		NameIndex++;
		out.str("");
		out.clear();

	}

	// Create output raster element and assoiciated descriptor and accessor //
	
	ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(OutputName,N_Row ,N_Col, FLT4BYTES));

	RasterDataDescriptor* pResultDesc = static_cast<RasterDataDescriptor*> (pResultCube->getDataDescriptor());

    FactoryResource<DataRequest> pResultRequest;
    pResultRequest->setWritable(true);
    DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());

	double NodeLat, NodeLon;

	for (unsigned int row = 0; row < N_Row; ++row)
	{
	  if (pProgress != NULL)
	  {
      pProgress->updateProgress("Calculating result", row * 100 / N_Row, NORMAL);
	  }

	  if (isAborted())
      {
         std::string msg = getName() + " has been aborted.";
         pStep->finalize(Message::Abort, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ABORT);
         }
         return false;
      }

      if (!pDestAcc.isValid())
      {
         std::string msg = "Unable to access the cube data.";
         pProgress->updateProgress(msg, 0, ERRORS);
		 pStep->finalize(Message::Abort, msg);
         return false;
      }

      for (unsigned int col = 0; col < N_Col; ++col)
      {

		  NodeLat = OrthoGrid.Lat_Min+row*OrthoGrid.Lat_Step;
		  NodeLon = OrthoGrid.Lon_Min+col*OrthoGrid.Lon_Step;

		  P_COORD NodeImage = Model->SAR_GroundToSlant(NodeLon,NodeLat,FlatHeight);

		  if ((NodeImage.I>1 && NodeImage.I< Model->Metadata.Width-1) && (NodeImage.J>1 && NodeImage.J< Model->Metadata.Height-1))
		  {
			switchOnEncoding(pResultDesc->getDataType(), copypixel3, pDestAcc->getColumn(), pSrcAcc, int(NodeImage.I), int(NodeImage.J),boxsize);	
		  }		  
		  pDestAcc->nextColumn();
	  }

      pDestAcc->nextRow();
	}

	Service<DesktopServices> pDesktop;

	Service<ModelServices> pMod;

	GcpList* GcpL = static_cast<GcpList*>(pMod->createElement("corner coordinate","GcpList",pResultCube.get()));
   
	// Update GCPs Information: to account for Opticks reading gcp lat&lon values the opposite way around, 
	// here it is necessary to switch the value to assign lat to gcp.mCoordinate.mX  and lon to gcp.mCoordinate.mY 

	GcpPoint Punto;

	Punto.mCoordinate.mX = OrthoGrid.Lat_Min;
	Punto.mCoordinate.mY = OrthoGrid.Lon_Min;
	Punto.mCoordinate.mZ = 0.0;
	Punto.mPixel.mX = 0.0;
	Punto.mPixel.mY = 0.0;

	GcpL->addPoint(Punto);

	Punto.mCoordinate.mX = OrthoGrid.Lat_Max;
	Punto.mCoordinate.mY = OrthoGrid.Lon_Min;
	Punto.mCoordinate.mZ = 0.0;
	Punto.mPixel.mX = 0.0;
	Punto.mPixel.mY = OrthoGrid.Y_Dim;
   
	GcpL->addPoint(Punto);

	Punto.mCoordinate.mX = OrthoGrid.Lat_Min;
	Punto.mCoordinate.mY = OrthoGrid.Lon_Max;
	Punto.mCoordinate.mZ = 0.0;
	Punto.mPixel.mX = OrthoGrid.X_Dim;
	Punto.mPixel.mY = 0.0;
   
	GcpL->addPoint(Punto);

	Punto.mCoordinate.mX = OrthoGrid.Lat_Max;
	Punto.mCoordinate.mY = OrthoGrid.Lon_Max;
	Punto.mCoordinate.mZ = 0.0;
	Punto.mPixel.mX = OrthoGrid.X_Dim;
	Punto.mPixel.mY = OrthoGrid.Y_Dim;
   
	GcpL->addPoint(Punto);

	SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(),
         SPATIAL_DATA_WINDOW));
 
	SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();  

	pView->setPrimaryRasterElement(pResultCube.get());

	pView->createLayer(RASTER, pResultCube.get());
   
	pView->createLayer(GCP_LAYER,GcpL,"GCP");

	pView->setDataOrigin(LOWER_LEFT);

	pResultCube.release();

	pProgress->updateProgress("Orthorectification is complete.", 100, NORMAL);
	pStep->addStep("End","app", "B4D426EC-E06D-11E1-83C8-42E56088709B");
    pStep->finalize();
    
	return true;

}

bool Orthorectification::execute(int type, RasterElement *pDSM, GRID DSMGrid, double Geoid_Offset, int DSM_resampling)
{
	StepResource pStep("Orthorectification Process", "app", "B4D426EC-E06D-11E1-83C8-42E56088709B");
	pStep->addStep("Start","app", "B4D426EC-E06D-11E1-83C8-42E56088709B");
	boxsize=0;

	res_type = type;

	if (res_type == 0) 
	{
		boxsize=0;	
	}
	else if (res_type == 1)
	{
		boxsize=1;		
	}
	else if (res_type == 2)
	{
		boxsize=2;		
	}
	else if (res_type == 3)
	{
		boxsize=3;	
	}

    ProgressResource pResource("ProgressBar");

	Progress *pProgress=pResource.get(); 

	pProgress->setSettingAutoClose(false);

	RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(Image->getDataDescriptor());
    
    FactoryResource<DataRequest> pRequest;
    DataAccessor pSrcAcc = Image->getDataAccessor(pRequest.release());

    RasterDataDescriptor* pDescDSM = static_cast<RasterDataDescriptor*>(pDSM->getDataDescriptor());

	FactoryResource<DataRequest> pRequestDSM;
    DataAccessor pDSMAcc = pDSM->getDataAccessor(pRequestDSM.release());

 
	unsigned int N_Row = int(OrthoGrid.Y_Dim)+1;
	unsigned int N_Col = int(OrthoGrid.X_Dim)+1;

	// Check name of raster element //
	Service<ModelServices> pModel;
    vector<string> mCubeNames = pModel->getElementNames("RasterElement");

	int NameIndex = 0, control=0;
	stringstream out;
	string OutputName=Image->getName();
	string OutputName1 = OutputName.substr(0,OutputName.find_last_of("."));

	while (control == 0)
	{
		control = 1;
		OutputName = OutputName1+"_ortho_";

		out << NameIndex;
		OutputName.append(out.str()+".tiff");		
		
		for (unsigned int k=0; k<mCubeNames.size(); k++)
		{
		if (OutputName.compare(mCubeNames[k]) == 0) control = 0;
		}

		NameIndex++;
		out.str("");
		out.clear();

	}

	// Create output raster element and assoiciated descriptor and accessor //
	
	ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(OutputName,N_Row ,N_Col, FLT4BYTES));

	RasterDataDescriptor* pResultDesc = static_cast<RasterDataDescriptor*> (pResultCube->getDataDescriptor());

    FactoryResource<DataRequest> pResultRequest;
    pResultRequest->setWritable(true);
    DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());

    double NodeLat, NodeLon, H_IJ=0;
	//int DSM_I, DSM_J;

    for (unsigned int row = 0; row < N_Row; ++row)
    {
	  if (pProgress != NULL)
	  {
      pProgress->updateProgress("Calculating result", row * 100 / N_Row, NORMAL);
	  }

	  if (isAborted())
      {
         std::string msg = getName() + " has been aborted.";
         pStep->finalize(Message::Abort, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ABORT);
         }
         return false;
      }

      if (!pDestAcc.isValid())
      {
         std::string msg = "Unable to access the cube data.";
         pProgress->updateProgress(msg, 0, ERRORS);
		 pStep->finalize(Message::Failure, msg);
         return false;
      }

      for (unsigned int col = 0; col < N_Col; ++col)
      {

		  NodeLat = OrthoGrid.Lat_Min+row*OrthoGrid.Lat_Step;
		  NodeLon = OrthoGrid.Lon_Min+col*OrthoGrid.Lon_Step;

		  // RETRIEVE HEIGHT VALUE FROM DSM 

		  if (DSM_resampling == 0) 
		  {
		  int DSM_I = int((NodeLon - DSMGrid.Lon_Min)/DSMGrid.Lon_Step);
		  int DSM_J = pDescDSM->getRowCount() - int((NodeLat - DSMGrid.Lat_Min)/DSMGrid.Lat_Step);		  
          pDSMAcc->toPixel(DSM_J,DSM_I);
	      VERIFY(pDSMAcc.isValid());
          H_IJ = (pDSMAcc->getColumnAsDouble());
		  }
		  else
		  {
		  double DSM_I = ((NodeLon - DSMGrid.Lon_Min)/DSMGrid.Lon_Step);
		  double DSM_J = pDescDSM->getRowCount() - ((NodeLat - DSMGrid.Lat_Min)/DSMGrid.Lat_Step);
		  H_IJ = bilinear_height(pDSMAcc,DSM_I,DSM_J);
		  }

		  P_COORD NodeImage = Model->SAR_GroundToSlant(NodeLon,NodeLat,H_IJ+Geoid_Offset);

		  if ((NodeImage.I>1 && NodeImage.I< Model->Metadata.Width-1) && (NodeImage.J>1 && NodeImage.J< Model->Metadata.Height-1))
		  {
			switchOnEncoding(pResultDesc->getDataType(), copypixel3, pDestAcc->getColumn(), pSrcAcc, int(NodeImage.I), int(NodeImage.J),boxsize);		
		  }		  
		  pDestAcc->nextColumn();
      }

      pDestAcc->nextRow();
    }

   Service<DesktopServices> pDesktop;

   Service<ModelServices> pMod;

   GcpList* GcpL = static_cast<GcpList*>(pMod->createElement("corner coordinate","GcpList",pResultCube.get()));
   
   // Update GCPs Information: to account for Opticks reading gcp lat&lon values the opposite way around, 
   // here it is necessary to switch the value to assign lat to gcp.mCoordinate.mX  and lon to gcp.mCoordinate.mY 

   GcpPoint Punto;

   Punto.mCoordinate.mX = OrthoGrid.Lat_Min;
   Punto.mCoordinate.mY = OrthoGrid.Lon_Min;
   Punto.mCoordinate.mZ = 0.0;
   Punto.mPixel.mX = 0.0;
   Punto.mPixel.mY = 0.0;

   GcpL->addPoint(Punto);

   Punto.mCoordinate.mX = OrthoGrid.Lat_Max;
   Punto.mCoordinate.mY = OrthoGrid.Lon_Min;
   Punto.mCoordinate.mZ = 0.0;
   Punto.mPixel.mX = 0.0;
   Punto.mPixel.mY = OrthoGrid.Y_Dim;
   
   GcpL->addPoint(Punto);

   Punto.mCoordinate.mX = OrthoGrid.Lat_Min;
   Punto.mCoordinate.mY = OrthoGrid.Lon_Max;
   Punto.mCoordinate.mZ = 0.0;
   Punto.mPixel.mX = OrthoGrid.X_Dim;
   Punto.mPixel.mY = 0.0;
   
   GcpL->addPoint(Punto);

   Punto.mCoordinate.mX = OrthoGrid.Lat_Max;
   Punto.mCoordinate.mY = OrthoGrid.Lon_Max;
   Punto.mCoordinate.mZ = 0.0;
   Punto.mPixel.mX = OrthoGrid.X_Dim;
   Punto.mPixel.mY = OrthoGrid.Y_Dim;
   
   GcpL->addPoint(Punto); 

   SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(),
         SPATIAL_DATA_WINDOW));
 
   SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();  

   pView->setPrimaryRasterElement(pResultCube.get());

   pView->createLayer(RASTER, pResultCube.get());
   
   pView->createLayer(GCP_LAYER,GcpL,"GCP");

   pView->setDataOrigin(LOWER_LEFT);

   pResultCube.release();

   pProgress->updateProgress("Orthorectification is complete.", 100, NORMAL);
   pStep->addStep("End","app", "B4D426EC-E06D-11E1-83C8-42E56088709B");
   pStep->finalize();

   return true;
   
}

Orthorectification::~Orthorectification(void)
{
}
