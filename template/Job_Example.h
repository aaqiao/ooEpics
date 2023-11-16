//=========================================================
// Job_Example.h
// 
// This is an example of a job based on the OOEPICS framework
// Auto created from the template of ooEpics framework
//=========================================================
#ifndef JOB_EXAMPLE_H
#define JOB_EXAMPLE_H

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "Job.h"
#include "LocalPV.h"
#include "ooEpicsMisc.h"
#include "MessageLogs.h"
#include "EPICSLib_wrapper.h"

#include "Service_Example.h"

#define JOBEXAM_SET_VALUE   0
#define JOBEXAM_GET_VALUE   1

//-----------------------------------------------
// class definition
//-----------------------------------------------
class Job_Example : public OOEPICS::Job
{
public:
    Job_Example(const char *moduleName, const char *jobName);
    virtual ~Job_Example();

    // interface functions for associations
    void setService_Example (Service_Example        *srvExampleIn);
    void setMessageLogging  (OOEPICS::MessageLogs   *msgLogIn);    

    // implement the virtual functions
    virtual void enableJobExt();                    // function that will be executed when setting the enable of the job
    virtual int execute(int flag);                  // main entrance to execute the job

private:    
    // pointers for the associations
    Service_Example      *ptr_srvExample;
    OOEPICS::MessageLogs *ptr_msgLog;

    // local PVs for the job
    OOEPICS::LocalPV lpv_enableDebug;               // bo, enable printing debug message on iocshell
    OOEPICS::LocalPV lpv_enableJob;

    OOEPICS::LocalPV lpv_boExample;                 // examples of output records for parameter settings
    OOEPICS::LocalPV lpv_mbboExample;               
    OOEPICS::LocalPV lpv_longoutExample;
    OOEPICS::LocalPV lpv_aoExample;

    OOEPICS::LocalPV lpv_waveformoutExample;
    OOEPICS::LocalPV lpv_stringoutExample;

    OOEPICS::LocalPV lpv_biExample;                 // examples of input records for results displaying
    OOEPICS::LocalPV lpv_mbbiExample;
    OOEPICS::LocalPV lpv_longinExample;
    OOEPICS::LocalPV lpv_aiExample;

    OOEPICS::LocalPV lpv_xaxis;
    OOEPICS::LocalPV lpv_waveforminExample;
    OOEPICS::LocalPV lpv_stringinExample;

    OOEPICS::LocalPV lpv_timeStr;                   // stringin, time string for run-time message
    OOEPICS::LocalPV lpv_msgStr;                    // stringin, run-time message

    OOEPICS::LocalPV lpv_counter1S;                 // ai, demonstrate 1 second scanning
    OOEPICS::LocalPV lpv_counter2S;                 // ai, demonstrate 2 second scanning
    OOEPICS::LocalPV lpv_counter5S;                 // ai, demonstrate 5 second scanning

    // locks and I/O interrupt scanning driver for local PVs
    EPICSLIB_type_mutexId   var_lockParameters;     // parameter PVs are shared by multiple thread, the mutex to protect them
    EPICSLIB_type_ioScanPvt var_scanResultPVs;      // collect PVs that can be scanned by the job with "I/O Inter" scan method

    // private variables
    int var_valCnt1S;
    int var_valCnt2S;
    int var_valCnt5S;

    double buf_wfo[SRVEXAM_BUF_SIZE];               // define buffers used for the waveform PVs
    double buf_wfx[SRVEXAM_BUF_SIZE];
    double buf_wfi[SRVEXAM_BUF_SIZE];

    // private general routines
    void initLocalPVs();    
    void updateResultPVs();
    void postMsg(OOEPICS::MessageLogs *msgLogIn, short debugEnabled, short infoErr, string msg);

    // callback functions
    static void lpvRcb_cnt1S(void *arg);
    static void lpvRcb_cnt2S(void *arg);
    static void lpvRcb_cnt5S(void *arg);
};

#endif


