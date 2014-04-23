#ifndef Test_GUI_H
#define Test_GUI_H

#include "Simulation_GUI.h"
#include "ViewerShell.h"

class SAR_Simulator_TSX : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   SAR_Simulator_TSX();
   ~SAR_Simulator_TSX();

   bool execute(PlugInArgList*, PlugInArgList*);
   QWidget* getWidget() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

public slots:
   void dialogClosed();

private:
   SAR_Simulator_TSX(const SAR_Simulator_TSX& rhs);
   SAR_Simulator_TSX& operator=(const SAR_Simulator_TSX& rhs);
   
   bool showGui();
   Simulation_GUI* mpGui;
};

#endif