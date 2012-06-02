/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

//#ifndef TestSAR_H
#define TestSAR_H

#include "executableshell.h"

class TestSAR : public ExecutableShell
{

public:
   TestSAR(void);
   virtual ~TestSAR(void);

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

};

