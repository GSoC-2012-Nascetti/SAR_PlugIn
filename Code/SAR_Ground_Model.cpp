#include "SAR_Ground_Model.h"


SAR_Ground_Model::SAR_Ground_Model(SAR_Metadata &InputMetadata): SAR_Model(InputMetadata)
{
	SlantRangePixelDistance.resize(Metadata.Width);
	PixelColumnSpacing.resize(Metadata.Width);

	double DG = 0.;

	SlantRangePixelDistance[0] = 0.;
	PixelColumnSpacing[0] = 0.;

	for (int i=1; i<Metadata.Width; i++)
	{
		DG = Metadata.ColumnSpacing*i-Metadata.GroundRange.G0;
		SlantRangePixelDistance[i] =	Metadata.GroundRange.S0 - Metadata.RangeD0 +
										Metadata.GroundRange.S1*DG +
										Metadata.GroundRange.S2*pow(DG,2) +
										Metadata.GroundRange.S3*pow(DG,3) +
										Metadata.GroundRange.S4*pow(DG,4) +
										Metadata.GroundRange.S5*pow(DG,5);
		PixelColumnSpacing[i] = SlantRangePixelDistance[i]-SlantRangePixelDistance[i-1];
	}
}

SAR_Ground_Model::SAR_Ground_Model(SAR_Metadata &InputMetadata, int p): SAR_Model(InputMetadata, p)
{
	SlantRangePixelDistance.resize(Metadata.Width);
	PixelColumnSpacing.resize(Metadata.Width);

	double DG = 0.;

	SlantRangePixelDistance[0] = 0.;
	PixelColumnSpacing[0] = 0.;

	for (int i=1; i<Metadata.Width; i++)
	{
		DG = Metadata.ColumnSpacing*i-Metadata.GroundRange.G0;
		SlantRangePixelDistance[i] =	Metadata.GroundRange.S0 - Metadata.RangeD0 +
										Metadata.GroundRange.S1*DG +
										Metadata.GroundRange.S2*pow(DG,2) +
										Metadata.GroundRange.S3*pow(DG,3) +
										Metadata.GroundRange.S4*pow(DG,4) +
										Metadata.GroundRange.S5*pow(DG,5);
		PixelColumnSpacing[i] = SlantRangePixelDistance[i]-SlantRangePixelDistance[i-1];
	}
}

SAR_Ground_Model::~SAR_Ground_Model(void)
{
}


P_COORD SAR_Ground_Model::SAR_GroundToImage(double Lon, double Lat, double H) 
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
		
	//GROUND RANGE MODEL

	/*ofstream file_out;
	file_out.open("C:\\Ground_Stampa.txt");
	file_out.precision(5);
	*/
	double min_cs = PixelColumnSpacing[1];
	double max_cs = PixelColumnSpacing[Metadata.Width-1];


	
	if(min_cs > max_cs)
	{
		double app = min_cs;
		min_cs = max_cs;
		max_cs = app;
	}
	
	if (Metadata.SatelliteName == "RADARSAT-2")
	{
		int min_index = 0; //int((Pos_Module-Metadata.RangeD0)/max_cs);
		int max_index = SlantRangePixelDistance.size(); // int((Pos_Module-Metadata.RangeD0)/min_cs)+1;


		std::vector<double> prova; //(&SlantRangePixelDistance[min_index],&SlantRangePixelDistance[max_index]);
		prova.resize(max_index-min_index+1);

		prova[0] = fabs(SlantRangePixelDistance[min_index]-(Pos_Module-Metadata.RangeD0));

		double min = prova[0];
		int min_idx = 0; 

		for(unsigned int i = 1; i< prova.size();i++)
		{
			prova[i] = fabs(SlantRangePixelDistance[i+min_index]-(Pos_Module-Metadata.RangeD0));
			if (prova[i]<min)
			{
				min = prova[i];
				min_idx = i;
			} 
		}

		if (Metadata.Orbit == "Ascending" && Metadata.SideLooking =="Right") 
		{
			Point.I = min_idx+min_index;

			Point.J = (Metadata.Height-1)- ((StateVectorsRows[Index[1]].DOY-Metadata.AzimutT0)/Metadata.AzimutTi);
		}

		if (Metadata.Orbit == "Descending" && Metadata.SideLooking =="Right")
		{
			//Point.I = (Metadata.Width-1)-(Pos_Module-Metadata.RangeD0)/Metadata.RangeDi; 
		    
			Point.I = (Metadata.Width-1) - min_idx+min_index;

			Point.J = (StateVectorsRows[Index[1]].DOY-Metadata.AzimutT0)/Metadata.AzimutTi;

			//file_out<<Point.I<<"\t"<<Point.J<<endl;
		}		

		//file_out.close();
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