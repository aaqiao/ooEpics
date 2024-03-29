//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// MessageLogs.h
//
// Basic class for logging messages
//===============================================================
#ifndef MESSAGE_LOGS_H
#define MESSAGE_LOGS_H

#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ooEpicsMisc.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

#define MSGLOG_STR_LEN 		128 							// string length for names
#define MSGLOG_MAX_NUM      32								// messages supported
#define MSGLOG_MAX_LEN 		80 							    // maximum 80 chars including \0

//-----------------------------------------------
// base class definition for the Job
//-----------------------------------------------
class MessageLogs
{
public:
    MessageLogs(const char *modNameIn);
   ~MessageLogs();

	// routines
    void  postMessage(char *msg);							// post a message
    void  copyMessage(char *dest, int id);                  // copy a message
    void  printMessage();

private:
    EPICSLIB_type_mutexId var_lockMsgBuf;

    char modName[MSGLOG_STR_LEN];
	char message[MSGLOG_MAX_NUM][MSGLOG_MAX_LEN];

	string msgStr[MSGLOG_MAX_NUM];
	int    curStr;
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

