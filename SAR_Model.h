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
	SAR_Model(SAR_Metadata &);
	SAR_Model(SAR_Metadata &, int);
	~SAR_Model(void);

	P_COORD SAR_GroundToSlant(double Lon,double Lat,double H);

	double Time;
	SAR_Metadata Metadata;

	//Orbit Estimated Coefficients//
	std::vector<STATEVECTOR> StateVectorsRows;
	std::vector<OrbitCoefficients> OrbitCoeff;

	int PrecisionIndex;
};

