//=========================================================
// DomainDevice.cc
//
// Realization of the basic class for domain device 
//
// Created by: Zheqiao Geng, zheqiao.geng@psi.ch
// Created on: 2013.11.12
//=========================================================
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
    if(!deviceName || !deviceName[0])
        cout << "ERROR:DomainDevice::DomainDevice: Illegal device name!\n";
    else
        strcpy(name,  deviceName);
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
DomainDevice::~DomainDevice() {}

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************





