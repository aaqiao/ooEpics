//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// ModuleManager.h
//
// Basic class for device manager
//===============================================================
#ifndef MODULEMANAGER_H
#define MODULEMANAGER_H

#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ellLib.h"
#include "epicsMutex.h"
#include "epicsExit.h"

#include "ooEpicsMisc.h"
#include "ModuleConfig.h"

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// forward declaration
//-----------------------------------------------
class ModuleConfig;
class ControlDevice;
class DomainDevice;
class Application;

//-----------------------------------------------
// class definition for module type
//-----------------------------------------------
class ModuleType
{
public:
    ELLNODE	       node;
    char           moduleTypeName[OOEPICS_NAMESTR_LEN];     // unique name of the module type
    ModuleConfig  *modConfig;                               // configuration API object for the module type
};

//-----------------------------------------------
// class definition for module instance
//-----------------------------------------------
class ModuleInstance
{
public:
    ELLNODE	       node;
    char           moduleTypeName[OOEPICS_NAMESTR_LEN];     // module type of this instance
    ModuleConfig  *modConfig;                               // configuration API object for the module type
    char           moduleName[OOEPICS_NAMESTR_LEN];         // module instance name
    int            moduleCategory;                          // 0 - control device; 1 - domain device; 2 - high level app
    ControlDevice *ctlDevice;                               // object of the control device instance
    DomainDevice  *domDevice;                               // object of the domain device instance
    Application   *app;                                     // object of the application instance
};

//-----------------------------------------------
// global routine for module type registeration
//-----------------------------------------------
void moduleTypeRegister(const char *moduleTypeName, ModuleConfig *modConfig);  // add new module type to the list

//-----------------------------------------------
// class definition for module manager
// note that the module manager only manage the instances of the modules
//-----------------------------------------------
class ModuleManager
{
public:
    ModuleManager();
   ~ModuleManager();

    // ---- register (for the module type register, see the C routine above) ----
    void moduleInstanceRegister(const char *moduleTypeName, ModuleConfig *modConfig, 
                                const char *moduleName, void *module, int modCategory);   // add new module instance to the list

    // ---- global interfaces (called from ioc startup script) ----
    void createModule(const char *moduleTypeName, const char *moduleName, const char *moduleInfo, int priority); 
    void initModule(const char *moduleName);
    void setModule(const char *moduleName, const char *cmd, char **data);

    // ---- search ----
    ModuleInstance *getModuleInstance(const char *moduleName);

    // ---- debug ----
    void printModuleType();
    void printModuleList();
 
    ELLLIST moduleInstanceList;                  // a list of the module instances

private:
    epicsMutex moduleInstanceListMutex;          // mutex for module instance list operation
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif




