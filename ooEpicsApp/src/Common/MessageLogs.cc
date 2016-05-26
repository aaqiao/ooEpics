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
}

MessageLogs::~MessageLogs()
{
    // destroy the epics mutex
    if(var_lockMsgBuf)      
        EPICSLIB_func_mutexDestroy(var_lockMsgBuf);
}

//-----------------------------------------------
// post message, now only put into the local buffer. 
// later should implement post the message into local file or remote servers
//-----------------------------------------------
void MessageLogs::postMessage(char *msg)
{
	int i, strId;
    char timeStr[20];

    EPICSLIB_func_mutexMustLock(var_lockMsgBuf);

	// get the current message
    getTimeString(timeStr, 1, "%4d%2d%2d%2d%2d%2d");
    stringReplace(timeStr, ' ', '0');
	
	if(curStr >= 0 && curStr < MSGLOG_MAX_NUM) {
		msgStr[curStr].assign("[");
		msgStr[curStr] += timeStr;
//      msgStr[curStr] += "][";                 // make it shorter
//      msgStr[curStr] += modName;
		msgStr[curStr] += "]: ";
		msgStr[curStr] += msg;
	}

	// update the pointer
	curStr ++;
	if(curStr >= MSGLOG_MAX_NUM)
		curStr = 0;

	// generate the message block
	memset((void *)message, 0, MSGLOG_MAX_LEN * MSGLOG_MAX_NUM * sizeof(char));
    strId = curStr;

    for(i = 0; i < MSGLOG_MAX_NUM; i++) {        
        // - copy the string
        strncpy(message[i], msgStr[strId].c_str(), MSGLOG_MAX_LEN - 1);

        // - get the string id
        strId ++;      
	    if(strId >= MSGLOG_MAX_NUM)
		    strId = 0;
    }

    EPICSLIB_func_mutexUnlock(var_lockMsgBuf);
}

//-----------------------------------------------
// copyout the message (should lock externally)
//-----------------------------------------------
void MessageLogs::copyMessage(char *dest, int id)
{
    if(dest && id >= 0 && id < MSGLOG_MAX_NUM)       
        memcpy((void *)dest, (void *)message[id], sizeof(char) * MSGLOG_MAX_LEN);
}

//-----------------------------------------------
// lock for block copying
//-----------------------------------------------
void MessageLogs::lockMessage()   {EPICSLIB_func_mutexMustLock(var_lockMsgBuf);}
void MessageLogs::unlockMessage() {EPICSLIB_func_mutexUnlock(var_lockMsgBuf);}

//-----------------------------------------------
// print the message, for debugging
//-----------------------------------------------
void MessageLogs::printMessage()
{
    int i;

	cout << "----------------------- Message (raw string) -----------------------" << endl;
    EPICSLIB_func_mutexMustLock(var_lockMsgBuf);
    for(i = 0; i < MSGLOG_MAX_NUM; i++) {
//    	cout << message[i] << endl;
        cout << msgStr[i] << endl;
    }
    EPICSLIB_func_mutexUnlock(var_lockMsgBuf);
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************




