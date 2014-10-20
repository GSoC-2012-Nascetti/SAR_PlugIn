#ifndef SAR_GROUND_MODEL_H
#define SAR_GROUND_MODEL_H

#include "SAR_Model.h"
#include "SAR_Metadata.h"
#include <vector>

class SAR_Ground_Model : public SAR_Model
{
public:

	SAR_Ground_Model(SAR_Metadata &InputMetadata);
	SAR_Ground_Model(SAR_Metadata &, int p);
	~SAR_Ground_Model(void);

	P_COORD SAR_GroundToImage(double Lon, double Lat, double H);

	//GROUND RANGE VECTORS
	std::vector<double> SlantRangePixelDistance;
	std::vector<double> PixelColumnSpacing;

};

#endif
