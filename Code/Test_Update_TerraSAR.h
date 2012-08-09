/*
* The information in this file is
* Copyright(c) 2012, Andrea Nascetti <andreanascetti@gmail.com>
* and is subject to the terms and conditions of the
* GNU Lesser General Public License Version 2.1
* The license text is available from
* http://www.gnu.org/licenses/lgpl.html
*/

#ifndef Test_Update_TerraSAR_H
#define Test_Update_TerraSAR_H

#include "ExecutableShell.h"

class Test_Update_TerraSAR : public ExecutableShell
{

public:
   Test_Update_TerraSAR(void);
   virtual ~Test_Update_TerraSAR(void);

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

};
#endif
