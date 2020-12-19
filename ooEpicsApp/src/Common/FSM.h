//=========================================================
// FSM.h
//
// Basic class for finit state machine and the states
//
// Created by Zheqiao Geng (zheqiao.geng@psi.ch) on 2014.04.09
//
// Modified by Zheqiao Geng on 2014.04.15
// Description: user pure virtual functions
//
// Modified by Zheqiao Geng on 2014.05.25
// Added object of jobs
// 
// Modified by Zheqiao Geng on 2014.05.30
// The state contains pointer of the owner FSM to execute the jobs in the FSM
//
// Modified by Zheqiao Geng on 2014.06.13
// Implement the message queue for the FSMEvent class. Later need to add exception handling
//
// Modified by Zheqiao Geng on 19.02.2016
// Added to the event the option only use event; delete the transient function from the FSM
//
// Modified by Zheqiao Geng on 22.03.2016
// Simplified the event implementation
//
// Modified by Zheqiao Geng on 12.04.2016
//  - Added the function of delay in the state class
//  - The entry time of a state will be record when the entry() function is executed
//  - Moved the event function all to the base class of the FSM
//
// Modified by Zheqiao Geng on 13.04.2016
// Implemented a timer in the FSM to be sure there are events sent for withDelay checking
//=========================================================
#ifndef FSM_H
#define FSM_H
#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "epicsTimer.h"

#include "EPICSLib_wrapper.h"
#include "Job.h"
#include "ooEpicsMisc.h"

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

#define FSM_MAX_NUM_STATES          256
#define FSM_MAX_NUM_JOBS		    256
#define FSM_STRING_LEN              256

#define FSM_EV_OK                   0
#define FSM_EV_ERR                  1
#define FSM_EV_TIMEOUT              2

#define FSM_EXE_SUCCESS			    0					// execution successfully
#define FSM_EXE_ERR_ENTRY_FAIL      1                   // entry function execution is failed
#define FSM_EXE_ERR_NOTRANS		    2					// dest state is invalid, do not transfer
#define FSM_EXE_ERR_TODEFAULT	    3					// dest/prev state are all invalid, transfer to default state
#define FSM_EXE_ERR_TO1STVALID	    4					// dest/prev/default state are all invalid, transfer to the first valid state in the set
#define FSM_EXE_ERR_TRANS_FAIL      5

#define FSM_EVENT_MAX_MSG_NUM	    32					// maximum number of message in the queue
#define FSM_EVENT_MAX_MSG_LEN	    32					// maximum length of message in the queue

#define FSM_MIN_TICK_TIME           0.1                 // minimum time to pull the FSM, second

#define FSM_INIT_CURR_ENTRY_EXE     0                   // execute the entry function when initializing the current state
#define FSM_INIT_CURR_ENTRY_NOEXE   1                   // do not execute the entry function when initializing the current state
#define FSM_INIT_CURR_FORCE_TRANS   2                   // force to transfer to the assigned current state from existing state

//-----------------------------------------------
// forward declare
//-----------------------------------------------
class FSM;

//-----------------------------------------------
// event code with command
//-----------------------------------------------
typedef struct {
    int eventCode;
    int cmd;
    int subCmd;
} FSMEventMsg;

//-----------------------------------------------
// event used in the state machine
//-----------------------------------------------
class FSMEvent
{
public:
    FSMEvent();
   ~FSMEvent();

    int  sendEvent              ();                                                   // use event 
    int  sendEvent              (int eventCodeIn, int cmdIn, int subCmdIn);           // use message queue
    void recvEvent              ();
    void recvEvent              (int *eventCodeOut, int *cmdOut, int *subCmdOut);
    void recvEventWithTimeout   (double timeout);
    void recvEventWithTimeout   (int *eventCodeOut, int *cmdOut, int *subCmdOut, double timeOut);

    int  getUnsolvedMsgNum      ();

private:
    EPICSLIB_type_eventId   eventId;
	EPICSLIB_type_msgQ		msgQ;
    epicsEventWaitStatus    status;
};

//-----------------------------------------------
// base class definition for the state
//-----------------------------------------------
class State
{
public:
    State                   (int stateCodeIn, const char *stateNameIn);
    virtual ~State          ();

    virtual int entry       () = 0;                         // entry functions for the state
    virtual int do_activity () = 0;                         // function during the state and accept events to do something including transition (the transition code will be returned)
    virtual int exit        () = 0;                         // exit functions

    void printState         ();                             // print the state info
    void setSourceStateCode (int prevStateCode);            // set the source state code
    void setDestStateCode   (int nextStateCode);            // set the destination state code
    int  getStateCode       ();
    int  getSourceStateCode ();
    int  getDestStateCode   ();

    void initDelay();                                       // init the delay, usually should be called in the entry() function
    int  withDelay(double timeout);                         // check the delay compared to the entry time of this state
    double getTimeExpectEvent();                            // get the time after which an event is expected
    void clearTimeExpectEvent();

    char stateName[FSM_STRING_LEN];

private:   
    int  stateCode;                                         // state code of the state
    int  sourceStateCode;                                   // state code of the source state transient to this
    int  destStateCode;                                     // the state that will be transfered to

    double entryTime;                                       // record the entry time
    double timeExpectEvent;                                 // remained time to the nearist desired delay
};

//-----------------------------------------------
// base class definition for the FSM
//-----------------------------------------------
class FSM
{
public:
    FSM                         (const char *modNameIn, const char *fsmNameIn);
    virtual ~FSM                ();

    int  executeFSM             ();                             // execute the FSM, this is generall for all FSM derived from the base class
    void printFSM               ();
    int  registerState          (State *state, int stateCode);  // register the state
    int  initCurrentState       (int stateCode, int sel);
	int  setDefaultState        (int stateCode);
	int  getCurrentStateCode    ();

	void registerJob            (Job *job, int jobCode);        // register the job
	int  executeJob             (int jobCode, int flag);        // execute the job
    void enableAllJobs          (int enabled);                  // enable or disable all jobs
    void enableJob              (int jobCode, int enabled);     // enable or disable a job

    void sendEvent              ();
    void sendEvent              (int eventCode, int cmd, int subCmd);
    void waitEvent              ();
    void waitEvent              (int *eventCode, int *cmd, int *subCmd);
    void waitEventWithTimeout   (double timeout);
    void waitEventWithTimeout   (int *eventCode, int *cmd, int *subCmd, double timeout);    
    int  withPendingEvents      ();
    int  isMsgQFull             ();

    virtual int initFSM         () = 0;                         // init the FSM, normally put the FSM to some default state
    virtual int executeExtFunc  () = 0;                         // extend the execution function

	char modName[FSM_STRING_LEN];
    char fsmName[FSM_STRING_LEN];

private:
    // states management
    State *stateSet[FSM_MAX_NUM_STATES];                        // state set, maintain the pointer of all states
    State *curState;                                            // current state
    State *preState;                                            // previous state
	State *defaultState;									    // default state avoid dead lock

    int nextStateCode;

	State *getFirstValidState();

    // jobs (optional, the FSM can also be used as engine for job executions)
	Job *jobSet[FSM_MAX_NUM_JOBS];						        // job set, mainain the pointer of all jobs that will be exectued by this FSM

    // event for this FSM
    FSMEvent var_event;

    // timer for this FSM
    epicsTimerQueueId   var_timerQueue;
    epicsTimerId        var_timer;

    // callback for the timer
    static void fun_timerCallback(void *arg);
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

