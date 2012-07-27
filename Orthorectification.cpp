#include "Orthorectification.h"

#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataElement.h"
//#include "DataRequest.h"
#include "DesktopServices.h"

#include "GcpLayer.h"
#include "GcpList.h"
#include "ModelServices.h"
//#include "MessageLogResource.h"
//#include "ObjectResource.h"
//#include "PlugInArgList.h"
//#include "PlugInManagerServices.h"
//#include "PlugInRegistration.h"
//#include "Progress.h"
#include "ProgressResource.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "switchOnEncoding.h"

#include <complex>

namespace
{  
   template<typename T> 
   void copypixel(T* pData, DataAccessor pSrcAcc, int I, int J)
   {
	  double pixel;
      pSrcAcc->toPixel(J,I);
	  VERIFYNRV(pSrcAcc.isValid());
      pixel = pSrcAcc->getColumnAsDouble();
      *pData = static_cast<T>(pixel); 
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

bool Orthorectification::execute()
{
    ProgressResource pResource("ProgressBar");

	Progress *pProgress=pResource.get(); 

	pProgress->setSettingAutoClose(true);

	RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(Image->getDataDescriptor());
    
    FactoryResource<DataRequest> pRequest;
    DataAccessor pSrcAcc = Image->getDataAccessor(pRequest.release());
 
	unsigned int N_Row = int(OrthoGrid.Y_Dim)+1;
	unsigned int N_Col = int(OrthoGrid.X_Dim)+1;

	ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(Image->getName()+"_Ortho",N_Row ,N_Col, FLT8BYTES));

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
        // pStep->finalize(Message::Abort, msg);
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
         return false;
      }

      for (unsigned int col = 0; col < N_Col; ++col)
      {

		  NodeLat = OrthoGrid.Lat_Min+row*OrthoGrid.Lat_Step;
		  NodeLon = OrthoGrid.Lon_Min+col*OrthoGrid.Lon_Step;

		  P_COORD NodeImage = Model->SAR_GroundToSlant(NodeLon,NodeLat,FlatHeight);

		  if ((NodeImage.I>1 && NodeImage.I< Model->Metadata.Width-1) && (NodeImage.J>1 && NodeImage.J< Model->Metadata.Height-1))
		  {
			  switchOnEncoding(pResultDesc->getDataType(), copypixel, pDestAcc->getColumn(), pSrcAcc, int(NodeImage.I), int(NodeImage.J));		  		  
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

   pProgress->updateProgress("Copy is complete.", 100, NORMAL);
   
return true;

}


Orthorectification::~Orthorectification(void)
{
}
