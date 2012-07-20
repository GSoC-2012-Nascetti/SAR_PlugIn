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
#include "Layer.h"
#include "LayerList.h"
#include "ModelServices.h"
#include "Ortho_GUI.h"
#include "PlugInResource.h"
#include "Progress.h"
#include "ProgressResource.h"
#include "RasterDataDescriptor.h"
#include "StringUtilities.h"
#include "UtilityServices.h"

#include <QtCore/QFileInfo>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtCore/QStringList>
#include <Qt/qevent.h>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QCheckBox>
#include <QtCore/QObject>
#include <QtGui/QAction>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialogButtonBox>

#include "boost\accumulators\accumulators.hpp"
#include "boost\accumulators\statistics\stats.hpp"
#include "boost\accumulators\statistics\mean.hpp"
#include "boost\accumulators\statistics\variance.hpp"

using namespace boost::accumulators;

Ortho_GUI::Ortho_GUI( QWidget* pParent, const char* pName, bool modal )
: QDialog(pParent)
{
	if (pName == NULL)
   {
      setObjectName( "TerraSAR-X Orthorectification" );
   }
   setModal( FALSE );
   setWindowTitle("Orthorectification TerraSAR-X imagery");

   // GUI widget object initialitation

   mpImageListCombo = new QComboBox( this );
   mpDSMListCombo = new QComboBox( this );
   mpInterpolationList = new QComboBox( this );

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

   // GUI Layout Design
   
   QGridLayout* MainLayout = new QGridLayout(this);
   
   //Input Image group box    
   QGridLayout* pLayout1 = new QGridLayout();

   pLayout1->addWidget(mpImageListCombo,0,0,1,3);
   pLayout1->addWidget(Resampling,1,0);
   pLayout1->addWidget(mpInterpolationList,1,1,1,2);
   pLayout1->addWidget(mpCheckModel,2,1);
   pLayout1->addWidget(mpCheckImage,2,0);

   QGroupBox* box1 = new QGroupBox(tr("Select Input Image"));
   box1->setLayout(pLayout1);
   MainLayout->addWidget( box1, 0, 0,3,3);
   
   // Datum & Projection Group Box
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
   QGridLayout* pLayout4 = new QGridLayout();
   pLayout4->addWidget(Ldsm,0,0);
   pLayout4->addWidget(mpDSMListCombo,0,1,1,2);
   pLayout4->addWidget(Lgeoid,1,1);
   pLayout4->addWidget(GeoidOffSet,1,2);

   box4 = new QGroupBox();
   box4->setLayout(pLayout4);
   MainLayout->addWidget( box4, 6, 0,2,3);

   // Button 

   MainLayout->addWidget(mpFlatten,3,0);
   MainLayout->addWidget(mpDsm,5,0);
   MainLayout->addWidget( mpCancelButton, 7, 5);
   MainLayout->addWidget( mpStartOrtho, 7, 4 );   

   mpCancelButton->setText("Close");
   
   // signals and slots connections
   VERIFYNRV(connect( mpCancelButton, SIGNAL( clicked() ), this, SLOT( reject() )));
   VERIFYNRV(connect( mpCheckImage, SIGNAL( clicked() ), this, SLOT( CheckImage() )));
   VERIFYNRV(connect( mpCheckModel, SIGNAL( clicked() ), this, SLOT( CheckModel() )));
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

   for (unsigned int i = 0; i < mCubeNames.size(); i++)
   {
      mpImageListCombo->insertItem(i, QString::fromStdString(mCubeNames[i]));
	  mpDSMListCombo->insertItem(i, QString::fromStdString(mCubeNames[i]));
   }

   mpInterpolationList->insertItem(0,QString::fromStdString("Nearest Neighbor Interpolation"));
   mpInterpolationList->insertItem(1,QString::fromStdString("Bilinear Interpolation"));

   X_Spacing->setMinimum(0);
   X_Spacing->setDecimals(3);
   X_Spacing->setMaximum(5000);

   Y_Spacing->setMinimum(0);
   Y_Spacing->setDecimals(3);
   Y_Spacing->setMaximum(5000);

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
   Service<ModelServices> pModel;	

   image_name = mCubeNames.at(mpImageListCombo->currentIndex());

   pCube =  dynamic_cast<RasterElement*>(pModel->getElement(image_name,"",NULL ));

   image_path =  pCube->getFilename();

   bool control = Metadata.ReadFile(image_path);

   if (control == false)
   { 
	   QMessageBox::information(this, QString::fromStdString("Error Information"), 
		                        QString::fromStdString("This is not a TerraSAR-X imagery") );
	   return;	
   }

   mpStartOrtho->setEnabled(true);
   mpFlatten->setEnabled(true);
   mpDsm->setEnabled(true);
   mpCheckModel->setEnabled(true);
   box2->setEnabled(true);
   box3->setEnabled(true);

}

void Ortho_GUI::CheckModel()
{

   //Service<ModelServices> pModel;


   ProgressResource pProgress("ProgressBar");

   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());

   FactoryResource<DataRequest> pRequest;
   DataAccessor pAcc = pCube->getDataAccessor(pRequest.release());

   DataDescriptor* dMeta = pCube->getDataDescriptor();

   DynamicObject* oMetadata = dMeta->getMetadata();

	// RETRIEVE & UPDATE METADATA INFORMATION //

	bool control = Metadata.ReadFile(image_path);

	Metadata.UpdateMetadata(oMetadata); 
  
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
     
	Punti = Metadata.UpdateGCP(Punti, image_path);

	SAR_Model ModProva(Metadata,100);

	COORD Punto;
	int N=Punti.size();
	int n=0;
	double Lat, Lon;

	accumulator_set<double, stats<tag::mean, tag::variance> > accX, accY;

	list<GcpPoint>::iterator pList;
	for (pList = Punti.begin(); pList != Punti.end(); pList++)
	{
		if(pList->mPixel.mX<Metadata.Width && pList->mPixel.mY<Metadata.Height)	
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