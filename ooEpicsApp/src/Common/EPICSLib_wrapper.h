/****************************************************
 * EPICSLib_wrapper.h
 * 
 * Here is the EPICS wrapper for all EPICS functions
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.07.07
 * Description: Initial creation
 ****************************************************/
#ifndef EPICSLIB_WRAPPER_H
#define EPICSLIB_WRAPPER_H

#include <stdlib.h>  
#include <stdio.h>             
#include <string.h>
#include <errlog.h>

#include <dbScan.h>
#include <epicsPrint.h>
#include <epicsMessageQueue.h>
#include <ellLib.h>               
#include <epicsThread.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <epicsTypes.h>
#include <epicsTime.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Constant definition
 */
#define EPICSLIB_CONST_NAME_LEN 128                                      /* string length for names */
#define EPICSLIB_CONST_PATH_LEN 256                                      /* string length for file path */
#define EPICSLIB_CONST_LINE_LEN 256                                      /* string length for a line */

/**
 * Wrappers for EPICS staff
 * I designed like this because I would like the body of this module are platform independent
 *   EPICS is also a platform to me! 
 * Macro definitions are used to reduce the latency caused by function call
 */
/* for linked list */
typedef ELLLIST EPICSLIB_type_linkedList;
typedef ELLNODE EPICSLIB_type_linkedListNode;

#define EPICSLIB_func_LinkedListInit(list)                ellInit(&(list))
#define EPICSLIB_func_LinkedListInsert(list, node)        ellAdd(&(list), &(node))
#define EPICSLIB_func_LinkedListDelete(list, node)        ellDelete(&(list), &(node))
#define EPICSLIB_func_LinkedListFindFirst(list)           ellFirst(&(list))
#define EPICSLIB_func_LinkedListFindNext(node)            ellNext(&(node))

/* for event handling */
typedef epicsEventId EPICSLIB_type_eventId;

#define EPICSLIB_func_eventMustCreate                     epicsEventMustCreate
#define EPICSLIB_func_eventDestroy                        epicsEventDestroy
#define EPICSLIB_func_eventSignal                         epicsEventSignal
#define EPICSLIB_func_eventWait                           epicsEventWait
#define EPICSLIB_func_eventMustWait                       epicsEventMustWait
#define EPICSLIB_func_eventWaitWithTimeout                epicsEventWaitWithTimeout

/* for message queue handling */
typedef epicsMessageQueue EPICSLIB_type_msgQ;

/* for thread handling */
typedef epicsThreadId EPICSLIB_type_threadId;

#define EPICSLIB_func_threadCreate(name, priority, func, arg) epicsThreadCreate(name, priority, \
                                                            epicsThreadGetStackSize(epicsThreadStackMedium), \
                                                            (EPICSTHREADFUNC)(func), arg)
#define EPICSLIB_func_threadSetPriority                       epicsThreadSetPriority
#define EPICSLIB_func_epicsThreadSleep                        epicsThreadSleep

/* for mutex handling */
typedef epicsMutexId EPICSLIB_type_mutexId;

#define EPICSLIB_func_mutexMustCreate                     epicsMutexMustCreate
#define EPICSLIB_func_mutexDestroy                        epicsMutexDestroy
#define EPICSLIB_func_mutexMustLock                       epicsMutexMustLock
#define EPICSLIB_func_mutexUnlock                         epicsMutexUnlock

/* for IO interrup scanning */
typedef IOSCANPVT EPICSLIB_type_ioScanPvt;

#define EPICSLIB_func_scanIoInit                          scanIoInit
#define EPICSLIB_func_scanIoRequest                       scanIoRequest

/* for others */
#define EPICSLIB_func_errlogPrintf                        errlogPrintf

#ifdef __cplusplus
}
#endif

#endif

