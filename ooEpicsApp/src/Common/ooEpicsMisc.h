//=========================================================
// ooEpicsMisc.h
//
// All kinds of constant or macro definitions
//
// Created by: Zheqiao Geng, gengzq@slac.stanford.edu
// Created on: Aug. 31, 2010
//
// Modified by: Zheqiao Geng
// Modified on: Sept. 25, 2010
// Description: remove the definition of the macro for epicsDataControlBlock creation
//=========================================================
#ifndef OOEPICSMISC_H
#define OOEPICSMISC_H

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

#include "EPICSLib_wrapper.h"

#define OOEPICS_DEBUG 0

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// constant definitions
//-----------------------------------------------
#define OOEPICS_NAMESTR_LEN 128                             // string length of the object/record instance name
#define OOEPICS_PATHSTR_LEN 256                             // string length of the file path (including file name)

#define OOEPICS_CTL_DEVICE  0                               // to indicate we have a control device
#define OOEPICS_DOM_DEVICE  1                               // to indicate we have a domain device
#define OOEPICS_APPLICATION 2                               // to indicate we have an application

//-----------------------------------------------
// tools Macros
//-----------------------------------------------
#define OOEPICS_DEL_EXIST(A) if(A){delete(A);}

//-----------------------------------------------
// tools routines
//-----------------------------------------------
void eraseChar(string *str, const char *c);
void replaceChar(string *str, const char *c1, const char *c2);
void stringToUpper(string *str);
void stringReplace(char *str, char src, char des);
void getTimeString(char *timeStr, int withYear, const char *formatStr);
char *getSystemTime();

int pvTimeGetCurrentDouble(double *pTime);
int calcRemainedTimeOut(double *remainedTimeOut, double startTime, double timeOut);

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif






