/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ModelServices.h"
#include "Ortho_GUI.h"
#include "PlugInRegistration.h"
#include "Service.h"
#include "SessionItemSerializer.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "StereoMeasurement.h"
#include "StereoMeasurement_GUI.h"
#include "Window.h"

#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

REGISTER_PLUGIN_BASIC(OpticksSAR, StereoMeasurement);

StereoMeasurement::StereoMeasurement() :
   mpGui(NULL)
{
   setCreator("Andrea Nascetti");
   setVersion("1.0");
   setCopyright("Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>");
   setProductionStatus(false);
   ViewerShell::setName("3D Stereo Measurement");
   setDescription("3D Stereo Measurement Tool for TerraSAR-X and RADARSAT-2 sensors");
   setMenuLocation("[SAR PlugIn]\\3D Stereo Measurement");
   setDescriptorId("{701D7CC4-BA34-11E1-B257-E0C36188709B}");
   destroyAfterExecute(false);
   //setWizardSupported(false);
}

StereoMeasurement::~StereoMeasurement()
{
}

bool StereoMeasurement::showGui()
{
   Service<DesktopServices> pDesktop;
   Service<ModelServices> pModel;
   StepResource pStep( "3D Stereo Measurement GUI", "app", "74E76008-BA34-11E1-9C1A-E4C36188709B" );

   std::vector<DataElement*> cubes = pModel->getElements( "RasterElement" );
   if ( cubes.size() == 0 )
   {
      QMessageBox::critical( NULL, "3D Stereo Measurement", "No RasterElement input found!", "OK" );
      pStep->finalize( Message::Failure, "No RasterElement input found!" );
      return false;
   }

   mpGui = new StereoMeasurement_GUI( pDesktop->getMainWidget(), "test", false );
   connect( mpGui, SIGNAL( finished( int ) ), this, SLOT( dialogClosed() ) );
   
   mpGui->show();

   pStep->finalize(Message::Success);
   return true;
}

bool StereoMeasurement::execute( PlugInArgList* inputArgList, PlugInArgList* outputArgList )
{  	
	return showGui();
}

QWidget* StereoMeasurement::getWidget() const
{
   return mpGui;
}

void StereoMeasurement::dialogClosed()
{
   abort();
}

bool StereoMeasurement::serialize(SessionItemSerializer &serializer) const
{
   return serializer.serialize(NULL, 0); // force recreation on session load
}

bool StereoMeasurement::deserialize(SessionItemDeserializer &deserializer)
{
   return showGui();
}
