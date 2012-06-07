/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef Test_Update_RADARSAT2_H
#define Test_Update_RADARSAT2_H

#include "executableshell.h"

class Test_Update_RADARSAT2 : public ExecutableShell
{

public:
   Test_Update_RADARSAT2(void);
   virtual ~Test_Update_RADARSAT2(void);

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

};
#endif
