//=========================================================
// Service_Example.h
//
// Class definition for the example service
// Auto created from the template of ooEpics framework
//=========================================================
#ifndef SERVICE_EXAMPLE_H
#define SERVICE_EXAMPLE_H

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "Service.h"
#include "LocalPV.h"
#include "RemotePV.h"
#include "ooEpicsMisc.h"
#include "EPICSLib_wrapper.h"

#include "ModuleDefs___MODULE_NAME__.h"

#define SRVEXAM_BUF_SIZE 2048

//-----------------------------------------------
// the class definition of the service of Service_EVR
//-----------------------------------------------
class Service_Example : public OOEPICS::Service
{
public:
    Service_Example(const char *moduleName, const char *serviceName);
   ~Service_Example();

    // routines for configuration
    void initRemotePVs         (OOEPICS::RemotePVList *ptr_rpvList);
    void initChannelAccess     ();   

    // service for external Jobs
    int  setRemoteDataPull     (epicsUInt16     boVal,              // set value to remote bo        record
                                epicsUInt16     mbboVal,            // set value to remote mbbo      record
                                epicsInt32      loVal,              // set value to remote longout   record
                                epicsFloat64    aoVal,              // set value to remote ao        record
                                epicsFloat64   *wfoVals,            // set value to remote waveform  record for output
                                const char     *soStr,              // set value to remote stringout record
                                unsigned int    wfoPno);            // number of points in the waveform record

    int  getRemoteDataPull     (epicsInt32     *liVal,              // read the remote longin   record
                                epicsFloat64   *aiVal,              // read the remote ai       record
                                epicsFloat64   *wfiVals,            // read the remote waveform record for input
                                unsigned int    wfiPno);            // number of points in the waveform record

    int  getRemoteDataMonitor  (epicsUInt16    *biVal,              // monitor the remote bi        record
                                epicsUInt16    *mbbiVal,            // monitor the remote mbbi      record
                                epicsFloat64   *wfiXaxisVals,       // monitor the remote waveform  record for xaxis input
                                char           *siStr,              // monitor the remote stringin  record
                                unsigned int    wfiPno);            // number of points in the waveform record

private:
    // define remote PVs   
    OOEPICS::RemotePV rpv_setBO;
    OOEPICS::RemotePV rpv_setMBBO;
    OOEPICS::RemotePV rpv_setLO;
    OOEPICS::RemotePV rpv_setAO;
    OOEPICS::RemotePV rpv_setWFO;
    OOEPICS::RemotePV rpv_setSO;
    
    OOEPICS::RemotePV rpv_getBI;
    OOEPICS::RemotePV rpv_getMBBI;
    OOEPICS::RemotePV rpv_getLI;
    OOEPICS::RemotePV rpv_getAI;
    OOEPICS::RemotePV rpv_getWFIXaxis;    
    OOEPICS::RemotePV rpv_getWFI;
    OOEPICS::RemotePV rpv_getSI;

    // locks for multiple thread data protection
    EPICSLIB_type_mutexId var_lockCAMonitors;
};

#endif


