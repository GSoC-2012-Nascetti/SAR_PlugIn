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
#include "stdlib.h"
#include <stdio.h>
#include "StringUtilities.h"
#include "switchOnEncoding.h"
#include "exportGCP.h"
#include "TerraSAR_Metadata.h"
#include "TypeConverter.h"
#include "UtilityServices.h"

#include <QtCore/QStringList>
#include <QtGui/QInputDialog>

#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics/stats.hpp"
#include "boost/accumulators/statistics/mean.hpp"
#include "boost/accumulators/statistics/variance.hpp"
#include "boost/lexical_cast.hpp"


//#include "opencv\cxcore.h"
//#include "opencv2\calib3d\calib3d.hpp"

#include <fstream>
using namespace boost::accumulators;

REGISTER_PLUGIN_BASIC(OpticksSAR, exportGCP);

exportGCP::exportGCP(void)
{
   setDescriptorId("{0FD5941E-0BE2-11E2-B828-99A56188709B}");
   setName("exportGCP");
   setDescription("Export GCPs data");
   setCreator("Andrea Nascetti");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[SAR PlugIn]/Tools/Export GCPs");
   setAbortSupported(true);
}

exportGCP::~exportGCP(void)
{
}

bool exportGCP::getInputSpecification(PlugInArgList* &pInArgList)
{
	//cv::Mat provaMatrice = cv::Mat(1000,1000,CV_16S);

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

bool exportGCP::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   pOutArgList->addArg<double>("Complex Mean", "The result is the mean value of Amplitude");

   return true;
}

bool exportGCP::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
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

    std::list<GcpPoint> Punti = GCPs->getSelectedPoints();
     

	int N=Punti.size();
	int n=0;
	//double Lat, Lon;

	ofstream file_out;
	file_out.open("C:\\GCPs_Stampa2.txt");
	file_out.precision(15);

	if ( ! file_out.is_open() ) {  
	   std::string msg = "Failed to open file";
       pProgress->updateProgress(msg, 0, ERRORS);
    }

	accumulator_set<double, stats<tag::mean, tag::variance> > accX, accY;

    file_out <<"Lon		Lat		Est		Nord	I	J	DI		DJ"<<endl;

	std::list<GcpPoint>::iterator pList;
	for (pList = Punti.begin(); pList != Punti.end(); pList++)
	{
		UtmPoint puntoUTM = UtmPoint(pList->mCoordinate);
		file_out <<pList->mCoordinate.mX<<"	"<<pList->mCoordinate.mY<<"	"<<puntoUTM.getEasting()<<"	"<<puntoUTM.getNorthing()<<" "<<pList->mPixel.mX<<" "<<pList->mPixel.mY<<" "<<pList->mRmsError.mX<<" "<<pList->mRmsError.mY <<endl;

/*		if(pList->mPixel.mX<Prova_metadata.Width && pList->mPixel.mY<Prova_metadata.Height)	
		{
			Lon = pList->mCoordinate.mX;
			Lat = pList->mCoordinate.mY;
			
			Punto = ModProva.SAR_GroundToSlant(pList->mCoordinate.mX,pList->mCoordinate.mY,pList->mCoordinate.mZ); 
			pList->mRmsError.mX = pList->mPixel.mX -Punto.I;
			pList->mRmsError.mY = pList->mPixel.mY -Punto.J;
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
*/
	}

	file_out.close();

	if (pProgress != NULL)
	{
		std::string msg = "GCPs Export Complete" ;
				  						                      
		pProgress->updateProgress(msg, 100, NORMAL);
	}

	pStep->finalize(); 

	return true;
}