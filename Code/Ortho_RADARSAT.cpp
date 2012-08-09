/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ModelServices.h"

#include "Ortho_RADARSAT.h"
#include "Ortho_GUI.h"

#include "PlugInRegistration.h"
#include "Service.h"
#include "SessionItemSerializer.h"
#include "Window.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

REGISTER_PLUGIN_BASIC(OpticksSAR, Ortho_RADARSAT);

Ortho_RADARSAT::Ortho_RADARSAT() :
   mpGui(NULL)
{
   setCreator("Andrea Nascetti");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   ViewerShell::setName("Orthorectification RADARSAT-2");
   setDescription("This plugin ia able to orthorectified RADARSAT-2 imagery");
   setMenuLocation("[SAR PlugIn]\\Orthorectification\\RADARSAT-2");
   setDescriptorId("{689B3124-DB38-11E1-AF0C-91426288709B}");
   destroyAfterExecute(false);
   setWizardSupported(true);
   setAbortSupported(true);
}

Ortho_RADARSAT::~Ortho_RADARSAT()
{
}

bool Ortho_RADARSAT::showGui()
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

   mpGui = new Ortho_GUI( pDesktop->getMainWidget(), "test", false,1 );
   connect( mpGui, SIGNAL( finished( int ) ), this, SLOT( dialogClosed() ) );
   
   mpGui->show();

   pStep->finalize(Message::Success);
   return true;

}

bool Ortho_RADARSAT::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{  	
	return showGui();
}

QWidget* Ortho_RADARSAT::getWidget() const
{
   return mpGui;
}

void Ortho_RADARSAT::dialogClosed()
{
   abort();
}

bool Ortho_RADARSAT::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session load
}

bool Ortho_RADARSAT::deserialize(SessionItemDeserializer &deserializer)
{
   return showGui();
}
