//=========================================================
// Application.cc
//
// Realize the basic class for the Application 
//
// Created by Zheqiao Geng (zheqiao.geng@psi.ch) on 2014.04.10
//=========================================================
#include "Application.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction
//-----------------------------------------------
Application::Application(const char *appName) 
{
    if(!appName || !appName[0]) {
        cout << "ERROR: Application::Application: Illegal module name!\n";
    } else {
        strcpy(name, appName);
        cout << "INFO: Application::Application: Module " << name << " created." << endl;
    }
}

//-----------------------------------------------
// destruction
//-----------------------------------------------
Application::~Application()
{
    cout << "INFO: Application::~Application: Module " << name << " deleted!" << endl;
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************





