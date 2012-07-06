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
#include "SAR_Metadata.h"
#include <string>
#include <stdexcept>
#include <vector>

using namespace std;

class RADARSAT_Metadata : public SAR_Metadata
{
public:

	//Constructor & Destructor
	RADARSAT_Metadata(void);
	~RADARSAT_Metadata(void);

	//Methods//
	bool ReadFile(std::string);
	void UpdateMetadata(DynamicObject*);
	std::list<GcpPoint> UpdateGCP(std::list<GcpPoint> PuntiGCPs, std::string path, Progress *pProgress);

};

