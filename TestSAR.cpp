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
#include "DynamicObject.h"
#include "FileDescriptor.h"
#include "Filename.h"
#include <limits>
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "RADARSAT_Metadata.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "stdlib.h"
#include <stdio.h>
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "TestSAR.h"
#include "TerraSAR_Metadata.h"
#include "UtilityServices.h"
#include "XercesIncludes.h"
#include "xmlbase.h"
#include "xmlreader.h" 
#include "xmlreaderSAR.h" 
#include "xmlwriter.h"

XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksSAR, TestSAR);

TestSAR::TestSAR(void)
{
   setDescriptorId("{30A7E348-ABCC-11E1-84E7-E7C06188709B}");
   setName("TestSAR");
   setDescription("Accessing SAR Metadata");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[SAR PlugIn]/Test SAR");
   setAbortSupported(true);
}

TestSAR::~TestSAR(void)
{
}

bool TestSAR::getInputSpecification(PlugInArgList* &pInArgList)
{
   pInArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pInArgList != NULL);
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Update SAR Metadaa for this raster element");
   return true;
}

bool TestSAR::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   pOutArgList->addArg<double>("Complex Mean", "The result is the mean value of Amplitude");

   return true;
}

bool TestSAR::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
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

   double min = std::numeric_limits<double>::max();
   double max = -min;
   
   double total = 0.0; 

   for (unsigned int row = 0; row < pDesc->getRowCount(); ++row)
   {
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
      if (!pAcc.isValid())
      {
         std::string msg = "Unable to access the cube data.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }

         return false;
      }

      if (pProgress != NULL)
      {
         pProgress->updateProgress("Calculating statistics", row * 100 / pDesc->getRowCount(), NORMAL);
      }

      for (unsigned int col = 0; col < pDesc->getColumnCount(); ++col)
      {		 
		 total+= Service<ModelServices>()->getDataValue(pDesc->getDataType(), pAcc->getColumn(), COMPLEX_MAGNITUDE, 0);

		 pAcc->nextColumn();
      }
      pAcc->nextRow();
   }

   std::string path = pCube->getFilename();

   unsigned int count = pDesc->getColumnCount() * pDesc->getRowCount();
   double mean = total / count;

   std::vector<std::string> Info, MetaInfo;

   Info = pCube->getPropertiesPages();

   DataDescriptor* dMeta = pCube->getDataDescriptor();

   DynamicObject* Metadata = dMeta->getMetadata();

	// RETRIEVE & UPDATE METADATA INFORMATION //
		   
	TerraSAR_Metadata Prova_metadata;

	Prova_metadata.ReadFile(path);

	Prova_metadata.UpdateMetadata(Metadata); 
     
	//   ************************************************************************************ //

   
	if (pProgress != NULL)
	{
		std::string msg = "Number of Rows : " + StringUtilities::toDisplayString(pDesc->getRowCount()) + "\n"
						  "Number of Columns : " + StringUtilities::toDisplayString(pDesc->getColumnCount()) + "\n\n"
						  "Complex mean value : " + StringUtilities::toDisplayString(mean) + "\n\n"
						  "XML file path : " + StringUtilities::toDisplayString(path) + "\n\n"
						  "Metadata update completed: " + "\n";
				  						                      
		pProgress->updateProgress(msg, 100, NORMAL);
	}

	pStep->finalize(); 

	return true;
}