//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// ModuleConfig.cc
//
// Realization of the basic class for device config
//===============================================================
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




