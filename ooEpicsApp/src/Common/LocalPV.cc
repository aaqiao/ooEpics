//=========================================================
// LocalPV.cc
// 
// Class definition for the local PV
// 
// Created by: Zheqiao Geng
// Created on: 12/11/2012
// Description: Initial creation 
// 
// Modified by: Zheqiao Geng
// Modified on: 12/16/2012
// Description: Add the function to support point numbers and callback functions
//
// Modified by Andreas Hauff on 16.11.2016
// Added EPICS data type oldString
//
// Modified by Zheqiao Geng on 21.03.2016
// Extended the implementation to provide the possibility with internal data buffers
//=========================================================
#include "LocalPV.h"

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction for LocalPV 
// The data node will be deleted by the INTD module when EPICS exits
//-----------------------------------------------
LocalPV::LocalPV()
{
    // init all variables
    rCallback           = NULL;
    wCallback           = NULL;

    node                = NULL;

    var_moduleName.assign   ("");
    var_dataName.assign     ("");
    var_supStr.assign       ("");
    var_unitStr.assign      ("");
    
    var_pointNum        = 1;
    ptr_privatePtr      = NULL;
    var_recordType      = INTD_AO; 
    var_scanType        = INTD_PASSIVE;  
    ptr_ioIntScan       = NULL; 
    var_mutexId         = NULL;
    var_eventId         = NULL; 

    var_wfDataType      = INTD_DOUBLE;   
    var_dataType        = INTD_DOUBLE;  

    ptr_dataBuf         = NULL; 

    var_onceCreated     = 0;
}

//-----------------------------------------------
// destruction for LocalPV
//-----------------------------------------------
LocalPV::~LocalPV() 
{
    if(ptr_dataBuf)
        free(ptr_dataBuf);
}

//-----------------------------------------------
// force to set the output PV values
//-----------------------------------------------
int LocalPV::forceOPVValue(void *dataPtr)
{
	return INTD_API_forceOPVValue(node, dataPtr);
}

//-----------------------------------------------
// force to process the PV
//-----------------------------------------------
int LocalPV::forcePVProcess()
{
	return INTD_API_forcePVProcess(node);
}

//-----------------------------------------------
// raise an alarm
//-----------------------------------------------
int LocalPV::raiseAlarm(epicsEnum16 nsta, epicsEnum16 nsevr)
{
    return INTD_API_raiseAlarm(node, nsta, nsevr);
}

//-----------------------------------------------
// set the timestamp of the record
//-----------------------------------------------
int LocalPV::setTimeStamp(epicsTimeStamp tsIn)
{
    // to be implemented later (need to modify the InternalData)
    return 0;
}
//-----------------------------------------------
// get status
//-----------------------------------------------
int LocalPV::isCreated()    {return var_onceCreated;}               // check if the creation is good or not
int LocalPV::isRecLinked()  {return 1;}                             // check if successfuly mapped with record or not (to be implemented later)

//-----------------------------------------------
// set waveform datatype, this should be called before initializing a waveform local PV
//-----------------------------------------------
void LocalPV::setWfDataType(INTD_enum_dataType wfDataType)
{
    var_wfDataType = wfDataType;
}

//-----------------------------------------------
// init the local PV for the case with local buffers
//-----------------------------------------------
void LocalPV::init( string                  moduleName,
                    string                  devName,
                    string                  valName,
                    string                  supStr,
                    string                  unitStr,
                    unsigned int            pointNum,
                    void                   *privatePtr,
                    INTD_CALLBACK           callback,
                    INTD_enum_recordType    recordType,
                    INTD_enum_scanType      scanType,
                    IOSCANPVT              *ioIntScan,
                    epicsMutexId            mutexId,
                    epicsEventId            eventId)
{
    unsigned int dataSize;

    // record the input
    var_moduleName      = moduleName;
    var_dataName        = devName + "|" + valName;
    var_supStr          = supStr;
    var_unitStr         = unitStr;
    var_pointNum        = pointNum;
    ptr_privatePtr      = privatePtr;
    var_recordType      = recordType;
    var_scanType        = scanType;
    ptr_ioIntScan       = ioIntScan;
    var_mutexId         = mutexId;
    var_eventId         = eventId;

    // manipulate the callback
    fun_maniCallback(recordType, callback);

    // get data type from the record type
    switch(recordType) {
        case INTD_AO    :
        case INTD_AI    : var_dataType = INTD_DOUBLE;   break;
        case INTD_BO    :
        case INTD_BI    : var_dataType = INTD_USHORT;   break;
        case INTD_LO    :
        case INTD_LI    : var_dataType = INTD_LONG;     break;
        case INTD_MBBO  :
        case INTD_MBBI  : var_dataType = INTD_USHORT;   break;
        case INTD_WFO   :
        case INTD_WFI   : var_dataType = var_wfDataType;break;
        case INTD_SO    :
        case INTD_SI    : var_dataType = INTD_CHAR;     break;
        default         : var_dataType = INTD_DOUBLE;   break;
    }

    // get data size from the data type
    dataSize = fun_getDataSize(var_dataType);

    if(dataSize == 0)
        return;

    // create buffer for the data and clear it
    ptr_dataBuf  = malloc(dataSize * pointNum);

    if(ptr_dataBuf) {
        // clear the buffer
        memset(ptr_dataBuf, 0, dataSize * pointNum);

        // create the data node
        node = INTD_API_createDataNode( var_moduleName.c_str(), 
                                        var_dataName.c_str(), 
                                        var_supStr.c_str(), 
                                        var_unitStr.c_str(), 
                                        ptr_dataBuf, 
                                        ptr_privatePtr, 
                                        var_pointNum, 
                                        ptr_ioIntScan, 
                                        var_dataType, 
                                        rCallback, 
                                        wCallback, 
                                        var_mutexId, 
                                        var_eventId, 
                                        var_recordType, 
                                        var_scanType);  

        // check the success of creation
        if(node)
            var_onceCreated = 1;
    }
}

//-----------------------------------------------
// get value of a scalar output PV. the data type will be converted to the required output
// type. please note that there may be resolution lost when converting to lower level format.
//-----------------------------------------------
#define LPV_VAL_READ(dtp)                                                               \
    {dtp val = 0;                                                                       \
                                                                                        \
    if(!var_onceCreated ) return 0;                                                     \
                                                                                        \
    if(var_mutexId) epicsMutexMustLock(var_mutexId);                                    \
                                                                                        \
    switch(var_recordType) {                                                            \
        case INTD_AO    : val = (dtp)(*((epicsFloat64 *)ptr_dataBuf)); break;           \
        case INTD_BO    : val = (dtp)(*((epicsUInt16  *)ptr_dataBuf)); break;           \
        case INTD_LO    : val = (dtp)(*((epicsInt32   *)ptr_dataBuf)); break;           \
        case INTD_MBBO  : val = (dtp)(*((epicsUInt16  *)ptr_dataBuf)); break;           \
        default: break;                                                                 \
    }                                                                                   \
                                                                                        \
    if(var_mutexId) epicsMutexUnlock(var_mutexId);                                      \
                                                                                        \
    return val;}

epicsInt8       LocalPV::getValueInt8    () {LPV_VAL_READ(epicsInt8)}
epicsUInt8      LocalPV::getValueUInt8   () {LPV_VAL_READ(epicsUInt8)}
epicsInt16      LocalPV::getValueInt16   () {LPV_VAL_READ(epicsInt16)}
epicsUInt16     LocalPV::getValueUInt16  () {LPV_VAL_READ(epicsUInt16)}
epicsInt32      LocalPV::getValueInt32   () {LPV_VAL_READ(epicsInt32)}
epicsUInt32     LocalPV::getValueUInt32  () {LPV_VAL_READ(epicsUInt32)}
epicsFloat32    LocalPV::getValueFloat32 () {LPV_VAL_READ(epicsFloat32)}
epicsFloat64    LocalPV::getValueFloat64 () {LPV_VAL_READ(epicsFloat64)}

int LocalPV::getValueString(char *strOut)                                       // return value is the number of char copied
{
    // check the data buffer, record type and input
    if(!strOut || !var_onceCreated || var_recordType != INTD_SO)
        return 0;

    // copy the data
    strncpy(strOut, (char *)ptr_dataBuf, MAX_STRING_SIZE);

    return strlen(strOut);
}

string LocalPV::getValueString()
{
    string strOut("");    

    // check record type and input
    if(!var_onceCreated || var_recordType != INTD_SO)
        return strOut;

    // copy the data
    strOut.assign((char *)ptr_dataBuf);

    return strOut;
}

//-----------------------------------------------
// set value of a scalar input PV, the input data will be converted to the datatype of the PV.
// here epicsFloat64 type is used as the argument because any input type will be first
// converted to it without losing resolutions.
// return:
//      0 - success; 1 - failure (wrong buffer or PV type not supported)
//-----------------------------------------------
int LocalPV::setValue(epicsFloat64 dataIn)
{
    int status = 0;

    // check the buffer
    if(!var_onceCreated) 
        return 1;

    // set the data based on record type
    if(var_mutexId) epicsMutexMustLock(var_mutexId);

    switch(var_recordType) {
        case INTD_AI    : *((epicsFloat64 *)ptr_dataBuf) =              dataIn; break;
        case INTD_BI    : *((epicsUInt16  *)ptr_dataBuf) = (epicsUInt16)dataIn; break;
        case INTD_LI    : *((epicsInt32   *)ptr_dataBuf) = (epicsInt32) dataIn; break;
        case INTD_MBBI  : *((epicsUInt16  *)ptr_dataBuf) = (epicsUInt16)dataIn; break;
        default: status = 1; break;            
    }

    if(var_mutexId) epicsMutexUnlock(var_mutexId);

    return status;
}

//-----------------------------------------------
// set value of a string input PV
// return:
//      0 - success; 1 - failure (wrong buffer or PV type not supported)
//-----------------------------------------------
int LocalPV::setString(char *strIn)
{
    // check the data buffer, record type and input
    if(!strIn || !var_onceCreated || var_recordType != INTD_SI)
        return 1;

    // copy the data
    strncpy((char *)ptr_dataBuf, strIn, MAX_STRING_SIZE);

    return 0;
}

int LocalPV::setString(string strIn)
{
    // check the data buffer, record type and input
    if(!var_onceCreated || var_recordType != INTD_SI)
        return 1;

    // copy the data
    strncpy((char *)ptr_dataBuf, strIn.c_str(), MAX_STRING_SIZE);

    return 0;
}

//-----------------------------------------------
// get data from a  waveform out PV without datatype conversion. 
// the return value is the number of points that is actually read
//-----------------------------------------------
int LocalPV::getValuesRaw(void *dataOut, unsigned int pointNum)
{
    unsigned int pno = pointNum;
    unsigned int dataSize;

    // check the input, data buffer and record type
    if(!dataOut || pointNum == 0 || !var_onceCreated || var_recordType != INTD_WFO)
        return 0;

    // get the actual number of point
    if(pno > var_pointNum)
        pno = var_pointNum;

    // get the data size
    dataSize = fun_getDataSize(var_dataType);
   
    if(dataSize == 0)
        return 0;    

    // copy the data
    if(var_mutexId) epicsMutexMustLock(var_mutexId);
    memcpy(dataOut, ptr_dataBuf, pno * dataSize);
    if(var_mutexId) epicsMutexUnlock(var_mutexId);

    return pno;
}

//-----------------------------------------------
// get a waveform out data with type conversion. 
// the return value is the number of points that is actually read
//-----------------------------------------------
#define LPV_WF_READ(dtp)                                                        \
    {unsigned int    i, pno = pointNum;                                         \
                                                                                \
    epicsInt8      *pValueChar;                                                 \
    epicsUInt8     *pValueUChar;                                                \
    epicsInt16     *pValueShort;                                                \
    epicsUInt16    *pValueUShort;                                               \
    epicsInt32     *pValueLong;                                                 \
    epicsUInt32    *pValueULong;                                                \
    epicsFloat32   *pValueFloat;                                                \
    epicsFloat64   *pValueDouble;                                               \
                                                                                \
    if(!dataOut || pointNum == 0 || !var_onceCreated                            \
        || var_recordType != INTD_WFO)                                          \
        return 0;                                                               \
                                                                                \
    if(pno > var_pointNum)                                                      \
        pno = var_pointNum;                                                     \
                                                                                \
    if(var_mutexId) epicsMutexMustLock(var_mutexId);                            \
                                                                                \
    switch(var_dataType) {                                                      \
        case INTD_CHAR      :                                                   \
            pValueChar     = (epicsInt8 *)ptr_dataBuf;                          \
            for(i = 0; i < pno; i ++)                                           \
                dataOut[i] = (dtp)pValueChar[i];                                \
            break;                                                              \
                                                                                \
        case INTD_UCHAR     :                                                   \
            pValueUChar    = (epicsUInt8 *)ptr_dataBuf;                         \
            for(i = 0; i < pno; i ++)                                           \
                dataOut[i] = (dtp)pValueUChar[i];                               \
            break;                                                              \
                                                                                \
        case INTD_SHORT     :                                                   \
            pValueShort    = (epicsInt16 *)ptr_dataBuf;                         \
            for(i = 0; i < pno; i ++)                                           \
                dataOut[i] = (dtp)pValueShort[i];                               \
            break;                                                              \
                                                                                \
        case INTD_USHORT    :                                                   \
            pValueUShort   = (epicsUInt16 *)ptr_dataBuf;                        \
            for(i = 0; i < pno; i ++)                                           \
                dataOut[i] = (dtp)pValueUShort[i];                              \
            break;                                                              \
                                                                                \
        case INTD_INT       :                                                   \
        case INTD_LONG      :                                                   \
            pValueLong     = (epicsInt32 *)ptr_dataBuf;                         \
            for(i = 0; i < pno; i ++)                                           \
                dataOut[i] = (dtp)pValueLong[i];                                \
            break;                                                              \
                                                                                \
        case INTD_UINT      :                                                   \
        case INTD_ULONG     :                                                   \
            pValueULong    = (epicsUInt32 *)ptr_dataBuf;                        \
            for(i = 0; i < pno; i ++)                                           \
                dataOut[i] = (dtp)pValueULong[i];                               \
            break;                                                              \
                                                                                \
        case INTD_FLOAT     :                                                   \
            pValueFloat    = (epicsFloat32 *)ptr_dataBuf;                       \
            for(i = 0; i < pno; i ++)                                           \
                dataOut[i] = (dtp)pValueFloat[i];                               \
            break;                                                              \
                                                                                \
        case INTD_DOUBLE    :                                                   \
            pValueDouble   = (epicsFloat64 *)ptr_dataBuf;                       \
            for(i = 0; i < pno; i ++)                                           \
                dataOut[i] = (dtp)pValueDouble[i];                              \
            break;                                                              \
                                                                                \
        default :                                                               \
            pno = 0;                                                            \
            break;                                                              \
    }                                                                           \
                                                                                \
    if(var_mutexId) epicsMutexUnlock(var_mutexId);                              \
                                                                                \
    return pno;}

int LocalPV::getValues(epicsInt8     *dataOut, unsigned int pointNum) {LPV_WF_READ(epicsInt8)}
int LocalPV::getValues(epicsUInt8    *dataOut, unsigned int pointNum) {LPV_WF_READ(epicsUInt8)}
int LocalPV::getValues(epicsInt16    *dataOut, unsigned int pointNum) {LPV_WF_READ(epicsInt16)}
int LocalPV::getValues(epicsUInt16   *dataOut, unsigned int pointNum) {LPV_WF_READ(epicsUInt16)}
int LocalPV::getValues(epicsInt32    *dataOut, unsigned int pointNum) {LPV_WF_READ(epicsInt32)}
int LocalPV::getValues(epicsUInt32   *dataOut, unsigned int pointNum) {LPV_WF_READ(epicsUInt32)}
int LocalPV::getValues(epicsFloat32  *dataOut, unsigned int pointNum) {LPV_WF_READ(epicsFloat32)}
int LocalPV::getValues(epicsFloat64  *dataOut, unsigned int pointNum) {LPV_WF_READ(epicsFloat64)}

//-----------------------------------------------
// set a waveform in data. the return value is the number of points that is actually writen
// there is no data conversion here
//-----------------------------------------------
int LocalPV::setValuesRaw(void *dataIn, unsigned int pointNum)
{
    unsigned int pno = pointNum;
    unsigned int dataSize;

    // check the input, data buffer and record type
    if(!dataIn || pointNum == 0 || !var_onceCreated || var_recordType != INTD_WFI)
        return 0;

    // get the actual number of point
    if(pno > var_pointNum)
        pno = var_pointNum;

    // get the data size
    dataSize = fun_getDataSize(var_dataType);
    
    if(dataSize == 0)
        return 0;    

    // copy the data
    if(var_mutexId) epicsMutexMustLock(var_mutexId);
    memcpy(ptr_dataBuf, dataIn, pno * dataSize);
    if(var_mutexId) epicsMutexUnlock(var_mutexId);

    return pno;
}

//-----------------------------------------------
// set a waveform in data. the return value is the number of points that is actually writen
// the data type will be converted
//-----------------------------------------------
#define LPV_WF_WRITE(dtp)                                                       \
    {unsigned int    i, pno = pointNum;                                         \
                                                                                \
    epicsInt8      *pValueChar;                                                 \
    epicsUInt8     *pValueUChar;                                                \
    epicsInt16     *pValueShort;                                                \
    epicsUInt16    *pValueUShort;                                               \
    epicsInt32     *pValueLong;                                                 \
    epicsUInt32    *pValueULong;                                                \
    epicsFloat32   *pValueFloat;                                                \
    epicsFloat64   *pValueDouble;                                               \
                                                                                \
    if(!dataIn || pointNum == 0 || !var_onceCreated                             \
       || var_recordType != INTD_WFI)                                           \
        return 0;                                                               \
                                                                                \
    if(pno > var_pointNum)                                                      \
        pno = var_pointNum;                                                     \
                                                                                \
    if(var_mutexId) epicsMutexMustLock(var_mutexId);                            \
                                                                                \
    switch(var_dataType) {                                                      \
        case INTD_CHAR      :                                                   \
            pValueChar     = (epicsInt8 *)ptr_dataBuf;                          \
            for(i = 0; i < pno; i ++)                                           \
                pValueChar[i] = (dtp)dataIn[i];                                 \
            break;                                                              \
                                                                                \
        case INTD_UCHAR     :                                                   \
            pValueUChar    = (epicsUInt8 *)ptr_dataBuf;                         \
            for(i = 0; i < pno; i ++)                                           \
                pValueUChar[i] = (dtp)dataIn[i];                                \
            break;                                                              \
                                                                                \
        case INTD_SHORT     :                                                   \
            pValueShort    = (epicsInt16 *)ptr_dataBuf;                         \
            for(i = 0; i < pno; i ++)                                           \
                pValueShort[i] = (dtp)dataIn[i];                                \
            break;                                                              \
                                                                                \
        case INTD_USHORT    :                                                   \
            pValueUShort   = (epicsUInt16 *)ptr_dataBuf;                        \
            for(i = 0; i < pno; i ++)                                           \
                pValueUShort[i] = (dtp)dataIn[i];                               \
            break;                                                              \
                                                                                \
        case INTD_INT       :                                                   \
        case INTD_LONG      :                                                   \
            pValueLong     = (epicsInt32 *)ptr_dataBuf;                         \
            for(i = 0; i < pno; i ++)                                           \
                pValueLong[i] = (dtp)dataIn[i];                                 \
            break;                                                              \
                                                                                \
        case INTD_UINT      :                                                   \
        case INTD_ULONG     :                                                   \
            pValueULong    = (epicsUInt32 *)ptr_dataBuf;                        \
            for(i = 0; i < pno; i ++)                                           \
                pValueULong[i] = (dtp)dataIn[i];                                \
            break;                                                              \
                                                                                \
        case INTD_FLOAT     :                                                   \
            pValueFloat    = (epicsFloat32 *)ptr_dataBuf;                       \
            for(i = 0; i < pno; i ++)                                           \
                pValueFloat[i] = (dtp)dataIn[i];                                \
            break;                                                              \
                                                                                \
        case INTD_DOUBLE    :                                                   \
            pValueDouble   = (epicsFloat64 *)ptr_dataBuf;                       \
            for(i = 0; i < pno; i ++)                                           \
                pValueDouble[i] = (dtp)dataIn[i];                               \
            break;                                                              \
                                                                                \
        default :                                                               \
            pno = 0;                                                            \
            break;                                                              \
    }                                                                           \
                                                                                \
    if(var_mutexId) epicsMutexUnlock(var_mutexId);                              \
                                                                                \
    return pno;}

int LocalPV::setValues(epicsInt8     *dataIn, unsigned int pointNum) {LPV_WF_WRITE(epicsInt8)}
int LocalPV::setValues(epicsUInt8    *dataIn, unsigned int pointNum) {LPV_WF_WRITE(epicsUInt8)}
int LocalPV::setValues(epicsInt16    *dataIn, unsigned int pointNum) {LPV_WF_WRITE(epicsInt16)}
int LocalPV::setValues(epicsUInt16   *dataIn, unsigned int pointNum) {LPV_WF_WRITE(epicsUInt16)}
int LocalPV::setValues(epicsInt32    *dataIn, unsigned int pointNum) {LPV_WF_WRITE(epicsInt32)}
int LocalPV::setValues(epicsUInt32   *dataIn, unsigned int pointNum) {LPV_WF_WRITE(epicsUInt32)}
int LocalPV::setValues(epicsFloat32  *dataIn, unsigned int pointNum) {LPV_WF_WRITE(epicsFloat32)}
int LocalPV::setValues(epicsFloat64  *dataIn, unsigned int pointNum) {LPV_WF_WRITE(epicsFloat64)}

//-----------------------------------------------
// manipulate the callback functions
//-----------------------------------------------
void LocalPV::fun_maniCallback(INTD_enum_recordType recordType, INTD_CALLBACK callback)
{
    switch(recordType) {
        case INTD_AO    :
        case INTD_BO    :
        case INTD_LO    :
        case INTD_MBBO  :
        case INTD_WFO   :
        case INTD_SO    :
            rCallback = NULL;
            wCallback = callback;
            break;

        case INTD_AI    :
        case INTD_BI    :
        case INTD_LI    :
        case INTD_MBBI  :
        case INTD_WFI   :
        case INTD_SI    :
            rCallback = callback;
            wCallback = NULL;
            break;
        
        default:
            rCallback = NULL;
            wCallback = NULL;
            break;            
    }
}

//-----------------------------------------------
// get data size
//-----------------------------------------------
unsigned int LocalPV::fun_getDataSize(INTD_enum_dataType dataType)
{   
    unsigned int dsize;

    switch(dataType) {
        case INTD_CHAR      :
        case INTD_UCHAR     :
            dsize = sizeof(epicsInt8);
            break;

        case INTD_SHORT     :
        case INTD_USHORT    :
            dsize = sizeof(epicsInt16);
            break;

        case INTD_INT       :
        case INTD_UINT      :
        case INTD_LONG      :
        case INTD_ULONG     :
            dsize = sizeof(epicsInt32);
            break;

        case INTD_FLOAT     :
            dsize = sizeof(epicsFloat32);
            break;

        case INTD_DOUBLE    :
            dsize = sizeof(epicsFloat64);
            break;

        default :
            dsize = 0;
            break;            
    }

    return dsize;
}

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************
