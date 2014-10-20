/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "AppVerify.h"
#include "Progress.h"
#include "ProgressResource.h"
#include "RADARSAT_Metadata.h"
#include "stdlib.h"
#include "sstream"
#include "XercesIncludes.h"
#include "xmlreader.h" 
#include "xmlreaderSAR.h" 

#include <iostream>
#include <fstream>
#include <boost\algorithm\string.hpp>
using namespace std;

XERCES_CPP_NAMESPACE_USE

RADARSAT_Metadata::RADARSAT_Metadata(void)
{
	CornerCoordinate.reserve(10);
	StateVectors.reserve(10);
}

RADARSAT_Metadata::~RADARSAT_Metadata(void)
{
}

bool RADARSAT_Metadata::ReadFile(std::string path)
{	
   XmlReaderSAR xml(Service<MessageLogMgr>()->getLog(), false);
   
   if (path.empty() || xml.parse(path) == NULL)
   {
      return false;
   }

   std::string current, Date, DefaultNamespace;

   DefaultNamespace = "http://www.rsi.ca/rs2/prod/xml/schemas";

   XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* pResult =
         xml.queryNamespace("//product/sourceAttributes/satellite/text()='RADARSAT-2'", DefaultNamespace ,XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);

   bool Control =  pResult->getBooleanValue();
   
   if (pResult == NULL || !pResult->getBooleanValue())
      {         	  
		     return false;
      }
	
	// RETRIVE ACQUISITION INFO //
	pResult = xml.queryNamespace("xs:string(//product/sourceAttributes/satellite/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	SatelliteName = XMLString::transcode(pResult->getStringValue());

	pResult = xml.queryNamespace("xs:string(//imageGenerationParameters/generalProcessingInformation/productType/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	Projection = XMLString::transcode(pResult->getStringValue());
	
    pResult = xml.queryNamespace("xs:string(//orbitAndAttitude/orbitInformation/passDirection/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	Orbit = XMLString::transcode(pResult->getStringValue());
	
	pResult = xml.queryNamespace("xs:string(//sourceAttributes/radarParameters/antennaPointing/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	SideLooking = XMLString::transcode(pResult->getStringValue());

	// RETRIVE RASTER DATA //
	pResult = xml.queryNamespace("xs:integer(//imageAttributes/rasterAttributes/numberOfLines/text())", DefaultNamespace, 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
    Height = pResult->getIntegerValue();

    pResult = xml.queryNamespace("xs:integer(//imageAttributes/rasterAttributes/numberOfSamplesPerLine/text())", DefaultNamespace, 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
   	Width = pResult->getIntegerValue();  
	
	pResult = xml.queryNamespace("xs:double(//imageAttributes/rasterAttributes/sampledPixelSpacing/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	ColumnSpacing = static_cast<double>(pResult->getNumberValue());

	pResult = xml.queryNamespace("xs:double(//imageAttributes/rasterAttributes/sampledLineSpacing/text())", DefaultNamespace, 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	RowSpacing = static_cast<double>(pResult->getNumberValue());

	pResult = xml.queryNamespace("xs:double(//slantRangeToGroundRange/slantRangeTimeToFirstRangeSample/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	RangeD0 = static_cast<double>(pResult->getNumberValue());
	RangeD0 = (RangeD0)*299792.460*1000/2;

	pResult = xml.queryNamespace("xs:string(//sarProcessingInformation/zeroDopplerTimeFirstLine/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	Date = XMLString::transcode(pResult->getStringValue());	
	AzimutT0 = DATEtoDOY(Date);

	pResult = xml.queryNamespace("xs:string(//sarProcessingInformation/zeroDopplerTimeLastLine/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	Date = XMLString::transcode(pResult->getStringValue());	
	AzimutTi = DATEtoDOY(Date);
	PRF = AzimutTi;
	//AzimutTi = 86400*(AzimutTi-AzimutT0)/double(Height); //DOY
	AzimutTi = (AzimutTi-AzimutT0)/double(Height-1);

    if (AzimutTi <0.0) 
		{
			AzimutT0 = AzimutT0+AzimutTi*(Height-1);
			AzimutTi = -AzimutTi;
		
		}
	
	RangeDi = ColumnSpacing;

	//COMPUTE CORNER INDEX

	int C2,C3,C4,Nx,Ny;
	std::stringstream index;

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[2]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	double DX = static_cast<double>(pResult->getNumberValue());

	Nx = int(Width/DX)+1;
	C2 = Nx;
	index<<(C2+1);

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	double DY = static_cast<double>(pResult->getNumberValue());

	Ny = int(Height/DY)+1;
	C3 = Nx*(Ny-1)+1;
	C4 = Nx*Ny;

	// RETRIVE CORNER COORDINATE //
	CornerCoordinate.resize(4);

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].J = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].I = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].Latitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/geodeticCoordinate/longitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].Longitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/geodeticCoordinate/height/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].Height = static_cast<double>(pResult->getNumberValue());	

	index.str(string());
	index.clear();
	index<<C2;

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].J = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].I = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].Latitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/longitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].Longitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/height/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].Height = static_cast<double>(pResult->getNumberValue());

	index.str(string());
    index.clear();
	index<<C3;

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].J = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].I = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].Latitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/longitude/text())", DefaultNamespace, 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].Longitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/height/text())", DefaultNamespace, 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].Height = static_cast<double>(pResult->getNumberValue());

	index.str(string());
    index.clear();
	index<<C4;

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].J = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].I = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].Latitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/longitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].Longitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+index.str()+"]/geodeticCoordinate/height/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].Height = static_cast<double>(pResult->getNumberValue());

	// RETRIVE ORBIT STATE VECTORS //

	NumStateVectors = 5;
	StateVectors.resize(NumStateVectors);
	
	for (int n=1; n<=NumStateVectors; n++ )
	{
		std::stringstream num;
		num<<n;
		current = "xs:double(//orbitInformation/stateVector["+num.str()+"]/xPosition/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].X = static_cast<double>(pResult->getNumberValue());	

		current = "xs:double(//orbitInformation/stateVector["+num.str()+"]/yPosition/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].Y = static_cast<double>(pResult->getNumberValue());
		
		current = "xs:double(//orbitInformation/stateVector["+num.str()+"]/zPosition/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].Z = static_cast<double>(pResult->getNumberValue());
		
		current = "xs:double(//orbitInformation/stateVector["+num.str()+"]/xVelocity/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].VelocityX = static_cast<double>(pResult->getNumberValue());

		current = "xs:double(//orbitInformation/stateVector["+num.str()+"]/yVelocity/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].VelocityY = static_cast<double>(pResult->getNumberValue());

		current = "xs:double(//orbitInformation/stateVector["+num.str()+"]/zVelocity/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].VelocityZ = static_cast<double>(pResult->getNumberValue());

		current = "xs:string(//orbitInformation/stateVector["+num.str()+"]/timeStamp/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		Date = XMLString::transcode(pResult->getStringValue());
		StateVectors[n-1].DOY = DATEtoDOY(Date);


	// RETRIEVE GROUND-RANGE INFORMATION

		current = "xs:string(//slantRangeToGroundRange/groundRangeOrigin/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		GroundRange.G0 = static_cast<double>(pResult->getNumberValue());

		current = "xs:string(//slantRangeToGroundRange/groundToSlantRangeCoefficients/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		std::string s_coeff = XMLString::transcode(pResult->getStringValue());
		
		std::vector<std::string> coeff;
		boost::split(coeff, s_coeff, boost::is_any_of(" "));

		GroundRange.S0 = (double)atof(coeff[0].c_str());
		GroundRange.S1 = (double)atof(coeff[1].c_str());
		GroundRange.S2 = (double)atof(coeff[2].c_str());
		GroundRange.S3 = (double)atof(coeff[3].c_str());
		GroundRange.S4 = (double)atof(coeff[4].c_str());
		GroundRange.S5 = (double)atof(coeff[5].c_str());
	}



	return true;
}

void RADARSAT_Metadata::UpdateMetadata(DynamicObject* DynamicMetadata) 
{
    // UPDATE ACQUISITION INFO // 
    DynamicMetadata->setAttributeByPath("SAR METADATA/ACQUISITION INFO/Satellite Name/",SatelliteName);
	DynamicMetadata->setAttributeByPath("SAR METADATA/ACQUISITION INFO/Projection/", Projection);  
	DynamicMetadata->setAttributeByPath("SAR METADATA/ACQUISITION INFO/Orbit Direction/", Orbit);
	DynamicMetadata->setAttributeByPath("SAR METADATA/ACQUISITION INFO/Side Looking/", SideLooking);  
	
	// UPDATE RASTER DATA //
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/Number of Rows/",Height);
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/Number of Columns/",Width);
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/Row Spacing/",RowSpacing);
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/Columns Spacing/",ColumnSpacing);
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/PRF/",PRF);
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/Range D0/",RangeD0); 
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/Range Di/",RangeDi);
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/Azimuth T0/",AzimutT0);
	DynamicMetadata->setAttributeByPath("SAR METADATA/IMAGE RASTER INFO/Azimuth Ti/",AzimutTi);


	// Ground Range Informtation
	DynamicMetadata->setAttributeByPath("SAR METADATA/GROUND-RANGE/G0/",GroundRange.G0);
	DynamicMetadata->setAttributeByPath("SAR METADATA/GROUND-RANGE/S0/",GroundRange.S0);
	DynamicMetadata->setAttributeByPath("SAR METADATA/GROUND-RANGE/S1/",GroundRange.S1);
	DynamicMetadata->setAttributeByPath("SAR METADATA/GROUND-RANGE/S2/",GroundRange.S2);
	DynamicMetadata->setAttributeByPath("SAR METADATA/GROUND-RANGE/S3/",GroundRange.S3);
	DynamicMetadata->setAttributeByPath("SAR METADATA/GROUND-RANGE/S4/",GroundRange.S4);
	DynamicMetadata->setAttributeByPath("SAR METADATA/GROUND-RANGE/S5/",GroundRange.S5);

	// UPDATE CORNER COORDINATE //
	std::string current;

    for (int n=0; n<4; n++ )
	{
		std::stringstream num;
		num<<n;
		current = "SAR METADATA/CORNER COORDINATE/Corner "+num.str()+"/Lat";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].Latitude);
		current = "SAR METADATA/CORNER COORDINATE/Corner "+num.str()+"/Lon";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].Longitude);
		current = "SAR METADATA/CORNER COORDINATE/Corner "+num.str()+"/Height";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].Height);
		current = "SAR METADATA/CORNER COORDINATE/Corner "+num.str()+"/RefColumn";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].I);
		current = "SAR METADATA/CORNER COORDINATE/Corner "+num.str()+"/RefRow";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].J);
	}


	// UPDATE ORBIT STATE VECTORS //
	for (int n=1; n<=NumStateVectors; n++ )
	{
		std::stringstream num;
		num<<(n+100);
		current = "SAR METADATA/STATE VECTORS/Num "+num.str().substr(1,2)+"/X";
		DynamicMetadata->setAttributeByPath(current,StateVectors[n-1].X);
		current = "SAR METADATA/STATE VECTORS/Num "+num.str().substr(1,2)+"/Y";
		DynamicMetadata->setAttributeByPath(current,StateVectors[n-1].Y);
		current = "SAR METADATA/STATE VECTORS/Num "+num.str().substr(1,2)+"/Z";
		DynamicMetadata->setAttributeByPath(current,StateVectors[n-1].Z);
		current = "SAR METADATA/STATE VECTORS/Num "+num.str().substr(1,2)+"/VelX";
		DynamicMetadata->setAttributeByPath(current,StateVectors[n-1].VelocityX);
		current = "SAR METADATA/STATE VECTORS/Num "+num.str().substr(1,2)+"/VelY";
		DynamicMetadata->setAttributeByPath(current,StateVectors[n-1].VelocityY);
		current = "SAR METADATA/STATE VECTORS/Num "+num.str().substr(1,2)+"/VelZ";
		DynamicMetadata->setAttributeByPath(current,StateVectors[n-1].VelocityZ);
		current = "SAR METADATA/STATE VECTORS/Num "+num.str().substr(1,2)+"/DOY";
		DynamicMetadata->setAttributeByPath(current,StateVectors[n-1].DOY);
	}

}

std::list<GcpPoint> RADARSAT_Metadata::UpdateGCP(std::list<GcpPoint> PuntiGCPs, std::string path, Progress *pProgress)
{
	int N=0;
	N = PuntiGCPs.size();

	std::string DefaultNamespace = "http://www.rsi.ca/rs2/prod/xml/schemas";

	XmlReaderSAR xml(Service<MessageLogMgr>()->getLog(), false);
	
	if (path.empty() || xml.parse(path) == NULL)
	{
      return PuntiGCPs;
	}

	list<GcpPoint>::iterator pList;	

	std::string current;
	XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* pResult;

	int n=1;

	for (pList = PuntiGCPs.begin(); pList != PuntiGCPs.end(); pList++)
	{
		std::stringstream num;
		num<<n;              
		current = "xs:double(//geolocationGrid/imageTiePoint["+num.str()+"]/geodeticCoordinate/height/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		pList->mCoordinate.mZ = static_cast<double>(pResult->getNumberValue());	
	    
	    pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+num.str()+"]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		pList->mCoordinate.mY = static_cast<double>(pResult->getNumberValue());
	    
		pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+num.str()+"]/geodeticCoordinate/longitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	    pList->mCoordinate.mX = static_cast<double>(pResult->getNumberValue());
		
		pProgress->updateProgress("Update GCPs Information",int(100*n/N), NORMAL);		
		n++;

	}

	return PuntiGCPs;
}

std::list<GcpPoint> RADARSAT_Metadata::UpdateGCP(std::list<GcpPoint> PuntiGCPs, std::string path)
{
	ProgressResource pProgress("ProgressBar");

	int N=0;
	N = PuntiGCPs.size();

	std::string DefaultNamespace = "http://www.rsi.ca/rs2/prod/xml/schemas";

	XmlReaderSAR xml(Service<MessageLogMgr>()->getLog(), false);
	
	if (path.empty() || xml.parse(path) == NULL)
	{
      return PuntiGCPs;
	}

	list<GcpPoint>::iterator pList;	

	std::string current;
	XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* pResult;

	int n=1;

	for (pList = PuntiGCPs.begin(); pList != PuntiGCPs.end(); pList++)
	{
		std::stringstream num;
		num<<n;              
		current = "xs:double(//geolocationGrid/imageTiePoint["+num.str()+"]/geodeticCoordinate/height/text())";
		pResult = xml.queryNamespace(current,DefaultNamespace, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		pList->mCoordinate.mZ = static_cast<double>(pResult->getNumberValue());	
	    
	    pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+num.str()+"]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		pList->mCoordinate.mY = static_cast<double>(pResult->getNumberValue());
	    
		pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint["+num.str()+"]/geodeticCoordinate/longitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	    pList->mCoordinate.mX = static_cast<double>(pResult->getNumberValue());
		
		pProgress->updateProgress("Update GCPs Information",int(100*n/N), NORMAL);		
		n++;

	}

	return PuntiGCPs;
}