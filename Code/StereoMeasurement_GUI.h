/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef StereoMeasurement_GUI_H
#define StereoMeasurement_GUI_H

#include "DockWindow.h"
#include "Layer.h"
#include "PlotWidget.h"

#include <QtGui/QDialog>
#include <QtGui/QGroupBox>

class QComboBox;
class QDoubleSpinBox;
class QLabel;

class StereoMeasurement_GUI : public QDialog
{
    Q_OBJECT

public:
    StereoMeasurement_GUI( QWidget* pParent = 0, const char* pName = 0, bool modal = FALSE );
    ~StereoMeasurement_GUI();

	std::string prova,name_left,name_right;

protected:
	void mousePressEvent(QMouseEvent *event);
	bool eventFilter(QObject* pObject, QEvent* pEvent);

public slots:

	void GetMapLocation();
	void GetMapFileLocation();
	void GetPixelLocation(bool);

private:
   StereoMeasurement_GUI(const StereoMeasurement_GUI& rhs);
   StereoMeasurement_GUI& operator=(const StereoMeasurement_GUI& rhs);

   QPushButton* mpCancelButton;
   QPushButton* mpApplyButton;
   QPushButton* mpGenerateViewButton;
   QPushButton* mpGetMapLocation;
   QPushButton* mpGetMapFileLocation;

   QComboBox* mpCubeListCombo;
   QComboBox* mpCubeListCombo_slave;

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
