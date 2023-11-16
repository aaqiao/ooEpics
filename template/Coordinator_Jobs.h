//=========================================================
// Coordinator_Jobs.h
// 
// Class definition for the coordinator example to execute the jobs
// Auto created from the template of ooEpics framework
//=========================================================
#ifndef COORDINATOR_JOBS_H
#define COORDINATOR_JOBS_H

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <fstream>

#include "LocalPV.h"
#include "RemotePV.h"
#include "Coordinator.h"
#include "ooEpicsMisc.h"
#include "EPICSLib_wrapper.h"

#include "ModuleDefs___MODULE_NAME__.h"

//-----------------------------------------------
// class forward declaration
//-----------------------------------------------
class Coordinator_Jobs;

//-----------------------------------------------
// thread of the coordinator
//-----------------------------------------------
class CRDJOBS_Thread : public epicsThreadRunable
{
public:
    CRDJOBS_Thread(const char *name, Coordinator_Jobs *ownerInput, int priority);
    virtual ~CRDJOBS_Thread();

    virtual void run();
    void start();

private:
    epicsThread thread;
    Coordinator_Jobs *owner;

    char trdName[128];
};

//-----------------------------------------------
// job code
//-----------------------------------------------
typedef enum {
    CRDJOBS_JOBCODE_NULL        = -1,                           // empty job
    CRDJOBS_JOBCODE_EXAMPLE                                     // job example

    // NOTE: add job codes for your other jobs here

} CRDJOBS_JOBCODE;

//-----------------------------------------------
// event code
//-----------------------------------------------
typedef enum {
    CRDJOBS_EVENT_JOB                                           // event to execute jobs

    // NOTE: You may add another category of event to trigger something 
    //       other than jobs that should be executed by this coordinator

} CRDJOBS_EVENTCODE;

//-----------------------------------------------
// class for Coordinator_Jobs
//-----------------------------------------------
class Coordinator_Jobs : public OOEPICS::Coordinator
{
public:
    Coordinator_Jobs(const char *modNameIn, 
                     const char *crdNameIn, 
                     int priority,
                     OOEPICS::ChannelAccessContext *caContextIn);
    virtual ~Coordinator_Jobs();

    // start the thread in this coordinator
    void startThread();

    // function for execution
    virtual int execute();

    // routine of thread
    void threadFunc_CRDJOBS();

private:
    OOEPICS::ChannelAccessContext *ptr_caContext;               // pointer of channel access context for the module

    CRDJOBS_Thread var_thread_CRDJOBS;                          // object of the thread

    int var_CRDJOBSThreadStopCmd;                               // thread stopping control
    int var_CRDJOBSThreadStopped;
};

#endif



