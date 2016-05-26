/****************************************************
 * InternalData.h
 * 
 * Header file for the InternalData module
 * This module acts as the isolation layer between EPICS records and inernal code
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.17
 * Description: Initial creation
 *              To be done: Define an interrupt I/O scanning list and API so that the user code could be
 *                able to trigger the scanning of specified records
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2011.06.07
 * Description: Introduce the I/O interrupt scanning
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2/7/2013
 * Description: allow to specify the path for saving the files
 *
 * Modified by Zheqiao Geng on 2014.05.20
 * Upgrade the API for database generateion and add the API for request file and list file gen.
 * Input supplementary string into the data node for bi,bo,mbbi,mbbi strings
 *
 * Modified by Zheqiao Geng on 2014.06.02
 * Added "volatile" keyword for the data pointer (dataPtr)
 *
 * Modified by Zheqiao Geng on 2014.09.27
 * Add optional sub-module name to make up the PV names
 *
 * Modified by Zheqiao Geng on 2015.04.24
 * Added the unit string in the PVs
 *
 * Modified by Andreas Hauff on 16.11.2015
 * Added epicsOldString as data type
 *
 * Modified by Zheqiao Geng on 18.12.2015
 * Added the function to generate save/restore request file and archiver conf file with different
 * module name string
 ****************************************************/
#ifndef INTERNAL_DATA_H
#define INTERNAL_DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dbAddr.h>
#include <dbScan.h>
#include <dbCommon.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsTypes.h>
#include <epicsExit.h>
#include <ellLib.h>
#include <alarm.h>
#include <recGbl.h>

#define INTD_STR_RECORD_LEN 40

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Data type supported
 */
typedef enum {
    INTD_CHAR,
    INTD_UCHAR,
    INTD_SHORT,
    INTD_USHORT,
    INTD_INT,
    INTD_UINT,
    INTD_LONG,
    INTD_ULONG,
    INTD_FLOAT,
    INTD_DOUBLE,
    INTD_OLDSTRING
} INTD_enum_dataType;

/**
 * Record type supported
 */
typedef enum {
    INTD_AO,
    INTD_AI,
    INTD_BO,
    INTD_BI,
    INTD_LO,
    INTD_LI,
    INTD_MBBO,
    INTD_MBBI,
    INTD_WFO,
    INTD_WFI,
    INTD_SO,
    INTD_SI
} INTD_enum_recordType;

typedef enum {
    INTD_PASSIVE,
    INTD_IOINT,
    INTD_10S,
    INTD_5S,
    INTD_2S,
    INTD_1S,
    INTD_D5S,
    INTD_D2S,
    INTD_D1S
} INTD_enum_scanType;

/**
 * Function pointer definition
 */
typedef void (*INTD_CALLBACK)(void*);

/**
 * Data structure of the internal data
 */
typedef struct {
    ELLNODE              node;                   /* node for elllist */
    char                 moduleName[128];        /* module name that this data belongs to */
    char                 dataName[128];          /* internal data name */
    char                 subModuleName[128];     /* sub-module name - used for db generation */
    char                 recName[128];           /* record name - used for db generation */
    char                 supStr[256];			 /* additional string for bi,bi,mbbi or mbbo */
    char                 unitStr[16];            /* unit string */
    void                *dataPtr;                /* pointer to the data (use volatile, later the module may be extended to allow dynamic internal variable association) */
    void                *privateData;            /* private data that might be useful in the call back functions */
    unsigned int         pno;                    /* number of the points */
    IOSCANPVT           *ioIntScan;              /* pointer of the ioscanpvt */
    INTD_enum_dataType   dataType;               /* data type */
    INTD_CALLBACK        readCallback;           /* call back function when reading the data */
    INTD_CALLBACK        writeCallback;          /* call back function when writing the data */
    dbCommon            *epicsRecord;            /* EPICS record that is associate with this internal data node */    
    epicsMutexId         mutexId;                /* provide the possibility for locking, the mutex should be defined in user code */
    epicsEventId         eventId;                /* allow signal a event when this internal data is accessed */
    INTD_enum_recordType recordType;             /* record type for this data */
    INTD_enum_scanType   scanType;               /* scan type for this data */
    int                  enableCallback;         /* enable/disable the callback */
    epicsEnum16          nsta;                   /* alarm status */
    epicsEnum16          nsevr;                  /* alarm severity */
} INTD_struc_node;

/**
 * Management routines
 */
INTD_struc_node *INTD_API_createDataNode(const char *moduleName, 
                            const char *dataName, 
                            const char *supStr,
                            const char *unitStr,
                            void *dataPtr, 
                            void *privateData,
                            unsigned int pno, 
                            IOSCANPVT *ioIntScan,
                            INTD_enum_dataType dataType,
                            INTD_CALLBACK readCallback,
                            INTD_CALLBACK writeCallback,
                            epicsMutexId mutexId,
                            epicsEventId eventId,
                            INTD_enum_recordType recordType,
                            INTD_enum_scanType   scanType);

INTD_struc_node *INTD_API_findDataNode(const char *moduleName, const char *dataName, dbCommon *epicsRecord);

int INTD_API_getData(INTD_struc_node *dataNode, unsigned int pno, void *data);
int INTD_API_putData(INTD_struc_node *dataNode, unsigned int pno, void *data);

int INTD_API_genRecord(const char *moduleName, const char *path, const char *dbFileName);
int INTD_API_genSRReqt(const char *moduleName, const char *path, const char *reqFileName, int withMacro, int sel, const char *destModuleName);
int INTD_API_genArchive(const char *moduleName, const char *path, const char *confFileName, const char *methodStr, int withMacro, int sel, const char *destModuleName);
int INTD_API_genDbList(const char *moduleName, const char *path, const char *lstFileName);

int INTD_API_syncWithRecords(int enaCallback);

int INTD_API_forceOPVValue(INTD_struc_node *dataNode, void *dataPtr);
int INTD_API_forcePVProcess(INTD_struc_node *dataNode);
int INTD_API_raiseAlarm(INTD_struc_node *dataNode, epicsEnum16 nsta, epicsEnum16 nsevr);

int INTD_API_getIocInitStatus();

#ifdef __cplusplus
}
#endif

#endif

