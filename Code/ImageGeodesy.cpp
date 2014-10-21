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
#include "ImageGeodesy.h"
#include "TerraSAR_Metadata.h"
#include "UtilityServices.h"
#include "XercesIncludes.h"
#include "xmlbase.h"
#include "xmlreader.h" 
#include "xmlreaderSAR.h" 
#include "xmlwriter.h"
#include "SAR_Model.h"
#include <iostream>
#include <fstream>

#include <QtGui/qfiledialog.h>

#include "Stereo_SAR_Model.h"


XERCES_CPP_NAMESPACE_USE

REGISTER_PLUGIN_BASIC(OpticksSAR, ImageGeodesy);

ImageGeodesy::ImageGeodesy(void)
{
   setDescriptorId("{31fca1d0-0830-11e4-9191-0800200c9a66}");
   setName("SAR-ImageGeodesy");
   setDescription("Accessing SAR Metadata");
   setCreator("Andrea Nascetti");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Statistics");
   setMenuLocation("[SAR PlugIn]/Image Geodesy");
   setAbortSupported(true);
}

ImageGeodesy::~ImageGeodesy(void)
{
}

bool ImageGeodesy::getInputSpecification(PlugInArgList* &pInArgList)
{
   pInArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pInArgList != NULL);
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   return true;
}

bool ImageGeodesy::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList();
   VERIFY(pOutArgList != NULL);
   pOutArgList->addArg<double>("Complex Mean", "The result is the mean value of Amplitude");
   return true;
}

bool ImageGeodesy::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
 
  StepResource pStep("Image Geodesy APP", "app", "0FD3C564-041D-4f8f-BBF8-96A7A165AB61");

  if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());


	// RETRIEVE & UPDATE METADATA INFORMATION //
		   
	TerraSAR_Metadata Prova_metadata;
	TerraSAR_Metadata Slave_metadata;
	SAR_Model *Model;

    string path_stereo = "E:/Database_TerraSAR-X/Berlino/dims_op_oc_dfd2_339140663_1/TSX-1.SAR.L1B/TSX1_SAR__SSC______HS_S_SRA_20080814T170102_20080814T170103/TSX1_SAR__SSC______HS_S_SRA_20080814T170102_20080814T170103.xml";

	Slave_metadata.ReadFile(path_stereo);
	SAR_Model Slave_Model(Slave_metadata,100); 

	//APERTURA E LETTURA DEL FILE
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select input image coordinate file"));

	//fstream input_file ("E:/Database_TerraSAR-X/Berlino/spot028A/File_Estratti/Coord_IMG_File_Punto3_OS2-2_20-200.txt");
	fstream input_file (fileName.toStdString());

	//ofstream output_file ("E:/Database_TerraSAR-X/Berlino/spot028A/File_Estratti/prova_imaging_punto3.txt");
	ofstream output_file (fileName.toStdString()+"_results.txt");	
	output_file.precision(16);

	std::string path;
	path.reserve(500);

	if(!input_file.is_open()) return false;

	double I=0, J=0, cc_max=0;
	double X=0,Y=0,Z=0;
	double VX=0,VY=0,VZ=0;
	double stereo_I=0., stereo_J=0.;
	std::string unused;  
	int N_righe=1, N=1;
	while(getline(input_file, unused)) N_righe++;
	
	input_file.close();
	input_file.open(fileName.toStdString());

	if(input_file >> path) input_file >> stereo_I >> stereo_J;

	output_file << "Acquisition Date" << "\t" << "Min Incidence Angle" << "\t" << "Max Incidence Angle" << "\t"
				<<	"I" << "\t" << "J" << "\t" << "Range" << "\t" << "Range Free" << "\t"
				<< "SX_ecef" << "\t" << "SY_ecef" << "\t" << "SZ_ecef" << "\t"
				<< "Vx_Ecef"<< "\t" << "Vy_Ecef"<< "\t" << "Vz_Ecef"<< "\t" 
				<< "PX_ecef" << "\t" << "PY_ecef" << "\t" << "PZ_ecef" << "\t" << N_righe << endl;

	while(input_file >> I >> J >> cc_max >> path)
	{
		Prova_metadata.ReadFile(path);
		Model = new SAR_Model(Prova_metadata,100);

		Stereo_SAR_Model sModel (Model,&Slave_Model);

		//COORD_Ecef GroundPoint = sModel.Stereo_SAR_SlantToGround(I,J,5556,513);
		//COORD_Ecef GroundPoint = sModel.Stereo_SAR_SlantToGround(I,J,2663.6,855.4);
		//COORD_Ecef GroundPoint = sModel.Stereo_SAR_SlantToGround(I,J,6416.5,2472.5);
		COORD_Ecef GroundPoint = sModel.Stereo_SAR_SlantToGround(I,J,stereo_I,stereo_J);

		X = Model->StateVectorsRows[int(J*Model->PrecisionIndex)].X;
		Y = Model->StateVectorsRows[int(J*Model->PrecisionIndex)].Y;
		Z = Model->StateVectorsRows[int(J*Model->PrecisionIndex)].Z;

		VX = Model->StateVectorsRows[int(J*Model->PrecisionIndex)].VelocityX;
		VY = Model->StateVectorsRows[int(J*Model->PrecisionIndex)].VelocityY;
		VZ = Model->StateVectorsRows[int(J*Model->PrecisionIndex)].VelocityZ;

		output_file  << Prova_metadata.Date << "\t" << Prova_metadata.Min_IncidenceAngle << "\t" << Prova_metadata.Max_IncidenceAngle << "\t"
		<< I << "\t" << J << "\t" << Model->Metadata.RangeD0 + Model->Metadata.RangeDi*I << "\t" << Model->Metadata.RangeD0_Free + Model->Metadata.RangeDi*I << "\t"
		<< X << "\t" << Y << "\t" << Z << "\t" << VX << "\t" << VY << "\t" <<  VZ << "\t"  
		<< GroundPoint.X_Ecef << "\t" << GroundPoint.Y_Ecef << "\t" << GroundPoint.Z_Ecef  << "\t" 
		<< Prova_metadata.RangeCoeff1 << "\t" << Prova_metadata.RangeCoeff2 <<  endl;
		
		pProgress->updateProgress("Computing", int(100*N/N_righe), NORMAL);
		N++;
		delete Model;
	} 

	//Prova_metadata.UpdateMetadata(Metadata); 
     
	//   ************************************************************************************ //

	if (pProgress != NULL)
	{
		std::string msg = "Number of Rows : "+ Prova_metadata.Orbit + " " + StringUtilities::toDisplayString(I) + path;
				  						                      
		pProgress->updateProgress(msg, 100, NORMAL);
	}

	input_file.close();
	output_file.close();
	pStep->finalize(); 

	return true;
}