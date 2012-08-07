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
#include "Ortho_GUI.h"

class Ortho_RADARSAT : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   Ortho_RADARSAT();
   ~Ortho_RADARSAT();

   bool execute(PlugInArgList*, PlugInArgList*);
   QWidget* getWidget() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

public slots:
   void dialogClosed();

private:
   Ortho_RADARSAT(const Ortho_RADARSAT& rhs);
   Ortho_RADARSAT& operator=(const Ortho_RADARSAT& rhs);
   
   bool showGui();
   Ortho_GUI* mpGui;
};

#endif
