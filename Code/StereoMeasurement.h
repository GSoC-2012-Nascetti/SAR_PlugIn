/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef StereoMeasurement_H
#define StereoMeasurement_H

#include "ViewerShell.h"
#include "StereoMeasurement_GUI.h"

class StereoMeasurement : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   StereoMeasurement();
   ~StereoMeasurement();

   bool execute(PlugInArgList*, PlugInArgList*);
   QWidget* getWidget() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

public slots:
   void dialogClosed();

private:
   StereoMeasurement(const StereoMeasurement& rhs);
   StereoMeasurement& operator=(const StereoMeasurement& rhs);
   bool showGui();
   StereoMeasurement_GUI* mpGui;
};

#endif
