
#ifndef SAR_SIMULATOR_PROCESSOR_H
#define SAR_SIMULATOR_PROCESSOR_H

#include "ExecutableShell.h"
#include "GeoPoint.h"
#include "Progress.h"
#include "RasterElement.h"
#include "SAR_Model.h"

struct GRID
{
	double X_Min;
	double Y_Min;
	double X_Max;
	double Y_Max;
	double X_Step;
	double Y_Step;

    double Lon_Min;
	double Lat_Min;
    double Lon_Max;
	double Lat_Max;
	double Lon_Step;
	double Lat_Step;
	
	double X_Dim;
	double Y_Dim;	
	
	int iZone;
	char hemisphere;

	double nodatavalue;

};

/*
enum ResamplingMethod
{
	NEAREST_NEIGHBOR=0,
	AVERAGEBOX3,
	AVERAGEBOX5,
	AVERAGEBOX7
};*/

class SAR_Simulator_Processor
{

public:
	void Start(void);

	SAR_Simulator_Processor(RasterElement *inputRaster, SAR_Model *inputModel,GRID inputGrid,double Height);
	~SAR_Simulator_Processor(void);

    virtual bool process(int type);
	virtual bool process(int type, RasterElement *pDSM, GRID DSMGrid, double Geoid_Offset,int DSM_resampling=1);
	
private:

    SAR_Model *Model;
	RasterElement *Image;
	Progress *pProgress;

	int res_type,boxsize;
	double FlatHeight;

	// GRID EXTENSION 
	GRID OrthoGrid;
};

#endif

