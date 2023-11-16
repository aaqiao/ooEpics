//=========================================================
// FSM_Example.cc
// 
// Class realization for the example state machine
// Auto created from the template of ooEpics framework
//=========================================================
#include "FSM_Example.h"

using namespace std;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// STATE ExampleState_OFF
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction, destruction and exit
//-----------------------------------------------
ExampleState_OFF::ExampleState_OFF(FSM_Example *ownerIn) : 
    OOEPICS::State(FSMEXM_STATECODE_OFF, "FSMEXM_STATE_OFF")    {owner = ownerIn;}
ExampleState_OFF::~ExampleState_OFF()                           {}
int ExampleState_OFF::exit()                                    {return FSM_EXE_SUCCESS;}

//-----------------------------------------------
// entry
//-----------------------------------------------
int ExampleState_OFF::entry()
{
    // general function that should be executed if the 
    // transient destination is OFF: 
    // switch off the light
    if(owner)
        owner -> action_setLightIntensity(0.0);

    return FSM_EXE_SUCCESS;
}

//-----------------------------------------------
// do
//-----------------------------------------------
int ExampleState_OFF::do_activity()
{
    STATECODE_Example currState = (STATECODE_Example)getStateCode();
    STATECODE_Example nextState;

    // check the owner FSM
    if(!owner) {
        EPICSLIB_func_epicsThreadSleep(FSM_MIN_TICK_TIME);          // avoid CPU load blowing up
        return currState;                                           // stay in the current state
    }

    // wait for event, this will suspend the thread
    owner -> waitEvent();

    // check conditions for transition
    //  - user command
    if(owner -> checkAndClearUserCommand()) {
        if(owner -> getUserCommand() == FSMEXM_CMD_ON) {
            nextState = FSMEXM_STATECODE_RAMP;
            return nextState;
        }
    }

    // stay in the current state
    return currState;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// STATE ExampleState_RAMP
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction, destruction, entry and exit
//-----------------------------------------------
ExampleState_RAMP::ExampleState_RAMP(FSM_Example *ownerIn) : 
    OOEPICS::State(FSMEXM_STATECODE_RAMP, "FSMEXM_STATE_RAMP")  {owner = ownerIn;}
ExampleState_RAMP::~ExampleState_RAMP()                         {}
int ExampleState_RAMP::entry()                                  {return FSM_EXE_SUCCESS;}
int ExampleState_RAMP::exit()                                   {return FSM_EXE_SUCCESS;}

//-----------------------------------------------
// do
//-----------------------------------------------
int ExampleState_RAMP::do_activity()
{
    int delay_s;

    STATECODE_Example currState = (STATECODE_Example)getStateCode();
    STATECODE_Example nextState;

    // check the owner FSM
    if(!owner) {
        EPICSLIB_func_epicsThreadSleep(FSM_MIN_TICK_TIME);
        return currState;
    }

    // wait for event
    owner -> waitEvent();

    // check conditions for transition
    //  - user command
    if(owner -> checkAndClearUserCommand()) {
        if(owner -> getUserCommand() == FSMEXM_CMD_OFF) {
            nextState = FSMEXM_STATECODE_OFF;
            return nextState;
        }
    }

    // - ramp the light with delays
    for(delay_s = 9; delay_s >= 1; delay_s--) {
        if(withDelay(delay_s)) {
            owner -> action_setLightIntensity(delay_s * 10.0);
            break;
        }
    }

    //  - check if the stay time in this state has expire 10s or not
    if(withDelay(10)) {
        nextState = FSMEXM_STATECODE_ON;
        return nextState;
    }

    // stay in the current state
    return currState;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// STATE ExampleState_ON
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction, destruction and exit
//-----------------------------------------------
ExampleState_ON::ExampleState_ON(FSM_Example *ownerIn) : 
    OOEPICS::State(FSMEXM_STATECODE_ON, "FSMEXM_STATE_ON")  {owner = ownerIn;}
ExampleState_ON::~ExampleState_ON()                         {}
int ExampleState_ON::exit()                                 {return FSM_EXE_SUCCESS;}

//-----------------------------------------------
// entry
//-----------------------------------------------
int ExampleState_ON::entry()
{
    if(owner)
        owner -> action_setLightIntensity(100.0);

    return FSM_EXE_SUCCESS;
}

//-----------------------------------------------
// do
//-----------------------------------------------
int ExampleState_ON::do_activity()
{
    STATECODE_Example currState = (STATECODE_Example)getStateCode();
    STATECODE_Example nextState;

    // check the owner FSM
    if(!owner) {
        EPICSLIB_func_epicsThreadSleep(FSM_MIN_TICK_TIME);
        return currState;
    }

    // wait for event
    owner -> waitEvent();

    // check conditions for transition
    //  - user command
    if(owner -> checkAndClearUserCommand()) {
        if(owner -> getUserCommand() == FSMEXM_CMD_OFF) {
            nextState = FSMEXM_STATECODE_OFF;
            return nextState;
        }
    }

    // stay in the current state
    return currState;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THREAD CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// realize the thread class
//-----------------------------------------------
FSMEXM_Thread::FSMEXM_Thread(const char *name, FSM_Example *ownerInput, int priority) :
    thread(*this, name, epicsThreadGetStackSize(epicsThreadStackBig), priority)
{
    owner = ownerInput;
    strcpy(trdName, name);
    cout << "INFO:FSMEXM_Thread: thread " << trdName << " created." << endl;
}

FSMEXM_Thread::~FSMEXM_Thread() 
{
    cout << "INFO:FSMEXM_Thread: thread " << trdName << " deleted." << endl;
}

void FSMEXM_Thread::start() 
{
    cout << "INFO:FSMEXM_Thread: thread " << trdName << " started." << endl;
    thread.start();
}

void FSMEXM_Thread::run()
{
    if(owner) {
        owner -> threadFunc_FSMEXM();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// FSM LIGHT CONTROL
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction
//-----------------------------------------------
FSM_Example::FSM_Example(const char *modNameIn, const char *fsmNameIn, int priority) : 
    OOEPICS::FSM        (modNameIn, fsmNameIn),
    var_state_OFF       (this),
    var_state_RAMP      (this),
    var_state_ON        (this),
    var_thread_FSMEXM   ("TRD_FSMEXM", this, priority)
{
    // init the data
    var_userCommand         = FSMEXM_CMD_OFF;
    var_userCommandArrived  = 0;

    var_FSMEXMThreadStopCmd = 0;
    var_FSMEXMThreadStopped = 1;

    // init the local PV I/O scan drivers before initializting the local PVs
    EPICSLIB_func_scanIoInit(&var_scanResultPVs);

    // init the local PVs
    initLocalPVs();

    cout << "INFO:FSM_Example: FSM " << fsmName << " created." << endl;
}

//-----------------------------------------------
// destruction
//-----------------------------------------------
FSM_Example::~FSM_Example() 
{
    int waitCnt = 0;

    // first stop the thread
    var_FSMEXMThreadStopCmd = 1;
    sendEvent();                 		                                            // be sure not to suspend the thread

    cout << "INFO:FSM_Example: Wait for the thread to stop ..." << endl;
    while(!var_FSMEXMThreadStopped && waitCnt < 30)                                 // 30s timeout
    {
        EPICSLIB_func_epicsThreadSleep(1.0);
        waitCnt ++;        
    } 

    if(waitCnt >= 30) {
        cout << "ERROR:FSM_Example: Failed to stop the thread!" << endl;
    } else {
        cout << "INFO:FSM_Example: FSM " << fsmName << " deleted." << endl;
    }
}

//-----------------------------------------------
// thread implementation
//-----------------------------------------------
void FSM_Example::threadFunc_FSMEXM()
{
    // init the stop flag
    var_FSMEXMThreadStopCmd = 0;
    var_FSMEXMThreadStopped = 0;

    // main loop
    while(1) { 
        // implement the stop
        if(var_FSMEXMThreadStopCmd)
            break;

        // execute the FSM
        executeFSM();
    }

    var_FSMEXMThreadStopped = 1;
    cout << "INFO:FSM_Example: Thread stopped." << endl;
}

//-----------------------------------------------
// init the local PVs
//-----------------------------------------------
void FSM_Example::initLocalPVs()
{
    string modn(modName);
    string devn(fsmName);
    string statename("OFF;RAMP;ON");

    // create the local PVs, the definition is as below
    // void init( string                  moduleName, 
    //            string                  devName, 
    //            string                  valName,
    //            string                  supStr,
    //            string                  unitStr,
    //            unsigned int            pointNum,
    //            void                   *privatePtr,
    //            INTD_CALLBACK           callback,
    //            INTD_enum_recordType    recordType,
    //            INTD_enum_scanType      scanType,
    //            IOSCANPVT              *ioIntScan, 
    //            epicsMutexId            mutexId, 
    //            epicsEventId            eventId);      
    lpv_currentState.init   (modn, devn, "MON-CURR-STATE",   statename, "",  1, (void *)this, NULL, INTD_MBBI, INTD_IOINT, &var_scanResultPVs, NULL, NULL);
    lpv_lightIntensity.init (modn, devn, "MON-LIGHT-INTENSITY",     "", "%", 1, (void *)this, NULL, INTD_AI,   INTD_IOINT, &var_scanResultPVs, NULL, NULL);
}

//-----------------------------------------------
// init the FSM
//-----------------------------------------------
int FSM_Example::initFSM()
{
    registerState(&var_state_OFF,   FSMEXM_STATECODE_OFF);
    registerState(&var_state_RAMP,  FSMEXM_STATECODE_RAMP);
    registerState(&var_state_ON,    FSMEXM_STATECODE_ON);

    initCurrentState(FSMEXM_STATECODE_OFF, FSM_INIT_CURR_ENTRY_EXE);  // init the current state
    setDefaultState (FSMEXM_STATECODE_OFF);                           // when there is errors in state transition, the FSM will transfer to the default state

    return FSM_EXE_SUCCESS;
}

//-----------------------------------------------
// execute the extended function, this will be called each time the FSM is executed
//-----------------------------------------------
int FSM_Example::executeExtFunc()
{
    // update the current state mbbi
    int currentState = getCurrentStateCode();
    lpv_currentState.setValue(currentState);

    // process the PV for displaying
    updateResultPVs();
    return FSM_EXE_SUCCESS;
}

//-----------------------------------------------
// update the local PVs for results
//-----------------------------------------------
void FSM_Example::updateResultPVs() 
{
    if(var_scanResultPVs)
        EPICSLIB_func_scanIoRequest(var_scanResultPVs);
}

//-----------------------------------------------
// start the thread
//-----------------------------------------------
void FSM_Example::startThread()
{
    var_thread_FSMEXM.start();
}

//-----------------------------------------------
// accept user command and send an event
//-----------------------------------------------
void FSM_Example::userCommand(int cmd)
{
    var_userCommand         = cmd;
    var_userCommandArrived  = 1;
    sendEvent();
}

//-----------------------------------------------
// check if the user command arrived or not
//-----------------------------------------------
int FSM_Example::checkAndClearUserCommand()
{
    int userCommandArrived = var_userCommandArrived;
    var_userCommandArrived = 0;
    return userCommandArrived;
}

//-----------------------------------------------
// get the user command
//-----------------------------------------------
int FSM_Example::getUserCommand()
{
    return var_userCommand;
}

//-----------------------------------------------
// actions that will be executed in the entry/do/exit functions of states
// return: 0 - success; 1 - failure
//-----------------------------------------------
// set light intensity
int FSM_Example::action_setLightIntensity(double percent)
{
    if(percent < 0.0 || percent > 100.0)
        return 1;

    lpv_lightIntensity.setValue(percent);
    updateResultPVs();
    
    return 0;
}









