//=========================================================
// LocalPV.h
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
// Modified by: Zheqiao Geng
// Modified on: 2013.11.13
// Description: 1. Use the epicsTypes to make the code adaptable to both 32-bit and 64-bit machines
//              2. Implement the localPV to support the I/O interrupt scanning, event and mutex
//
// Modified by Andreas Hauff on 16.11.2015
// Added EPICS data type oldString
//
// Modified by Zheqiao Geng on 21.03.2016
// Extended the implementation to provide the possibility with internal data buffers
//
// Modified by Zheqiao Geng on 27.05.2016
// Deleted the old interfaces
//=========================================================
#ifndef LOCALPV_H
#define LOCALPV_H

#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <string>

#include "InternalData.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// the class definition for local PV. It is an OO wrapper for the InternalData,
// now only support a subset of the functions 
//-----------------------------------------------
class LocalPV
{
public:
    LocalPV();    
   ~LocalPV();

    // force the value of the output PV (AO, BO, LO, MBBO, WFO) to change (also process the PV)
    int forceOPVValue   (void *dataPtr);
    int forcePVProcess  ();
    int raiseAlarm      (epicsEnum16 nsta, epicsEnum16 nsevr);
    int setTimeStamp    (epicsTimeStamp tsIn);

    // get status
    int isCreated();
    int isRecLinked();

    // init the object for the case with local data buffers
    void setWfDataType(INTD_enum_dataType wfDataType);

    void init(  string                  moduleName, 
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
                epicsEventId            eventId);  

    // get values and set values for the case with local data buffers
    epicsInt8       getValueInt8    ();                                 // get value of an output PV with data conversion
    epicsUInt8      getValueUInt8   ();
    epicsInt16      getValueInt16   ();
    epicsUInt16     getValueUInt16  ();
    epicsInt32      getValueInt32   ();
    epicsUInt32     getValueUInt32  ();
    epicsFloat32    getValueFloat32 ();
    epicsFloat64    getValueFloat64 ();
    int             getValueString  (char *strOut);
    string          getValueString  ();

    int setValue (epicsFloat64 dataIn);                                 // set value of a input PV with data conversion
    int setString(char *strIn);                                         // set value of a stringin record
    int setString(string strIn);

    int getValuesRaw(void          *dataOut, unsigned int pointNum);    // get values of a waveform out PV without data conversion
    int getValues   (epicsInt8     *dataOut, unsigned int pointNum);    // get values of a waveform out PV with data conversion
    int getValues   (epicsUInt8    *dataOut, unsigned int pointNum);
    int getValues   (epicsInt16    *dataOut, unsigned int pointNum);
    int getValues   (epicsUInt16   *dataOut, unsigned int pointNum);
    int getValues   (epicsInt32    *dataOut, unsigned int pointNum);
    int getValues   (epicsUInt32   *dataOut, unsigned int pointNum);
    int getValues   (epicsFloat32  *dataOut, unsigned int pointNum);
    int getValues   (epicsFloat64  *dataOut, unsigned int pointNum);

    int setValuesRaw(void          *dataIn, unsigned int pointNum);
    int setValues   (epicsInt8     *dataIn, unsigned int pointNum);
    int setValues   (epicsUInt8    *dataIn, unsigned int pointNum);
    int setValues   (epicsInt16    *dataIn, unsigned int pointNum);
    int setValues   (epicsUInt16   *dataIn, unsigned int pointNum);
    int setValues   (epicsInt32    *dataIn, unsigned int pointNum);
    int setValues   (epicsUInt32   *dataIn, unsigned int pointNum);
    int setValues   (epicsFloat32  *dataIn, unsigned int pointNum);
    int setValues   (epicsFloat64  *dataIn, unsigned int pointNum);

private:
    INTD_CALLBACK rCallback;
    INTD_CALLBACK wCallback;

    INTD_struc_node *node;

    // data for the case with local data buffers
    string                  var_moduleName;
    string                  var_dataName;
    string                  var_supStr;
    string                  var_unitStr;
    unsigned int            var_pointNum;
    void                   *ptr_privatePtr;
    INTD_enum_recordType    var_recordType; 
    INTD_enum_scanType      var_scanType;  
    IOSCANPVT              *ptr_ioIntScan; 
    epicsMutexId            var_mutexId;
    epicsEventId            var_eventId;

    INTD_enum_dataType      var_wfDataType;                     // datatype for the array
    INTD_enum_dataType      var_dataType;                       // datatype finally used

    void                   *ptr_dataBuf;

    // status
    int                     var_onceCreated;                    // 1 means successfully created
    
    // private functions
    void fun_maniCallback(INTD_enum_recordType recordType, INTD_CALLBACK callback);
    unsigned int fun_getDataSize(INTD_enum_dataType dataType);
};

} 
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif
