/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "AppConfig.h"
#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DataVariant.h"
#include "DataVariantAnyData.h" 
#include "DataVariantFactory.h" 
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include "GcpList.h"
#include "GcpLayer.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "SAR_Model.h"
#include "SAR_Ground_Model.h"
#include "SAR_Slant_Model.h"
#include "stdlib.h"
#include <stdio.h>
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "Test_Update_TerraSAR.h"
#include "TerraSAR_Metadata.h"
#include "TypeConverter.h"
#include "UtilityServices.h"

#include <QtCore/QStringList>
#include <QtGui/QInputDialog>

#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics/stats.hpp"
#include "boost/accumulators/statistics/mean.hpp"
#include "boost/accumulators/statistics/variance.hpp"

using namespace boost::accumulators;

REGISTER_PLUGIN_BASIC(OpticksSAR, Test_Update_TerraSAR);

Test_Update_TerraSAR::Test_Update_TerraSAR(void)
{
   setDescriptorId("{501590E6-AFEF-11E1-A4F3-393D6288709B}");
   setName("Test_Update_TerraSAR");
   setDescription("Accessing TerraSAR Metadata");
   setCreator("Andrea Nascetti");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[SAR PlugIn]/Orientation Model/Check TerraSAR-X");
   setAbortSupported(true);
}

Test_Update_TerraSAR::~Test_Update_TerraSAR(void)
{
}

bool Test_Update_TerraSAR::getInputSpecification(PlugInArgList* &pInArgList)
{
   pInArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pInArgList != NULL);
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Update SAR Metadaa for this raster element");

   if (isBatch())
   {
      pInArgList->addArg<GcpList>("GcpList", NULL, "The GCP List to calculate statistics over");
   }

   return true;
}

bool Test_Update_TerraSAR::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   pOutArgList->addArg<double>("Complex Mean", "The result is the mean value of Amplitude");

   return true;
}

bool Test_Update_TerraSAR::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
  StepResource pStep("Tutorial CEO", "app", "0FD3C564-041D-4f8f-BBF8-96A7A165AB61");

   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   
   if (pCube == NULL)
   {
      std::string msg = "A raster cube must be specified.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL)
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }

      return false;
   }

   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);

   FactoryResource<DataRequest> pRequest;
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());

   std::string path = pCube->getFilename();

   DataDescriptor* dMeta = pCube->getDataDescriptor();

   DynamicObject* Metadata = dMeta->getMetadata();

	// RETRIEVE & UPDATE METADATA INFORMATION //
		   
	TerraSAR_Metadata Prova_metadata;

	bool control = Prova_metadata.ReadFile(path);

	if (control == false)
	{
	std::string msg = "This is not a TerraSAR-X SLC Files, Metadata can't be updated";
	pProgress->updateProgress(msg, 100, ERRORS);
	return false;
	}

	Prova_metadata.UpdateMetadata(Metadata); 

	// WIDGET SELECT & RETRIEVE GCPs INFORMATION  //
	GcpList * GCPs = NULL;

    Service<ModelServices> pModel;
	std::vector<DataElement*> pGcpLists = pModel->getElements(pCube, TypeConverter::toString<GcpList>());

    if (!pGcpLists.empty())
	{
         QStringList aoiNames("<none>");
         for (std::vector<DataElement*>::iterator it = pGcpLists.begin(); it != pGcpLists.end(); ++it)
         {
            aoiNames << QString::fromStdString((*it)->getName());
         }
         QString aoi = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Select a GCP List", "Select a GCP List for validate the orientation model", aoiNames);
         
         if (aoi != "<none>")
         {
            std::string strAoi = aoi.toStdString();
            for (std::vector<DataElement*>::iterator it = pGcpLists.begin(); it != pGcpLists.end(); ++it)
            {
               if ((*it)->getName() == strAoi)
               {
                  GCPs = static_cast<GcpList*>(*it);
                  break;
               }
            }
            if (GCPs == NULL)
            {
               std::string msg = "Invalid GCPList.";
               pStep->finalize(Message::Failure, msg);
               if (pProgress != NULL)
               {
                  pProgress->updateProgress(msg, 0, ERRORS);
               }

               return false;
            }
         }
		 else
		 {
			 std::string msg = "A set of GCPs must be specified.";
             pStep->finalize(Message::Failure, msg);
             if (pProgress != NULL)
             {
				 pProgress->updateProgress(msg, 0, ERRORS);
             }
			 return false;
		 }
	} // End if GcpList
   
	
	// UPDATE GCPs HEIGHT INFORMATION AND SWITCH Lat&Lon COORDINATE FOR CORRECT VISUALIZAZION IN THE GCPs EDITOR

    std::list<GcpPoint> Punti = GCPs->getSelectedPoints();
     
	Punti = Prova_metadata.UpdateGCP(Punti, path, pProgress);

	//SAR_Model ModProva(Prova_metadata,1000);

	SAR_Model *ModProva;
	//ModProva = new SAR_Ground_Model(Prova_metadata);
	ModProva = new SAR_Slant_Model(Prova_metadata);

	if(Prova_metadata.Projection == "SGF")
	{
		ModProva = new SAR_Ground_Model(Prova_metadata);
	}
	//else
	//{
	//	ModProva = new SAR_Slant_Model(Prova_metadata);
	//}

	P_COORD Punto;
	int N=Punti.size();
	int n=0, indexP=0;
	double Lat, Lon;

	accumulator_set<double, stats<tag::mean, tag::variance> > accX, accY;

	list<GcpPoint>::iterator pList;
	for (pList = Punti.begin(); pList != Punti.end(); pList++)
	{
		if(pList->mPixel.mX<Prova_metadata.Width && pList->mPixel.mY<Prova_metadata.Height)	
		{
			Lon = pList->mCoordinate.mX;
			Lat = pList->mCoordinate.mY;
			
			Punto = ModProva->SAR_GroundToImage(pList->mCoordinate.mX,pList->mCoordinate.mY,pList->mCoordinate.mZ); 
			//pList->mRmsError.mX = pList->mPixel.mX -Punto.I;
			//pList->mRmsError.mY = pList->mPixel.mY -Punto.J;

			pList->mPixel.mX = (Prova_metadata.Grid_Range_Time/Prova_metadata.RowSpacing)*(indexP-int(indexP/Prova_metadata.Grid_N_Range)*Prova_metadata.Grid_N_Range);
			pList->mRmsError.mX = (Prova_metadata.Grid_Range_Time/Prova_metadata.RowSpacing)*(indexP-int(indexP/Prova_metadata.Grid_N_Range)*Prova_metadata.Grid_N_Range) -Punto.I;

			pList->mPixel.mY = (Prova_metadata.Grid_Azimuth_Time*Prova_metadata.PRF)*int(indexP/Prova_metadata.Grid_N_Range);
			pList->mRmsError.mY = (Prova_metadata.Grid_Azimuth_Time*Prova_metadata.PRF)*int(indexP/Prova_metadata.Grid_N_Range) - Punto.J;
			
			accX(pList->mRmsError.mX);
			accY(pList->mRmsError.mY);

			pList->mCoordinate.mX = Lat;
			pList->mCoordinate.mY = Lon;

		}
		else
		{
			Lon = pList->mCoordinate.mX;
			Lat = pList->mCoordinate.mY;
			pList->mRmsError.mX = -9999;
			pList->mRmsError.mY = -9999;
			pList->mCoordinate.mX = Lat;
			pList->mCoordinate.mY = Lon;
		}

		if (pProgress != NULL)
		{
         pProgress->updateProgress("Calculating statistics", int(100*n/N), NORMAL);
		}
		n++;
		indexP++;
	}

	 double meanX = mean(accX);
	 double meanY = mean(accY);

	 double varX = variance(accX);
	 double varY = variance(accY);
	
     GCPs->clearPoints();
     GCPs->addPoints(Punti);
	
	if (pProgress != NULL)
	{
		std::string msg = "Number of Rows : " + StringUtilities::toDisplayString(pDesc->getRowCount()) + "\n"
						  "Number of Columns : " + StringUtilities::toDisplayString(pDesc->getColumnCount()) + "\n\n"
						  "Metadata update completed" + "\n\n"					  
						  "**********     Validation Results     **********" "\n\n"
						  "Number of GCPs: " + StringUtilities::toDisplayString(Punti.size()) + "\n\n"
						  "Mean I : " + StringUtilities::toDisplayString(meanX) + "\n"
						  "Variance I : " + StringUtilities::toDisplayString(varX) + "\n\n"
						  "Mean J : " + StringUtilities::toDisplayString(meanY) + "\n"
						  "Variance J : " + StringUtilities::toDisplayString(varY) + "\n\n" ;
				  						                      
		pProgress->updateProgress(msg, 100, NORMAL);
	}

	pStep->finalize(); 

	return true;
}