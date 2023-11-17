//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// Coordinator.cc
//
// Implementation of base class of Coordinator
//===============================================================
#include "Coordinator.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction and destruction for Coordinator
//-----------------------------------------------
Coordinator::Coordinator(const char *modNameIn, const char *crdNameIn)
{
    int i;

    // check the input
    if(!modNameIn || !modNameIn[0] || !crdNameIn || !crdNameIn[0])
        cout << "ERROR: Coordinator::Coordinator: Wrong coordinator name!\n";

    // remember the input
	strncpy(modName, modNameIn, CRD_STRING_LEN-1);
    strncpy(crdName, crdNameIn, CRD_STRING_LEN-1);
 
	for(i = 0; i < CRD_MAX_NUM_JOBS; i ++)
		jobSet[i] = NULL;

    cout << "INFO: Coordinator::Coordinator: Object " << crdName << " for module " << modName << " created." << endl;
}

Coordinator::~Coordinator() 
{
    cout << "INFO: Coordinator::~Coordinator: Object " << crdName << " for module " << modName << " deleted!" << endl;
}

//-----------------------------------------------
// register and execute the job
//-----------------------------------------------
void Coordinator::registerJob(Job *job, int jobCode) 
{
	if(jobCode >= 0 && jobCode < CRD_MAX_NUM_JOBS) 
		jobSet[jobCode] = job;
}

int  Coordinator::executeJob(int jobCode, int flag) 
{
	if(jobCode >= 0 && jobCode < CRD_MAX_NUM_JOBS && jobSet[jobCode]) 
		return jobSet[jobCode] -> execute(flag);
	else 
		return -1;
}

void Coordinator::enableAllJobs(int enabled)
{
    for(int i = 0; i < CRD_MAX_NUM_JOBS; i ++) {
        if(jobSet[i])
            jobSet[i] -> enableJob((short)enabled);
    }
}

void Coordinator::enableJob(int jobCode, int enabled)
{
    if(jobCode >= 0 && jobCode < CRD_MAX_NUM_JOBS && jobSet[jobCode]) 
        jobSet[jobCode] -> enableJob((short)enabled);
}

//-----------------------------------------------
// event handling functions
//-----------------------------------------------
void Coordinator::sendEvent              ()                                                         {var_event.sendEvent();}
void Coordinator::sendEvent              (int eventCode, int cmd, int subCmd)                       {var_event.sendEvent(eventCode, cmd, subCmd);}
void Coordinator::waitEvent              ()                                                         {var_event.recvEvent();}
void Coordinator::waitEvent              (int *eventCode, int *cmd, int *subCmd)                    {var_event.recvEvent(eventCode, cmd, subCmd);}
void Coordinator::waitEventWithTimeout   (double timeout)                                           {var_event.recvEventWithTimeout(timeout);}
void Coordinator::waitEventWithTimeout   (int *eventCode, int *cmd, int *subCmd, double timeout)    {var_event.recvEventWithTimeout(eventCode, cmd, subCmd, timeout);}
int  Coordinator::withPendingEvents      ()                                                         {return var_event.getUnsolvedMsgNum() > 0 ? 1 : 0;}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************




