
#include "SAR_Simulator_TSX.h"
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

#include <fstream>

REGISTER_PLUGIN_BASIC(OpticksSAR, SAR_Simulator_TSX);

SAR_Simulator_TSX::SAR_Simulator_TSX() :
   mpGui(NULL)
{
   setCreator("Andrea Nascetti");
   setVersion("1.0");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   ViewerShell::setName("Ortho TerraSAR");
   setDescription("This plugin ia able to orthorectified TerraSAR-X");
   setMenuLocation("[SAR PlugIn]\\SAR_Simulator\\Simulator_TSX");
   setDescriptorId("{e8b86f10-af81-11e3-a5e2-0800200c9a66}");
   destroyAfterExecute(false);
   setWizardSupported(true);\
   setAbortSupported(true);
}

SAR_Simulator_TSX::~SAR_Simulator_TSX()
{
}

bool SAR_Simulator_TSX::showGui()
{
   Service<DesktopServices> pDesktop;
   Service<ModelServices> pModel;
   StepResource pStep( "Orthorectification TerraSAR-X", "app", "C2B877AC-CF6C-11E1-802E-BEE56188709B" );

   std::vector<DataElement*> cubes = pModel->getElements( "RasterElement" );
   if ( cubes.size() == 0 )
   {
      QMessageBox::critical( NULL, "Pixel Aspect Ratio Test", "No RasterElement input found!", "OK" );
      pStep->finalize( Message::Failure, "No RasterElement input found!" );
      return false;
   }

   mpGui = new Simulation_GUI( pDesktop->getMainWidget(), "test", false );
   connect( mpGui, SIGNAL( finished( int ) ), this, SLOT( dialogClosed() ) );
   
   mpGui->show();

   pStep->finalize(Message::Success);
   return true;

}

bool SAR_Simulator_TSX::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{  	
	return showGui();
}

QWidget* SAR_Simulator_TSX::getWidget() const
{
   return mpGui;
}

void SAR_Simulator_TSX::dialogClosed()
{
   abort();
}

bool SAR_Simulator_TSX::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session load
}

bool SAR_Simulator_TSX::deserialize(SessionItemDeserializer &deserializer)
{
   return showGui();
}

