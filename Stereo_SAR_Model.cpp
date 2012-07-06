/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/


#include "Stereo_SAR_Model.h"
#include <stdio.h>
#include <math.h>

#include <iostream>
#include <fstream>
using namespace std;

#include "boost/numeric/ublas/lu.hpp"
#include "boost/numeric/ublas/matrix.hpp"
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace boost::numeric::ublas;

Stereo_SAR_Model::Stereo_SAR_Model(SAR_Model *Left, SAR_Model *Right)
{
	Mod_Left = Left;
	Mod_Right = Right;
}

Stereo_SAR_Model::~Stereo_SAR_Model(void)
{
}

COORD_Ecef Stereo_SAR_Model::Stereo_SAR_SlantToGround(double Left_I,double Left_J,double Right_I, double Right_J)
{
//	ofstream output;
//	output.open("prova_matrix.txt");

	int index_left = int(Mod_Left->PrecisionIndex*Left_J);
	int index_right = int(Mod_Right->PrecisionIndex*Right_J);

//	output<<index_left<<endl;
//	output<<index_right<<endl;

	SAT_COORD_Left.DOY = Mod_Left->StateVectorsRows[index_left].DOY;
	SAT_COORD_Left.X =   Mod_Left->StateVectorsRows[index_left].X;
	SAT_COORD_Left.Y =   Mod_Left->StateVectorsRows[index_left].Y;
	SAT_COORD_Left.Z =   Mod_Left->StateVectorsRows[index_left].Z;
	SAT_COORD_Left.VelocityX = Mod_Left->StateVectorsRows[index_left].VelocityX;
	SAT_COORD_Left.VelocityY = Mod_Left->StateVectorsRows[index_left].VelocityY;
	SAT_COORD_Left.VelocityZ = Mod_Left->StateVectorsRows[index_left].VelocityZ;

	SAT_COORD_Right.DOY = Mod_Right->StateVectorsRows[index_right].DOY;
	SAT_COORD_Right.X =   Mod_Right->StateVectorsRows[index_right].X;
	SAT_COORD_Right.Y =   Mod_Right->StateVectorsRows[index_right].Y;
	SAT_COORD_Right.Z =   Mod_Right->StateVectorsRows[index_right].Z;
	SAT_COORD_Right.VelocityX = Mod_Right->StateVectorsRows[index_right].VelocityX;
	SAT_COORD_Right.VelocityY = Mod_Right->StateVectorsRows[index_right].VelocityY;
	SAT_COORD_Right.VelocityZ = Mod_Right->StateVectorsRows[index_right].VelocityZ;

//	output<<SAT_COORD_Left.X<<" "<<SAT_COORD_Left.Y<<" "<<SAT_COORD_Left.Z<<endl;
//	output<<SAT_COORD_Right.X<<" "<<SAT_COORD_Right.Y<<" "<<SAT_COORD_Right.Z<<endl;

	// Retrieve approximated coordinate 

	ALFAUNO  = SAT_COORD_Left.VelocityX;     
	BETAUNO  = SAT_COORD_Left.VelocityY;
	GAMMAUNO = SAT_COORD_Left.VelocityZ;
	
	ALFADUE  = SAT_COORD_Right.VelocityX;    
    BETADUE  = SAT_COORD_Right.VelocityY; 
    GAMMADUE = SAT_COORD_Right.VelocityZ;

	K10 = Mod_Left->Metadata.RangeD0 + Mod_Left->Metadata.RangeDi*Left_I;  //metadata_left.DI + metadata_left.CS*I_L;
    K14 = Mod_Right->Metadata.RangeD0 + Mod_Right->Metadata.RangeDi*Right_I;  //metadata_right.DI + metadata_right.CS*I_R;   
    
    K0= BETAUNO/ALFAUNO;
    K1= GAMMAUNO/ALFAUNO;
    K2= SAT_COORD_Left.X + SAT_COORD_Left.Y*K0+SAT_COORD_Left.Z*K1;
    K3= -K0*ALFADUE+BETADUE;
    K4=  K1*ALFADUE-GAMMADUE;
    K5=  -K2*ALFADUE+SAT_COORD_Right.X*ALFADUE+SAT_COORD_Right.Y*BETADUE+SAT_COORD_Right.Z*GAMMADUE;
    K6=K0*K4/K3+K1;
    K7=K2-K0*K5/K3;
    K8=K7-SAT_COORD_Left.X;
    K9=K5/K3-SAT_COORD_Left.Y;
    K11=K6*K6+(K4*K4)/(K3*K3)+1;
    K12=K8*K6-K9*K4/K3+SAT_COORD_Left.Z;
    K13=K8*K8+K9*K9+SAT_COORD_Left.Z *SAT_COORD_Left.Z -K10*K10;

    ZG1=(K12+sqrt(K12*K12-K11*K13))/(K11);
    ZG2=(K12-sqrt(K12*K12-K11*K13))/(K11);

	if (ZG1<ZG2) 
	{
		Point.Z_Ecef = ZG1;
		Point.X_Ecef = -ZG1*K6+K7;
		Point.Y_Ecef = (K4/K3)*ZG1+K5/K3;
	}
	else
	{
		Point.Z_Ecef = ZG2;
		Point.X_Ecef = -ZG2*K6+K7;
		Point.Y_Ecef = (K4/K3)*ZG2+K5/K3;
	}

	// LEAST SQUARES //
		
	matrix<double> A (4,3);
	matrix<double> TN (4,1);
	matrix<double> AT (3,4);
	matrix<double> pseudoInversa (3,3);
	matrix<double> input (3,3);

	int ITERAZIONI=-4;
    
	double sigma_zero_step = 10000000; 
	double VEL_X,VEL_Y,VEL_Z;
	double LOSx,LOSy,LOSz,MODULO,MODULO_V,DEN,NUM;

	while (ITERAZIONI<0)
	{
		VEL_X = SAT_COORD_Left.VelocityX;
		VEL_Y = SAT_COORD_Left.VelocityY;
		VEL_Z = SAT_COORD_Left.VelocityZ;
        LOSx=Point.X_Ecef-SAT_COORD_Left.X;
        LOSy=Point.Y_Ecef-SAT_COORD_Left.Y;
        LOSz=Point.Z_Ecef-SAT_COORD_Left.Z;

		MODULO = sqrt(LOSx*LOSx+LOSy*LOSy+LOSz*LOSz);
        MODULO_V =sqrt(VEL_X*VEL_X+VEL_Y*VEL_Y+VEL_Z*VEL_Z);

        DEN = MODULO_V*MODULO;
        NUM = LOSx*VEL_X+LOSy*VEL_Y+LOSz*VEL_Z;

        A(0,0) = (1.0/(DEN*DEN))*(VEL_X*DEN-NUM*(MODULO_V/MODULO*LOSx));
          
        A(0,1) = (1.0/(DEN*DEN))*(VEL_Y*DEN-NUM*(MODULO_V/MODULO*LOSy));
          
        A(0,2) = (1.0/(DEN*DEN))*(VEL_Z*DEN-NUM*(MODULO_V/MODULO*LOSz));
          
        A(1,0) = (1.0/MODULO)*LOSx;
          
        A(1,1) = (1.0/MODULO)*LOSy;
          
        A(1,2) = (1.0/MODULO)*LOSz;
    
        TN(0,0)= (LOSx*VEL_X+ LOSy*VEL_Y+LOSz*VEL_Z)/(MODULO*MODULO_V);
      
        TN(1,0)= sqrt(LOSx*LOSx+LOSy*LOSy+LOSz*LOSz)-( Mod_Left->Metadata.RangeD0 + Mod_Left->Metadata.RangeDi*Left_I ); //metadata_left.DI + metadata_left.CS*I_L)

		VEL_X=SAT_COORD_Right.VelocityX;
		VEL_Y=SAT_COORD_Right.VelocityY;
		VEL_Z=SAT_COORD_Right.VelocityZ;
    
        LOSx=Point.X_Ecef-SAT_COORD_Right.X;
        LOSy=Point.Y_Ecef-SAT_COORD_Right.Y; 
        LOSz=Point.Z_Ecef-SAT_COORD_Right.Z;    
    
        MODULO = sqrt(LOSx*LOSx+LOSy*LOSy+LOSz*LOSz);
        MODULO_V = sqrt(VEL_X*VEL_X+VEL_Y*VEL_Y+VEL_Z*VEL_Z);
        DEN = MODULO_V*MODULO;
        NUM = LOSx*VEL_X+LOSy*VEL_Y+LOSz*VEL_Z;
    
        A(2,0) = (1.0/(DEN*DEN))*(VEL_X*DEN-NUM*(MODULO_V/MODULO*LOSx));
          
        A(2,1) = (1.0/(DEN*DEN))*(VEL_Y*DEN-NUM*(MODULO_V/MODULO*LOSy));
          
        A(2,2) = (1.0/(DEN*DEN))*(VEL_Z*DEN-NUM*(MODULO_V/MODULO*LOSz));
          
        A(3,0) = (1.0/MODULO)*LOSx;
          
        A(3,1) = (1.0/MODULO)*LOSy;
          
        A(3,2) = (1.0/MODULO)*LOSz;
    
        TN(2,0)=(LOSx*VEL_X+ LOSy*VEL_Y+LOSz*VEL_Z)/(MODULO*MODULO_V);
      
        TN(3,0)=sqrt(LOSx*LOSx+LOSy*LOSy+LOSz*LOSz)-(Mod_Right->Metadata.RangeD0 + Mod_Right->Metadata.RangeDi*Right_I); //metadata_right.DI + metadata_right.CS*I_R)

/*		output<< "Matrice A"<<endl;
		output<<A<<endl;

		output<<"Termine noto normale"<<endl;
		output<<TN<<endl; */
	
		AT = trans(A);

//		output<<"A Trasposta"<<endl;
//		output<<AT<<endl;

		input = prod(AT,A);

//		output<<"A Trasposta*A"<<endl;
//		output<<input<<endl;

		int n = 3;
		typedef permutation_matrix<std::size_t> pmatrix;
		// create a working copy of the input
 		matrix<double> A(input);
 		// create a permutation matrix for the LU-factorization
 		pmatrix pmi(A.size1());
		
		int res = lu_factorize(A,pmi);
	//	if( res != 0 ) output<<"non è invertibile";

	/*    output<<"Fattore A"<<endl;
		output<<A<<endl;

	    output<<"Fattore B"<<endl;
		output<<pmi<<endl;
	*/
		matrix<double> X(n,1);		

		X = prod(AT,TN);

		lu_substitute(A,pmi,X);

	//    output<<X<<endl;
	
		Point.X_Ecef-=X(0,0);
        Point.Y_Ecef-=X(1,0);
        Point.Z_Ecef-=X(2,0);

		matrix<double> sigma(1,1);
		sigma = prod(trans(X),X);
		
		double sigma_zero = sqrt(sigma(0,0));

	//	output<<"Sigma Zero ="<<sigma_zero<<endl;
				
		if (sigma_zero < 0.0000000001) ITERAZIONI =1;
		if (sigma_zero > sigma_zero_step) ITERAZIONI =1;
		ITERAZIONI++;
	}
		
	// output.close();
	
	// COORDINATES TRANSFORMATION ECEF TO PHI-LAMBDA//
	double aao=6378137.0;
	double e=0.08181919084;
	double pm=0.0000001;
	double Pi_Greco = 3.1415926535;
	int avvero=0;
    
	avvero=1;
	int k=0;
    
	Point.Longitude =atan(Point.Y_Ecef/Point.X_Ecef);
    
	if (Point.Longitude < 0)
	{
		Point.Longitude += 2.0*Pi_Greco;
	}
    
	double temp = sqrt(((Point.X_Ecef*Point.X_Ecef)+(Point.Y_Ecef*Point.Y_Ecef))*(1.0-(e*e)));
	Point.Latitude =atan(Point.Z_Ecef/temp);
    
	double temp2 = aao/sqrt(1-(e*e)*(sin(Point.Latitude)*sin(Point.Latitude)));
	Point.Height =(sqrt((Point.X_Ecef*Point.X_Ecef)+(Point.Y_Ecef*Point.Y_Ecef))/cos(Point.Latitude))-temp2;
    
	while ( avvero == 1 )
	{
      avvero=0;
      double fiw=Point.Latitude;
	  double ahw=Point.Height;

	  double sinfi= Point.Z_Ecef/(temp2*(1.0-(e*e))+(Point.Height));
	  double cosfi= sqrt((Point.X_Ecef*Point.X_Ecef)+(Point.Y_Ecef*Point.Y_Ecef))/(temp2+(Point.Height));

      Point.Latitude =atan(sinfi/cosfi);
      temp2=aao/sqrt(1.0-(e*e)*(sin(Point.Latitude)*sin(Point.Latitude)));
      Point.Height =(sqrt((Point.X_Ecef*Point.X_Ecef)+(Point.Y_Ecef*Point.Y_Ecef))/cos(Point.Latitude))-temp2;
      k=k+1;
      
	  if (abs(Point.Latitude-fiw) > (pm/aao) || abs(Point.Height-ahw) > (pm) ) 
	  {
		  avvero=1;	
	  }    
	}
        
	Point.Longitude =(Point.Longitude/Pi_Greco)*180.0 ;
    Point.Latitude =(Point.Latitude/Pi_Greco)*180.0;
		
	return Point;
}

