/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "SAR_Model.h"
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <boost\tuple\tuple.hpp>
#include <utility>

#include <fstream>

std::vector<OrbitCoefficients> LagrangeCoeff(std::vector<STATEVECTOR> &StateVect)
{
	int N = 0 ;
	N = StateVect.size();

	std::vector<OrbitCoefficients> Coeff;

	Coeff.resize(N);

	vector<vector<double> > F_X,F_Y,F_Z,F_Vx,F_Vy,F_Vz;

	// Set up sizes. (HEIGHT x WIDTH)
    F_X.resize(N);
	F_Y.resize(N);
	F_Z.resize(N);
	F_Vx.resize(N);
	F_Vy.resize(N);
	F_Vz.resize(N);

	for (int i = 0; i < N; ++i)
	{
		F_X[i].resize(N);
		F_Y[i].resize(N);
		F_Z[i].resize(N);
		F_Vx[i].resize(N);
		F_Vy[i].resize(N);
		F_Vz[i].resize(N);		
	}

	double ppppp = StateVect[0].X;

	for (int i=0; i<N; i++ )
	{
		F_X[i][0] = StateVect[i].X;
		F_Y[i][0] = StateVect[i].Y;
		F_Z[i][0] = StateVect[i].Z;

		F_Vx[i][0] = StateVect[i].VelocityX;
		F_Vy[i][0] = StateVect[i].VelocityY;
		F_Vz[i][0] = StateVect[i].VelocityZ;		
	}

	int k,m;

	for (int i=1; i<N; i++ )
	{
		k=i-1;
		m=-1;
		for (int j=i; j<N; j++)
		{
			m++;
			F_X[j][i]=(F_X[j-1][k]-F_X[j][k])/(StateVect[m].DOY-StateVect[i+m].DOY);
			F_Y[j][i]=(F_Y[j-1][k]-F_Y[j][k])/(StateVect[m].DOY-StateVect[i+m].DOY);
			F_Z[j][i]=(F_Z[j-1][k]-F_Z[j][k])/(StateVect[m].DOY-StateVect[i+m].DOY);

			F_Vx[j][i]=(F_Vx[j-1][k]-F_Vx[j][k])/(StateVect[m].DOY-StateVect[i+m].DOY);
			F_Vy[j][i]=(F_Vy[j-1][k]-F_Vy[j][k])/(StateVect[m].DOY-StateVect[i+m].DOY);
			F_Vz[j][i]=(F_Vz[j-1][k]-F_Vz[j][k])/(StateVect[m].DOY-StateVect[i+m].DOY);
		}
	}

    for (int i=0; i<N; i++ )
	{
		Coeff[i].X = F_X[i][i];
		Coeff[i].Y = F_Y[i][i];
		Coeff[i].Z = F_Z[i][i];
			
		Coeff[i].Vx= F_Vx[i][i];
		Coeff[i].Vy= F_Vy[i][i];
		Coeff[i].Vz= F_Vz[i][i];
	}
	
	return Coeff;

}

STATEVECTOR LagrangeInterpolation (double AzimuthTime, std::vector<STATEVECTOR> StateVect, std::vector<OrbitCoefficients> Coeff)
{
	int N = StateVect.size();
	STATEVECTOR *Result= new STATEVECTOR; 

	vector<double> A;
	A.resize(N);

	Result->DOY = AzimuthTime;
	Result->X = 0;
	Result->Y = 0;
	Result->Z = 0;
	Result->VelocityX = 0;
	Result->VelocityY = 0;
	Result->VelocityZ = 0;

	A[0]=1;
	for (int j=1; j<N; j++)
	{
		A[j] = A[j-1]*(AzimuthTime-StateVect[j-1].DOY);	
	}

	for (int j=0; j<N; j++)
	{
		Result->X += A[j]*Coeff[j].X;
		Result->Y += A[j]*Coeff[j].Y;
		Result->Z += A[j]*Coeff[j].Z;

		Result->VelocityX += A[j]*Coeff[j].Vx;
		Result->VelocityY += A[j]*Coeff[j].Vy;
		Result->VelocityZ += A[j]*Coeff[j].Vz;
	}

	return *Result;
}

SAR_Model::SAR_Model(SAR_Metadata &InputMetadata)
{
	PrecisionIndex = 10;
	Metadata = InputMetadata;
	
	StateVectorsRows.resize(PrecisionIndex*Metadata.Height);

	// Computation of Lagrange coefficients using State Vectors position&velocity //
	OrbitCoeff = LagrangeCoeff(Metadata.StateVectors);
	
	// Computation of satellite position&velocity for each rows using Lagrange interpolator //  
	for (int i=0; i<PrecisionIndex*Metadata.Height; i++) 
	{		
		Time = Metadata.AzimutT0 + (Metadata.AzimutTi/PrecisionIndex)*i;
		StateVectorsRows[i] = LagrangeInterpolation(Time,Metadata.StateVectors,OrbitCoeff);
	}

}

SAR_Model::SAR_Model(SAR_Metadata &InputMetadata, int Precision)
{
	PrecisionIndex = Precision;
	Metadata = InputMetadata;
	
	StateVectorsRows.resize(PrecisionIndex*Metadata.Height);

	// Computation of Lagrange coefficients using State Vectors position&velocity //
	OrbitCoeff = LagrangeCoeff(Metadata.StateVectors);
	
	// Computation of satellite position&velocity for each rows using Lagrange interpolator //  
	for (int i=0; i<PrecisionIndex*Metadata.Height; i++) 
	{		
		Time = Metadata.AzimutT0 + (Metadata.AzimutTi/PrecisionIndex)*i;
		StateVectorsRows[i] = LagrangeInterpolation(Time,Metadata.StateVectors,OrbitCoeff);
	}

}

SAR_Model::~SAR_Model(void)
{
	while(!StateVectorsRows.empty())
	{
		//delete StateVectorsRows.back();
		
		StateVectorsRows.pop_back();
	}

	while(!OrbitCoeff.empty())
	{
		//delete OrbitCoeff.back();

		OrbitCoeff.pop_back();
	}

	StateVectorsRows.clear();
	OrbitCoeff.clear();
}

P_COORD SAR_Model::SAR_GroundToImage(double Lon, double Lat, double H)
{
	P_COORD Result;
	return Result;
}