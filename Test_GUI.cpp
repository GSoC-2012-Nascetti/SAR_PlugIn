/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include <QtGui\QApplication>
#include <QtGui\QMessageBox>


#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "Test_GUI.h"
#include "SAR_GUI.h"
#include "Ortho_GUI.h"
#include "PlugInRegistration.h"
#include "Service.h"
#include "SessionItemSerializer.h"

#include "Window.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"

REGISTER_PLUGIN_BASIC(OpticksSAR, Test_GUI);

Test_GUI::Test_GUI() :
   mpGui(NULL)
{
   setCreator("Opticks Community");
   setVersion("Sample");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   ViewerShell::setName("Test SAR GUI");
   setDescription("Verifies MPR1 Requirement 180.");
   setMenuLocation("[SAR PlugIn]\\3D Stereo Measurement");
   setDescriptorId("{701D7CC4-BA34-11E1-B257-E0C36188709B}");
   destroyAfterExecute(false);
   setWizardSupported(false);
}

Test_GUI::~Test_GUI()
{
}

bool Test_GUI::showGui()
{
   Service<DesktopServices> pDesktop;
   Service<ModelServices> pModel;
   StepResource pStep( "Test GUI", "app", "74E76008-BA34-11E1-9C1A-E4C36188709B" );

   std::vector<DataElement*> cubes = pModel->getElements( "RasterElement" );
   if ( cubes.size() == 0 )
   {
      QMessageBox::critical( NULL, "Pixel Aspect Ratio Test", "No RasterElement input found!", "OK" );
      pStep->finalize( Message::Failure, "No RasterElement input found!" );
      return false;
   }

   mpGui = new SAR_GUI( pDesktop->getMainWidget(), "test", false );
   connect( mpGui, SIGNAL( finished( int ) ), this, SLOT( dialogClosed() ) );
   
   mpGui->show();

   pStep->finalize(Message::Success);
   return true;
}

bool Test_GUI::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{  	
	return showGui();
}

QWidget* Test_GUI::getWidget() const
{
   return mpGui;
}

void Test_GUI::dialogClosed()
{
   abort();
}

bool Test_GUI::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session load
}

bool Test_GUI::deserialize(SessionItemDeserializer &deserializer)
{
   return showGui();
}
