//=========================================================
// ChannelAccess.h
//
// Basic class for Channel Access. This is an OO version of the Channel Access routines
//
// Created by: Zheqiao Geng, gengzq@slac.stanford.edu
// Created on: Jan. 03, 2011
//
// Modified by: Zheqiao Geng
// Modified on: Jan. 04, 2011
// Description: add the function to put value
//
// Modified by: Zheqiao Geng
// Modified on: Jan. 06, 2011
// Description: add the flag for data access status
// 
// Modified by Zheqiao Geng on 2014.04.02
// Description: redesign the channel access to use the advance feather of CA library
//
// Modified by Zheqiao Geng on 23.03.2016
// The following updates:
//  - PV name passed by string
//  - Read time stamp and alarm info
//  - Provide function to do datatype conversion
//=========================================================
#ifndef CHANNELACCESS_H
#define CHANNELACCESS_H
#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "cadef.h"
#include "alarm.h"
#include "EPICSLib_wrapper.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

#define CA_STRING_LEN       256
#define CA_DEFAULT_TIMEOUT  5
#define CA_DEFAULT_PRIORITY 10

//-----------------------------------------------
// wrapper of general CA request functions
//-----------------------------------------------
#define sendCARequestAndWaitFinish  ca_pend_io
#define sendCARequestAndProceed     ca_flush_io

//-----------------------------------------------
// the user callback function
//-----------------------------------------------
extern "C" {
typedef void (*CAUSR_CALLBACK)(void*);
}

//-----------------------------------------------
// definition of the control of CA reading and writing
//-----------------------------------------------
typedef enum {
    CA_READ_DISABLED,
    CA_READ_PULL,
    CA_READ_CALLBACK,
    CA_READ_MONITOR
} CA_enum_readCtrl;

typedef enum {
    CA_WRITE_DISABLED,
    CA_WRITE_PULL,
    CA_WRITE_CALLBACK
} CA_enum_writeCtrl;

typedef enum {
    CA_SINGLE_THREAD,
    CA_MULTIPLE_THREAD
} CA_enum_contextCtrl;

//-----------------------------------------------
// class definition
//-----------------------------------------------
class ChannelAccess
{
public:
    ChannelAccess(string                    pvNameIn,                   // remote PV name
                  unsigned long             reqElemsReadIn,             // number of elements of the request for reading
                  CA_enum_readCtrl          rdCtrlIn,                   // reading type
                  CA_enum_writeCtrl         wtCtrlIn,                   // writing type
                  CAUSR_CALLBACK            connUserCallbackIn,         // user callback for connection monitoring
                  CAUSR_CALLBACK            rdUserCallbackIn,           // user callback when reading is done if the reading callback is enabled
                  CAUSR_CALLBACK            wtUserCallbackIn,           // user callback when writing is done if the writing callback is enabled
                  void                     *dataPtrIn,                  // this is used for the PV with monitring reading
                  void                     *userPtrIn,                  // user pointer that will be passed to the user callback functions
				  EPICSLIB_type_mutexId     mutexIdIn,					// used to lock the data buffer when monitoring or callback reading have data arrived
                  EPICSLIB_type_eventId     connEventIn,                // optional event that will be fired with the connection callback
                  EPICSLIB_type_eventId     rdEventIn,                  // optional event that will be fired with the reading callback
                  EPICSLIB_type_eventId     wtEventIn,                  // optional event that will be fired with the writing callback
                  int                       callBackPriorityIn);

   ~ChannelAccess();

    // old interfaces, should not be used in new development
    int  getValue(void *dataBuf);
    int  putValue(void *dataBuf);    

    // PV access routines
    void connect            ();                                                     // setup the connection which is managed by the general callback function

    int caReadRequest       ();                                                     // data read in will be saved in the dataBufRead (point number has been determined internally)

    int caWriteRequestVal   (epicsFloat64   dataIn);                                // write a scalar, data will be converted in the server side
    int caWriteRequestStr   (char          *strIn);                                 // write a stringout record
    int caWriteRequestStr   (string         strIn);                                 // write a stringout record

    int caWriteRequestWfRaw (void           *dataBufIn, unsigned long pointNum);    // write a waveform, no data type conversion
    int caWriteRequestWf    (epicsInt8      *dataBufIn, unsigned long pointNum);    // write a waveform, do data type conversion at the server side (costly)
    int caWriteRequestWf    (epicsUInt8     *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsInt16     *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsUInt16    *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsInt32     *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsUInt32    *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsFloat32   *dataBufIn, unsigned long pointNum);
    int caWriteRequestWf    (epicsFloat64   *dataBufIn, unsigned long pointNum);

    epicsInt8       getValueInt8    ();                                             // get scalar of reading, data type will be converted locally
    epicsUInt8      getValueUInt8   ();
    epicsInt16      getValueInt16   ();
    epicsUInt16     getValueUInt16  ();
    epicsInt32      getValueInt32   ();
    epicsUInt32     getValueUInt32  ();
    epicsFloat32    getValueFloat32 ();
    epicsFloat64    getValueFloat64 ();
    int             getValueString  (char *strOut);
    string          getValueString  ();

    int getValuesRaw(void          *dataBufOut, unsigned long pointNum);            // get a waveform, data type will not be converted
    int getValues   (epicsInt8     *dataBufOut, unsigned long pointNum);            // get a waveform, data type will be converted locally one by one
    int getValues   (epicsUInt8    *dataBufOut, unsigned long pointNum);
    int getValues   (epicsInt16    *dataBufOut, unsigned long pointNum);
    int getValues   (epicsUInt16   *dataBufOut, unsigned long pointNum);
    int getValues   (epicsInt32    *dataBufOut, unsigned long pointNum);
    int getValues   (epicsUInt32   *dataBufOut, unsigned long pointNum);
    int getValues   (epicsFloat32  *dataBufOut, unsigned long pointNum);
    int getValues   (epicsFloat64  *dataBufOut, unsigned long pointNum);    

    // interface functions
    int             getConnected        ();
    int             getCAStatus         ();
    epicsTimeStamp  getTimeStamp        ();
    void            getTimeStampStr     (char *tsStr);
    epicsInt16      getAlarmStatus      ();
    epicsInt16      getAlarmSeverity    ();
    unsigned long   getRPVElemCount     ();
    
private:
    // ## CA configurations ##
    char                    pvName[CA_STRING_LEN];  // the remote PV name of this channel
    int                     callBackPriority;       // the call back priority
    chid                    channelID;              // the channel ID

    CA_enum_readCtrl        rdCtrl;                 // control the reading behavior of the remote PV 
    CA_enum_writeCtrl       wtCtrl;                 // control the writing behavior of the remote PV

    CAUSR_CALLBACK          connUserCallback;       // user callback that will be executed in the general callbacks
    CAUSR_CALLBACK          rdUserCallback;         // see up
    CAUSR_CALLBACK          wtUserCallback;         // see up

	EPICSLIB_type_mutexId   mutexId;
    EPICSLIB_type_eventId   connEvent;              // user connection event
    EPICSLIB_type_eventId   rdEvent;                // user read event
    EPICSLIB_type_eventId   wtEvent;                // user write event

    chtype                  dbfType;                // database field type in server side
    chtype                  dbrTypeRead;            // request type in the CA client side for reading (if not match, conversion will be done by the server)
    chtype                  dbrTypeWrite;           // request type in the CA client side for writing (if not match, conversion will be done by the server)

    unsigned long           nElems;                 // True length of data in value
    unsigned long           reqElemsRead;           // Requested length of data
    unsigned long           bufSizeRead;            // buffer size for reading all data from the remote PV

    void                   *userPtr;                // the user pointer that will be pass back to the user callback functions
    void                   *dataBufRead;            // buffer of the data for saving reading results
    void                   *dataBufMonitor;         // this is the external buffer that automatically accept the monitored results

    // ## flags ##
    int                     onceCreated;            // indicate if this channel has been once created or not
    int                     onceConnected;          // flag to show if the CA is first time connected or not
    int                     caConnected;            // status of the channel access
    int                     caStatus;               // the status

    // ## for CA reading ##
    struct dbr_time_string *ptr_tString;
    struct dbr_time_short  *ptr_tShort;
    struct dbr_time_float  *ptr_tFloat;
    struct dbr_time_enum   *ptr_tEnum;
    struct dbr_time_char   *ptr_tChar;
    struct dbr_time_long   *ptr_tLong;
    struct dbr_time_double *ptr_tDouble;

    epicsTimeStamp          var_timeStamp;
    char                    buf_timeStampStr[32];
    epicsInt16              var_alarmStatus;        // defined in EPICS base dbStatic\alarm.h
    epicsInt16              var_alarmSeverity;

    // ## common functions ##
    int fun_writeRequest                    (chtype dbr, unsigned long pointNum, void *dataPtr);
    
    // ## generic callback functions ##
    static void callBackFunc_connection     (struct connection_handler_args arg);
    static void callBackFunc_readAndMonitor (struct event_handler_args arg);
    static void callBackFunc_write          (struct event_handler_args arg);
};

//-----------------------------------------------
// Function for managing the Channel Access Context
//-----------------------------------------------
class ChannelAccessContext
{
public:
    ChannelAccessContext        (CA_enum_contextCtrl contextCtrl);
   ~ChannelAccessContext        ();

    // Channel Access context operations
    int attachCurrentThread     ();
    int displayStatus           ();

    // status
    int caContextStatus;

private:
    struct ca_client_context *caContext;
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

