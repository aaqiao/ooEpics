//=========================================================
// Coordinator.h
//
// Basic class for coordinator for job execution
//
// Created by Zheqiao Geng (zheqiao.geng@psi.ch) on 29.09.2015
//
// Modified by Zheqiao Geng on 14.04.2016
// Updated the event implementation according to the FSM base class
//=========================================================
#ifndef COORDINATOR_H
#define COORDINATOR_H
#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "EPICSLib_wrapper.h"
#include "Job.h"
#include "FSM.h"                                    // will use the FSM event

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

#define CRD_MAX_NUM_JOBS		256
#define CRD_STRING_LEN          256

//-----------------------------------------------
// base class definition for the Coordinator
//-----------------------------------------------
class Coordinator
{
public:
    Coordinator                 (const char *modNameIn, const char *crdNameIn);
    virtual ~Coordinator        ();

    virtual int execute         () = 0;                             // execute the coordinator

	void registerJob            (Job *job, int jobCode);		    // register the job
	int  executeJob             (int jobCode, int flag);		    // execute the job
    void enableAllJobs          (int enabled);                      // enable or disable all jobs
    void enableJob              (int jobCode, int enabled);         // enable or disable a job

    void sendEvent              ();
    void sendEvent              (int eventCode, int cmd, int subCmd);
    void waitEvent              ();
    void waitEvent              (int *eventCode, int *cmd, int *subCmd);
    void waitEventWithTimeout   (double timeout);
    void waitEventWithTimeout   (int *eventCode, int *cmd, int *subCmd, double timeout);    
    int  withPendingEvents      ();

	char modName[CRD_STRING_LEN];
    char crdName[CRD_STRING_LEN];

private:
    // jobs identified by the job code
	Job *jobSet[CRD_MAX_NUM_JOBS];

    // event for this FSM
    FSMEvent var_event;
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

