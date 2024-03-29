//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// DomainDevice.h
//
// Basic class for the physical device in a specific domain that need to be finally controlled 
//===============================================================
#ifndef DOMAINDEVICE_H
#define DOMAINDEVICE_H

#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
                     
#include "ooEpicsMisc.h"
#include "ModuleManager.h"

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

extern ModuleManager gobj_moduleManager;         // global object for the device manager

//-----------------------------------------------
// class definition
//-----------------------------------------------
class DomainDevice
{
public:
    DomainDevice(const char *deviceName);
    virtual ~DomainDevice();

public:
    char name[OOEPICS_NAMESTR_LEN];              // name of the device
};

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

