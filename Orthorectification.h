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

#include "SAR_Model.h"
#include "RasterElement.h"
#include "GeoPoint.h"
#include "ExecutableShell.h"


// template<typename T>
 //void copy3(T* pData, double Value);

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

};

class Orthorectification : public ExecutableShell
{


public:
	void Start(void);

	Orthorectification(RasterElement *inputRaster, SAR_Model *inputModel,GRID inputGrid,double Height);
	~Orthorectification(void);
	SAR_Model *Model;

	virtual bool getInputSpecification(PlugInArgList*& pInArgList);
    virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
    virtual bool execute();
	virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
	
private:

	RasterElement *Image;

	double FlatHeight;
	// GRID EXTENSION 


    GRID OrthoGrid;
};

#endif

