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
#include "Test_Update_RADARSAT2.h"
#include "UtilityServices.h"

REGISTER_PLUGIN_BASIC(OpticksSAR, Test_Update_RADARSAT2);

Test_Update_RADARSAT2::Test_Update_RADARSAT2(void)
{
   setDescriptorId("{5B0E9306-AFF1-11E1-9476-083F6288709B}");
   setName("Test_Update_RADARSAT2");
   setDescription("Accessing RADARSAT2 Metadata");
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[SAR PlugIn]/Test Update RADARSAT2 Metadata");
   setAbortSupported(true);
}

Test_Update_RADARSAT2::~Test_Update_RADARSAT2(void)
{
}

bool Test_Update_RADARSAT2::getInputSpecification(PlugInArgList* &pInArgList)
{
   pInArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pInArgList != NULL);
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Update SAR Metadaa for this raster element");
   return true;
}

bool Test_Update_RADARSAT2::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   pOutArgList->addArg<double>("Complex Mean", "The result is the mean value of Amplitude");

   return true;
}

bool Test_Update_RADARSAT2::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
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
		   
	RADARSAT_Metadata Prova_metadata;

    bool control = Prova_metadata.ReadFile(path);

	if (control == false)
	{
	std::string msg = "This is not a RADARSAT-2 SLC Files, Metadata can't be updated";
	pProgress->updateProgress(msg, 100, ERRORS);
	return false;
	}

	Prova_metadata.UpdateMetadata(Metadata); 
     
	//   ************************************************************************************ //

	if (pProgress != NULL)
	{
		std::string msg = "Number of Rows : " + StringUtilities::toDisplayString(pDesc->getRowCount()) + "\n"
						  "Number of Columns : " + StringUtilities::toDisplayString(pDesc->getColumnCount()) + "\n\n"
						  "XML file path : " + StringUtilities::toDisplayString(path) + "\n\n"
						  "Metadata update completed: " + "\n";
				  						                      
		pProgress->updateProgress(msg, 100, NORMAL);
	}

	pStep->finalize(); 

	return true;
}