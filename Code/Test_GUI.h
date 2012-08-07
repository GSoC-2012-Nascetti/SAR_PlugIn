/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef Test_GUI_H
#define Test_GUI_H

#include "ViewerShell.h"
#include "SAR_GUI.h"

class Test_GUI : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   Test_GUI();
   ~Test_GUI();

   bool execute(PlugInArgList*, PlugInArgList*);
   QWidget* getWidget() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

public slots:
   void dialogClosed();

private:
   Test_GUI(const Test_GUI& rhs);
   Test_GUI& operator=(const Test_GUI& rhs);
   bool showGui();
   SAR_GUI* mpGui;
};

#endif
