/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "AppVerify.h"
#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "DynamicObject.h"
#include "GeoPoint.h"
#include "Layer.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "Ortho_GUI.h"
#include "Orthorectification.h"
#include "PlugInArgList.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "ProgressResource.h"
#include "RADARSAT_Metadata.h"
#include "RasterDataDescriptor.h"
#include "StringUtilities.h"
#include "UtilityServices.h"

#include <Qt/qevent.h>
#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtGui/QAction>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QInputDialog>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

#include "boost/accumulators/accumulators.hpp"
#include "boost/accumulators/statistics/stats.hpp"
#include "boost/accumulators/statistics/mean.hpp"
#include "boost/accumulators/statistics/variance.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/tokenizer.hpp"

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>


using namespace std;

using namespace boost::accumulators;

using namespace boost;

Ortho_GUI::Ortho_GUI( QWidget* pParent, const char* pName, bool modal,int sensor_type)
: QDialog(pParent)
{

   // Select sensor type //

   if (sensor_type ==0)
   {
	   Metadata = new TerraSAR_Metadata();
	   sensor_name = "TerraSAR-X";
   }
   if (sensor_type ==1) 
   {
	   Metadata = new RADARSAT_Metadata();
	   sensor_name = "RADARSAT-2";
   }

   if (pName == NULL)
   {
      setObjectName(QString::fromStdString(sensor_name+" Orthorectification"));
   }
   setModal( FALSE );
   setWindowTitle(QString::fromStdString("Orthorectification "+sensor_name+" imagery"));

   // GUI widget object initialitation //

   mpImageListCombo = new QComboBox( this );
   mpDSMListCombo = new QComboBox( this );
   mpInterpolationList = new QComboBox( this );
   mpDSMInterpolationList = new QComboBox( this );

   mpImageListCombo->setFixedSize(400,20);
   mpDSMListCombo->setFixedSize(300,20);

   Datum  = new QLabel( "DATUM: WGS84", this );

   Resampling = new QLabel( "Resampling Method", this );

   mpStartOrtho = new QPushButton( "Start", this );
   mpCheckModel = new QPushButton( "Check Model", this );
   mpCancelButton = new QPushButton( "cancelButton", this );
   mpCheckImage = new QPushButton( "Check Image", this );

   Height = new QDoubleSpinBox(this); 
   GeoidOffSet =  new QDoubleSpinBox(this);
   X_Spacing =  new QDoubleSpinBox(this);
   Y_Spacing = new QDoubleSpinBox(this);

   mpFlatten = new QRadioButton("Flatten Earth", this);
   mpDsm = new QRadioButton("DSM Orthorectification", this);
   
   pOrthoModeGroup = new QButtonGroup(this);
   pOrthoModeGroup->setExclusive(true);
   pOrthoModeGroup->addButton(mpFlatten);
   pOrthoModeGroup->addButton(mpDsm);

   // GUI Layout Design //
   
   QGridLayout* MainLayout = new QGridLayout(this);
   
   // Input Image group box //
   QGridLayout* pLayout1 = new QGridLayout();

   pLayout1->addWidget(mpImageListCombo,0,0,1,3);
   pLayout1->addWidget(Resampling,1,0);
   pLayout1->addWidget(mpInterpolationList,1,1,1,2);
   pLayout1->addWidget(mpCheckModel,2,1);
   pLayout1->addWidget(mpCheckImage,2,0);

   QGroupBox* box1 = new QGroupBox(tr("Select Input Image"));
   box1->setLayout(pLayout1);
   MainLayout->addWidget( box1, 0, 0,3,3);
   
   // Datum & Projection Group Box //
   QLabel *Project = new QLabel("Default Projection: UTM");
   QLabel *unitX = new QLabel("Meters");
   QLabel *unitY = new QLabel("Meters");
   QLabel *X_spac = new QLabel("X Pixel Size:");
   QLabel *Y_spac = new QLabel("Y Pixel Size:");

   QGridLayout* pLayout2 = new QGridLayout();
   pLayout2->addWidget(Datum,0,0);
   pLayout2->addWidget(Project,1,0);
   pLayout2->addWidget(X_spac,2,0);
   pLayout2->addWidget(Y_spac,3,0);
   pLayout2->addWidget(X_Spacing,2,1);
   pLayout2->addWidget(Y_Spacing,3,1);
   pLayout2->addWidget(unitX,2,2);
   pLayout2->addWidget(unitY,3,2);

   box2 = new QGroupBox(tr("Projection Information"));
   box2->setLayout(pLayout2);
   MainLayout->addWidget( box2, 0, 3,3,3);

   // Flatten Earth GroupBox
   QLabel *Lheight = new QLabel("Ellipsoidal Height:");
   QGridLayout* pLayout3 = new QGridLayout();
   pLayout3->addWidget(Lheight,0,0);
   pLayout3->addWidget(Height,0,1);

   box3 = new QGroupBox();
   box3->setLayout(pLayout3);
   MainLayout->addWidget( box3, 4, 0,1,3);
   
   // Input DSM GroupBox
   QLabel *Lgeoid = new QLabel("Insert Geoid Offset:");
   QLabel *Ldsm = new QLabel("Select Input DSM:");
   DSMResampling = new QLabel( "DSM Resampling Method", this );
   QGridLayout* pLayout4 = new QGridLayout();
   pLayout4->addWidget(Ldsm,0,0);
   pLayout4->addWidget(mpDSMListCombo,0,1,1,2);
   pLayout4->addWidget(DSMResampling,1,0);
   pLayout4->addWidget(mpDSMInterpolationList,1,1,1,2);
   pLayout4->addWidget(Lgeoid,2,1);
   pLayout4->addWidget(GeoidOffSet,2,2);

   box4 = new QGroupBox();
   box4->setLayout(pLayout4);
   MainLayout->addWidget( box4, 6, 0,3,3);

   // Button 

   MainLayout->addWidget(mpFlatten,3,0);
   MainLayout->addWidget(mpDsm,5,0);
   MainLayout->addWidget( mpCancelButton, 8, 5);
   MainLayout->addWidget( mpStartOrtho, 8, 4 );   

   mpCancelButton->setText("Close");
   
   // signals and slots connections
   VERIFYNRV(connect( mpCancelButton, SIGNAL( clicked() ), this, SLOT( reject() )));
   VERIFYNRV(connect( mpCheckImage, SIGNAL( clicked() ), this, SLOT( CheckImage() )));
   VERIFYNRV(connect( mpCheckModel, SIGNAL( clicked() ), this, SLOT( CheckModel() )));
   VERIFYNRV(connect( mpStartOrtho, SIGNAL( clicked() ), this, SLOT( StartOrtho() )));
   VERIFYNRV(connect(mpFlatten, SIGNAL(toggled(bool)), box3, SLOT(setEnabled(bool))));
   VERIFYNRV(connect(mpDsm, SIGNAL(toggled(bool)), box4, SLOT(setEnabled(bool))));

   init();

}

Ortho_GUI::~Ortho_GUI(void)
{
}

void Ortho_GUI::init()
{
   Service<ModelServices> pModel;
   mCubeNames = pModel->getElementNames("RasterElement");

   int ii=0;
   for (unsigned int i = 0; i < mCubeNames.size(); i++)
   {
	  size_t pos;
	  std::string file_ext;      	  
	  pos = mCubeNames[i].find_last_of(".");
	  file_ext = mCubeNames[i].substr(pos);	  

      mpImageListCombo->insertItem(i, QString::fromStdString(mCubeNames[i]));
	
	  if (file_ext.compare(".asc") ==0) 
	  {
		  mpDSMListCombo->insertItem(ii, QString::fromStdString(mCubeNames[i]));	
		  ii++;
	  }

   }

   mpInterpolationList->insertItem(0,QString::fromStdString("Nearest Neighbor Interpolation"));
   mpInterpolationList->insertItem(1,QString::fromStdString("Box Average 3x3"));
   mpInterpolationList->insertItem(2,QString::fromStdString("Box Average 5x5"));
   mpInterpolationList->insertItem(3,QString::fromStdString("Box Average 7x7"));

   mpDSMInterpolationList->insertItem(0,QString::fromStdString("Nearest Neighbor Interpolation"));
   mpDSMInterpolationList->insertItem(1,QString::fromStdString("Bilinear Interpolation"));
   mpDSMInterpolationList->setCurrentIndex(1);

   X_Spacing->setMinimum(0);
   X_Spacing->setDecimals(3);
   X_Spacing->setMaximum(50000000);

   Y_Spacing->setMinimum(0);
   Y_Spacing->setDecimals(3);
   Y_Spacing->setMaximum(50000000);

   Height->setMinimum(-100);
   Height->setDecimals(4);
   Height->setMaximum(10000);

   mpFlatten->setChecked(true);

   mpFlatten->setEnabled(false);
   mpDsm->setEnabled(false);
   mpCheckModel->setEnabled(false);
   box2->setEnabled(false);
   box3->setEnabled(false);
   box4->setEnabled(false);
   mpStartOrtho->setEnabled(false);

}

void Ortho_GUI::StartOrtho()
{
   mpStartOrtho->setEnabled(false);

   // Update Grid Information
   OrthoGrid.X_Step = X_Spacing->value();
   OrthoGrid.Y_Step = Y_Spacing->value();

   OrthoGrid.X_Dim = int(OrthoGrid.X_Dim/OrthoGrid.X_Step)+1.0;
   OrthoGrid.Y_Dim = int(OrthoGrid.Y_Dim/OrthoGrid.Y_Step)+1.0; 

   OrthoGrid.Lon_Step = (OrthoGrid.Lon_Max-OrthoGrid.Lon_Min)/OrthoGrid.X_Dim;
   
   OrthoGrid.Lat_Step = (OrthoGrid.Lat_Max-OrthoGrid.Lat_Min)/OrthoGrid.Y_Dim;

   //Start Ortho
   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());

   FactoryResource<DataRequest> pRequest;
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());

   DataDescriptor* dMeta = pCube->getDataDescriptor();

   DynamicObject* oMetadata = dMeta->getMetadata();

	// RETRIEVE & UPDATE METADATA INFORMATION //

	bool control = Metadata->ReadFile(image_path);

	Metadata->UpdateMetadata(oMetadata); 

	SAR_Model *ModProva;
		
	ModProva = new SAR_Model(*Metadata,10);

	Orthorectification ProcessOrtho(pCube, ModProva, OrthoGrid, Height->value());
	
	// RETRIVE SELECTED RESAMPLING METHOD AND EXECUTE ORTHO

	int indexR = mpInterpolationList->currentIndex();
	
	if (mpFlatten->isChecked() ==true) 
	{
		VERIFYNRV(ProcessOrtho.process(indexR));
		
		
	}
	else 
	{
		VERIFYNRV(RetrieveDSMGrid());
		VERIFYNRV(ProcessOrtho.process(indexR, pDSM, DSMGrid, GeoidOffSet->value(),mpDSMInterpolationList->currentIndex()));
	}


}

void Ortho_GUI::CheckButton(QAbstractButton* pButton)
{
   if (pButton == mpFlatten)
   {
       box3->setEnabled(true);
	   box4->setEnabled(false);
   }
   else if (pButton == mpDsm)
   {
	   box3->setEnabled(false);
	   box4->setEnabled(true);
   }
}

void Ortho_GUI::CheckImage()
{
   mpCheckImage->setEnabled(false);
	
   Service<ModelServices> pModel;	

   image_name = mCubeNames.at(mpImageListCombo->currentIndex());

   pCube =  dynamic_cast<RasterElement*>(pModel->getElement(image_name,"",NULL ));

   image_path =  pCube->getFilename();
 
   bool control = Metadata->ReadFile(image_path);

   if (control == false)
   { 
	   QMessageBox::information(this, QString::fromStdString("Error Information"), 
		                        QString::fromStdString("This is not a "+sensor_name+" SLC standard product") );
	   return;	
   }

   mpStartOrtho->setEnabled(true);
   mpFlatten->setEnabled(true);
   mpDsm->setEnabled(true);
   mpCheckModel->setEnabled(true);
   box2->setEnabled(true);
   box3->setEnabled(true);

   mpImageListCombo->setEnabled(false);

   Height->setValue(Metadata->CornerCoordinate[0].Height);

   ComputeGrid();
}

void Ortho_GUI::CheckModel()
{
   ProgressResource pProgress("ProgressBar");

   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());

   FactoryResource<DataRequest> pRequest;
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());

   DataDescriptor* dMeta = pCube->getDataDescriptor();

   DynamicObject* oMetadata = dMeta->getMetadata();

	// RETRIEVE & UPDATE METADATA INFORMATION //

	bool control = Metadata->ReadFile(image_path);

	Metadata->UpdateMetadata(oMetadata); 
  
	// WIDGET SELECT & RETRIEVE GCPs INFORMATION  //
	GcpList * GCPs = NULL;

    Service<ModelServices> pModel;
	std::vector<DataElement*> pGcpLists = pModel->getElements(pCube, TypeConverter::toString<GcpList>());

    if (!pGcpLists.empty())
	{
         QStringList aoiNames("<none>");
         for (std::vector<DataElement*>::iterator it = pGcpLists.begin(); it != pGcpLists.end(); ++it)
         {
            aoiNames << QString::fromStdString((*it)->getName());
         }
         QString aoi = QInputDialog::getItem(Service<DesktopServices>()->getMainWidget(),
            "Select a GCP List", "Select a GCP List for validate the orientation model", aoiNames);
         
         if (aoi != "<none>")
         {
            std::string strAoi = aoi.toStdString();
            for (std::vector<DataElement*>::iterator it = pGcpLists.begin(); it != pGcpLists.end(); ++it)
        {
               if ((*it)->getName() == strAoi)
               {
                  GCPs = static_cast<GcpList*>(*it);
                  break;
               }
            }
            if (GCPs == NULL)
            {
               std::string msg = "Invalid GCPList.";
             //  pStep->finalize(Message::Failure, msg);

               pProgress->updateProgress(msg, 0, ERRORS);

               return;
            }
         }
	} // End if GcpList
   
  	
	// UPDATE GCPs HEIGHT INFORMATION AND SWITCH Lat&Lon COORDINATE FOR CORRECT VISUALIZAZION IN THE GCPs EDITOR
 
    std::list<GcpPoint> Punti = GCPs->getSelectedPoints();
     
	Punti = Metadata->UpdateGCP(Punti, image_path);

	SAR_Model ModProva(*Metadata,100);

	P_COORD Punto;
	int N=Punti.size();
	int n=0;
	double Lat, Lon;

	accumulator_set<double, stats<tag::mean, tag::variance> > accX, accY;

	list<GcpPoint>::iterator pList;
	for (pList = Punti.begin(); pList != Punti.end(); pList++)
	{
		if(pList->mPixel.mX<Metadata->Width && pList->mPixel.mY<Metadata->Height)	
		{
			Lon = pList->mCoordinate.mX;
			Lat = pList->mCoordinate.mY;
			
			Punto = ModProva.SAR_GroundToSlant(pList->mCoordinate.mX,pList->mCoordinate.mY,pList->mCoordinate.mZ); 
			pList->mRmsError.mX = pList->mPixel.mX -Punto.I;
			pList->mRmsError.mY = pList->mPixel.mY -Punto.J;
			accX(pList->mRmsError.mX);
			accY(pList->mRmsError.mY);

			pList->mCoordinate.mX = Lat;
			pList->mCoordinate.mY = Lon;

		}
		else
		{
			Lon = pList->mCoordinate.mX;
			Lat = pList->mCoordinate.mY;
			pList->mRmsError.mX = -9999;
			pList->mRmsError.mY = -9999;
			pList->mCoordinate.mX = Lat;
			pList->mCoordinate.mY = Lon;
		}


        pProgress->updateProgress("Calculating statistics", int(100*n/N), NORMAL);
		
		n++;
	}

	 double meanX = mean(accX);
	 double meanY = mean(accY);

	 double varX = variance(accX);
	 double varY = variance(accY);
	
     GCPs->clearPoints();
     GCPs->addPoints(Punti);
	

		std::string msg = "Number of Rows : " + StringUtilities::toDisplayString(pDesc->getRowCount()) + "\n"
						  "Number of Columns : " + StringUtilities::toDisplayString(pDesc->getColumnCount()) + "\n\n"
						  "Metadata update completed" + "\n\n"					  
						  "**********     Validation Results     **********" "\n\n"
						  "Number of GCPs: " + StringUtilities::toDisplayString(Punti.size()) + "\n\n"
						  "Mean I : " + StringUtilities::toDisplayString(meanX) + "\n"
						  "Variance I : " + StringUtilities::toDisplayString(varX) + "\n\n"
						  "Mean J : " + StringUtilities::toDisplayString(meanY) + "\n"
						  "Variance J : " + StringUtilities::toDisplayString(varY) + "\n\n" ;
				  						                      
		pProgress->updateProgress(msg, 100, NORMAL);

     //	pStep->finalize(); 
}

void Ortho_GUI::ComputeGrid()
{
   // Compute grid information

   ofstream output;
   output.open("prova_grid.txt");
   output.precision(10);
   
   int n = Metadata->CornerCoordinate.size();

   output<<"Numero corner "<<n<<endl;

   vector<double> Latitude, Longitude, Nord, East;
   vector<LatLonPoint> Corner;
   vector<UtmPoint> CornerUTM;
   string Lat, Lon;
   Corner.resize(n);
   CornerUTM.reserve(n);
   Latitude.resize(n);
   Longitude.resize(n);
   Nord.resize(n);
   East.resize(n);

   for (int i=0;i<n;i++)
   {
	   Lat = boost::lexical_cast<string>(Metadata->CornerCoordinate[i].Latitude);
       Lon = boost::lexical_cast<string>(Metadata->CornerCoordinate[i].Longitude);  
	   Latitude[i] = Metadata->CornerCoordinate[i].Latitude;
	   Longitude[i] = Metadata->CornerCoordinate[i].Longitude;

	   output<<Lat<<" "<<Lon<<endl;

	   Corner[i] = LatLonPoint(Lat,Lon);
	   CornerUTM[i] = UtmPoint(Corner[i]);
	   East[i]=CornerUTM[i].getEasting();
	   Nord[i]=CornerUTM[i].getNorthing();
	   
	   output<<Nord[i]<<" "<<East[i]<<endl;

   }

   vector<double>::iterator index;
   index = min_element(East.begin(),East.end());
   double Min_East = East.at(distance(East.begin(),index));
   double Min_Lon = Longitude.at(distance(East.begin(),index));

   OrthoGrid.hemisphere = CornerUTM[distance(East.begin(),index)].getHemisphere();
   OrthoGrid.iZone = CornerUTM[distance(East.begin(),index)].getZone();
   
   index = max_element(East.begin(),East.end());
   double Max_East = East.at(distance(East.begin(),index)); 
   double Max_Lon = Longitude.at(distance(East.begin(),index)); 
   
   index = min_element(Nord.begin(),Nord.end());
   double Min_Nord = Nord.at(distance(Nord.begin(),index));
   double Min_Lat = Latitude.at(distance(Nord.begin(),index));

   index = max_element(Nord.begin(),Nord.end());
   double Max_Nord = Nord.at(distance(Nord.begin(),index));
   double Max_Lat = Latitude.at(distance(Nord.begin(),index));


   output<<endl;
   output<<Min_East<<"	"<<Max_East<<"	"<<Max_East-Min_East<<endl;
   output<<Min_Nord<<"	"<<Max_Nord<<"	"<<Max_Nord-Min_Nord<<endl;	

   OrthoGrid.X_Min = Min_East;
   OrthoGrid.Y_Min = Min_Nord;
   OrthoGrid.X_Max = Max_East;
   OrthoGrid.Y_Max = Max_Nord;
   OrthoGrid.X_Dim = (Max_East-Min_East);
   OrthoGrid.Y_Dim = (Max_Nord-Min_Nord);

   X_Spacing->setValue(OrthoGrid.X_Dim/Metadata->Width);
   Y_Spacing->setValue(OrthoGrid.Y_Dim/Metadata->Height);   
   OrthoGrid.X_Step = X_Spacing->value();
   OrthoGrid.Y_Step = Y_Spacing->value();

   OrthoGrid.Lon_Min = Min_Lon;
   OrthoGrid.Lat_Min = Min_Lat;
   OrthoGrid.Lon_Max = Max_Lon;
   OrthoGrid.Lat_Max = Max_Lat;

   output.close();
}

bool Ortho_GUI::RetrieveDSMGrid()
{

	// Retrieve DSMgrid information from asc file

    ofstream output;
    output.open("prova_dsm.txt");
    output.precision(10);

	Service<ModelServices> pModel;	
	DSM_name = mCubeNames.at(mpDSMListCombo->currentIndex());
	pDSM =  dynamic_cast<RasterElement*>(pModel->getElement(DSM_name,"",NULL ));
	DSM_path =  pDSM->getFilename();

	string line;
	size_t pos;
	ifstream fileDSM (DSM_path);

	getline (fileDSM, line);
	pos = line.find_last_of(" ");
	line = line.substr(pos+1);
	DSMGrid.X_Dim = atof(line.c_str());
    output<<"n-cols "<< DSMGrid.Y_Dim<<endl;

	getline (fileDSM, line);
	pos = line.find_last_of(" ");
	line = line.substr(pos+1);
	DSMGrid.Y_Dim = atof(line.c_str());
	output<<"n-rows "<< DSMGrid.Y_Dim<<endl;

    getline (fileDSM, line);
	pos = line.find_last_of(" ");
	line = line.substr(pos+1);
    DSMGrid.Lon_Min = atof(line.c_str());
	output<<"Lon min "<< DSMGrid.Lon_Min<<endl;

	getline (fileDSM, line);
	pos = line.find_last_of(" ");
	line = line.substr(pos+1);
    DSMGrid.Lat_Min = atof(line.c_str());
	output<<"Lat min "<< DSMGrid.Lat_Min<<endl;	

	getline (fileDSM, line);
	pos = line.find_last_of(" ");
	line = line.substr(pos+1);
	DSMGrid.Lon_Step = atof(line.c_str());
	output<<"Lon Step "<< DSMGrid.Lon_Step<<endl;	
	DSMGrid.Lat_Step = atof(line.c_str());
	output<<"Lat Step "<< DSMGrid.Lat_Step<<endl;

    getline (fileDSM, line);
	pos = line.find_last_of(" ");
	line = line.substr(pos+1);
	DSMGrid.nodatavalue = atof(line.c_str());
	output<<"NaN "<< DSMGrid.nodatavalue<<endl;	

	output.close();

	DSMGrid.Lat_Max = DSMGrid.Lat_Min + DSMGrid.Lat_Step*DSMGrid.Y_Dim;
	DSMGrid.Lon_Max = DSMGrid.Lon_Min + DSMGrid.Lon_Step*DSMGrid.X_Dim;

	//COMPARE DSM and IMAGE grid in order to verify if DSM covers all image data

	std::string msg = ("The DSM does not cover the entire image. \n\n"
		              "Geographical Image Extension:\n\n" 
					  "Lat Min: " + StringUtilities::toDisplayString(DSMGrid.Lat_Min) + "\n"
					  "Lat Max: " + StringUtilities::toDisplayString(DSMGrid.Lat_Max) + "\n"
					  "Lon Min: " + StringUtilities::toDisplayString(DSMGrid.Lon_Min) + "\n"
					  "Lon Max: " + StringUtilities::toDisplayString(DSMGrid.Lon_Max) + "\n");

	if ((DSMGrid.Lat_Max < OrthoGrid.Lat_Max) || (DSMGrid.Lat_Min > OrthoGrid.Lat_Min)) 
	{
		QMessageBox::information(this, QString::fromStdString("Error Information"), 
		               QString::fromStdString(msg));
	    return false;
	}
	
	if ((DSMGrid.Lon_Max < OrthoGrid.Lon_Max) || (DSMGrid.Lon_Min > OrthoGrid.Lon_Min)) 
	{
	    QMessageBox::information(this, QString::fromStdString("Error Information"), 
		               QString::fromStdString(msg));
	    return false;	
	}


	return true;

}