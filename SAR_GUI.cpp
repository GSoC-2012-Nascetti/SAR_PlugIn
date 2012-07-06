/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DesktopServices.h"
#include "SAR_GUI.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "Progress.h"
#include "RasterElement.h"
#include "RasterDataDescriptor.h"
#include "Service.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

#include "TerraSAR_Metadata.h"
#include "RADARSAT_Metadata.h"
#include "SAR_Model.h"
#include "Stereo_SAR_Model.h"
#include "StringUtilities.h"
#include <ProgressResource.h>
#include "Test_Update_TerraSAR.h"

#include <QtCore\QFileInfo>
#include <QtGui\QComboBox>
#include <QtGui\QDoubleSpinBox>
#include <QtGui\QGridLayout>
#include <QtGui\QLabel>
#include <QtGui\QPushButton>

SAR_GUI::SAR_GUI( QWidget* pParent, const char* pName, bool modal )
: QDialog(pParent)
{
   if (pName == NULL)
   {
      setObjectName( "PixelAspectRatioGui" );
   }
   setModal( FALSE );
   setWindowTitle("3D Stero Measurements");

   // GUI label creation
   labelImageLeft  = new QLabel( "Select Image Left", this );
   labelImageRight = new QLabel( "Select Image Right", this );

   labelImagePoints = new QLabel( "Insert Image Points Coordinate", this );
   labelLeftX  =  new QLabel( "Left X", this );
   labelLeftY  =  new QLabel( "Left Y", this );
   labelRightX  = new QLabel( "Right X", this );
   labelRightY  = new QLabel( "Right Y", this );

   labelGroundPoint = new QLabel( "Ground Points Coordinate", this );
   labelLat =     new QLabel( "Latitude", this );
   labelLon =     new QLabel( "Longitude", this );
   labelHeight =  new QLabel( "Height", this );

   mpGetMapLocation = new QPushButton( "Get Map Location", this );

   mpCubeListCombo = new QComboBox( this );
   mpCubeListCombo_slave = new QComboBox( this );

   leftX =  new QDoubleSpinBox(this);
   leftY =  new QDoubleSpinBox(this);
   rightX = new QDoubleSpinBox(this);
   rightY = new QDoubleSpinBox(this);

   Lat = new QDoubleSpinBox(this);
   Lon = new QDoubleSpinBox(this);
   Height = new QDoubleSpinBox(this);

   mpCancelButton = new QPushButton( "cancelButton", this );

   // GUI Layout 
   
   QGridLayout* pLayout = new QGridLayout(this);
   
   pLayout->addWidget( labelImageLeft, 0, 0 );
   pLayout->addWidget( labelImageRight, 0, 3 );

   pLayout->addWidget( mpCubeListCombo, 1, 0, 1, 2 );
   pLayout->addWidget( mpCubeListCombo_slave, 1, 3, 1, 2 );

   mpCubeListCombo->setFixedSize(400,20);
   mpCubeListCombo_slave->setFixedSize(400,20);
   
   pLayout->addWidget( labelImagePoints, 2, 0 );
      
   QGridLayout* pLayout2 = new QGridLayout();

   pLayout2->addWidget( labelLeftX, 0, 0 );
   pLayout2->addWidget( labelLeftY, 1, 0 );
   pLayout2->addWidget( leftX, 0, 1 );
   pLayout2->addWidget( leftY, 1, 1 );
   pLayout2->addWidget( labelRightX, 0, 3 );
   pLayout2->addWidget( labelRightY, 1, 3 );
   pLayout2->addWidget( rightX, 0, 4 );
   pLayout2->addWidget( rightY, 1, 4 );      
   QGroupBox* box1 = new QGroupBox();
   box1->setLayout(pLayout2);
   pLayout->addWidget( box1, 3, 0,2,5);

   pLayout->addWidget( labelGroundPoint, 5, 0 );
       
   QGridLayout* pLayout3 = new QGridLayout();
   
   pLayout3->addWidget( labelLat, 0, 0, Qt::AlignRight );
   pLayout3->addWidget( labelLon, 1, 0, Qt::AlignRight  );
   pLayout3->addWidget( labelHeight, 0, 3, Qt::AlignRight );   
   pLayout3->addWidget( Lat,0, 1);
   pLayout3->addWidget( Lon,1, 1);
   pLayout3->addWidget( Height, 0, 4);   
   QGroupBox* box2 = new QGroupBox();
   box2->setLayout(pLayout3);
   pLayout->addWidget( box2, 6, 0,2,3);

   pLayout->addWidget( mpCancelButton, 8, 4);
   pLayout->addWidget( mpGetMapLocation, 8, 0 );   

   mpCancelButton->setText("Close");
   //resize( QSize(600, 150).expandedTo(minimumSizeHint()) );
   
   // signals and slots connections
   bool ok = connect( mpCancelButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
   ok = connect( mpGetMapLocation, SIGNAL( clicked() ),this, SLOT( GetMapLocation()));
   init();

}

/*
*  Destroys the object and frees any allocated resources
*/
SAR_GUI::~SAR_GUI()
{

}

void SAR_GUI::init()
{
   Service<ModelServices> pModel;
   mCubeNames = pModel->getElementNames("RasterElement");

   for (unsigned int i = 0; i < mCubeNames.size(); i++)
   {
      mpCubeListCombo->insertItem(i, QString::fromStdString(mCubeNames[i]));
	  mpCubeListCombo_slave->insertItem(i, QString::fromStdString(mCubeNames[i]));
   }

   leftX->setMinimum(0);
   leftX->setDecimals(3);
   leftX->setMaximum(50000);

   leftY->setMinimum(0);
   leftY->setDecimals(3);
   leftY->setMaximum(50000);

   rightX->setMinimum(0);
   rightX->setDecimals(3);
   rightX->setMaximum(50000);

   rightY->setMinimum(0);
   rightY->setDecimals(3);
   rightY->setMaximum(50000);

   Lat->setMinimum(-180);
   Lat->setDecimals(8);
   Lat->setMaximum(360);

   Lon->setMinimum(-180);
   Lon->setDecimals(8);
   Lon->setMaximum(360);

   Height->setMinimum(-100);
   Height->setDecimals(4);
   Height->setMaximum(10000);

}
  
void SAR_GUI::GetMapLocation()
{
	Service<ModelServices> pModel;

	ProgressResource pProgress("pProgress");

    pProgress->updateProgress("Pluto", 0, NORMAL);
    
	name_left = mCubeNames.at(mpCubeListCombo->currentIndex());
	name_right = mCubeNames.at(mpCubeListCombo_slave->currentIndex());

    RasterElement *pCube_Left =  dynamic_cast<RasterElement*>(pModel->getElement(name_left,"",NULL ));
	RasterElement *pCube_Right = dynamic_cast<RasterElement*>(pModel->getElement(name_right,"",NULL ));
    
	std::string path_left =  pCube_Left->getFilename();
	std::string path_right = pCube_Right->getFilename();

	DataDescriptor* dMeta_Left =  pCube_Left->getDataDescriptor();
	DataDescriptor* dMeta_Right = pCube_Right->getDataDescriptor();

	DynamicObject* Metadata_Left = dMeta_Left->getMetadata();
	DynamicObject* Metadata_Right = dMeta_Right->getMetadata();

	// RETRIEVE & UPDATE METADATA INFORMATION //
		   
	RADARSAT_Metadata SAR_Metadata_Left,SAR_Metadata_Right;

	// Update Metadata Left
	SAR_Metadata_Left.ReadFile(path_left);

    bool control = SAR_Metadata_Left.ReadFile(path_left);

	if (control == false)
	{
	std::string msg = "This is not a RADARSAT-2 Stereopair";
	pProgress->updateProgress(msg, 100, ERRORS);	
	}

	SAR_Metadata_Left.UpdateMetadata(Metadata_Left); 

	// Update Metadata Right	
	SAR_Metadata_Right.ReadFile(path_right);

    control = SAR_Metadata_Right.ReadFile(path_right);

	if (control == false)
	{
	std::string msg = "This is not a RADARSAT-2 Stereopair";
	pProgress->updateProgress(msg, 100, ERRORS);	
	}

	SAR_Metadata_Right.UpdateMetadata(Metadata_Right);

	// STEREO MODEL INITIALITATION //

	SAR_Model *Model_Left;
	Model_Left = new SAR_Model(SAR_Metadata_Left);
	
	SAR_Model *Model_Right; 
	Model_Right = new SAR_Model(SAR_Metadata_Right);

	Stereo_SAR_Model sModel (Model_Left,Model_Right);

	// RADARSAT IMAGE COORDINATE INVERSION //
	
	double Lx=0.0,Ly=0.0,Rx=0.0,Ry=0.0;

	if (SAR_Metadata_Left.SatelliteName == "RADARSAT-2")
	{
		if (SAR_Metadata_Left.Orbit == "Ascending" && SAR_Metadata_Left.SideLooking =="Right") 
		{
			Lx = leftX->value();
			Ly = (SAR_Metadata_Left.Height-1)-leftY->value();
		}
		if (SAR_Metadata_Left.Orbit == "Descending" && SAR_Metadata_Left.SideLooking =="Right")
		{
			Lx = (SAR_Metadata_Left.Width-1)-leftX->value();
			Ly = leftY->value();
		}			
	}
	else
	{
		Lx = leftX->value();
		Ly = leftY->value();
	}

	if (SAR_Metadata_Right.SatelliteName == "RADARSAT-2")
	{
		if (SAR_Metadata_Right.Orbit == "Ascending" && SAR_Metadata_Right.SideLooking =="Right") 
		{
			Rx = rightX->value();
			Ry = (SAR_Metadata_Right.Height-1)-rightY->value();
		}
		if (SAR_Metadata_Right.Orbit == "Descending" && SAR_Metadata_Right.SideLooking =="Right")
		{
			Rx = (SAR_Metadata_Right.Width-1)-rightX->value();
			Ry = rightY->value();
		}			
	}
	else
	{
		Rx = rightX->value();
		Ry = rightY->value();

	}
	
	// RETRIEVE 3D COORDINATE //
	COORD_Ecef Result = sModel.Stereo_SAR_SlantToGround(Lx,Ly,Rx,Ry);

	Lat->setValue(Result.Latitude);
	Lon->setValue(Result.Longitude);
	Height->setValue(Result.Height);
	
	std::string msg = "Coord X Ecef : " + StringUtilities::toDisplayString(Result.X_Ecef) + "\n"
					  "Coord Y Ecef : " + StringUtilities::toDisplayString(Result.Y_Ecef) + "\n"
					  "Coord Z Ecef : " + StringUtilities::toDisplayString(Result.Z_Ecef) + "\n"
					  "Coord Lon : " + StringUtilities::toDisplayString(Result.Longitude) + "\n"
					  "Coord Lat : " + StringUtilities::toDisplayString(Result.Latitude) + "\n"
					  "Coord Height : " + StringUtilities::toDisplayString(Result.Height) + "\n";	
				  						                      
	pProgress->updateProgress(msg, 100, NORMAL);
	
}

