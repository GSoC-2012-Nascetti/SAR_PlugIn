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

#include "Ortho_GUI.h"
#include "ViewerShell.h"

class Ortho_TerraSAR : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   Ortho_TerraSAR();
   ~Ortho_TerraSAR();

   bool execute(PlugInArgList*, PlugInArgList*);
   QWidget* getWidget() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

public slots:
   void dialogClosed();

private:
   Ortho_TerraSAR(const Ortho_TerraSAR& rhs);
   Ortho_TerraSAR& operator=(const Ortho_TerraSAR& rhs);
   
   bool showGui();
   Ortho_GUI* mpGui;
};

#endif
