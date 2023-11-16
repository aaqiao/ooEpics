//=========================================================
// Module___MODULE_NAME__.h
// 
// Class definition for the __MODULE_NAME__ module
// Auto created from the template of ooEpics framework
//=========================================================
#ifndef MODULE___MODULE_NAME___H
#define MODULE___MODULE_NAME___H

// C/C++ standard libraries
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <fstream>

// ooEpics
#include "ControlDevice.h"
#include "DomainDevice.h"
#include "Application.h"
#include "LocalPV.h"
#include "RemotePV.h"
#include "dbCommon.h"
#include "ooEpicsMisc.h"
#include "MessageLogs.h"
#include "EPICSLib_wrapper.h"

// local classes for this module
#include "Coordinator_Jobs.h"
#include "Job_Example.h"
#include "Service_Example.h"
#include "FSM_Example.h"

using namespace std;

// NOTE: please change these strings before making a GIT or CVS tag
#define __MODULE_NAME___VERS "0.0.0"                     // version of the module
#define __MODULE_NAME___DATE "20.06.2016"                // release date

//-----------------------------------------------
// class definition for Module___MODULE_NAME__
//-----------------------------------------------
class Module___MODULE_NAME__ : public OOEPICS::Application
{
public:
    Module___MODULE_NAME__(const char *moduleName, int priority);
    virtual ~Module___MODULE_NAME__();

    // start threads in this module
    void startThreads();

    // interface for channel access initialization for this module
    string rpvMapFilePath;                              // path of remote PV name mapping file
    string rpvMapFileName;                              // name of remote PV name mapping file
    string rpvMapMacrosFileSpecific;                    // specific macros applied on remote PVs in the file
    string rpvMapMacrosGeneral;                         // general macros applied on all remote PVs

    void initRPVMaps();                                 // init the mapping of remote PV names
    void initChannelAccess();                           // init channel access in this module
    void printRPVList(int sel);                         // print the remote PV list on iocschell

private:
    // channel access
    OOEPICS::ChannelAccessContext caContext;            // channel access client context for this module

    // objects of this module
    Coordinator_Jobs  var_crd_Jobs;                     // coordinator to execute jobs
    Job_Example       var_job_Example;                  // example job
    Service_Example   var_srv_Example;                  // example service
    FSM_Example       var_fsm_Example;                  // example FSM
   
    // local PVs (EPICS records) for top level control of the module
    OOEPICS::LocalPV  lpv_cmd_jobExampleSet;            // bo, command to execute the example job to set remote data
    OOEPICS::LocalPV  lpv_cmd_jobExampleGet;            // bo, command to execute the example job to get remote data
    OOEPICS::LocalPV  lpv_cmd_fsmExample;               // mbbo, command the FSM

    OOEPICS::LocalPV  lpv_monRPVNum;                    // longin, display the total number of remote PVs in this module
    OOEPICS::LocalPV  lpv_monRPVMappedNum;              // longin, display the total number of remote PVs with names mapped
    OOEPICS::LocalPV  lpv_monRPVConnectedNum;           // longin, display the total number of remote PVs with connections

    OOEPICS::LocalPV  lpv_msgLog[MSGLOG_MAX_NUM];       // stringin[], display the run-time messages

    OOEPICS::LocalPV  lpv_versionStr;                   // stringin, display the version
    OOEPICS::LocalPV  lpv_compileDateStr;               // stringin, display the release (compile) date

    // for remote PVs
    OOEPICS::RemotePVList rpvList;                      // contains two lists: RemotePV and NamePair

    // module private data
    OOEPICS::MessageLogs  var_msgLog;                   // run-time message for this module

    // private routines
    void initLocalPVs();
    void initRemotePVs();

    // callback functions
    static void lpvWcb_cmdJobExample    (void *arg);    // callback function when user setting the example job execution PVs
    static void lpvWcb_cmdFSMExample    (void *arg);    // callback function when user setting the example FSM command
    static void lpvRcb_monRPVCnts       (void *arg);    // callback function for remote PV counters
    static void lpvRcb_getLogMsg        (void *arg);    // callback function for getting run-time messages in PVs
};

#endif

