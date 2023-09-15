//=========================================================
// ControlDevice.h
//
// Basic class for the instrumentation device insterted by the control system for control purpose. 
//
// Created by: Zheqiao Geng, zheqiao.geng@psi.ch
// Created on: 2013.11.12
//=========================================================
#ifndef CONTROLDEVICE_H
#define CONTROLDEVICE_H

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
class ControlDevice
{
public:
    ControlDevice(const char *deviceName);
    virtual ~ControlDevice();

public:
    char name[OOEPICS_NAMESTR_LEN];              // name of the device
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

