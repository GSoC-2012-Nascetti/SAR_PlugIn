/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef SAR_MODEL_H
#define SAR_MODEL_H

#include "GcpList.h"
#include "SAR_Metadata.h"

struct OrbitCoefficients
{
	double X;
	double Y;
	double Z;
	double Vx;
	double Vy;
	double Vz;
}; 

struct COORD_Ecef: public P_COORD
{
	double X_Ecef;
	double Y_Ecef;
	double Z_Ecef;
}; 

std::vector<OrbitCoefficients> LagrangeCoeff(std::vector<STATEVECTOR> &StateVect);

STATEVECTOR LagrangeInterpolation (double ,std::vector<STATEVECTOR>, std::vector<OrbitCoefficients>);

class SAR_Model
{
public:
	SAR_Model();
	SAR_Model(SAR_Metadata &);
	SAR_Model(SAR_Metadata &, int);
	~SAR_Model(void);

	//P_COORD SAR_GroundToSlant(double Lon,double Lat,double H);

	//P_COORD SAR_GroundToGroundRange(double Lon,double Lat,double H);

	virtual P_COORD SAR_GroundToImage(double Lon, double Lat, double H);

	double Time;
	int PrecisionIndex;

	SAR_Metadata Metadata;

	//Orbit Estimated Coefficients//
	std::vector<STATEVECTOR> StateVectorsRows;
	std::vector<OrbitCoefficients> OrbitCoeff;

	//GROUND RANGE VECTORS
	//std::vector<double> SlantRangePixelDistance;
	//std::vector<double> PixelColumnSpacing;

	
};

#endif