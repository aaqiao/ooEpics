//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// DomainDevice.cc
//
// Realization of the basic class for domain device 
//===============================================================
#include "DomainDevice.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// constructionDeviceManager.h
//-----------------------------------------------
DomainDevice::DomainDevice(const char *deviceName) 
{
    if(!deviceName || !deviceName[0]) {
        cout << "ERROR: DomainDevice::DomainDevice: Illegal module name!\n";
    } else {
        strcpy(name,  deviceName);
        cout << "INFO: DomainDevice::DomainDevice: Module " << name << " created." << endl;
    }
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
DomainDevice::~DomainDevice() 
{
    cout << "INFO: DomainDevice::~DomainDevice: Module " << name << " deleted!" << endl;
}

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************





