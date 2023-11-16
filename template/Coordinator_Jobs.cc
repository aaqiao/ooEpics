//=========================================================
// Coordinator_Jobs.cc
// 
// Implementation of the job execution coordinator class
// Auto created from the template of ooEpics framework
//=========================================================
#include "Coordinator_Jobs.h"

using namespace std;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// THREAD CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// realize the thread class
//-----------------------------------------------
CRDJOBS_Thread::CRDJOBS_Thread(const char *name, Coordinator_Jobs *ownerInput, int priority) :
    thread(*this, name, epicsThreadGetStackSize(epicsThreadStackBig), priority)
{
    owner = ownerInput;
    strcpy(trdName, name);
    cout << "INFO:CRDJOBS_Thread: thread " << trdName << " created." << endl;
}

CRDJOBS_Thread::~CRDJOBS_Thread() 
{
    cout << "INFO:CRDJOBS_Thread: thread " << trdName << " deleted." << endl;
}

void CRDJOBS_Thread::start() 
{
    cout << "INFO:CRDJOBS_Thread: thread " << trdName << " started." << endl;
    thread.start();
}

void CRDJOBS_Thread::run()
{
    if(owner) {
        owner -> threadFunc_CRDJOBS();
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// COORDINATOR CLASS
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction
//-----------------------------------------------
Coordinator_Jobs::Coordinator_Jobs( const char *modNameIn, 
                                    const char *crdNameIn, 
                                    int priority,
                                    OOEPICS::ChannelAccessContext *caContextIn) : 
    OOEPICS::Coordinator(modNameIn, crdNameIn),
    var_thread_CRDJOBS("TRD_CRDJOBS", this, priority)
{
    ptr_caContext = caContextIn;

    var_CRDJOBSThreadStopCmd = 0;
    var_CRDJOBSThreadStopped = 1;

    cout << "INFO:Coordinator_Jobs: Coordinator " << crdName << " created." << endl;
}

//-----------------------------------------------
// destruction
//-----------------------------------------------
Coordinator_Jobs::~Coordinator_Jobs() 
{
    int waitCnt = 0;

    // first stop the thread
    var_CRDJOBSThreadStopCmd = 1;
    sendEvent(CRDJOBS_EVENT_JOB, CRDJOBS_JOBCODE_NULL, 0);                 		    // be sure not to suspend the thread

    cout << "INFO:Coordinator_Jobs: Wait for the thread to stop ..." << endl;
    while(!var_CRDJOBSThreadStopped && waitCnt < 30)                                // 30s timeout
    {
        EPICSLIB_func_epicsThreadSleep(1.0);
        waitCnt ++;        
    } 

    if(waitCnt >= 30) {
        cout << "ERROR:Coordinator_Jobs: Failed to stop the thread!" << endl;
    } else {
        cout << "INFO:Coordinator_Jobs: Coordinator " << crdName << " deleted." << endl;
    }
}

//-----------------------------------------------
// thread implementation
//-----------------------------------------------
void Coordinator_Jobs::threadFunc_CRDJOBS()
{
    // attach this thread to the channel access context
    if(ptr_caContext) {
        ptr_caContext -> attachCurrentThread();
    } else {
        cout << "ERROR:Coordinator_Jobs: Channel Access context does not exist!" << endl;
        return;
    }

    // wait for the iocInit done
    while(!INTD_API_getIocInitStatus()) {
        EPICSLIB_func_epicsThreadSleep(1.0);
    }

    cout << "INFO:Coordinator_Jobs: IOC init finished." << endl;

    // synch PV values with internal data (callback disabled)
    INTD_API_syncWithRecords(0);

    cout << "INFO:Coordinator_Jobs: Internal variables synchronized with records." << endl;

    // init the stop flag
    var_CRDJOBSThreadStopCmd = 0;
    var_CRDJOBSThreadStopped = 0;

    // main loop
    while(1) { 
        // implement the stop
        if(var_CRDJOBSThreadStopCmd)
            break;

        // execute the coordinator
        execute();
    }

    var_CRDJOBSThreadStopped = 1;
    cout << "INFO:Coordinator_Jobs: Thread stopped." << endl;
}

//-----------------------------------------------
// start the thread
//-----------------------------------------------
void Coordinator_Jobs::startThread()
{
    var_thread_CRDJOBS.start();
}

//-----------------------------------------------
// execution
//-----------------------------------------------
int Coordinator_Jobs::execute()
{
    int status = __MODULE_NAME___JOB_FAILED;
    int evt, cmd, subCmd;   

    // wait for the event
    waitEvent(&evt, &cmd, &subCmd);

    // response to the event
    switch(evt) {
        case CRDJOBS_EVENT_JOB:
            status = executeJob(cmd, subCmd);                   // "cmd" is the job code; "subCmd" is the flag passing into "execute" function of job
            break;
        
        default:
            EPICSLIB_func_epicsThreadSleep(0.1);
            break;
    }

    return status;
}

   




