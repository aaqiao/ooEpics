//=========================================================
// RemotePV.h
// 
// Class definition for the remote PV
// 
// Created by: Zheqiao Geng
// Created on: 12/11/2012
// Description: Initial creation 
// 
// Modified by: Zheqiao Geng
// Modified on: 2013.11.14
// Description: 1. Redesign the RemotePVList class
//              2. Update the data type definition for adpating both the 32-bit and 64-bit machines
//
// Modified by Zheqiao Geng on 2014.04.08
// Description: updated with the new Channel Access implementation
//
// Modified by Zheqiao Geng on 2014.05.19
// Applied macro to the remote PV mapping files
//
// Modified by Zheqiao Geng on 28.09.2015
// Enable dynamic changes of the macros during operation, change the two functions of
//      int  getPVNameList(string fileName, string macros);
//      int  mapPVNodeNames();
// to
//      int  getPVNameList(string fileName);
//      int  mapPVNodeNames(string macros);
// which means, the macro will be only substituted when mapping the remote PV nodes
//
// Modified by Zheqiao Geng on 30.09.2015
// Added the function to provide counter for:
//   - Remote PVs
//   - Remote PVs successfully mapped
//   - Remtoe PVs successfully connected
//
// Modified by Andreas Hauff on 16+.11.2015
// Additional constructor to use it without PV mapping
//
// Modified by Zheqiao Geng on 15.12.2015
// Added in prtPVNodeList function to have more choices for printing
//=========================================================
#ifndef REMOTEPV_H
#define REMOTEPV_H

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "macLib.h"

#include "ooEpicsMisc.h"
#include "EPICSLib_wrapper.h"
#include "ChannelAccess.h"

#define RPVLIST_PNT_SUMMARY     0           // print only the summary
#define RPVLIST_PNT_CONN        1           // print all connected
#define RPVLIST_PNT_NCONN       2           // print all not-connected
#define RPVLIST_PNT_ALL         3           // print all

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// forward declaration
//-----------------------------------------------
class RemotePV;

//-----------------------------------------------
// the class definition for remote PV name list
// this is the list of pv name mapping to local id string read from a file
//-----------------------------------------------
// --- Remote PV name pare (used to store the contents in the file) ---
class RemotePVName
{
public:
    EPICSLIB_type_linkedListNode node;
    string pvLocalIdStr;                                                // local ID string for the PV
    string pvNameStr;                                                   // EPICS PV name    
    string pvLocalIdStrWithMacro;                                       // with raw macros
    string pvNameStrWithMacro;                                          // with raw macros
};

// --- Remote PV node (for real PVs) ---
class RemotePVNode
{
public:
    EPICSLIB_type_linkedListNode node;
    RemotePV *pv;                                                       // the corresponding remote PV
};
     
// -- PV node/name list and the operations ---
class RemotePVList
{
public:
    RemotePVList                ();
   ~RemotePVList                ();

    // get the mapping from file
    int  getPVNameList          (string fileName, string localMacros);  // it will fill the pvNameList

    // remote PV handling
    int  addPVNodeList          (RemotePV *pv);                         // it will fill the pvNodeList
    int  mapPVNodeNames         (string macros);
    
    // get the counter
    int  getCntRemotePV         ();
    int  getCntRemotePVMappend  ();
    int  getCntRemotePVConnected();

    // for managing
    int  fndPVNameList          (string localIdStr, string *nameStr);   // lookup the PV name list
    int  savPVNodeList          (string fileName);
    void prtPVNameList          ();
    void prtPVNodeList          ();
    void prtPVNodeList          (int sel);

private:
    EPICSLIB_type_linkedList pvNameList;
    EPICSLIB_type_linkedList pvNodeList;

    int cntRemotePV;
    int cntRemotePVMapped;

    void prtRPVInfo(RemotePV *pv);
};

//-----------------------------------------------
// the class definition for remote PV 
//-----------------------------------------------
class RemotePV
{
public:
    RemotePV            ();
    explicit RemotePV   (const std::string pvNameStrIn);
    RemotePV            (const char *moduleName, const char *localIDStr, RemotePVList *pvList);
   ~RemotePV            ();  

    // routine to create or delete channel access
    void init           (string modName, string srvName, string localIDStr, RemotePVList *pvList);       // work together with default construction

    int createCA        (unsigned long             reqElemsReadIn,              // number of elements of the request
                         CA_enum_readCtrl          rdCtrlIn,                    // reading type
                         CA_enum_writeCtrl         wtCtrlIn,                    // writing type
                         CAUSR_CALLBACK            connUserCallbackIn,          // user callback for connection monitoring
                         CAUSR_CALLBACK            rdUserCallbackIn,            // user callback when reading is done if the reading callback is enabled
                         CAUSR_CALLBACK            wtUserCallbackIn,            // user callback when writing is done if the writing callback is enabled
                         void                     *dataPtrIn,                   // this is used for the PV with monitring reading
                         void                     *userPtrIn,                   // user pointer that will be passed to the user callback functions
                         EPICSLIB_type_mutexId     mutexIdIn,                    // used to lock the data buffer when monitoring or callback reading have data arrived
                         EPICSLIB_type_eventId     connEventIn,                 // optional event that will be fired with the connection callback
                         EPICSLIB_type_eventId     rdEventIn,                   // optional event that will be fired with the reading callback
                         EPICSLIB_type_eventId     wtEventIn);                  // optional event that will be fired with the writing callback                 

    int createCA        (unsigned long             reqElemsReadIn,              // simplfied version
                         CA_enum_readCtrl          rdCtrlIn,
                         CA_enum_writeCtrl         wtCtrlIn,
                         EPICSLIB_type_mutexId     mutexIdIn,
                         EPICSLIB_type_eventId     eventIn);                    // general event for connection, read and write callbacks

    void deleteCA       ();

    // interfaces for CA access (direct wrapper of Channel Access methods)
    int caReadRequest       ();

    int caWriteRequestVal   (epicsFloat64   dataIn);
    int caWriteRequestStr   (char          *strIn);
    int caWriteRequestStr   (string         strIn);

    int caWriteRequestWfRaw (void           *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsInt8      *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsUInt8     *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsInt16     *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsUInt16    *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsInt32     *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsUInt32    *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsFloat32   *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsFloat64   *dataBufIn, unsigned long pointNum);

    epicsInt8       getValueInt8    ();
    epicsUInt8      getValueUInt8   ();
    epicsInt16      getValueInt16   ();
    epicsUInt16     getValueUInt16  ();
    epicsInt32      getValueInt32   ();
    epicsUInt32     getValueUInt32  ();
    epicsFloat32    getValueFloat32 ();
    epicsFloat64    getValueFloat64 ();
    int             getValueString  (char *strOut);
    string          getValueString  ();

    int getValuesRaw(void          *dataBufOut, unsigned long pointNum);
    int getValues   (epicsInt8     *dataBufOut, unsigned long pointNum);
    int getValues   (epicsUInt8    *dataBufOut, unsigned long pointNum);
    int getValues   (epicsInt16    *dataBufOut, unsigned long pointNum);
    int getValues   (epicsUInt16   *dataBufOut, unsigned long pointNum);
    int getValues   (epicsInt32    *dataBufOut, unsigned long pointNum);
    int getValues   (epicsUInt32   *dataBufOut, unsigned long pointNum);
    int getValues   (epicsFloat32  *dataBufOut, unsigned long pointNum);
    int getValues   (epicsFloat64  *dataBufOut, unsigned long pointNum);        

    // routines to get the status
    int  isConnected    ();   
    int  isOK           ();  

    epicsTimeStamp  getTimeStamp        ();
    void            getTimeStampStr     (char *tsStr);
    epicsInt16      getAlarmStatus      ();
    epicsInt16      getAlarmSeverity    ();
        

    // the strings
    string pvLocalIdStr;                                                        // local ID string for the PV
    string pvNameStr;                                                           // EPICS PV name

private:
    ChannelAccess      *pvCAChannel;                                            // channel access channel for the PV
};

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

