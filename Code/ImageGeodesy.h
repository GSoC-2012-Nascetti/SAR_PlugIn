/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

//#ifndef TestSAR_H
#define ImageGeodesy_H

#include "ExecutableShell.h"
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>

class ImageGeodesy : public ExecutableShell, QDialog
{

public:
   ImageGeodesy(void);

   virtual ~ImageGeodesy(void);

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

};

