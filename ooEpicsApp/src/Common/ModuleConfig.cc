//=========================================================
// ModuleConfig.cc
//
// Realization of the basic class for device config
//
// Created by: Zheqiao Geng, gengzq@slac.stanford.edu
// Created on: Sept. 24, 2010
//
// Modified by Zheqiao Geng on 2014.04.02
// Description: Change the name from DeviceConfig to ModuleConfig
//=========================================================
#include "ModuleConfig.h"
#include "ModuleManager.h"                    

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction
//-----------------------------------------------
ModuleConfig::ModuleConfig(const char *moduleTypeName)
{
    // ---- check the input parameters ----  
    if(!moduleTypeName || !moduleTypeName[0]) {
        cout << "ERROR: ModuleConfig::ModuleConfig: Illegal module type name!\n";
        return;
    }

    // ---- register the device type ----  
    moduleTypeRegister(moduleTypeName, this);

    strcpy(typeName, moduleTypeName);

    cout << "INFO: ModuleConfig::ModuleConfig: Object for module type " << typeName << " created." << endl;
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
ModuleConfig::~ModuleConfig()
{
    cout << "INFO: ModuleConfig::~ModuleConfig: Object for module type " << typeName << " deleted!" << endl;
}

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************




