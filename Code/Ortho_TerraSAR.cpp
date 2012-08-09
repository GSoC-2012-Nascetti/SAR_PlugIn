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

#include "Ortho_TerraSAR.h"
#include "Ortho_GUI.h"

#include "PlugInRegistration.h"
#include "Service.h"
#include "SessionItemSerializer.h"
#include "Window.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

REGISTER_PLUGIN_BASIC(OpticksSAR, Ortho_TerraSAR);

Ortho_TerraSAR::Ortho_TerraSAR() :
   mpGui(NULL)
{
   setCreator("Andrea Nascetti");
   setVersion("1.0");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   ViewerShell::setName("Orthorectification TerraSAR");
   setDescription("This plugin ia able to orthorectified TerraSAR-X imagery");
   setMenuLocation("[SAR PlugIn]\\Orthorectification\\TerraSAR-X");
   setDescriptorId("{976C6FC2-CF6C-11E1-B13F-A1E56188709B}");
   destroyAfterExecute(false);
   setWizardSupported(true);
   setAbortSupported(true);
}

Ortho_TerraSAR::~Ortho_TerraSAR()
{
}

bool Ortho_TerraSAR::showGui()
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

   mpGui = new Ortho_GUI( pDesktop->getMainWidget(), "test", false );
   connect( mpGui, SIGNAL( finished( int ) ), this, SLOT( dialogClosed() ) );
   
   mpGui->show();

   pStep->finalize(Message::Success);
   return true;
}

bool Ortho_TerraSAR::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{  	
	return showGui();
}

QWidget* Ortho_TerraSAR::getWidget() const
{
   return mpGui;
}

void Ortho_TerraSAR::dialogClosed()
{
   abort();
}

bool Ortho_TerraSAR::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session load
}

bool Ortho_TerraSAR::deserialize(SessionItemDeserializer &deserializer)
{
   return showGui();
}
