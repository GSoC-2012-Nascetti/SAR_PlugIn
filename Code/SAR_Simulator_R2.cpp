
#include "SAR_Simulator_R2.h"
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "Simulation_GUI.h"
#include "PlugInRegistration.h"
#include "Service.h"
#include "SessionItemSerializer.h"
#include "Window.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

REGISTER_PLUGIN_BASIC(OpticksSAR, SAR_Simulator_R2);

SAR_Simulator_R2::SAR_Simulator_R2() :
   mpGui(NULL)
{
   setCreator("Andrea Nascetti");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   ViewerShell::setName("PRova");
   setDescription("This plugin ia able to ");
   setMenuLocation("[SAR PlugIn]\\SAR_Simulator\\Simulator_R2");
   setDescriptorId("{0D9615F0-AB75-11E3-A5E2-0800200C9A66}");
   destroyAfterExecute(false);
   setWizardSupported(true);
   setAbortSupported(true);
}

SAR_Simulator_R2::~SAR_Simulator_R2()
{
}

bool SAR_Simulator_R2::showGui()
{
   Service<DesktopServices> pDesktop;
   Service<ModelServices> pModel;
   StepResource pStep( "Orthorectification RADARSAT-2", "app", "6FD4CAA4-DB38-11E1-9512-9A426288709B" );

   std::vector<DataElement*> cubes = pModel->getElements( "RasterElement" );
   if ( cubes.size() == 0 )
   {
      QMessageBox::critical( NULL, "Pixel Aspect Ratio Test", "No RasterElement input found!", "OK" );
      pStep->finalize( Message::Failure, "No RasterElement input found!" );
      return false;
   }

   mpGui = new Simulation_GUI( pDesktop->getMainWidget(), "test", false,1 );
   connect( mpGui, SIGNAL( finished( int ) ), this, SLOT( dialogClosed() ) );
   
   mpGui->show();

   pStep->finalize(Message::Success);
   return true;

}

bool SAR_Simulator_R2::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{  	
	return showGui();
}

QWidget* SAR_Simulator_R2::getWidget() const
{
   return mpGui;
}

void SAR_Simulator_R2::dialogClosed()
{
   abort();
}

bool SAR_Simulator_R2::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session load
}

bool SAR_Simulator_R2::deserialize(SessionItemDeserializer &deserializer)
{
   return showGui();
}

