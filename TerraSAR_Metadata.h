/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#pragma once

#include "DynamicObject.h"
#include "GcpList.h"
#include <string>
#include <stdexcept>
#include <vector>
#include "SAR_Metadata.h"

using namespace std;

class TerraSAR_Metadata : public SAR_Metadata
{
public:
	//Constructor & Destructor
	TerraSAR_Metadata(void);
	~TerraSAR_Metadata(void);
	
	//Methods//
	bool ReadFile(std::string);
	void UpdateMetadata(DynamicObject*);

	// ANNOTATION CORRECTION //
	double RangeCoeff1,RangeCoeff2,AzimuthCoeff;

};

