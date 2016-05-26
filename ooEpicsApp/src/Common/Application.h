//=========================================================
// Application.h
//
// Basic class for the Application
//
// Created by Zheqiao Geng (zheqiao.geng@psi.ch) on 2014.04.10
//=========================================================
#ifndef APPLICATION_H
#define APPLICATION_H

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
class Application
{
public:
    Application(const char *appName);
    virtual ~Application();

public:
    char name[OOEPICS_NAMESTR_LEN];              // name of the application
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

