//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// Service.h
//
// Basic class for the Service. The Service normally collect the data and processing algorithms for a device
// in the applications. So it defines more the remote PVs 
//===============================================================
#ifndef SERVICE_H
#define SERVICE_H

#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
                     
#include "ooEpicsMisc.h"

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// class definition
//-----------------------------------------------
class Service
{
public:
    Service(const char *moduleName, const char *serviceName);
   ~Service();

    char modName[OOEPICS_NAMESTR_LEN];              // name of the module
    char srvName[OOEPICS_NAMESTR_LEN];              // name of the service
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

