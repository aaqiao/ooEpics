//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// ControlDevice.cc
//
// Realization of the basic class for control device 
//===============================================================
#include "ControlDevice.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction
//-----------------------------------------------
ControlDevice::ControlDevice(const char *deviceName) 
{
    if(!deviceName || !deviceName[0]) {
        cout << "ERROR: ControlDevice::ControlDevice: Illegal module name!\n";
    } else {
        strcpy(name,  deviceName);
        cout << "INFO: ControlDevice::ControlDevice: Module " << name << " created." << endl;
    }
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
ControlDevice::~ControlDevice() 
{
    cout << "INFO: ControlDevice::~ControlDevice: Module " << name << " deleted!" << endl;
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************





