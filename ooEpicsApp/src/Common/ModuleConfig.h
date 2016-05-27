//=========================================================
// ModuleConfig.h
//
// Basic class for device configuration. Each device type should have one device configuration class.
// It is used to create and setup the instances of the device with the device type
//
// Created by: Zheqiao Geng, gengzq@slac.stanford.edu
// Created on: Sept. 24, 2010
//
// Modified by: Zheqiao Geng, zheqiao.geng@psi.ch
// Modified on: 2013.11.12
// Description: Removed the deviceAssociation function. The device association means the functions in one device module 
//              uses the data and routines directly from another device module. The first device module will contain an
//              pointer to another device. The device association happens between DomainDevice <> ControlDevice, 
//              Application <> DomainDevice. The device association will be done in the deviceSet function
//              Association with the interfaces (transaction layer) should be also implemented in the deviceSet function
//
// Modified by Zheqiao Geng on 2014.04.02
// Description: Change the name from DeviceConfig to ModuleConfig
//
// Modified by Zheqiao Geng on 2014.05.19
// Changed the interface of setModule allowing more data passed by a command
//=========================================================
#ifndef MODULECONFIG_H
#define MODULECONFIG_H

#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ooEpicsMisc.h"

#define OOEPICS_MAX_MODULE_NUM      1024
#define OOEPICS_MAX_CMD_DATA_NUM    16

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
};

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

