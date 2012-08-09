/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef STEREO_SAR_MODEL_H
#define STEREO_SAR_MODEL_H

#include "SAR_Model.h"

class Stereo_SAR_Model
{
public:
	Stereo_SAR_Model(SAR_Model *Left, SAR_Model *Right);
	~Stereo_SAR_Model(void);

	COORD_Ecef Stereo_SAR_SlantToGround(double Left_I,double Left_J,double Right_I, double Right_J);
	COORD_Ecef Point;

private:
	double ALFAUNO,ALFADUE,BETAUNO,BETADUE,GAMMAUNO,GAMMADUE;
	double K0,K1,K2,K3,K4,K5,K6,K7,K8,K9,K10,K11,K12,K13,K14;
	double ZG1,ZG2;

	STATEVECTOR SAT_COORD_Left,SAT_COORD_Right;

	SAR_Model *Mod_Left, *Mod_Right;
};

#endif