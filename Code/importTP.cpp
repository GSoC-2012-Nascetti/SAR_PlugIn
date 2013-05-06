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
#include "GeoPoint.h"
#include "importTP.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "TypeConverter.h"
#include "UtilityServices.h"

#include <QtCore/QStringList>
#include <QtGui/QInputDialog>

#include "boost/algorithm/string/split.hpp"


#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics/stats.hpp"
#include "boost/accumulators/statistics/mean.hpp"
#include "boost/accumulators/statistics/variance.hpp"
#include "boost/lexical_cast.hpp"

#include "stdlib.h"
#include <stdio.h>
#include <fstream>

using namespace std;
using namespace boost::accumulators;

REGISTER_PLUGIN_BASIC(OpticksSAR, importTP);

importTP::importTP(void)
{
   setDescriptorId("{2657F696-147F-11E2-B196-8EC66088709B}");
   setName("importTP");
   setDescription("Import TPsdata");
   setCreator("Andrea Nascetti");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[SAR PlugIn]/Tools/Import TPs");
   setAbortSupported(true);
}

importTP::~importTP(void)
{
}

bool importTP::getInputSpecification(PlugInArgList* &pInArgList)
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

bool importTP::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   pOutArgList->addArg<double>("Complex Mean", "The result is the mean value of Amplitude");

   return true;
}

bool importTP::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
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

	int n=0;

	FILE *file_in;
	file_in = fopen("C:/input.txt","rt");

	double CoordI=0, CoordJ=0;

	while (! feof(file_in))
	{

		fscanf(file_in,"%lf %lf",&CoordI,&CoordJ);

		GcpPoint Punto;

		Punto.mPixel.mX = CoordI;
		
		Punto.mPixel.mY = CoordJ;

		GCPs->addPoint(Punto);

		pProgress->updateProgress("Calculating statistics", int(n), NORMAL);
	}

	fclose(file_in);
	
	if (pProgress != NULL)
	{
		std::string msg = "GCPs Import Complete" ;
				  						                      
		pProgress->updateProgress(msg, 100, NORMAL);
	}

	pStep->finalize(); 

	return true;
}