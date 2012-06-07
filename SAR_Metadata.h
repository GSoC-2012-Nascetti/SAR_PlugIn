/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <cstdlib>

using namespace std;

struct COORD
{
	double I;
	double J;	
	double Latitude;  // degrees //
	double Longitude; // degrees //	
	double Height;
}; 

struct STATEVECTOR
{
	double X;
	double Y;
	double Z;
	double VelocityX;
	double VelocityY;
	double VelocityZ;
	double DOY; 

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

	// RASTER DATA //
	int     Height, Width;
	double  ColumnSpacing;
	double  RowSpacing;
	double  RangeD0;
	double  RangeDi;
	double  PRF; //Pulse Repetition Frequency //
	double  AzimutT0; // First pixel azimuth time //
	double  AzimutTi; // 1/PRF //

	// COORDINATE SCENE //
	double SceneHeight;
	std::vector<COORD> CornerCoordinate;

	// STATE VECTORS INFORMATION //
	int NumStateVectors;
	std::vector<STATEVECTOR> StateVectors;

};