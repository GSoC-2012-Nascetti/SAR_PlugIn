/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "SAR_Metadata.h"

double DATEtoDOY(std::string date)
{
		size_t pos;
		double Year,Month,Day,Time,DOY;

		Year = atof(date.substr(0,4).c_str());
		Month = atof(date.substr(5,2).c_str());
		Day = atof(date.substr(8,2).c_str());
		pos = date.find("T");
		date = date.substr(pos);
		Time = atof(date.substr(1,2).c_str())*3600+atof(date.substr(4,2).c_str())*60
								 +atof(date.substr(7).c_str());
		DOY = 367.0*Year-int(7.0*(Year+int((Month+9.0)/12.0))/4.0)+
              int(275.0*(Month)/9.0)+(Day)-730531.5+(Time/3600.0)/24.0;

		return DOY;
}