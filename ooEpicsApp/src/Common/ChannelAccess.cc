//=========================================================
// ChannelAccess.cc
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
// Modified by Zheqiao Geng on 2014.04.07
// Description: redesign the CA object to use call back functions
//
// Modified by Zheqiao Geng on 02.04.2015
// Disable the message printing when the action is successful
//=========================================================
#include "ChannelAccess.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction
// Assume the CA context has been created
//-----------------------------------------------
ChannelAccess::ChannelAccess(string                    pvNameIn, 
                             unsigned long             reqElemsReadIn, 
                             CA_enum_readCtrl          rdCtrlIn, 
                             CA_enum_writeCtrl         wtCtrlIn, 
                             CAUSR_CALLBACK            connUserCallbackIn,
                             CAUSR_CALLBACK            rdUserCallbackIn,
                             CAUSR_CALLBACK            wtUserCallbackIn,
                             void                     *dataPtrIn,
                             void                     *userPtrIn,
							 EPICSLIB_type_mutexId     mutexIdIn,
                             EPICSLIB_type_eventId     connEventIn,
                             EPICSLIB_type_eventId     rdEventIn,
                             EPICSLIB_type_eventId     wtEventIn,  
                             int                       callBackPriorityIn)
{
    // check the input parameters
    if(pvNameIn.empty())
        cout << "ERROR: ChannelAccess::ChannelAccess: Wrong initial settings!\n";

    // init the CA configurations
    strncpy(pvName, pvNameIn.c_str(), CA_STRING_LEN);       // remember the name

    reqElemsRead        = reqElemsReadIn;

    rdCtrl              = rdCtrlIn;
    wtCtrl              = wtCtrlIn;
    connUserCallback    = connUserCallbackIn;
    rdUserCallback      = rdUserCallbackIn;
    wtUserCallback      = wtUserCallbackIn;
    dataBufMonitor      = dataPtrIn;
    userPtr             = userPtrIn;
	mutexId				= mutexIdIn;
    connEvent           = connEventIn;
    rdEvent             = rdEventIn;
    wtEvent             = wtEventIn;
    callBackPriority    = callBackPriorityIn;

    if(reqElemsRead == 0) 
        reqElemsRead = 1;

    // init priviate data
    nElems              = 0;
    bufSizeRead         = 0;

    dataBufRead         = NULL;

    onceCreated         = 0;                                // not created
    onceConnected       = 0;                                // not connected
    caConnected         = 0;                                // not connected
    caStatus            = ECA_DISCONN;                      // not connected
    
    ptr_tString         = NULL;
    ptr_tShort          = NULL;
    ptr_tFloat          = NULL;
    ptr_tEnum           = NULL;
    ptr_tChar           = NULL;
    ptr_tLong           = NULL;
    ptr_tDouble         = NULL;

    memset(&var_timeStamp,   0, sizeof(var_timeStamp));
    memset(buf_timeStampStr, 0, sizeof(buf_timeStampStr));

    var_alarmStatus     = NO_ALARM;
    var_alarmSeverity   = NO_ALARM;    
}

//-----------------------------------------------
// destruction
//-----------------------------------------------
ChannelAccess::~ChannelAccess()
{
    // clear the channel
    if(onceCreated && channelID)
        caStatus = ca_clear_channel(channelID);

    // clear the buffers
    if(dataBufRead)
        free(dataBufRead);
}

//-----------------------------------------------
// get the PV value (old interface, do not use in new development)
// this is used for the mode CA_READ_PULL
// return:
//    0 - success; 1 - failed
//-----------------------------------------------
int ChannelAccess::getValue(void *dataBuf)
{
    // check the input
    if(!dataBuf || ca_state(channelID) != cs_conn) 
        return 1;

    // get the data with pulling
    if(rdCtrl == CA_READ_PULL) {
        caStatus = ca_array_get(dbrTypeWrite, reqElemsRead, channelID, dataBuf);    // the DBR type use the naked type same as for writing
    } else {
        return 1;
    }

    // check the return
    if(caStatus == ECA_NORMAL) {
        return 0;
    } else {
        return 1;
    }
}

//-----------------------------------------------
// put the PV value (old interface, do not use in new development)
// this is used for the mode of CA_WRITE_PULL
// return:
//    0 - success; 1 - failed
//-----------------------------------------------
int ChannelAccess::putValue(void *dataBuf)
{
    // check the input
    if(!dataBuf || ca_state(channelID) != cs_conn) 
        return 1;

    // put the data with different style
    if(wtCtrl == CA_WRITE_PULL) {
        caStatus = ca_array_put(dbrTypeWrite, nElems, channelID, dataBuf);          // write the number of element as remote PV
    } else {
        return 1;
    }

    // check the return
    if(caStatus == ECA_NORMAL)
        return 0;
    else 
        return 1;
}

//-----------------------------------------------
// do the connection
//-----------------------------------------------
void ChannelAccess::connect()
{
    // create the CA channel (pass the pointer of the Channel Access object as private pointer)
    if(!onceCreated) {
        onceCreated = 1;
        caStatus    = ca_create_channel(pvName, callBackFunc_connection, (void *)this, callBackPriority, &channelID);
        
        if(caStatus == ECA_NORMAL) 
            return;
        else if(caStatus == ECA_BADTYPE)
            cout << "ERROR: ChannelAccess::connect: Failed (invalid DBR type) to create CA channel for " << pvName << ".\n";
        else if(caStatus == ECA_STRTOBIG)
            cout << "ERROR: ChannelAccess::connect: Failed (unusually large string) to create CA channel for " << pvName << ".\n";
        else if(caStatus == ECA_ALLOCMEM)
            cout << "ERROR: ChannelAccess::connect: Failed (unable to allocate memory) to create CA channel for " << pvName << ".\n";
        else
            cout << "ERROR: ChannelAccess::connect: Failed (unknown reason) to create CA channel for " << pvName << ".\n";   
    }
}

//-----------------------------------------------
// send a request for CA reading
// this is used for the two mode of: CA_READ_PULL, CA_READ_CALLBACK
// return:
//    0 - success; 1 - failed
//-----------------------------------------------
int ChannelAccess::caReadRequest()
{    
    // check the input and the buffer for data reading, check the connections
    if(!dataBufRead || ca_state(channelID) != cs_conn) 
        return 1;

    // get the data with different style
    if(rdCtrl == CA_READ_PULL) {
        caStatus = ca_array_get(dbrTypeRead, reqElemsRead, channelID, dataBufRead);                                         // get with pulling, later should call ca_pend_io(timeout)
    } else if(rdCtrl == CA_READ_CALLBACK) {
        caStatus = ca_array_get_callback(dbrTypeRead, reqElemsRead, channelID, callBackFunc_readAndMonitor, (void *)this);  // initialize the get with callback
    } else {
        return 1;
    }

    // check the return
    if(caStatus == ECA_NORMAL) {
        return 0;
    } else {
        return 1;
    }
}

//-----------------------------------------------
// send a request for writing a scalar value. we will ask the server
// to do the data type conversion
// return:
//    0 - success; 1 - failed
//-----------------------------------------------
int ChannelAccess::caWriteRequestVal(epicsFloat64 dataIn) 
{
    epicsFloat64 data = dataIn; 
    return fun_writeRequest(DBR_DOUBLE, 1, (void *)&data);
}

int ChannelAccess::caWriteRequestStr(char *strIn) 
{
    char str[256];                                                                                  // buffer to store the string record value, make it larger reserving for future
    
    // check the input
    if(!strIn) 
        return 1;    

    // get the input
    strcpy(str, strIn);

    // write the request
    return fun_writeRequest(DBR_STRING, 1, (void *)str);
}

int ChannelAccess::caWriteRequestStr(string strIn) 
{
    char str[256];                                                                                  // buffer to store the string record value, make it larger reserving for future
    
    // get the input
    strcpy(str, strIn.c_str());

    // write the request
    return fun_writeRequest(DBR_STRING, 1, (void *)str);
}

//-----------------------------------------------
// send a request for writing a waveform. we will ask the server
// to do the data type conversion except the one to write raw data
// return:
//    0 - success; 1 - failed
//-----------------------------------------------
int ChannelAccess::caWriteRequestWfRaw (void           *dataBufIn, unsigned long pointNum) {return fun_writeRequest(dbrTypeWrite, pointNum, (void *)dataBufIn);}    // use the remote PV type to avoid conversion
int ChannelAccess::caWriteRequestWf    (epicsInt8      *dataBufIn, unsigned long pointNum) {return fun_writeRequest(DBR_CHAR,     pointNum, (void *)dataBufIn);}
int ChannelAccess::caWriteRequestWf    (epicsUInt8     *dataBufIn, unsigned long pointNum) {return fun_writeRequest(DBR_CHAR,     pointNum, (void *)dataBufIn);}
int ChannelAccess::caWriteRequestWf    (epicsInt16     *dataBufIn, unsigned long pointNum) {return fun_writeRequest(DBR_SHORT,    pointNum, (void *)dataBufIn);}
int ChannelAccess::caWriteRequestWf    (epicsUInt16    *dataBufIn, unsigned long pointNum) {return fun_writeRequest(DBR_SHORT,    pointNum, (void *)dataBufIn);}
int ChannelAccess::caWriteRequestWf    (epicsInt32     *dataBufIn, unsigned long pointNum) {return fun_writeRequest(DBR_LONG,     pointNum, (void *)dataBufIn);}
int ChannelAccess::caWriteRequestWf    (epicsUInt32    *dataBufIn, unsigned long pointNum) {return fun_writeRequest(DBR_LONG,     pointNum, (void *)dataBufIn);}
int ChannelAccess::caWriteRequestWf    (epicsFloat32   *dataBufIn, unsigned long pointNum) {return fun_writeRequest(DBR_FLOAT,    pointNum, (void *)dataBufIn);}
int ChannelAccess::caWriteRequestWf    (epicsFloat64   *dataBufIn, unsigned long pointNum) {return fun_writeRequest(DBR_DOUBLE,   pointNum, (void *)dataBufIn);}

//-----------------------------------------------
// common private function to send write request
// return:
//    0 - success; 1 - failed
//-----------------------------------------------
int ChannelAccess::fun_writeRequest(chtype dbr, unsigned long pointNum, void *dataPtr)
{
    unsigned long pno = pointNum;

    // check the input and connections
    if(!dataPtr || pointNum == 0 || ca_state(channelID) != cs_conn) 
        return 1;

    // get the number of point
    if(pno > nElems)
        pno = nElems;

    // put the data with different style
    if(wtCtrl == CA_WRITE_PULL) {
        caStatus = ca_array_put(dbr, pno, channelID, dataPtr);                                              // put with pulling, later should call ca_pend_io(timeout)
    } else if(wtCtrl == CA_WRITE_CALLBACK) {
        caStatus = ca_array_put_callback(dbr, pno, channelID, dataPtr, callBackFunc_write, (void *)this);   // initialize the get with callback
    } else {
        return 1;
    }

    // check the return
    if(caStatus == ECA_NORMAL)
        return 0;
    else 
        return 1;
}

//-----------------------------------------------
// get a single value from the data read in via CA. data conversion
// will be done here. 
// Please note that the resolution of the data may be lost if converted
// to a lower level format.
//-----------------------------------------------
#define CA_VAL_READ(dtp)                                                            \
    {dtp val;                                                                       \
                                                                                    \
    if(!dataBufRead) return 0;                                                      \
                                                                                    \
    if(mutexId) EPICSLIB_func_mutexMustLock(mutexId);                               \
                                                                                    \
    switch(dbrTypeRead) {                                                           \
        case DBR_TIME_SHORT:                                                        \
            ptr_tShort                  = (struct dbr_time_short *)dataBufRead;     \
            val                         = (dtp)ptr_tShort -> value;                 \
            var_timeStamp               = ptr_tShort -> stamp;                      \
            var_alarmStatus             = ptr_tShort -> status;                     \
            var_alarmSeverity           = ptr_tShort -> severity;                   \
            break;                                                                  \
                                                                                    \
        case DBR_TIME_FLOAT:                                                        \
            ptr_tFloat                  = (struct dbr_time_float *)dataBufRead;     \
            val                         = (dtp)ptr_tFloat -> value;                 \
            var_timeStamp               = ptr_tFloat -> stamp;                      \
            var_alarmStatus             = ptr_tFloat -> status;                     \
            var_alarmSeverity           = ptr_tFloat -> severity;                   \
            break;                                                                  \
                                                                                    \
        case DBR_TIME_ENUM:                                                         \
            ptr_tEnum                   = (struct dbr_time_enum *)dataBufRead;      \
            val                         = (dtp)ptr_tEnum -> value;                  \
            var_timeStamp               = ptr_tEnum -> stamp;                       \
            var_alarmStatus             = ptr_tEnum -> status;                      \
            var_alarmSeverity           = ptr_tEnum -> severity;                    \
            break;                                                                  \
                                                                                    \
        case DBR_TIME_CHAR:                                                         \
            ptr_tChar                   = (struct dbr_time_char *)dataBufRead;      \
            val                         = (dtp)ptr_tChar -> value;                  \
            var_timeStamp               = ptr_tChar -> stamp;                       \
            var_alarmStatus             = ptr_tChar -> status;                      \
            var_alarmSeverity           = ptr_tChar -> severity;                    \
            break;                                                                  \
                                                                                    \
        case DBR_TIME_LONG:                                                         \
            ptr_tLong                   = (struct dbr_time_long *)dataBufRead;      \
            val                         = (dtp)ptr_tLong -> value;                  \
            var_timeStamp               = ptr_tLong -> stamp;                       \
            var_alarmStatus             = ptr_tLong -> status;                      \
            var_alarmSeverity           = ptr_tLong -> severity;                    \
            break;                                                                  \
                                                                                    \
        case DBR_TIME_DOUBLE:                                                       \
            ptr_tDouble                 = (struct dbr_time_double *)dataBufRead;    \
            val                         = (dtp)ptr_tDouble -> value;                \
            var_timeStamp               = ptr_tDouble -> stamp;                     \
            var_alarmStatus             = ptr_tDouble -> status;                    \
            var_alarmSeverity           = ptr_tDouble -> severity;                  \
            break;                                                                  \
                                                                                    \
        default:                                                                    \
            val                         = 0;                                        \
            var_timeStamp.secPastEpoch  = 0;                                        \
            var_timeStamp.nsec          = 0;                                        \
            var_alarmStatus             = NO_ALARM;                                 \
            var_alarmSeverity           = NO_ALARM;                                 \
            break;                                                                  \
    }                                                                               \
                                                                                    \
    if(mutexId) EPICSLIB_func_mutexUnlock(mutexId);                                 \
                                                                                    \
    epicsTimeToStrftime(buf_timeStampStr, sizeof(buf_timeStampStr),                 \
                        "%a %b %d %Y %H:%M:%S.%f", &var_timeStamp);                 \
                                                                                    \
    return val;}

epicsInt8    ChannelAccess::getValueInt8    () {CA_VAL_READ(epicsInt8);}
epicsUInt8   ChannelAccess::getValueUInt8   () {CA_VAL_READ(epicsUInt8);}
epicsInt16   ChannelAccess::getValueInt16   () {CA_VAL_READ(epicsInt16);}
epicsUInt16  ChannelAccess::getValueUInt16  () {CA_VAL_READ(epicsUInt16);}
epicsInt32   ChannelAccess::getValueInt32   () {CA_VAL_READ(epicsInt32);}
epicsUInt32  ChannelAccess::getValueUInt32  () {CA_VAL_READ(epicsUInt32);}
epicsFloat32 ChannelAccess::getValueFloat32 () {CA_VAL_READ(epicsFloat32);}
epicsFloat64 ChannelAccess::getValueFloat64 () {CA_VAL_READ(epicsFloat64);}

//-----------------------------------------------
// get a string record reading result, the return value will be the number of characters
//-----------------------------------------------
int ChannelAccess::getValueString(char *strOut)
{
    // check the input and request type
    if(!dataBufRead || !strOut || dbrTypeRead != DBR_TIME_STRING)
        return 0;

    // get the string, timestamp and the alarm status/severity
    if(mutexId) EPICSLIB_func_mutexMustLock(mutexId);

    ptr_tString         = (struct dbr_time_string *)dataBufRead;
    var_timeStamp       = ptr_tString -> stamp;
    var_alarmStatus     = ptr_tString -> status;
    var_alarmSeverity   = ptr_tString -> severity;

    strcpy(strOut, ptr_tString -> value);

    if(mutexId) EPICSLIB_func_mutexUnlock(mutexId);

    epicsTimeToStrftime(buf_timeStampStr, sizeof(buf_timeStampStr),
                        "%a %b %d %Y %H:%M:%S.%f", &var_timeStamp);
    
    // return the number of char read out
    return strlen(strOut);
}

string ChannelAccess::getValueString()
{
    string strOut("");

    // check the input and request type
    if(!dataBufRead || dbrTypeRead != DBR_TIME_STRING)
        return 0;

    // get the string, timestamp and the alarm status/severity
    if(mutexId) EPICSLIB_func_mutexMustLock(mutexId);

    ptr_tString         = (struct dbr_time_string *)dataBufRead;
    var_timeStamp       = ptr_tString -> stamp;
    var_alarmStatus     = ptr_tString -> status;
    var_alarmSeverity   = ptr_tString -> severity;

    strOut.assign(ptr_tString -> value);

    if(mutexId) EPICSLIB_func_mutexUnlock(mutexId);

    epicsTimeToStrftime(buf_timeStampStr, sizeof(buf_timeStampStr),
                        "%a %b %d %Y %H:%M:%S.%f", &var_timeStamp);
    
    // return the number of char read out
    return strOut;
}

//-----------------------------------------------
// get a waveform without data type conversion, please note that the data buffer
// should have the same type as the remote PV. the returned value is the number of
// point read out
//-----------------------------------------------
int ChannelAccess::getValuesRaw(void *dataBufOut, unsigned long pointNum)
{
    unsigned long   pno         = pointNum;
    unsigned long   dataSize    = 0;
    void           *dataBuf     = NULL;
    
    // check the input
    if(!dataBufRead || !dataBufOut || pointNum == 0) 
        return 0;

    // define number of point
    if(pno > nElems)
        pno = nElems;

    // get the data
    if(mutexId) EPICSLIB_func_mutexMustLock(mutexId);

    switch(dbrTypeRead) {
        case DBR_TIME_SHORT:
            ptr_tShort                  = (struct dbr_time_short *)dataBufRead;
            var_timeStamp               = ptr_tShort -> stamp;
            var_alarmStatus             = ptr_tShort -> status;
            var_alarmSeverity           = ptr_tShort -> severity;
            dataBuf                     = (void *)&ptr_tShort -> value;
            dataSize                    = sizeof(dbr_short_t);
            break;

        case DBR_TIME_FLOAT:
            ptr_tFloat                  = (struct dbr_time_float *)dataBufRead;
            var_timeStamp               = ptr_tFloat -> stamp;
            var_alarmStatus             = ptr_tFloat -> status;
            var_alarmSeverity           = ptr_tFloat -> severity;
            dataBuf                     = (void *)&ptr_tFloat -> value;
            dataSize                    = sizeof(dbr_float_t);
            break;

        case DBR_TIME_ENUM:
            ptr_tEnum                   = (struct dbr_time_enum *)dataBufRead;
            var_timeStamp               = ptr_tEnum -> stamp;
            var_alarmStatus             = ptr_tEnum -> status;
            var_alarmSeverity           = ptr_tEnum -> severity;
            dataBuf                     = (void *)&ptr_tEnum -> value;
            dataSize                    = sizeof(dbr_enum_t);
            break;

        case DBR_TIME_CHAR:
            ptr_tChar                   = (struct dbr_time_char *)dataBufRead;
            var_timeStamp               = ptr_tChar -> stamp;
            var_alarmStatus             = ptr_tChar -> status;
            var_alarmSeverity           = ptr_tChar -> severity;
            dataBuf                     = (void *)&ptr_tChar -> value;
            dataSize                    = sizeof(dbr_char_t);
            break;

        case DBR_TIME_LONG:
            ptr_tLong                   = (struct dbr_time_long *)dataBufRead;
            var_timeStamp               = ptr_tLong -> stamp;
            var_alarmStatus             = ptr_tLong -> status;
            var_alarmSeverity           = ptr_tLong -> severity;
            dataBuf                     = (void *)&ptr_tLong -> value;
            dataSize                    = sizeof(dbr_long_t);
            break;

        case DBR_TIME_DOUBLE:
            ptr_tDouble                 = (struct dbr_time_double *)dataBufRead;
            var_timeStamp               = ptr_tDouble -> stamp;
            var_alarmStatus             = ptr_tDouble -> status;
            var_alarmSeverity           = ptr_tDouble -> severity;
            dataBuf                     = (void *)&ptr_tDouble -> value;
            dataSize                    = sizeof(dbr_double_t);
            break;

        default:
            var_timeStamp.secPastEpoch  = 0;
            var_timeStamp.nsec          = 0;
            var_alarmStatus             = NO_ALARM;
            var_alarmSeverity           = NO_ALARM;
            break;
    }

    if(dataBuf)
        memcpy(dataBufOut, dataBuf, pno * dataSize);

    if(mutexId) EPICSLIB_func_mutexUnlock(mutexId);

    // convert the timestamp to string
    epicsTimeToStrftime(buf_timeStampStr, sizeof(buf_timeStampStr),
                        "%a %b %d %Y %H:%M:%S.%f", &var_timeStamp);

    // return the number of point
    return pno;
}

//-----------------------------------------------
// get a waveform with data type conversion, this is quite
// slow process because the data copy has to be done one by one
//-----------------------------------------------
#define CA_WF_READ(dtp)                                                         \
    {unsigned long   i, pno = pointNum;                                         \
    dbr_short_t     *pValueShort;                                               \
    dbr_float_t     *pValueFloat;                                               \
    dbr_enum_t      *pValueEnum;                                                \
    dbr_char_t      *pValueChar;                                                \
    dbr_long_t      *pValueLong;                                                \
    dbr_double_t    *pValueDouble;                                              \
                                                                                \
    if(!dataBufRead || !dataBufOut || pointNum == 0) return 0;                  \
    if(pno > nElems) pno = nElems;                                              \
                                                                                \
    if(mutexId) EPICSLIB_func_mutexMustLock(mutexId);                           \
                                                                                \
    switch(dbrTypeRead) {                                                       \
        case DBR_TIME_SHORT:                                                         \
            ptr_tShort                  = (struct dbr_time_short *)dataBufRead; \
            var_timeStamp               = ptr_tShort -> stamp;                  \
            var_alarmStatus             = ptr_tShort -> status;                 \
            var_alarmSeverity           = ptr_tShort -> severity;               \
                                                                                \
            pValueShort                 = &ptr_tShort -> value;                 \
            for(i = 0; i < pno; i ++)                                           \
                dataBufOut[i] = (dtp)pValueShort[i];                            \
                                                                                \
            break;                                                              \
                                                                                \
        case DBR_TIME_FLOAT:                                                         \
            ptr_tFloat                  = (struct dbr_time_float *)dataBufRead; \
            var_timeStamp               = ptr_tFloat -> stamp;                  \
            var_alarmStatus             = ptr_tFloat -> status;                 \
            var_alarmSeverity           = ptr_tFloat -> severity;               \
                                                                                \
            pValueFloat                 = &ptr_tFloat -> value;                 \
            for(i = 0; i < pno; i ++)                                           \
                dataBufOut[i] = (dtp)pValueFloat[i];                            \
                                                                                \
            break;                                                              \
                                                                                \
        case DBR_TIME_ENUM:                                                          \
            ptr_tEnum                   = (struct dbr_time_enum *)dataBufRead;  \
            var_timeStamp               = ptr_tEnum -> stamp;                   \
            var_alarmStatus             = ptr_tEnum -> status;                  \
            var_alarmSeverity           = ptr_tEnum -> severity;                \
                                                                                \
            pValueEnum                  = &ptr_tEnum -> value;                  \
            for(i = 0; i < pno; i ++)                                           \
                dataBufOut[i] = (dtp)pValueEnum[i];                             \
                                                                                \
            break;                                                              \
                                                                                \
        case DBR_TIME_CHAR:                                                          \
            ptr_tChar                   = (struct dbr_time_char *)dataBufRead;  \
            var_timeStamp               = ptr_tChar -> stamp;                   \
            var_alarmStatus             = ptr_tChar -> status;                  \
            var_alarmSeverity           = ptr_tChar -> severity;                \
                                                                                \
            pValueChar                  = &ptr_tChar -> value;                  \
            for(i = 0; i < pno; i ++)                                           \
                dataBufOut[i] = (dtp)pValueChar[i];                             \
                                                                                \
            break;                                                              \
                                                                                \
        case DBR_TIME_LONG:                                                          \
            ptr_tLong                   = (struct dbr_time_long *)dataBufRead;  \
            var_timeStamp               = ptr_tLong -> stamp;                   \
            var_alarmStatus             = ptr_tLong -> status;                  \
            var_alarmSeverity           = ptr_tLong -> severity;                \
                                                                                \
            pValueLong                  = &ptr_tLong -> value;                  \
            for(i = 0; i < pno; i ++)                                           \
                dataBufOut[i] = (dtp)pValueLong[i];                             \
                                                                                \
            break;                                                              \
                                                                                \
        case DBR_TIME_DOUBLE:                                                        \
            ptr_tDouble                 = (struct dbr_time_double *)dataBufRead;\
            var_timeStamp               = ptr_tDouble -> stamp;                 \
            var_alarmStatus             = ptr_tDouble -> status;                \
            var_alarmSeverity           = ptr_tDouble -> severity;              \
                                                                                \
            pValueDouble                = &ptr_tDouble -> value;                \
            for(i = 0; i < pno; i ++)                                           \
                dataBufOut[i] = (dtp)pValueDouble[i];                           \
                                                                                \
            break;                                                              \
                                                                                \
        default:                                                                \
            var_timeStamp.secPastEpoch  = 0;                                    \
            var_timeStamp.nsec          = 0;                                    \
            var_alarmStatus             = NO_ALARM;                             \
            var_alarmSeverity           = NO_ALARM;                             \
            break;                                                              \
    }                                                                           \
                                                                                \
    if(mutexId) EPICSLIB_func_mutexUnlock(mutexId);                             \
                                                                                \
    epicsTimeToStrftime(buf_timeStampStr, sizeof(buf_timeStampStr),             \
                        "%a %b %d %Y %H:%M:%S.%f", &var_timeStamp);             \
                                                                                \
    return pno;}

int ChannelAccess::getValues(epicsInt8     *dataBufOut, unsigned long pointNum) {CA_WF_READ(epicsInt8)}
int ChannelAccess::getValues(epicsUInt8    *dataBufOut, unsigned long pointNum) {CA_WF_READ(epicsUInt8)}
int ChannelAccess::getValues(epicsInt16    *dataBufOut, unsigned long pointNum) {CA_WF_READ(epicsInt16)}
int ChannelAccess::getValues(epicsUInt16   *dataBufOut, unsigned long pointNum) {CA_WF_READ(epicsUInt16)}
int ChannelAccess::getValues(epicsInt32    *dataBufOut, unsigned long pointNum) {CA_WF_READ(epicsInt32)}
int ChannelAccess::getValues(epicsUInt32   *dataBufOut, unsigned long pointNum) {CA_WF_READ(epicsUInt32)}
int ChannelAccess::getValues(epicsFloat32  *dataBufOut, unsigned long pointNum) {CA_WF_READ(epicsFloat32)}
int ChannelAccess::getValues(epicsFloat64  *dataBufOut, unsigned long pointNum) {CA_WF_READ(epicsFloat64)}

//-----------------------------------------------
// get status
//-----------------------------------------------
int             ChannelAccess::getConnected     ()              {return caConnected;}
int             ChannelAccess::getCAStatus      ()              {return caStatus;}
epicsTimeStamp  ChannelAccess::getTimeStamp     ()              {return var_timeStamp;}
void            ChannelAccess::getTimeStampStr  (char *tsStr)   {if(tsStr) strcpy(tsStr, buf_timeStampStr);}
epicsInt16      ChannelAccess::getAlarmStatus   ()              {return var_alarmStatus;}
epicsInt16      ChannelAccess::getAlarmSeverity ()              {return var_alarmSeverity;}
unsigned long   ChannelAccess::getRPVElemCount  ()              {return nElems;}

//-----------------------------------------------
// connection call back function
//-----------------------------------------------
void ChannelAccess::callBackFunc_connection(struct connection_handler_args arg)
{
    // get the user pointer to the object
    ChannelAccess *pv = (ChannelAccess *)ca_puser(arg.chid);

    // update the status
    if(arg.op == CA_OP_CONN_UP) {

        // update the connection status
        pv -> caConnected = 1;
        pv -> caStatus    = ECA_CONN;

        // for the first time connection, get the configuration and meta data of the channel
        if(!pv -> onceConnected) {           
            // mark to be already once connected
            pv -> onceConnected = 1;            

            // init the CA properties
            pv -> dbfType           = ca_field_type         (pv -> channelID);                                      // get the database field type on the server
            pv -> dbrTypeRead       = dbf_type_to_DBR_TIME  (pv -> dbfType);                                        // reading: value + status + severity + timestamp
            pv -> dbrTypeWrite      = dbf_type_to_DBR       (pv -> dbfType);                                        // writing: naked value

            pv -> nElems            = ca_element_count(pv -> channelID);                                            // get the number of elements on the server
            pv -> reqElemsRead      = (pv -> reqElemsRead) > (pv -> nElems) ? (pv -> nElems) : (pv -> reqElemsRead);// get the smaller one
            pv -> bufSizeRead       = dbr_size_n(pv -> dbrTypeRead,  pv -> nElems);                                 // buffer size for reading all elements

            // create data buffer for the remote PV reading and writing (the pointer will be checked when using it)
            if(pv -> rdCtrl != CA_READ_DISABLED)
                pv -> dataBufRead  = malloc(pv -> bufSizeRead);

            // for monitoring, subscribe the handlers (only use the clean data for monitoring)
            if(pv -> rdCtrl == CA_READ_MONITOR) {
                // do the subscription for the PV with CA monitoring
                pv -> caStatus = ca_create_subscription(pv -> dbrTypeRead,
                                                        pv -> reqElemsRead,
                                                        pv -> channelID,
                                                        DBE_VALUE | DBE_ALARM,
                                                        callBackFunc_readAndMonitor,
                                                        (void *)pv,
                                                        NULL);

                // display the message
                if(pv -> caStatus == ECA_NORMAL) 
                    pv -> onceConnected = 1;                                                                        // just repeat to avoid warning
                else if(pv -> caStatus == ECA_BADCHID)
                    cout << "ERROR: ChannelAccess::callBackFunc_connection: Failed (invalid CHID) to create CA monitor subscription for " << pv -> pvName << ".\n";
                else if(pv -> caStatus == ECA_BADTYPE)
                    cout << "ERROR: ChannelAccess::callBackFunc_connection: Failed (invalid DBR type) to create CA monitor subscription for " << pv -> pvName << ".\n";
                else if(pv -> caStatus == ECA_ALLOCMEM)
                    cout << "ERROR: ChannelAccess::callBackFunc_connection: Failed (unable to allocate memory) to create CA monitor subscription for " << pv -> pvName << ".\n";
                else
                    cout << "ERROR: ChannelAccess::callBackFunc_connection: Failed (unknown reason) to create CA monitor subscription for " << pv -> pvName << ".\n";
            }
        }

    } else if(arg.op == CA_OP_CONN_DOWN) {

        // update the connection status
        pv -> caConnected = 0;
        pv -> caStatus    = ECA_DISCONN;

        cout << "ERROR: ChannelAccess::callBackFunc_connection: Connection broken for CA channel for " << pv -> pvName << "\n";
    }

    // execute the user call back
    if(pv -> connUserCallback && pv -> userPtr)
        (*pv -> connUserCallback)(pv -> userPtr);

    // fire the event for other applications
    if(pv -> connEvent)
        EPICSLIB_func_eventSignal(pv -> connEvent);
}

//-----------------------------------------------
// read or montoring call back function (executed when the reading is done)
//-----------------------------------------------
void ChannelAccess::callBackFunc_readAndMonitor(struct event_handler_args arg)
{
    // get the user pointer to the object
    ChannelAccess *pv = (ChannelAccess *)arg.usr;

    // get data returned
    pv -> caStatus = arg.status;

    if(arg.status == ECA_NORMAL) {
        pv -> nElems  = arg.count;

        if(pv -> dataBufRead) {
			if(pv -> mutexId) EPICSLIB_func_mutexMustLock(pv -> mutexId);
            memcpy(pv -> dataBufRead, arg.dbr, dbr_size_n(arg.type, arg.count));
			if(pv -> mutexId) EPICSLIB_func_mutexUnlock(pv -> mutexId);
        }
    }

    // save the data to external buffer mainly for monitored data
    if(pv -> dataBufMonitor) {
        pv -> getValuesRaw(pv -> dataBufMonitor, pv -> reqElemsRead);

        /*if(strcmp(ca_name(arg.chid), "MINSB03-RLLE-STA:MASTER-OPSTATESTATUS-GUI") == 0) {
            cout << "=== Get monitor value for " << ca_name(arg.chid) << ", NELM = " << pv -> reqElemsRead << ", value = " << *((epicsUInt16 *)pv -> dataBufMonitor) <<  endl;
            cout << "=== Time stamp is " << pv -> buf_timeStampStr << ", alarm status = " << pv -> var_alarmStatus << ", severity is " << pv -> var_alarmSeverity << endl;
        }*/
    }

    // execute the user call back
    if(pv -> rdUserCallback && pv -> userPtr)
        (*pv -> rdUserCallback)(pv -> userPtr);

    // fire the event for other applications
    if(pv -> rdEvent)
        EPICSLIB_func_eventSignal(pv -> rdEvent);
}

//-----------------------------------------------
// write call back function (executed when the writing is done)
//-----------------------------------------------
void ChannelAccess::callBackFunc_write(struct event_handler_args arg)
{
    // get the user pointer to the object
    ChannelAccess *pv = (ChannelAccess *)arg.usr;

    // get status returned
    pv -> caStatus = arg.status;

    // execute the user call back
    if(pv -> wtUserCallback && pv -> userPtr)
        (*pv -> wtUserCallback)(pv -> userPtr);

    // fire the event for other applications
    if(pv -> wtEvent)
        EPICSLIB_func_eventSignal(pv -> wtEvent);
}

//-----------------------------------------------
// construction for CA context
//-----------------------------------------------
ChannelAccessContext::ChannelAccessContext(CA_enum_contextCtrl contextCtrl)
{
    // init the pointer
    caContext = NULL;

    // create the context
    if(contextCtrl == CA_SINGLE_THREAD) {
        caContextStatus = ca_context_create(ca_disable_preemptive_callback);
    } else if(contextCtrl == CA_MULTIPLE_THREAD) {
        caContextStatus = ca_context_create(ca_enable_preemptive_callback);
    }

    // show the status
    if(caContextStatus == ECA_NORMAL) {
        caContext = ca_current_context();
        cout << "INFO: ChannelAccessContext::ChannelAccessContext: Succeed to created the CA context.\n";
    } else if(caContextStatus == ECA_NOTTHREADED) {
        cout << "ERROR: ChannelAccessContext::ChannelAccessContext: Failed (a nonpreemptive CA context exists) to created the CA context!\n";
    } else if(caContextStatus == ECA_ALLOCMEM) {
        cout << "ERROR: ChannelAccessContext::ChannelAccessContext: Failed (memory allocation error) to created the CA context!\n";
    } else {
        cout << "ERROR: ChannelAccessContext::ChannelAccessContext: Failed (unknown reason) to created the CA context!\n";    
    }
}

//-----------------------------------------------
// destruction for CA context
//-----------------------------------------------
ChannelAccessContext::~ChannelAccessContext()
{
    ca_context_destroy();
}

//-----------------------------------------------
// attach the caller thread to the context
//-----------------------------------------------
int ChannelAccessContext::attachCurrentThread()
{
    // attach
    caContextStatus = ca_attach_context(caContext);

    // show the status
    if(caContextStatus == ECA_NORMAL) {
        cout << "INFO: ChannelAccessContext::attachCurrentThread: Succeed to attach to the CA context.\n";
    } else if(caContextStatus == ECA_NOTTHREADED) {
        cout << "ERROR: ChannelAccessContext::attachCurrentThread: Failed (CA context not preemptive) to attach to the CA context!\n";
    } else if(caContextStatus == ECA_ISATTACHED) {
        cout << "ERROR: ChannelAccessContext::attachCurrentThread: Failed (already attached) to attach to the CA context!\n";
    } else {
        cout << "ERROR: ChannelAccessContext::attachCurrentThread: Failed (unknow reason) to attach to the CA context!\n";    
    }

    return caContextStatus;
}

//-----------------------------------------------
// display the status of the context
//-----------------------------------------------
int  ChannelAccessContext::displayStatus()
{
    ca_context_status(caContext, 100000);
    return 0;
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

