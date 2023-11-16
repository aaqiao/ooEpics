//=========================================================
// Job_Example.cc
//  
// This is an example of a job based on the OOEPICS framework
// Auto created from the template of ooEpics framework
//=========================================================
#include "Job_Example.h"

using namespace std;

//-----------------------------------------------
// construction
//-----------------------------------------------
Job_Example::Job_Example(const char *moduleName, const char *jobName) : 
    OOEPICS::Job(moduleName, jobName)
{
    // init all data values
    jobEnabled      = 1;                                            // set the job default to be enabled

    ptr_srvExample  = NULL;
    ptr_msgLog      = NULL;

    var_valCnt1S    = 0;
    var_valCnt2S    = 0;
    var_valCnt5S    = 0;

    memset((void *)buf_wfo, 0, sizeof(double) * SRVEXAM_BUF_SIZE);
    memset((void *)buf_wfx, 0, sizeof(double) * SRVEXAM_BUF_SIZE);
    memset((void *)buf_wfi, 0, sizeof(double) * SRVEXAM_BUF_SIZE);

    // init the local PV I/O scan drivers and locks before initializting the local PVs
    var_lockParameters = EPICSLIB_func_mutexMustCreate();
    EPICSLIB_func_scanIoInit(&var_scanResultPVs);

    // init the local PVs
    initLocalPVs();
    
    // message
    cout << "INFO:Job_Example: Object " << jobName << " created for module " << modName << "." << endl;
}

//-----------------------------------------------
// destruction
//-----------------------------------------------
Job_Example::~Job_Example()
{
    // destroy the epics mutex
    if(var_lockParameters)
        EPICSLIB_func_mutexDestroy(var_lockParameters);

    // message
    cout << "INFO:Job_Example: Object " << jobName << " for module " << modName << " deleted." << endl;
}

//-----------------------------------------------
// init the local PVs
//-----------------------------------------------
void Job_Example::initLocalPVs()
{
    string modn(modName);
    string devn(jobName);
    string funsel("Choice1;Choice2;Choice3;Choice4;Choice5;Choice6;Choice7;Choice8");           // maximum 16 strings for choices
    string fundis("Dis1;Dis2;Dis3");                                                            // maximum 16 strings for displaying

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

    // it is necessary for the waveform records to specify the data type
    lpv_waveformoutExample.setWfDataType(INTD_DOUBLE);
    lpv_xaxis.setWfDataType             (INTD_DOUBLE);
    lpv_waveforminExample.setWfDataType (INTD_DOUBLE);

    // initalize the local PVs
    lpv_enableDebug.init        (modn, devn, "ENA-DEBUG",       "", "",                         1, (void *)this,         NULL, INTD_BO,  INTD_PASSIVE,             NULL, var_lockParameters, NULL);

    lpv_boExample.init          (modn, devn, "EXAMPLE-BO",      "", "",                         1, (void *)this,         NULL, INTD_BO,  INTD_PASSIVE,             NULL, var_lockParameters, NULL);
    lpv_mbboExample.init        (modn, devn, "EXAMPLE-MBBO",funsel, "",                         1, (void *)this,         NULL, INTD_MBBO,INTD_PASSIVE,             NULL, var_lockParameters, NULL);
    lpv_longoutExample.init     (modn, devn, "EXAMPLE-LO",      "", "DEG",                      1, (void *)this,         NULL, INTD_LO,  INTD_PASSIVE,             NULL, var_lockParameters, NULL);
    lpv_aoExample.init          (modn, devn, "EXAMPLE-AO",      "", "GeV",                      1, (void *)this,         NULL, INTD_AO,  INTD_PASSIVE,             NULL, var_lockParameters, NULL);

    lpv_waveformoutExample.init (modn, devn, "EXAMPLE-WFO",     "", "DIG",      SRVEXAM_BUF_SIZE,  (void *)this,         NULL, INTD_WFO, INTD_PASSIVE,             NULL, var_lockParameters, NULL);
    lpv_stringoutExample.init   (modn, devn, "EXAMPLE-SO",      "", "",      INTD_STR_RECORD_LEN,  (void *)this,         NULL, INTD_SO,  INTD_PASSIVE,             NULL, var_lockParameters, NULL);

    lpv_biExample.init          (modn, devn, "EXAMPLE-BI",      "", "",                         1, (void *)this,         NULL, INTD_BI,  INTD_IOINT, &var_scanResultPVs,               NULL, NULL);
    lpv_mbbiExample.init        (modn, devn, "EXAMPLE-MBBI",fundis, "",                         1, (void *)this,         NULL, INTD_MBBI,INTD_IOINT, &var_scanResultPVs,               NULL, NULL);
    lpv_longinExample.init      (modn, devn, "EXAMPLE-LI",      "", "DEG",                      1, (void *)this,         NULL, INTD_LI,  INTD_IOINT, &var_scanResultPVs,               NULL, NULL);
    lpv_aiExample.init          (modn, devn, "EXAMPLE-AI",      "", "GeV",                      1, (void *)this,         NULL, INTD_AI,  INTD_IOINT, &var_scanResultPVs,               NULL, NULL);
    
    lpv_xaxis.init              (modn, devn, "EXAMPLE-WFX",     "", "SEC",       SRVEXAM_BUF_SIZE, (void *)this,         NULL, INTD_WFI, INTD_IOINT, &var_scanResultPVs,               NULL, NULL);
    lpv_waveforminExample.init  (modn, devn, "EXAMPLE-WFI",     "", "DIG",       SRVEXAM_BUF_SIZE, (void *)this,         NULL, INTD_WFI, INTD_IOINT, &var_scanResultPVs,               NULL, NULL);
    lpv_stringinExample.init    (modn, devn, "EXAMPLE-SI",      "", "",       INTD_STR_RECORD_LEN, (void *)this,         NULL, INTD_SI,  INTD_IOINT, &var_scanResultPVs,               NULL, NULL);

    lpv_counter1S.init          (modn, devn, "CNT1S",           "", "",                         1, (void *)this, lpvRcb_cnt1S, INTD_AI,  INTD_1S,                  NULL,               NULL, NULL);
    lpv_counter2S.init          (modn, devn, "CNT2S",           "", "",                         1, (void *)this, lpvRcb_cnt2S, INTD_AI,  INTD_2S,                  NULL,               NULL, NULL);
    lpv_counter5S.init          (modn, devn, "CNT5S",           "", "",                         1, (void *)this, lpvRcb_cnt5S, INTD_AI,  INTD_5S,                  NULL,               NULL, NULL);

    lpv_timeStr.init            (modn, devn, "MON-TIME-STR",    "", "",       INTD_STR_RECORD_LEN, (void *)this,         NULL, INTD_SI,  INTD_IOINT, &var_scanResultPVs,               NULL, NULL);
    lpv_msgStr.init             (modn, devn, "MON-MSG-STR",     "", "",       INTD_STR_RECORD_LEN, (void *)this,         NULL, INTD_SI,  INTD_IOINT, &var_scanResultPVs,               NULL, NULL);

    lpv_enableJob.init          (modn, devn, "MON-ENA-JOB",     "", "",                         1, (void *)this,         NULL, INTD_BI,  INTD_IOINT, &var_scanResultPVs,               NULL, NULL);

    // assign default values after the initialization
    lpv_enableJob.setValue(jobEnabled);
}

//-----------------------------------------------
// set the associations
//-----------------------------------------------
void Job_Example::setService_Example    (Service_Example        *srvExampleIn)  {ptr_srvExample = srvExampleIn;}
void Job_Example::setMessageLogging     (OOEPICS::MessageLogs   *msgLogIn)      {ptr_msgLog     = msgLogIn;}

//-----------------------------------------------
// extend function when the job is enabled/disabled
//-----------------------------------------------
void Job_Example::enableJobExt()
{
    updateResultPVs();
}

//-----------------------------------------------
// implement the virtual function to execute the job which will be 
// called by the high level modules like the coordinator or FSM
//-----------------------------------------------
int Job_Example::execute(int flag)
{
    int i;
    int status = __MODULE_NAME___RPVACCESS_FAILED;

    // get the parameter settings from OPVs
    short  enableDebug      = lpv_enableDebug.getValueUInt16();    
    
    short  boExample        = lpv_boExample.getValueUInt16();
    short  mbboExample      = lpv_mbboExample.getValueUInt16();
    int    longoutExample   = lpv_longoutExample.getValueInt32();
    double aoExample        = lpv_aoExample.getValueFloat64();

    string stringoutExample = lpv_stringoutExample.getValueString();

    lpv_waveformoutExample.getValues(buf_wfo, SRVEXAM_BUF_SIZE);

    // variables to store the value of IPVs
    short  biExample;
    short  mbbiExample;
    int    longinExample;
    double aiExample;
    char   stringinExample[INTD_STR_RECORD_LEN];

    // check if the Job is enabled or not
    if(!jobEnabled) {
        postMsg(ptr_msgLog, enableDebug, __MODULE_NAME___MSG_WARN, "Job is not active! Skipped!");
        updateResultPVs();
        return __MODULE_NAME___JOB_NOT_ACTIVE;
    } 

    // check the service
    if(!ptr_srvExample) {
        postMsg(ptr_msgLog, enableDebug, __MODULE_NAME___MSG_ERR, "Invalid Service_Example!");
        updateResultPVs();
        return __MODULE_NAME___JOB_FAILED;
    }

    postMsg(ptr_msgLog, enableDebug, __MODULE_NAME___MSG_INFO, "Execute example job.");

    // execute the job based on the selections
    switch(flag) {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // SET VALUES TO REMOTE IOC
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case JOBEXAM_SET_VALUE:

            // set the value via the service
            status = ptr_srvExample -> setRemoteDataPull(   boExample,
                                                            mbboExample,
                                                            longoutExample,
                                                            aoExample,
                                                            buf_wfo,
                                                            stringoutExample.c_str(),
                                                            SRVEXAM_BUF_SIZE);

            break;

        //~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // GET VALUES FROM REMOTE IOC
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~
        case JOBEXAM_GET_VALUE:

            // get the values via pulling
            status = ptr_srvExample -> getRemoteDataPull(   &longinExample, 
                                                            &aiExample,
                                                            buf_wfi,
                                                            SRVEXAM_BUF_SIZE);

            if(status == __MODULE_NAME___RPVACCESS_SUCCESS) {
                lpv_longinExample.setValue      (longinExample);
                lpv_aiExample.setValue          (aiExample);
                lpv_waveforminExample.setValues (buf_wfi, SRVEXAM_BUF_SIZE);

                // display the data for debugging
                cout << "Data pulling read with CA:" << endl;
                cout << " LI: " << longinExample << endl;
                cout << " AI: " << aiExample << endl;
                cout << " WFI: ";

                for(i = 0; i < SRVEXAM_BUF_SIZE; i ++)
                    cout << buf_wfi[i] << " ";
                cout << endl;

            } else {
                postMsg(ptr_msgLog, enableDebug, __MODULE_NAME___MSG_ERR, "Failed to get remote data by pulling!");
                break;
            }

            // get the values via monitoring
            status = ptr_srvExample -> getRemoteDataMonitor((epicsUInt16 *)&biExample,
                                                            (epicsUInt16 *)&mbbiExample,
                                                            buf_wfx,
                                                            stringinExample,
                                                            SRVEXAM_BUF_SIZE);

            if(status == __MODULE_NAME___RPVACCESS_SUCCESS) {
                lpv_biExample.setValue          (biExample);
                lpv_mbbiExample.setValue        (mbbiExample);
                lpv_xaxis.setValues             (buf_wfx, SRVEXAM_BUF_SIZE);
                lpv_stringinExample.setString   (stringinExample);

                // display the data for debugging
                cout << "Data monitoring read with CA:" << endl;
                cout << " BI:   " << biExample << endl;
                cout << " MBBI: " << mbbiExample << endl;
                cout << " SI:   " << stringinExample << endl;
                cout << " WFX: ";

                for(i = 0; i < SRVEXAM_BUF_SIZE; i ++)
                    cout << buf_wfx[i] << " ";
                cout << endl;

            } else {
                postMsg(ptr_msgLog, enableDebug, __MODULE_NAME___MSG_ERR, "Failed to get remote data by monitoring!");
                break;
            }

            break;

        default:
            postMsg(ptr_msgLog, enableDebug, __MODULE_NAME___MSG_ERR, "Invalid execution command!");
            break;
    }

    // scan the result PVs 
    updateResultPVs();

    // return
    if(status == __MODULE_NAME___RPVACCESS_SUCCESS) {
        postMsg(ptr_msgLog, enableDebug, __MODULE_NAME___MSG_INFO, "Execute example job done.");
        return __MODULE_NAME___JOB_SUCCESS;
    } else {
        postMsg(ptr_msgLog, enableDebug, __MODULE_NAME___MSG_ERR, "Execute example job failure!");
        return __MODULE_NAME___JOB_FAILED;   
    }
}

//-----------------------------------------------
// update the local PVs for results
//-----------------------------------------------
void Job_Example::updateResultPVs() 
{
    if(var_scanResultPVs)
        EPICSLIB_func_scanIoRequest(var_scanResultPVs);
}

//-----------------------------------------------
// post message and print debug info
//-----------------------------------------------
void Job_Example::postMsg(OOEPICS::MessageLogs *msgLogIn, short debugEnabled, short infoErr, string msg)
{
    char timeStr[20], timeStr2[30];
    string logMsg;

	// get the current message
    OOEPICS::getTimeString(timeStr, 1, "%4d%2d%2d%2d%2d%2d");
    OOEPICS::stringReplace(timeStr, ' ', '0');

    OOEPICS::getTimeString(timeStr2, 1, "%4d-%2d-%2d,%2d:%2d:%2d");
    OOEPICS::stringReplace(timeStr2, ' ', '0');

    // The formal log message
    if(msgLogIn) {
        switch(infoErr) {
            case __MODULE_NAME___MSG_INFO:  logMsg.assign("JOBEXAM_INFO: "); break;
            case __MODULE_NAME___MSG_WARN:  logMsg.assign("JOBEXAM_WARN: "); break;
            default:                       logMsg.assign("JOBEXAM_ERR: ");  break;
        }

        logMsg += msg; 
        msgLogIn -> postMessage((char *)logMsg.c_str());    
    }

    // print the debug message
    if(debugEnabled) {
        switch(infoErr) {
            case __MODULE_NAME___MSG_INFO:  cout << timeStr << ":DEBUG: JOBEXAM_INFO: " << msg << endl; break;
            case __MODULE_NAME___MSG_WARN:  cout << timeStr << ":DEBUG: JOBEXAM_WARN: " << msg << endl; break;
            default:                       cout << timeStr << ":DEBUG: JOBEXAM_ERR: "  << msg << endl; break;
        }
    }

    // get the time and message string
    lpv_timeStr.setString(timeStr2);
    lpv_msgStr.setString(msg);

    // raise alarm for the message
    switch(infoErr) {
        case __MODULE_NAME___MSG_INFO:
            lpv_timeStr.raiseAlarm  (READ_ALARM, NO_ALARM);
            lpv_msgStr.raiseAlarm   (READ_ALARM, NO_ALARM);
            break;

        case __MODULE_NAME___MSG_WARN:
            lpv_timeStr.raiseAlarm  (READ_ALARM, MINOR_ALARM);
            lpv_msgStr.raiseAlarm   (READ_ALARM, MINOR_ALARM);
            break;

        default:
            lpv_timeStr.raiseAlarm  (READ_ALARM, MAJOR_ALARM);
            lpv_msgStr.raiseAlarm   (READ_ALARM, MAJOR_ALARM);
            break;
    }
}

//-----------------------------------------------
// callback functions
//-----------------------------------------------
void Job_Example::lpvRcb_cnt1S(void *arg)
{
    Job_Example *job = (Job_Example *)arg; 

    if(job) {
        job -> var_valCnt1S ++;
        job -> lpv_counter1S.setValue(job -> var_valCnt1S);
    }
}

void Job_Example::lpvRcb_cnt2S(void *arg)
{
    Job_Example *job = (Job_Example *)arg; 

    if(job) {
        job -> var_valCnt2S ++;
        job -> lpv_counter2S.setValue(job -> var_valCnt2S);
    }
}

void Job_Example::lpvRcb_cnt5S(void *arg)
{
    Job_Example *job = (Job_Example *)arg; 

    if(job) {
        job -> var_valCnt5S ++;
        job -> lpv_counter5S.setValue(job -> var_valCnt5S);
    }
}



