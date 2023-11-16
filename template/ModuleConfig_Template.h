//=========================================================
// ModuleConfig___MODULE_NAME__.h
// 
// Class definition for the __MODULE_NAME__ module configuration
// Auto created from the template of ooEpics framework
//=========================================================
#ifndef MODULECONFIG___MODULE_NAME___H
#define MODULECONFIG___MODULE_NAME___H

#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <iostream>
#include <fstream>

#include "ModuleConfig.h"

using namespace std;

//-----------------------------------------------
// class forward declaration
//-----------------------------------------------
class Module___MODULE_NAME__;

//-----------------------------------------------
// class definition of the module configuration of ModuleConfig___MODULE_NAME__
//-----------------------------------------------
class ModuleConfig___MODULE_NAME__ : public OOEPICS::ModuleConfig
{
public:
    ModuleConfig___MODULE_NAME__();
    virtual ~ModuleConfig___MODULE_NAME__();

    virtual int moduleCreate(const char *moduleName, const char *moduleInfo, int priority);
    virtual int moduleInit(OOEPICS::ModuleInstance *module);
    virtual int moduleSet(OOEPICS::ModuleInstance *module, const char *cmd, char **data);
};

#endif
