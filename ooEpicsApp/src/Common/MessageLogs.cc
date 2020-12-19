//=========================================================
// MessageLogs.cc
//
// Basic class for logging messages
//
// Created by Zheqiao Geng (zheqiao.geng@psi.ch) on 2014.06.02
//
// Modified by Zheqiao Geng on 27.02.2015
// Added lock to protect the buffer
//=========================================================
#include "MessageLogs.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction and distruction
//-----------------------------------------------
MessageLogs::MessageLogs(const char *modNameIn)
{
	int i;

    // init variables
    strncpy(modName, modNameIn, MSGLOG_STR_LEN);
	curStr = 0;

	for(i = 0; i < MSGLOG_MAX_NUM; i ++) {
        strcpy(message[i], "");
		msgStr[i].assign("");	
    }

    // init the mutex
    var_lockMsgBuf = EPICSLIB_func_mutexMustCreate();

    cout << "INFO: MessageLogs::MessageLogs: Object for module " << modName << " created." << endl;
}

MessageLogs::~MessageLogs()
{
    // destroy the epics mutex
    if(var_lockMsgBuf)      
        EPICSLIB_func_mutexDestroy(var_lockMsgBuf);

    cout << "INFO: MessageLogs::~MessageLogs: Object for module " << modName << " deleted!" << endl;
}

//-----------------------------------------------
// post message, now only put into the local buffer. 
// later should implement post the message into local file or remote servers
//-----------------------------------------------
void MessageLogs::postMessage(char *msg)
{
	int i, strId;
    char timeStr[20];

	// get the current message
    getTimeString(timeStr, 1, "%4d%2d%2d%2d%2d%2d");
    stringReplace(timeStr, ' ', '0');
	
	if(curStr >= 0 && curStr < MSGLOG_MAX_NUM) {
		msgStr[curStr].assign("[");
		msgStr[curStr] += timeStr;
		msgStr[curStr] += "]: ";
		msgStr[curStr] += msg;
	}

	// update the pointer
	curStr ++;
	if(curStr >= MSGLOG_MAX_NUM)
		curStr = 0;

    strId = curStr;

	// generate the message block
    for(i = 0; i < MSGLOG_MAX_NUM; i++) {        
        // copy the string
        EPICSLIB_func_mutexMustLock(var_lockMsgBuf);
        strncpy(message[i], msgStr[strId].c_str(), MSGLOG_MAX_LEN - 1);
        EPICSLIB_func_mutexUnlock(var_lockMsgBuf);

        // get the string id
        strId ++;      
	    if(strId >= MSGLOG_MAX_NUM)
		    strId = 0;
    }    
}

//-----------------------------------------------
// copyout the message (should lock externally)
//-----------------------------------------------
void MessageLogs::copyMessage(char *dest, int id)
{
    if(dest && id >= 0 && id < MSGLOG_MAX_NUM) {
        EPICSLIB_func_mutexMustLock(var_lockMsgBuf);
        memcpy((void *)dest, (void *)message[id], sizeof(char) * MSGLOG_MAX_LEN);
        EPICSLIB_func_mutexUnlock(var_lockMsgBuf);
    }
}

//-----------------------------------------------
// print the message, for debugging
//-----------------------------------------------
void MessageLogs::printMessage()
{
    int i;

	cout << "----------------------- Message (raw string) -----------------------" << endl;
    for(i = 0; i < MSGLOG_MAX_NUM; i++) {
        cout << msgStr[i] << endl;
    }
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************




