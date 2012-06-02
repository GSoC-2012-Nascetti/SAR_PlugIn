#pragma once

#include "DynamicObject.h"
#include "GcpList.h"
#include <string>
#include <stdexcept>
#include <vector>
#include "SAR_Metadata.h"

using namespace std;

class TerraSAR_Metadata
{
public:
	//Constructor & Destructor
	TerraSAR_Metadata(void);
	~TerraSAR_Metadata(void);
	
	//Methods//
	bool ReadFile(std::string);
	void UpdateMetadata(DynamicObject*);

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

	// ANNOTATION CORRECTION //
	double RangeCoeff1,RangeCoeff2,AzimuthCoeff;

};

