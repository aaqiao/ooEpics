//=========================================================
// Service_Example.cc
// 
// Class implementation for the example service
// Auto created from the template of ooEpics framework
//=========================================================
#include "Service_Example.h"

using namespace std;

//-----------------------------------------------
// construction
//-----------------------------------------------
Service_Example::Service_Example(const char *moduleName, const char *serviceName) : 
    OOEPICS::Service(moduleName, serviceName)
{
    // init the locks
    var_lockCAMonitors = EPICSLIB_func_mutexMustCreate();

    cout << "INFO:Service_Example: Object " << srvName << " created." << endl;
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
Service_Example::~Service_Example()
{
    // destroy the epics mutex
    if(var_lockCAMonitors)  
        EPICSLIB_func_mutexDestroy(var_lockCAMonitors);

    cout << "INFO:Service_Example: Object " << srvName << " deleted." << endl;
}

//-----------------------------------------------
// init the remote PVs
//-----------------------------------------------
void Service_Example::initRemotePVs(OOEPICS::RemotePVList *ptr_rpvList)
{
    // init the remote PVs
    rpv_setBO.init          (modName, srvName, "RPV-SET-BO",    ptr_rpvList);
    rpv_setMBBO.init        (modName, srvName, "RPV-SET-MBBO",  ptr_rpvList);
    rpv_setLO.init          (modName, srvName, "RPV-SET-LO",    ptr_rpvList);
    rpv_setAO.init          (modName, srvName, "RPV-SET-AO",    ptr_rpvList);
    rpv_setWFO.init         (modName, srvName, "RPV-SET-WFO",   ptr_rpvList);
    rpv_setSO.init          (modName, srvName, "RPV-SET-SO",    ptr_rpvList);
    
    rpv_getBI.init          (modName, srvName, "RPV-MON-BI",    ptr_rpvList);
    rpv_getMBBI.init        (modName, srvName, "RPV-MON-MBBI",  ptr_rpvList);
    rpv_getLI.init          (modName, srvName, "RPV-GET-LI",    ptr_rpvList);
    rpv_getAI.init          (modName, srvName, "RPV-GET-AI",    ptr_rpvList);
    rpv_getWFIXaxis.init    (modName, srvName, "RPV-MON-WFI-X", ptr_rpvList);    
    rpv_getWFI.init         (modName, srvName, "RPV-GET-WFI",   ptr_rpvList);
    rpv_getSI.init          (modName, srvName, "RPV-MON-SI",    ptr_rpvList);
}

//-----------------------------------------------
// init the remote PVs
//-----------------------------------------------
void Service_Example::initChannelAccess()
{
    // create channel access
    //      int createCA(   unsigned long             reqElemsReadIn,   // number of elements of the CA request
    //                      CA_enum_readCtrl          rdCtrlIn,         // reading type
    //                      CA_enum_writeCtrl         wtCtrlIn,         // writing type
    //                      EPICSLIB_type_mutexId     mutexIdIn,        // used to lock the data buffer when monitoring or callback reading have data arrived
    //                      EPICSLIB_type_eventId     eventIn);         // general event for connection, read and write callbacks
    //
    rpv_setBO.createCA          (1,                 OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_PULL,     var_lockCAMonitors, NULL);    // writing PVs can also be monitored to verify the changes
    rpv_setMBBO.createCA        (1,                 OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_PULL,     var_lockCAMonitors, NULL);
    rpv_setLO.createCA          (1,                 OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_PULL,     var_lockCAMonitors, NULL);
    rpv_setAO.createCA          (1,                 OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_PULL,     var_lockCAMonitors, NULL);
    rpv_setWFO.createCA         (SRVEXAM_BUF_SIZE,  OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_PULL,     var_lockCAMonitors, NULL);
    rpv_setSO.createCA          (1,                 OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_PULL,     var_lockCAMonitors, NULL);
    
    rpv_getBI.createCA          (1,                 OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_DISABLED, var_lockCAMonitors, NULL);
    rpv_getMBBI.createCA        (1,                 OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_DISABLED, var_lockCAMonitors, NULL);
    rpv_getLI.createCA          (1,                 OOEPICS::CA_READ_PULL,    OOEPICS::CA_WRITE_DISABLED, NULL,               NULL);
    rpv_getAI.createCA          (1,                 OOEPICS::CA_READ_PULL,    OOEPICS::CA_WRITE_DISABLED, NULL,               NULL);
    rpv_getWFIXaxis.createCA    (SRVEXAM_BUF_SIZE,  OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_DISABLED, var_lockCAMonitors, NULL);
    rpv_getWFI.createCA         (SRVEXAM_BUF_SIZE,  OOEPICS::CA_READ_PULL,    OOEPICS::CA_WRITE_DISABLED, NULL,               NULL);
    rpv_getSI.createCA          (1,                 OOEPICS::CA_READ_MONITOR, OOEPICS::CA_WRITE_DISABLED, var_lockCAMonitors, NULL);
}

//-----------------------------------------------
// set the remote PV data (similar as caput)
//-----------------------------------------------
int Service_Example::setRemoteDataPull (epicsUInt16     boVal,
                                        epicsUInt16     mbboVal,
                                        epicsInt32      loVal,
                                        epicsFloat64    aoVal,
                                        epicsFloat64   *wfoVals,
                                        const char     *soStr,
                                        unsigned int    wfoPno)
{
    int status;

    // check the input
    if(!wfoVals || !soStr || wfoPno <= 0)
        return __MODULE_NAME___RPVACCESS_FAILED;

    // check the connection
    if( !rpv_setBO.isConnected()    ||
        !rpv_setMBBO.isConnected()  ||
        !rpv_setLO.isConnected()    ||
        !rpv_setAO.isConnected()    ||
        !rpv_setWFO.isConnected()   ||
        !rpv_setSO.isConnected())
        return __MODULE_NAME___RPVACCESS_NOT_CONNECTED;

    // prepare the CA write requests
    rpv_setBO.caWriteRequestVal     (boVal);
    rpv_setMBBO.caWriteRequestVal   (mbboVal);
    rpv_setLO.caWriteRequestVal     (loVal);
    rpv_setAO.caWriteRequestVal     (aoVal);

    rpv_setWFO.caWriteRequestWf     (wfoVals, wfoPno);

    rpv_setSO.caWriteRequestStr     (soStr);

    // send the requests
    status = sendCARequestAndWaitFinish(CA_DEFAULT_TIMEOUT);

    // check the return status
    if(status == ECA_NORMAL) 
        return __MODULE_NAME___RPVACCESS_SUCCESS;
    else if(status == ECA_TIMEOUT) 
        return __MODULE_NAME___RPVACCESS_TIMEOUT;
    else
        return __MODULE_NAME___RPVACCESS_FAILED;
}

//-----------------------------------------------
// get the remote PV data (similar as caget)
//-----------------------------------------------
int Service_Example::getRemoteDataPull (epicsInt32     *liVal,
                                        epicsFloat64   *aiVal,
                                        epicsFloat64   *wfiVals,
                                        unsigned int    wfiPno)
{
    int status;

    // check the input
    if(!liVal || !aiVal || !wfiVals || wfiPno <= 0)
        return __MODULE_NAME___RPVACCESS_FAILED;

    // check the connection
    if( !rpv_getLI.isConnected()    ||
        !rpv_getAI.isConnected()    ||
        !rpv_getWFI.isConnected())
        return __MODULE_NAME___RPVACCESS_NOT_CONNECTED;

    // prepare the CA read requests
    rpv_getLI.caReadRequest     ();
    rpv_getAI.caReadRequest     ();
    rpv_getWFI.caReadRequest    ();

    // send the requests
    status = sendCARequestAndWaitFinish(CA_DEFAULT_TIMEOUT);

    // check the return status, if good, get the data
    if(status == ECA_NORMAL) {
        // get data
        *liVal = rpv_getLI.getValueInt32();
        *aiVal = rpv_getAI.getValueFloat64();
        
        rpv_getWFI.getValues(wfiVals, wfiPno);

        return __MODULE_NAME___RPVACCESS_SUCCESS;

    } else if(status == ECA_TIMEOUT) {
        return __MODULE_NAME___RPVACCESS_TIMEOUT;
    } else {
        return __MODULE_NAME___RPVACCESS_FAILED;    
    }
}

//-----------------------------------------------
// get monitored remote PV data (similar as camonitor)
//-----------------------------------------------
int Service_Example::getRemoteDataMonitor  (epicsUInt16    *biVal,
                                            epicsUInt16    *mbbiVal,
                                            epicsFloat64   *wfiXaxisVals,
                                            char           *siStr,
                                            unsigned int    wfiPno)
{
    // check the input
    if(!biVal || !mbbiVal || !wfiXaxisVals || !siStr || wfiPno <= 0)
        return __MODULE_NAME___RPVACCESS_FAILED;

    // check the connection
    if( !rpv_getBI.isConnected()        ||
        !rpv_getMBBI.isConnected()      ||
        !rpv_getWFIXaxis.isConnected()  ||
        !rpv_getSI.isConnected())
        return __MODULE_NAME___RPVACCESS_NOT_CONNECTED;

    // directly get the values because the PVs should contain the latest monitoring results
    *biVal   = rpv_getBI.getValueUInt16();
    *mbbiVal = rpv_getMBBI.getValueUInt16();
    
    rpv_getWFIXaxis.getValues(wfiXaxisVals, wfiPno);

    rpv_getSI.getValueString(siStr);

    // return success
    return __MODULE_NAME___RPVACCESS_SUCCESS;
}







