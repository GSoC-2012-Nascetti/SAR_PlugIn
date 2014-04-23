#ifndef SAR_SIMULATOR_R2_H
#define SAR_SIMULATOR_R2_H

#include "Simulation_GUI.h"
#include "ViewerShell.h"

class SAR_Simulator_R2 : public QObject, public ViewerShell
{
   Q_OBJECT

public:
   SAR_Simulator_R2();
   ~SAR_Simulator_R2();

   bool execute(PlugInArgList*, PlugInArgList*);
   QWidget* getWidget() const;
   bool serialize(SessionItemSerializer& serializer) const;
   bool deserialize(SessionItemDeserializer& deserializer);

public slots:
   void dialogClosed();

private:
   SAR_Simulator_R2(const SAR_Simulator_R2& rhs);
   SAR_Simulator_R2& operator=(const SAR_Simulator_R2& rhs);
   
   bool showGui();
   Simulation_GUI* mpGui;
};

#endif