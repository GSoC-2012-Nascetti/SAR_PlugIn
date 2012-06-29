/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef SAR_GUI_H
#define SAR_GUI_H

#include <QtGui\QDialog>
#include "PlotWidget.h"
#include "DockWindow.h"
#include "Layer.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;

class SAR_GUI : public QDialog
{
    Q_OBJECT

public:
    SAR_GUI( QWidget* pParent = 0, const char* pName = 0, bool modal = FALSE );
    ~SAR_GUI();

	std::string prova;

public slots:

    void generateNewView();
	void GetMapLocation();

private:
   SAR_GUI(const SAR_GUI& rhs);
   SAR_GUI& operator=(const SAR_GUI& rhs);

   QPushButton* mpCancelButton;
   QPushButton* mpApplyButton;
   QPushButton* mpGenerateViewButton;
   QPushButton* mpGetMapLocation;

   QComboBox* mpCubeListCombo;
   QComboBox* mpCubeListCombo_slave;

   QDoubleSpinBox* mpXScaleFactor;
   QDoubleSpinBox* mpYScaleFactor;

   QDoubleSpinBox* leftX;
   QDoubleSpinBox* rightX;
   QDoubleSpinBox* leftY;
   QDoubleSpinBox* rightY;

   QDoubleSpinBox* Lat;
   QDoubleSpinBox* Lon;
   QDoubleSpinBox* Height;

   QLabel* labelLeftX;
   QLabel* labelLeftY;
   QLabel* labelRightX;
   QLabel* labelRightY;

   QLabel* labelImageLeft;
   QLabel* labelImageRight;
   QLabel* labelImagePoints;
   QLabel* labelGroundPoint;
   QLabel* labelLat;
   QLabel* labelLon;
   QLabel* labelHeight;



   void init();
   std::vector<std::string> mCubeNames;

   Layer* mpScaledLayer;
   bool mbScalingApplied;

};

#endif
