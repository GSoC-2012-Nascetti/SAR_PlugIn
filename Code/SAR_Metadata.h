/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#pragma once

#include "GcpList.h"
#include <string>
#include <stdexcept>
#include <vector>
#include <cstdlib>

using namespace std;

struct P_COORD
{
	double I;
	double J;	
	double Latitude;  // degrees //
	double Longitude; // degrees //	
	double Height;
}; 

class STATEVECTOR  
{
public:
	double X;
	double Y;
	double Z;
	double VelocityX;
	double VelocityY;
	double VelocityZ;
	double DOY; 

	~STATEVECTOR()
	{	
	}

}; 

struct GR_Polynomial
{
	double S0,S1,S2,S3,S4,S5;
	double G0;
};

double DATEtoDOY(std::string);

class SAR_Metadata
{
public:

	//Constructor & Destructor
	SAR_Metadata(void);
	~SAR_Metadata(void);

	// ACQUISITION INFO //
	std::string SatelliteName;
	std::string Projection;
	std::string Orbit;
	std::string SideLooking;
	std::string ProductType;
	std::string Date;

	// RASTER DATA //
	int     Height, Width;
	double  ColumnSpacing;
	double  RowSpacing;
	double  RangeD0;
	double  RangeD0_Free;
	double  RangeDi;
	double  PRF; //Pulse Repetition Frequency //
	double  AzimutT0; // First pixel azimuth time //
	double  AzimutTi; // 1/PRF //
	double Min_IncidenceAngle;
	double Max_IncidenceAngle;

	// COORDINATE SCENE //
	double SceneHeight;
	std::vector<P_COORD> CornerCoordinate;

	// STATE VECTORS INFORMATION //
	int NumStateVectors;
	std::vector<STATEVECTOR> StateVectors; 

	//GROUND RANGE POLYNOMIAL

	GR_Polynomial GroundRange;

	virtual bool ReadFile(std::string) 
	{ return true;}

	virtual void UpdateMetadata(DynamicObject*)
	{ return;}

	virtual std::list<GcpPoint> UpdateGCP(std::list<GcpPoint> PuntiGCPs, std::string path)
	{ return PuntiGCPs;}

};