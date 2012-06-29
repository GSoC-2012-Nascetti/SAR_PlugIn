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

   QGridLayout* pLayout = new QGridLayout( this );

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
   
   pLayout->addWidget( labelImageLeft, 0, 0 );
   pLayout->addWidget( labelImageRight, 0, 3 );

   pLayout->addWidget( mpCubeListCombo, 1, 0, 1, 2 );
   pLayout->addWidget( mpCubeListCombo_slave, 1, 3, 1, 2 );
   
   pLayout->addWidget( labelImagePoints, 2, 0 );
    
   pLayout->addWidget( labelLeftX, 3, 0 );
   pLayout->addWidget( labelLeftY, 4, 0 );
   pLayout->addWidget( leftX, 3, 1 );
   pLayout->addWidget( leftY, 4, 1 );

   pLayout->addWidget( labelRightX, 3, 3 );
   pLayout->addWidget( labelRightY, 4, 3 );
   pLayout->addWidget( rightX, 3, 4 );
   pLayout->addWidget( rightY, 4, 4 );
   
   pLayout->addWidget( labelGroundPoint, 5, 0 );
   
   pLayout->addWidget( labelLat, 6, 0 );
   pLayout->addWidget( labelLon, 7, 0 );
   pLayout->addWidget( labelHeight, 6, 3 );

   pLayout->addWidget( Lat,6, 1);
   pLayout->addWidget( Lon,7, 1);
   pLayout->addWidget( Height, 6, 4);

   pLayout->addWidget( mpCancelButton, 8, 4);
   pLayout->addWidget( mpGetMapLocation, 8, 0 );

   mpCancelButton->setText("Close");
   resize( QSize(600, 150).expandedTo(minimumSizeHint()) );

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
   Service<DesktopServices> pDesktop;

   StepResource pStep( "SAR GUI Closed.", "app", "95253952-BA2B-11E1-B31D-37BC6188709B" );
   if ( mbScalingApplied ) //successful state
   {
      pStep->finalize( Message::Success );
   }
   else //cancelled state
   {
      pStep->finalize( Message::Abort, "Plug-in Cancelled!" );
   }

   SpatialDataWindow* pScaledWindow = dynamic_cast<SpatialDataWindow*>( pDesktop->getWindow(
      "scaledCubeWindow", SPATIAL_DATA_WINDOW ) );
   if ( pScaledWindow != NULL )
   {
      pDesktop->deleteWindow( pScaledWindow );
      pScaledWindow = NULL;
   }
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
  
 
void SAR_GUI::generateNewView()
               
	{
	/*
   Service<DesktopServices> pDesktop;

   Service<ModelServices> pModel;

   ProgressResource pProgress("pProgress");

   pProgress->updateProgress("Pluto", 0, NORMAL);
    
   

   SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(pDesktop->getWindow(mCubeNames.at(
      mpCubeListCombo->currentIndex()), SPATIAL_DATA_WINDOW));
   
   prova = "pluto";

   if (prova.compare("pluto")==0 ){
	   prova = "pippo";
   }
   
   prova = mCubeNames.at(mpCubeListCombo->currentIndex());

  // DataElement *pElem = dynamic_cast<DataElement*>(pModel->getElement(mCubeNames.at(
    //  mpCubeListCombo->currentIndex()),"",NULL ));

      RasterElement *pCube = dynamic_cast<RasterElement*>(pModel->getElement(mCubeNames.at(
      mpCubeListCombo->currentIndex()),"",NULL ));
   
   
   //  ********************************************************************************** //

   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());

   FactoryResource<DataRequest> pRequest;
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());

   double min = std::numeric_limits<double>::max();
   double max = -min;
   
   double total = 0.0; 

   for (unsigned int row = 0; row < pDesc->getRowCount(); ++row)
   {
      for (unsigned int col = 0; col < pDesc->getColumnCount(); ++col)
      {		 
		 total+= Service<ModelServices>()->getDataValue(pDesc->getDataType(), pAcc->getColumn(), COMPLEX_MAGNITUDE, 0);

		 pAcc->nextColumn();
      }
      pAcc->nextRow();
	   pProgress->updateProgress("computing", int(100*row/pDesc->getRowCount()), NORMAL);
   }

   pProgress->updateProgress("end", 100, NORMAL);
   
   std::string path = pCube->getFilename();

   DataDescriptor* dMeta = pCube->getDataDescriptor();

   DynamicObject* Metadata = dMeta->getMetadata();

	// RETRIEVE & UPDATE METADATA INFORMATION //
		   
	TerraSAR_Metadata Prova_metadata;

	Prova_metadata.ReadFile(path);

    bool control = Prova_metadata.ReadFile(path);

	if (control == false)
	{
	std::string msg = "This is not a TerraSAR-X SLC Files, Metadata can't be updated";
	pProgress->updateProgress(msg, 100, ERRORS);
	
	}

	Prova_metadata.UpdateMetadata(Metadata); 


   //  ********************************************************************************** //

   if (pWindow != NULL)
   {
      SpatialDataView* pView = dynamic_cast<SpatialDataView*>(pWindow->getView());
      if (pView != NULL)
      {
         Layer* pLayer = pView->getTopMostLayer(RASTER);
         if (pLayer != NULL)
         {
            SpatialDataWindow* pScaledWindow =
               dynamic_cast<SpatialDataWindow*>(pDesktop->createWindow("scaledCubeWindow", SPATIAL_DATA_WINDOW));
            
			pScaledWindow->signalsEnabled();

			if (pScaledWindow != NULL)
            {
               SpatialDataView* pScaledView = dynamic_cast<SpatialDataView*>(pScaledWindow->getView());
               if (pScaledView != NULL)
               {
                  mpScaledLayer = pLayer->copy(std::string(), true, pLayer->getDataElement());
                  pScaledView->addLayer(mpScaledLayer);
                  pScaledView->refresh();
                  pScaledView->zoomExtents();
				  
				  pScaledView->signalMousePanEnabled();
				  pScaledView->panToCenter();

                  mpXScaleFactor->setEnabled(true);
                  mpYScaleFactor->setEnabled(true);
                  mpApplyButton->setEnabled(true);
               }
            }
         }
      }
   }*/
}

void SAR_GUI::GetMapLocation()
{


}


