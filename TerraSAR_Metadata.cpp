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
#include "stdlib.h"
#include "sstream"
#include "TerraSAR_Metadata.h"
#include "XercesIncludes.h"
#include "xmlreader.h" 

#include "xmlreaderSAR.h" 

XERCES_CPP_NAMESPACE_USE

TerraSAR_Metadata::TerraSAR_Metadata(void)
{
	CornerCoordinate.reserve(10);
	StateVectors.reserve(20);
}

TerraSAR_Metadata::~TerraSAR_Metadata(void)
{
}

bool TerraSAR_Metadata::ReadFile(std::string path)
{	
   XmlReaderSAR xml(Service<MessageLogMgr>()->getLog(), false);
   
   if (path.empty() || xml.parse(path) == NULL)
   {
      return false;
   }

   std::string current, Date;

   // CHECK IF IS A TerraSAR-X Mission //
   XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* pResult =
     xml.query("//generalHeader/mission/text()='TSX-1'", XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
   
   if (pResult == NULL || !pResult->getBooleanValue())
      {
         return false;
      }
	
	// RETRIEVE ACQUISITION INFO //
	pResult = xml.query("xs:string(//generalHeader/mission/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	SatelliteName = XMLString::transcode(pResult->getStringValue());

	pResult = xml.query("xs:string(//productInfo/productVariantInfo/projection/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	Projection = XMLString::transcode(pResult->getStringValue());
	
    pResult = xml.query("xs:string(//productInfo/missionInfo/orbitDirection/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	Orbit = XMLString::transcode(pResult->getStringValue());
	
	pResult = xml.query("xs:string(//productInfo/acquisitionInfo/lookDirection/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	SideLooking = XMLString::transcode(pResult->getStringValue());

	// RETRIEVE RASTER DATA //
	pResult = xml.query("xs:integer(//productInfo/imageDataInfo/imageRaster/numberOfRows/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
    Height = pResult->getIntegerValue();

    pResult = xml.query("xs:integer(//productInfo/imageDataInfo/imageRaster/numberOfColumns/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	Width = pResult->getIntegerValue();  
	
	pResult = xml.query("xs:double(//productInfo/imageDataInfo/imageRaster/columnSpacing/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	ColumnSpacing = static_cast<double>(pResult->getNumberValue());

	pResult = xml.query("xs:double(//productInfo/imageDataInfo/imageRaster/rowSpacing/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	RowSpacing = static_cast<double>(pResult->getNumberValue());

	pResult = xml.query("xs:double(//productInfo/sceneInfo/rangeTime/firstPixel/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	RangeD0 = static_cast<double>(pResult->getNumberValue());									

	pResult = xml.query("xs:double(//productSpecific/complexImageInfo/commonPRF/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	PRF = static_cast<double>(pResult->getNumberValue());

	pResult = xml.query("xs:double(//productSpecific/complexImageInfo/projectedSpacingRange/slantRange/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	RangeDi = static_cast<double>(pResult->getNumberValue());

	pResult = xml.query("xs:string(//productInfo/sceneInfo/start/timeUTC/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	Date = XMLString::transcode(pResult->getStringValue());
	AzimutT0 = DATEtoDOY(Date);

	// RETRIEVE CORNER COORDINATE //
	CornerCoordinate.resize(5);

	pResult = xml.query("xs:integer(//productInfo/sceneInfo/sceneCenterCoord/refRow/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].J = pResult->getIntegerValue();

	pResult = xml.query("xs:integer(//productInfo/sceneInfo/sceneCenterCoord/refColumn/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].I = pResult->getIntegerValue();

	pResult = xml.query("xs:double(//productInfo/sceneInfo/sceneCenterCoord/lat/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].Latitude = static_cast<double>(pResult->getNumberValue());

	pResult = xml.query("xs:double(//productInfo/sceneInfo/sceneCenterCoord/lon/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].Longitude = static_cast<double>(pResult->getNumberValue());

	pResult = xml.query("xs:double(//productInfo/sceneInfo/sceneAverageHeight/text())", 
						 XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	CornerCoordinate[0].Height = static_cast<double>(pResult->getNumberValue());
		
	for (int n=1; n<5; n++ )
	{
		std::stringstream num,num2;
		num<<n;
		current = "xs:double(//productInfo/sceneInfo/sceneCornerCoord["+num.str()+"]/lat/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		CornerCoordinate[n].Latitude = static_cast<double>(pResult->getNumberValue());

		current = "xs:double(//productInfo/sceneInfo/sceneCornerCoord["+num.str()+"]/lon/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		CornerCoordinate[n].Longitude = static_cast<double>(pResult->getNumberValue());

		current = "xs:integer(//productInfo/sceneInfo/sceneCornerCoord["+num.str()+"]/refColumn/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		CornerCoordinate[n].I = pResult->getIntegerValue();

		current = "xs:integer(//productInfo/sceneInfo/sceneCornerCoord["+num.str()+"]/refRow/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		CornerCoordinate[n].J = pResult->getIntegerValue();

		CornerCoordinate[n].Height = CornerCoordinate[0].Height;
	}
	
	// RETRIEVE ORBIT STATE VECTORS //
	current = "xs:integer(//orbit/orbitHeader/numStateVectors/text())";
	pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
	VERIFY(pResult != NULL);
	NumStateVectors = pResult->getIntegerValue();

	StateVectors.resize(NumStateVectors);
	
	for (int n=1; n<=NumStateVectors; n++ )
	{
		std::stringstream num;
		num<<n;
		current = "xs:double(//orbit/stateVec["+num.str()+"]/posX/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].X = static_cast<double>(pResult->getNumberValue());	

		current = "xs:double(//orbit/stateVec["+num.str()+"]/posY/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].Y = static_cast<double>(pResult->getNumberValue());
		
		current = "xs:double(//orbit/stateVec["+num.str()+"]/posZ/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].Z = static_cast<double>(pResult->getNumberValue());
		
		current = "xs:double(//orbit/stateVec["+num.str()+"]/velX/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].VelocityX = static_cast<double>(pResult->getNumberValue());

		current = "xs:double(//orbit/stateVec["+num.str()+"]/velY/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].VelocityY = static_cast<double>(pResult->getNumberValue());

		current = "xs:double(//orbit/stateVec["+num.str()+"]/velZ/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		StateVectors[n-1].VelocityZ = static_cast<double>(pResult->getNumberValue());

		current = "xs:string(//orbit/stateVec["+num.str()+"]/timeUTC/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
		Date = XMLString::transcode(pResult->getStringValue());
		StateVectors[n-1].DOY = DATEtoDOY(Date);
	}

	// RETRIEVE AZIMUTH AND RANGE ANNOTATION CORRECTION //
	size_t pos;
	std::string path2;
	pos = path.find_last_of("/");
	path2 = path.substr(0,pos);
	path2 += "/ANNOTATION/GEOREF.xml";

	if (path2.empty() || xml.parse(path2) == NULL)
	{
      return false;
	}

	current = "xs:double(//signalPropagationEffects/rangeDelay[1]/coefficient/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
	RangeCoeff1 = static_cast<double>(pResult->getNumberValue());

	current = "xs:double(//signalPropagationEffects/rangeDelay[2]/coefficient/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
	RangeCoeff2 = static_cast<double>(pResult->getNumberValue());

	RangeD0 = (RangeD0-(RangeCoeff1+RangeCoeff2))*299792.460*1000/2;

	current = "xs:double(//signalPropagationEffects/azimuthShift/coefficient/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		VERIFY(pResult != NULL);
	AzimuthCoeff = static_cast<double>(pResult->getNumberValue());
	AzimutT0 = AzimutT0 - AzimuthCoeff;  //86400;
	AzimutTi = 1/PRF;

	return true;
}

void TerraSAR_Metadata::UpdateMetadata(DynamicObject* DynamicMetadata) 
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
	DynamicMetadata->setAttributeByPath("SAR METADATA/CORNER COORDIATE/Center/Lat",CornerCoordinate[0].Latitude);
	DynamicMetadata->setAttributeByPath("SAR METADATA/CORNER COORDIATE/Center/Lon",CornerCoordinate[0].Longitude);
	DynamicMetadata->setAttributeByPath("SAR METADATA/CORNER COORDIATE/Center/Height",CornerCoordinate[0].Height);
	DynamicMetadata->setAttributeByPath("SAR METADATA/CORNER COORDIATE/Center/RefColumn",CornerCoordinate[0].I);
	DynamicMetadata->setAttributeByPath("SAR METADATA/CORNER COORDIATE/Center/RefRow",CornerCoordinate[0].J);

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

	// UPDATE ANNOTATION CORRECTION //
	DynamicMetadata->setAttributeByPath("SAR METADATA/ANNOTATION CORRECTION /Range/Atm_Corr",RangeCoeff1);
	DynamicMetadata->setAttributeByPath("SAR METADATA/ANNOTATION CORRECTION /Range/Iono_Corr",RangeCoeff2);
	DynamicMetadata->setAttributeByPath("SAR METADATA/ANNOTATION CORRECTION /Azimuth/Shift_Corr",AzimuthCoeff);

}

std::list<GcpPoint> TerraSAR_Metadata::UpdateGCP(std::list<GcpPoint> PuntiGCPs, std::string path, Progress *pProgress)
{
	int N=0;
	N = PuntiGCPs.size();
	size_t pos;
	std::string path2;
	pos = path.find_last_of("/");
	path2 = path.substr(0,pos);
	path2 += "/ANNOTATION/GEOREF.xml";

	XmlReaderSAR xml(Service<MessageLogMgr>()->getLog(), false);
	
	if (path2.empty() || xml.parse(path2) == NULL)
	{
      return PuntiGCPs;
	}

	list<GcpPoint>::iterator pList;	

	std::string current;
	XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult* pResult;

	int n=1;

	/* current = "fn:tokenize(//geolocationGrid/gridPoint/lon/text(), '\\s+')";
	pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::ITERATOR_RESULT_TYPE);
	pList->mCoordinate.mZ = static_cast<double>(pResult->getNumberValue());	

	pResult->iterateNext();
	pList->mCoordinate.mZ = static_cast<double>(pResult->getNumberValue()); */
	
	
	for (pList = PuntiGCPs.begin(); pList != PuntiGCPs.end(); pList++)
	{
		std::stringstream num;
		num<<n;
		current = "xs:double(//geolocationGrid/gridPoint["+num.str()+"]/lon/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		pList->mCoordinate.mX = static_cast<double>(pResult->getNumberValue());	
		current = "xs:double(//geolocationGrid/gridPoint["+num.str()+"]/lat/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		pList->mCoordinate.mY = static_cast<double>(pResult->getNumberValue());	

		current = "xs:double(//geolocationGrid/gridPoint["+num.str()+"]/height/text())";
		pResult = xml.query(current, XERCES_CPP_NAMESPACE_QUALIFIER DOMXPathResult::FIRST_RESULT_TYPE);
		pList->mCoordinate.mZ = static_cast<double>(pResult->getNumberValue());	

		n++;
		pProgress->updateProgress("Update GCPs Information",int(100*n/N), NORMAL);

	}

	return PuntiGCPs;
}