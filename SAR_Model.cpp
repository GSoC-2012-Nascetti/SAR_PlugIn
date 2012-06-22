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


std::vector<OrbitCoefficients> LagrangeCoeff(std::vector<STATEVECTOR> &StateVect)
{
	int N = 0 ;
	N = StateVect.size();

	std::vector<OrbitCoefficients> Coeff;

	Coeff.resize(N);

	vector<vector<double>> F_X,F_Y,F_Z,F_Vx,F_Vy,F_Vz;

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
	Metadata = InputMetadata;
	
	StateVectorsRows.resize(100*Metadata.Height);

	// Computation of Lagrange coefficients using State Vectors position&velocity //
	OrbitCoeff = LagrangeCoeff(Metadata.StateVectors);
	
	// Computation of satellite position&velocity for each rows using Lagrange interpolator //  
	for (int i=0; i<100*Metadata.Height; i++) 
	{		
		Time = Metadata.AzimutT0 + (Metadata.AzimutTi/100.0)*i;
		StateVectorsRows[i] = LagrangeInterpolation(Time,Metadata.StateVectors,OrbitCoeff);
	}

}

SAR_Model::~SAR_Model(void)
{
}

COORD SAR_Model::SAR_GroundToSlant(double Lon,double Lat, double H) 
{
	COORD_Ecef Point, Control;

	Point.Longitude = Lon;
    Point.Latitude =  Lat;
	Point.Height = H;
	Point.I=-1;
	Point.J=-1;

	double Pi_Greco = 3.1415926535;
	double aao = 6378137;
    double e2 =0.006694379990;
	double GranNorm = 0.0;
	double Rlambda = (Lon/180.00)*Pi_Greco;
	double Rphi    = (Lat/180.00)*Pi_Greco;
    double Pos_Module=0;
	double Vel_Module=0;
	double Versor_X,Versor_Y,Versor_Z;
	double Versor_Vx,Versor_Vy,Versor_Vz;

	GranNorm=aao/(sqrt(1-e2*sin(Rphi)*sin(Rphi)));

	Control.X_Ecef = GranNorm;
	Control.Y_Ecef = Rlambda;
	Control.Z_Ecef = Rphi;

	Point.X_Ecef =(GranNorm+H)*cos(Rphi)*cos(Rlambda);
	Point.Y_Ecef =(GranNorm+H)*cos(Rphi)*sin(Rlambda);
	Point.Z_Ecef =(GranNorm*(1-e2)+H)*sin(Rphi);

	vector<double> T_Angles;
	T_Angles.resize(3);
	T_Angles[1]=0;

	vector<int> Index;
	Index.resize(3); 
	Index[0]= 0;
	Index[2]= 100*Metadata.Height-1;
	Index[1]= int(((Index[0]+Index[2])/2));

	while ( (abs((T_Angles[1]-Pi_Greco)/2) > 0.00000001) && (Index[0] != Index[1])) 
	{
		for (int i=0; i<=2;i++)
		{				
			Pos_Module = sqrt(pow((StateVectorsRows[Index[i]].X-Point.X_Ecef),2.0) +pow((StateVectorsRows[Index[i]].Y-Point.Y_Ecef),2.0) +pow((StateVectorsRows[Index[i]].Z-Point.Z_Ecef),2.0)); 
			Versor_X = (Point.X_Ecef-StateVectorsRows[Index[i]].X)/Pos_Module;
			Versor_Y = (Point.Y_Ecef-StateVectorsRows[Index[i]].Y)/Pos_Module;
			Versor_Z = (Point.Z_Ecef-StateVectorsRows[Index[i]].Z)/Pos_Module;

			Vel_Module = sqrt(pow((StateVectorsRows[Index[i]].VelocityX),2.0) +pow((StateVectorsRows[Index[i]].VelocityY),2.0) +pow((StateVectorsRows[Index[i]].VelocityZ),2.0));
			Versor_Vx = (StateVectorsRows[Index[i]].VelocityX)/Vel_Module;
			Versor_Vy = (StateVectorsRows[Index[i]].VelocityY)/Vel_Module;
			Versor_Vz = (StateVectorsRows[Index[i]].VelocityZ)/Vel_Module;

			T_Angles[i] = acos(Versor_X*Versor_Vx+Versor_Y*Versor_Vy+Versor_Z*Versor_Vz);
		}

		if (T_Angles[1]>(Pi_Greco/2.0))
		{
			Index[0]=Index[0];
			Index[2]=Index[1];
			Index[1]=Index[0] +int(((Index[2]-Index[0])/2.0));	
		}
		else 
		{
			Index[0]=Index[1];
			Index[2]=Index[2];
			Index[1]=Index[0] +int(((Index[2]-Index[0])/2.0));			
		}
    }
		
	Pos_Module = sqrt(pow((StateVectorsRows[Index[1]].X-Point.X_Ecef),2.0) +pow((StateVectorsRows[Index[1]].Y-Point.Y_Ecef),2.0) +pow((StateVectorsRows[Index[1]].Z-Point.Z_Ecef),2.0));
		
	if (Metadata.SatelliteName == "RADARSAT-2")
	{
		if (Metadata.Orbit == "Ascending" && Metadata.SideLooking =="Right") 
		{
			Point.I = (Pos_Module-Metadata.RangeD0)/Metadata.RangeDi; 
			Point.J = (Metadata.Height-1)- ((StateVectorsRows[Index[1]].DOY-Metadata.AzimutT0)/Metadata.AzimutTi);
		}
		if (Metadata.Orbit == "Descending" && Metadata.SideLooking =="Right")
		{
			Point.I = (Metadata.Width-1)-(Pos_Module-Metadata.RangeD0)/Metadata.RangeDi; 
		    Point.J = (StateVectorsRows[Index[1]].DOY-Metadata.AzimutT0)/Metadata.AzimutTi;
		}			
	}
	else
	{
		Point.I = (Pos_Module-Metadata.RangeD0)/Metadata.RangeDi ; 	
		Point.J = (StateVectorsRows[Index[1]].DOY-Metadata.AzimutT0)/Metadata.AzimutTi;
	}	
	
	COORD Result = static_cast<COORD>(Point);

	return Result;
}

