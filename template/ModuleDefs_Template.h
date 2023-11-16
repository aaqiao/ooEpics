//=========================================================
// ModuleDefs___MODULE_NAME__.h
//
// Global definitions for the module
// Auto created from the template of ooEpics framework
//=========================================================
#ifndef MODULE_DEFS___MODULE_NAME___H
#define MODULE_DEFS___MODULE_NAME___H

// info or error for message
#define __MODULE_NAME___MSG_INFO                     0
#define __MODULE_NAME___MSG_WARN                     1
#define __MODULE_NAME___MSG_ERR                      2

// return code for service routines related with remote PV access
#define __MODULE_NAME___RPVACCESS_FAILED            -1       // failed
#define __MODULE_NAME___RPVACCESS_SUCCESS            0       // successful
#define __MODULE_NAME___RPVACCESS_NOT_CONNECTED      1       // some remote PVs are not connected (some may be connected, only the connected ones will be changed)
#define __MODULE_NAME___RPVACCESS_TIMEOUT            2       // timeout for PV access

// return code for jobs
#define __MODULE_NAME___JOB_FAILED                  -1
#define __MODULE_NAME___JOB_SUCCESS                  0
#define __MODULE_NAME___JOB_STOPPED_EXCEPTION        1
#define __MODULE_NAME___JOB_STOPPED_USER             2
#define __MODULE_NAME___JOB_NOT_ACTIVE               3
#define __MODULE_NAME___JOB_BUSY                     4

// enable and disable settings
#define __MODULE_NAME___SET_DISABLED                 0
#define __MODULE_NAME___SET_ENABLED                  1

#endif








