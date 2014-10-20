#include "SAR_Slant_Model.h"


SAR_Slant_Model::SAR_Slant_Model(SAR_Metadata &InputMetadata): SAR_Model(InputMetadata)
{
}

SAR_Slant_Model::SAR_Slant_Model(SAR_Metadata &InputMetadata, int p): SAR_Model(InputMetadata, p)
{
}


SAR_Slant_Model::~SAR_Slant_Model(void)
{
}

P_COORD SAR_Slant_Model::SAR_GroundToImage(double Lon,double Lat, double H) 
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
	Index[2]= PrecisionIndex*Metadata.Height-1;
	Index[1]= int(((Index[0]+Index[2])/2));

	while ( (fabs((T_Angles[1]-Pi_Greco)/2) > 0.0000000001) && (Index[0] != Index[1])) 
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
		//Point.I = Pos_Module;
		//double cccc  = Pos_Module;
		Point.I = (Pos_Module-Metadata.RangeD0)/Metadata.RangeDi ; 	
		Point.J = (StateVectorsRows[Index[1]].DOY-Metadata.AzimutT0)/Metadata.AzimutTi;
	}	
	
	P_COORD Result = static_cast<P_COORD>(Point);

	return Result;
}