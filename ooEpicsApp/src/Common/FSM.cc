//=========================================================
// FSM.cc
//
// Basic class for finit state machine and the states
//
// Created by Zheqiao Geng (zheqiao.geng@psi.ch) on 2014.04.09
//
// Modified by Zheqiao Geng on 2014.05.25
// Added object of jobs
//=========================================================
#include "FSM.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CLASS OF FSM EVENT
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction
//-----------------------------------------------
FSMEvent::FSMEvent() : 
	msgQ(FSM_EVENT_MAX_MSG_NUM, FSM_EVENT_MAX_MSG_LEN)
{   
    cout << "INFO: FSMEvent::FSMEvent: Create the event and msgQ...";

    eventId = EPICSLIB_func_eventMustCreate(epicsEventEmpty);

    cout << "Done!\n";
}

//-----------------------------------------------
// destruction
//-----------------------------------------------
FSMEvent::~FSMEvent()
{
    if(eventId)
        EPICSLIB_func_eventDestroy(eventId);

    cout << "INFO: FSMEvent::~FSMEvent: deleted!" << endl;
}

//-----------------------------------------------
// send event
//-----------------------------------------------
int FSMEvent::sendEvent()
{
    if(eventId) {
        EPICSLIB_func_eventSignal(eventId);  
    } else {
       	return FSM_EV_ERR;
    }

    return FSM_EV_OK;    
}

//-----------------------------------------------
// send event with msgQ
//-----------------------------------------------
int FSMEvent::sendEvent(int eventCodeIn, int cmdIn, int subCmdIn)
{
    FSMEventMsg msg;
    msg.eventCode = eventCodeIn;
    msg.cmd       = cmdIn;
	msg.subCmd 	  = subCmdIn;

    msgQ.send((void *)&msg, sizeof(FSMEventMsg)); 

    return FSM_EV_OK;
}

//-----------------------------------------------
// receive event
//-----------------------------------------------
void FSMEvent::recvEvent()
{
    if(eventId)
        EPICSLIB_func_eventWait(eventId);        
}

//-----------------------------------------------
// receive event with msgQ
//-----------------------------------------------
void FSMEvent::recvEvent(int *eventCodeOut, int *cmdOut, int *subCmdOut)
{
    FSMEventMsg msg;

    msgQ.receive((void *)&msg, sizeof(FSMEventMsg));
    
    if(eventCodeOut) {
        *eventCodeOut = msg.eventCode;
    }
    if(cmdOut) {
        *cmdOut = msg.cmd;
    }
	if(subCmdOut) {
		*subCmdOut = msg.subCmd;
    }
}

//-----------------------------------------------
// receive event with timeout
//-----------------------------------------------
void FSMEvent::recvEventWithTimeout(double timeOut)
{
    if(eventId)
        EPICSLIB_func_eventWaitWithTimeout(eventId, timeOut);
}

//-----------------------------------------------
// receive event with timeout with msgQ
//-----------------------------------------------
void FSMEvent::recvEventWithTimeout(int *eventCodeOut, int *cmdOut, int *subCmdOut, double timeOut)
{
    FSMEventMsg msg;

    msgQ.receive((void *)&msg, sizeof(FSMEventMsg), timeOut);
    
    if(eventCodeOut) {
        *eventCodeOut = msg.eventCode;
    }
    if(cmdOut) {
        *cmdOut = msg.cmd;
    }
	if(subCmdOut) {
		*subCmdOut = msg.subCmd;
    }
}

//-----------------------------------------------
// get the pending message number in the queue
//-----------------------------------------------
int FSMEvent::getUnsolvedMsgNum()
{
    return msgQ.pending();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CLASS OF STATE
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction and destruction for State
//-----------------------------------------------
State::State(int stateCodeIn, const char *stateNameIn)
{
    // check the input
    if(stateCodeIn < 0 || !stateNameIn || !stateNameIn[0])
        cout << "ERROR: State::State: Wrong state code or name!\n";

    // remember the input
    strncpy(stateName, stateNameIn, FSM_STRING_LEN);
    stateCode           = stateCodeIn;
    sourceStateCode     = stateCodeIn;
    destStateCode       = stateCodeIn;	

    entryTime           = 0.0;
    timeExpectEvent     = 0.0;

    cout << "INFO: State::State: Object " << stateName << " created." << endl;
}

State::~State() 
{
    cout << "INFO: State::~State: Object " << stateName << " deleted!" << endl;
}

//-----------------------------------------------
// print the info of the state, set the source state 
// transient from and get the current state code
//-----------------------------------------------
void State::printState          ()                  {cout << "State Code = " << stateCode << ", State Name = " << stateName << "\n";}
void State::setSourceStateCode  (int prevStateCode) {sourceStateCode    = prevStateCode;}
void State::setDestStateCode    (int nextStateCode) {destStateCode      = nextStateCode;}
int  State::getStateCode        ()                  {return stateCode;}
int  State::getSourceStateCode  ()                  {return sourceStateCode;}
int  State::getDestStateCode    ()                  {return destStateCode;}

//-----------------------------------------------
// init delays
//-----------------------------------------------
void State::initDelay()
{
    pvTimeGetCurrentDouble(&entryTime);
    timeExpectEvent = 0.0;
}

//-----------------------------------------------
// detect if the delay expires or not
// return: 1 - delay expires
//-----------------------------------------------
int State::withDelay(double timeout)
{
    double remainedTimeOut = 0.0;

    calcRemainedTimeOut(&remainedTimeOut, entryTime, timeout);

    if(remainedTimeOut > 0.0) {
        if(timeExpectEvent <= 0.0)
            timeExpectEvent = remainedTimeOut;
        else
            timeExpectEvent = timeExpectEvent > remainedTimeOut ? remainedTimeOut : timeExpectEvent;

        return 0;

    } else {
        return 1;
    }
}

//-----------------------------------------------
// get the time after which an event is expected
//-----------------------------------------------
double State::getTimeExpectEvent()
{
    return timeExpectEvent;
}

//-----------------------------------------------
// clear the time after which an event is expected
//-----------------------------------------------
void State::clearTimeExpectEvent()
{
    timeExpectEvent = 0.0;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CLASS OF FSM
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction of FSM
//-----------------------------------------------
FSM::FSM(const char *modNameIn, const char *fsmNameIn)
{
    int i;

    // check the input
    if(!modNameIn || !modNameIn[0] || !fsmNameIn || !fsmNameIn[0])
        cout << "ERROR: FSM::FSM: Wrong FSM name!\n";

    // remember the input
	strncpy(modName, modNameIn, FSM_STRING_LEN);
    strncpy(fsmName, fsmNameIn, FSM_STRING_LEN);

    // init private variables
    nextStateCode   = -1;
    curState        = NULL;
    preState        = NULL;
    defaultState    = NULL;

    for(i = 0; i < FSM_MAX_NUM_STATES; i ++) {
        stateSet[i] = NULL;
    }
	for(i = 0; i < FSM_MAX_NUM_JOBS; i ++) {
		jobSet[i] = NULL;
    }

    // create the timer
    var_timerQueue = epicsTimerQueueAllocate(1, epicsThreadPriorityScanHigh);
    var_timer      = epicsTimerQueueCreateTimer(var_timerQueue, fun_timerCallback, (void *)&var_event);

    cout << "INFO: FSM::FSM: Object" << fsmName << " for module " << modName << " created." << endl;
}

//-----------------------------------------------
// destruction of FSM
//-----------------------------------------------
FSM::~FSM() 
{
    // destroy the timer
    //epicsTimerQueueDestroyTimer(var_timerQueue, var_timer);
    epicsTimerQueueRelease(var_timerQueue);

    cout << "INFO: FSM::~FSM: Object " << fsmName << " for module " << modName << " deleted!" << endl;
}

//-----------------------------------------------
// execute the FSM
//-----------------------------------------------
int FSM::executeFSM()
{
	int status = FSM_EXE_SUCCESS;
    double timeExpectEvent = 0.0;

    // entry of a new state
    if(curState && curState != preState) {

        // - assign the source state code of the new state
        if(preState) 
            curState -> setSourceStateCode(preState -> getStateCode());
        else
            curState -> setSourceStateCode(-1);

        // - the entry function of the new state must be successful
        if(curState -> entry() != FSM_EXE_SUCCESS) {
            executeExtFunc();
            return FSM_EXE_ERR_ENTRY_FAIL;
        }

        // - rememter the entry time of the new state
        curState -> initDelay();

        // - to be sure the entry function is executed only when first entering
        preState = curState;

        // - send a event to execute at least once the "do" function of the new state 
        sendEvent();
    }

    // execute the do function
    if(curState)
        nextStateCode = curState -> do_activity();

    // check if an event is expected after some time
    if(curState) {
        timeExpectEvent = curState -> getTimeExpectEvent();

        if(timeExpectEvent > 0.0) {
            timeExpectEvent = timeExpectEvent < FSM_MIN_TICK_TIME ? FSM_MIN_TICK_TIME : timeExpectEvent;
            epicsTimerStartDelay(var_timer, timeExpectEvent);
            curState -> clearTimeExpectEvent();
        }
    }
    
    // determine the transition state and avoid transferring to undefined state
	if(nextStateCode >= 0 && nextStateCode < FSM_MAX_NUM_STATES) {
	    curState = stateSet[nextStateCode];

	    if(!curState) {
		    if(preState) {
			    curState = preState;                    // first choise, do not transient
			    status   = FSM_EXE_ERR_NOTRANS;
		    } else if(defaultState) {
			    curState = defaultState;                // second choice, go to the default state
			    status   = FSM_EXE_ERR_TODEFAULT;
		    } else {
			    curState = getFirstValidState();        // third choice, transfer to the first valid state in the set
			    status   = FSM_EXE_ERR_TO1STVALID;
		    }					
	    }
    } else {
        status = FSM_EXE_ERR_NOTRANS;
    }

    // execute the exit function and transient function
    if(preState && curState != preState) {
        if(curState) 
            preState -> setDestStateCode(curState -> getStateCode());
        else
            preState -> setDestStateCode(-1);

        preState -> exit();
    }

    // execute the extended function
    executeExtFunc();

	return status;
}

//-----------------------------------------------
// print the info of the FSM
//-----------------------------------------------
void FSM::printFSM()
{
    int i;

    cout << "-------------------------------------\n";
    cout << "FSM Name: " << fsmName << endl;
    cout << "-------------------------------------\n";
    for(i = 0; i < FSM_MAX_NUM_STATES; i ++) {
        if(stateSet[i]) stateSet[i] -> printState();       
    }
    cout << "-------------------------------------\n";
}

//-----------------------------------------------
// register the state to the FSM
// return: 0 - success, 1 - failure
//-----------------------------------------------
int FSM::registerState(State *state, int stateCode)
{
    if(!state || stateCode < 0 || stateCode >= FSM_MAX_NUM_STATES)
        return 1;

    stateSet[stateCode] = state;

    return 0;
}

//-----------------------------------------------
// init the current state
// return: 0 - success, 1 - failure
//-----------------------------------------------
int FSM::initCurrentState(int stateCode, int sel)
{
    if(stateCode < 0 || stateCode >= FSM_MAX_NUM_STATES)
        return 1;

    switch(sel) {
        case FSM_INIT_CURR_ENTRY_EXE:
            preState = NULL;
            curState = stateSet[stateCode];
            break;

        case FSM_INIT_CURR_ENTRY_NOEXE:
            preState = stateSet[stateCode];
            curState = stateSet[stateCode];
            break;

        case FSM_INIT_CURR_FORCE_TRANS:
            preState = curState;
            curState = stateSet[stateCode];
            break;

        default:
            break;
    }
    
    return 0;
}

//-----------------------------------------------
// set the default state
// return: 0 - success, 1 - failure
//-----------------------------------------------
int FSM::setDefaultState(int stateCode)
{
    if(stateCode < 0 || stateCode >= FSM_MAX_NUM_STATES)
        return 1;

    defaultState = stateSet[stateCode];

    return 0;
}

//-----------------------------------------------
// get the current state code
//-----------------------------------------------
int FSM::getCurrentStateCode()
{
    if(curState) 
        return curState -> getStateCode(); 
    else 
        return -1;
}

//-----------------------------------------------
// register and execute the job
//-----------------------------------------------
void FSM::registerJob(Job *job, int jobCode) 
{
	if(jobCode >= 0 && jobCode < FSM_MAX_NUM_JOBS) 
		jobSet[jobCode] = job;
}

int  FSM::executeJob(int jobCode, int flag) 
{
	if(jobCode >= 0 && jobCode < FSM_MAX_NUM_JOBS && jobSet[jobCode]) 
		return jobSet[jobCode] -> execute(flag);
	else 
		return -1;
}

void FSM::enableAllJobs(int enabled)
{
    for(int i = 0; i < FSM_MAX_NUM_JOBS; i ++) {
        if(jobSet[i])
            jobSet[i] -> enableJob((short)enabled);
    }
}

void FSM::enableJob(int jobCode, int enabled)
{
    if(jobCode >= 0 && jobCode < FSM_MAX_NUM_JOBS && jobSet[jobCode]) 
        jobSet[jobCode] -> enableJob((short)enabled);
}

//-----------------------------------------------
// event handling functions
//-----------------------------------------------
void FSM::sendEvent              ()                                                         {var_event.sendEvent();}
void FSM::sendEvent              (int eventCode, int cmd, int subCmd)                       {var_event.sendEvent(eventCode, cmd, subCmd);}
void FSM::waitEvent              ()                                                         {var_event.recvEvent();}
void FSM::waitEvent              (int *eventCode, int *cmd, int *subCmd)                    {var_event.recvEvent(eventCode, cmd, subCmd);}
void FSM::waitEventWithTimeout   (double timeout)                                           {var_event.recvEventWithTimeout(timeout);}
void FSM::waitEventWithTimeout   (int *eventCode, int *cmd, int *subCmd, double timeout)    {var_event.recvEventWithTimeout(eventCode, cmd, subCmd, timeout);}
int  FSM::withPendingEvents      ()                                                         {return var_event.getUnsolvedMsgNum() > 0 ? 1 : 0;}
int  FSM::isMsgQFull             ()                                                         {return var_event.getUnsolvedMsgNum() > FSM_EVENT_MAX_MSG_NUM-2 ? 1 : 0;}

//-----------------------------------------------
// private functions
//-----------------------------------------------
State *FSM::getFirstValidState() 
{
	int i;
	State *stat = NULL;

    for(i = 0; i < FSM_MAX_NUM_STATES; i ++) {
        if(stateSet[i]) {
            stat = stateSet[i];  
			break;     
		}
    }

	return stat;
}

//-----------------------------------------------
// callback function for the timer
//-----------------------------------------------
void FSM::fun_timerCallback(void *arg)
{
    FSMEvent *evt = (FSMEvent *)arg;
    if(!evt)
        return;

    evt -> sendEvent();
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************




