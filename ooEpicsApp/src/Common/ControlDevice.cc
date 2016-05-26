//=========================================================
// ControlDevice.cc
//
// Realization of the basic class for control device 
//
// Created by: Zheqiao Geng, zheqiao.geng@psi.ch
// Created on: 2013.11.12
//=========================================================
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
    if(!deviceName || !deviceName[0])
        cout << "ERROR:ControlDevice::ControlDevice: Illegal device name!\n";
    else
        strcpy(name,  deviceName);
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
ControlDevice::~ControlDevice() {}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************





