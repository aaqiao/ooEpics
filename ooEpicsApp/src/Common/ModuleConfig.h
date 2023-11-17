//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// ModuleConfig.h
//
// Basic class for device configuration. Each device type should have one device configuration class.
// It is used to create and setup the instances of the device with the device type
//===============================================================
#ifndef MODULECONFIG_H
#define MODULECONFIG_H

#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ooEpicsMisc.h"

#define OOEPICS_MAX_MODULE_NUM  	1024
#define OOEPICS_MAX_CMD_DATA_NUM 	16

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// forward declaration
//-----------------------------------------------
class ModuleInstance;

//-----------------------------------------------
// class definition
// this class should be reloaded for each device
//-----------------------------------------------
class ModuleConfig
{
public:
    ModuleConfig(const char *moduleTypeName);
    virtual ~ModuleConfig();

    virtual int moduleCreate(const char *moduleName, const char *moduleInfo, int priority)  = 0;                    
    virtual int moduleInit(ModuleInstance *module)                                          = 0;
    virtual int moduleSet(ModuleInstance *module, const char *cmd, char **data)             = 0;

private:
    char typeName[128];
};

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

