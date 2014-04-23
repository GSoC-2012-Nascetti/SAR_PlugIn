/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef SIMULATION_GUI_H
#define SIMULATION_GUI_H

#include "DockWindow.h"
#include "Layer.h"
#include "SAR_Simulator_Processor.h"
#include "PlotWidget.h"
#include "RasterElement.h"
#include "SAR_Model.h"
#include "TerraSAR_Metadata.h"

#include <QtGui/QAction>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QRadioButton>

class QComboBox;
class QDoubleSpinBox;
class QLabel;

class Simulation_GUI : public QDialog
{
    Q_OBJECT

public:
    Simulation_GUI( QWidget* pParent = 0, const char* pName = 0, bool modal = FALSE,int sensor_type=0);
    ~Simulation_GUI();

public slots:

   void StartOrtho();
   void CheckImage();
   void CheckModel();
   void ComputeGrid();
   bool RetrieveDSMGrid();

private:
	
  // TerraSAR_Metadata Metadata;
   SAR_Metadata *Metadata;

   SAR_Model *Model;
   
   RasterElement *pCube;
   RasterElement *pDSM;
   GRID OrthoGrid;
   GRID DSMGrid;

   Simulation_GUI(const Simulation_GUI& rhs);
   Simulation_GUI& operator=(const Simulation_GUI& rhs);

   void CheckButton(QAbstractButton* pButton);

   std::string image_path;
   std::string image_name;
   std::string DSM_path;
   std::string DSM_name;
   std::string sensor_name;

   QPushButton* mpCancelButton;
   QPushButton* mpApplyButton;
   QPushButton* mpStartOrtho;
   QPushButton* mpCheckModel;
   QPushButton* mpCheckImage;

   QComboBox* mpImageListCombo;
   QComboBox* mpDSMListCombo;
   QComboBox* mpInterpolationList;
   QComboBox* mpDSMInterpolationList;

   QDoubleSpinBox* Height;
   QDoubleSpinBox* GeoidOffSet;
   QDoubleSpinBox* X_Spacing;
   QDoubleSpinBox* Y_Spacing;

   QLabel* Datum;
   QLabel* Resampling;
   QLabel* DSMResampling;

   QRadioButton* mpFlatten;
   QRadioButton* mpDsm;

   QButtonGroup* pOrthoModeGroup;
   QGroupBox* box2;
   QGroupBox* box3;
   QGroupBox* box4;

   void init();
   std::vector<std::string> mCubeNames;

};

#endif
