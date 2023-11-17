//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// Service.cc
//
// Realize the basic class for the service 
//===============================================================
#include "Service.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction
//-----------------------------------------------
Service::Service(const char *moduleName, const char *serviceName) 
{
    if(!moduleName || !moduleName[0] || !serviceName || !serviceName[0])
        cout << "ERROR: Service::Service: Illegal service name!\n";
    else {
        strcpy(modName, moduleName);
        strcpy(srvName, serviceName);

        cout << "INFO: Service::Service: Object " << srvName << " for module " << modName << " created." << endl;
    }
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
Service::~Service() 
{
    cout << "INFO: Service::~Service: Object " << srvName << " for module " << modName << " deleted!" << endl;
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************





