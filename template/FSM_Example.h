//=========================================================
// FSM_Example.h
// 
// Class definition for the example state machine
// Auto created from the template of ooEpics framework
//=========================================================
#ifndef FSM_EXAMPLE_H
#define FSM_EXAMPLE_H

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <fstream>

#include "FSM.h"
#include "LocalPV.h"
#include "ooEpicsMisc.h"
#include "MessageLogs.h"
#include "EPICSLib_wrapper.h"

#include "ModuleDefs___MODULE_NAME__.h"

#define FSMEXM_CMD_OFF 0                // user command, switch off the light
#define FSMEXM_CMD_ON  1                // user command, switch on the light

//-----------------------------------------------
// forward declare
//-----------------------------------------------
class FSM_Example;

//-----------------------------------------------
// state code
//-----------------------------------------------
typedef enum {
    FSMEXM_STATECODE_OFF,               // light is off
    FSMEXM_STATECODE_RAMP,              // light is ramping
    FSMEXM_STATECODE_ON                 // light is on
} STATECODE_Example;

//-----------------------------------------------
// derived classes for states
//-----------------------------------------------
// class definition for the state of ExampleState_OFF
class ExampleState_OFF : public OOEPICS::State
{
public:
    ExampleState_OFF            (FSM_Example *ownerIn);
    virtual ~ExampleState_OFF   ();

    virtual int entry           ();
    virtual int do_activity     ();
    virtual int exit            ();

private:
    FSM_Example *owner;
};

// class definition for the state of ExampleState_RAMP
class ExampleState_RAMP : public OOEPICS::State
{
public:
    ExampleState_RAMP           (FSM_Example *ownerIn);
    virtual ~ExampleState_RAMP  ();

    virtual int entry           ();
    virtual int do_activity     ();
    virtual int exit            ();

private:
    FSM_Example *owner;
};

// class definition for the state of ExampleState_ON
class ExampleState_ON : public OOEPICS::State
{
public:
    ExampleState_ON             (FSM_Example *ownerIn);
    virtual ~ExampleState_ON    ();

    virtual int entry           ();
    virtual int do_activity     ();
    virtual int exit            ();

private:
    FSM_Example *owner;
};

//-----------------------------------------------
// class of thread for the FSM_Example
//-----------------------------------------------
class FSMEXM_Thread : public epicsThreadRunable
{
public:
    FSMEXM_Thread(const char *name, FSM_Example *ownerInput, int priority);
    virtual ~FSMEXM_Thread();

    virtual void run();
    void start();

private:
    epicsThread thread;
    FSM_Example *owner;

    char trdName[128];
};

//-----------------------------------------------
// class for FSM_Example
//-----------------------------------------------
class FSM_Example : public OOEPICS::FSM
{
public:
    FSM_Example(const char *modNameIn, const char *fsmNameIn, int priority);
    virtual ~FSM_Example();

    // implement the virtual functions
    virtual int initFSM             (); 
    virtual int executeExtFunc      ();   

    // interface functions for users
    void startThread                ();             // start the thread
    void userCommand                (int cmd);      // input the user command

    // interface functions for states
    int  checkAndClearUserCommand   ();
    int  getUserCommand             ();

    int  action_setLightIntensity   (double percent);
  
    // routine of thread
    void threadFunc_FSMEXM();

private:
    // instance the states 
    ExampleState_OFF    var_state_OFF;
    ExampleState_RAMP   var_state_RAMP;
    ExampleState_ON     var_state_ON;

    // local PVs
    OOEPICS::LocalPV lpv_currentState;                              // mbbi, current state displaying
    OOEPICS::LocalPV lpv_lightIntensity;                            // ai, display the light intensity in percent

    // I/O interrupt scanning driver for local PVs
    EPICSLIB_type_ioScanPvt var_scanResultPVs;    

    // private variables
    int var_userCommand;
    int var_userCommandArrived;

    // variables for thread
    FSMEXM_Thread var_thread_FSMEXM;                                // object of the thread

    int var_FSMEXMThreadStopCmd;                                    // thread stopping control
    int var_FSMEXMThreadStopped;

    // private general routines
    void initLocalPVs();
    void updateResultPVs();   
};


#endif



