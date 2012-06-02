/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#pragma once

#include <string>
#include <stdexcept>
#include <vector>

using namespace std;

struct COORD
{
	double I;
	double J;	
	double Latitude;  // degrees //
	double Longitude; // degrees //	
	double Height;
}; 

struct STATEVECTOR
{
	double X;
	double Y;
	double Z;
	double VelocityX;
	double VelocityY;
	double VelocityZ;
	double DOY; 

}; 

double DATEtoDOY(std::string);