/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "AppVerify.h"
#include "RADARSAT_Metadata.h"
#include "stdlib.h"
#include "sstream"
#include "XercesIncludes.h"
#include "xmlreader.h" 
#include "xmlreaderSAR.h" 

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
	AzimutTi = 86400*(AzimutTi-AzimutT0)/double(Height);
	
	RangeDi = ColumnSpacing;


	// RETRIVE CORNER COORDINATE //

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].J = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].I = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].Latitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/geodeticCoordinate/longitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].Longitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[1]/geodeticCoordinate/height/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[1].Height = static_cast<double>(pResult->getNumberValue());	

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[16]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].J = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[16]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].I = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[16]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].Latitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[16]/geodeticCoordinate/longitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].Longitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[16]/geodeticCoordinate/height/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[2].Height = static_cast<double>(pResult->getNumberValue());

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[209]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].J = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[209]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].I = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[209]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].Latitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[209]/geodeticCoordinate/longitude/text())", DefaultNamespace, 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].Longitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[209]/geodeticCoordinate/height/text())", DefaultNamespace, 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[3].Height = static_cast<double>(pResult->getNumberValue());

	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[224]/imageCoordinate/pixel/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[4].J = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[224]/imageCoordinate/line/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[4].I = static_cast<int>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[224]/geodeticCoordinate/latitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[4].Latitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[224]/geodeticCoordinate/longitude/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[4].Longitude = static_cast<double>(pResult->getNumberValue());
	pResult = xml.queryNamespace("xs:double(//geolocationGrid/imageTiePoint[224]/geodeticCoordinate/height/text())", DefaultNamespace,
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[4].Height = static_cast<double>(pResult->getNumberValue());

	// RETRIVE ORBIT STATE VECTORS //

	NumStateVectors = 5;
	
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


	// UPDATE CORNER COORDINATE //
	std::string current;

    for (int n=1; n<5; n++ )
	{
		std::stringstream num;
		num<<n;
		current = "SAR METADATA/CORNER COORDIATE/Corner "+num.str()+"/Lat";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].Latitude);
		current = "SAR METADATA/CORNER COORDIATE/Corner "+num.str()+"/Lon";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].Longitude);
		current = "SAR METADATA/CORNER COORDIATE/Corner "+num.str()+"/Height";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].Height);
		current = "SAR METADATA/CORNER COORDIATE/Corner "+num.str()+"/RefColumn";
		DynamicMetadata->setAttributeByPath(current,CornerCoordinate[n].I);
		current = "SAR METADATA/CORNER COORDIATE/Corner "+num.str()+"/RefRow";
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