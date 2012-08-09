/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef ORTHORECTIFICATION_H
#define ORTHORECTIFICATION_H

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

class Orthorectification
{

public:
	void Start(void);

	Orthorectification(RasterElement *inputRaster, SAR_Model *inputModel,GRID inputGrid,double Height);
	~Orthorectification(void);

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

