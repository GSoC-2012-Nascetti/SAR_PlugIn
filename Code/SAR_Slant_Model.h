#pragma once
#include "SAR_Model.h"
#include "SAR_Metadata.h"

class SAR_Slant_Model : public SAR_Model
{
public:
	SAR_Slant_Model(SAR_Metadata &InputMetadata);
	SAR_Slant_Model(SAR_Metadata &, int p);
	~SAR_Slant_Model(void);

	P_COORD SAR_GroundToImage(double Lon, double Lat, double H);
};

