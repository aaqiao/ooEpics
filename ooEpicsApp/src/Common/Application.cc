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
    if(!appName || !appName[0])
        cout << "ERROR:Application::Application: Illegal application name!\n";
    else
        strcpy(name, appName);
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
Application::~Application() {}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************





