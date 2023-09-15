/****************************************************
 * InternalData_iocShell.c
 * 
 * Define ioc shell commands
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.06.30
 * Description: Initial creation
 ****************************************************/
#include <epicsTypes.h>
#include <epicsExport.h>
#include <iocsh.h>

#include "InternalData.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================
 * IOC shell Routines
 *======================================*/
/* INTD_API_genRecord */
static const iocshArg        INTD_generateRecords_Arg0    = {"moduleName",  iocshArgString};
static const iocshArg        INTD_generateRecords_Arg1    = {"path",        iocshArgString};
static const iocshArg        INTD_generateRecords_Arg2    = {"dbFileName",  iocshArgString};
static const iocshArg *const INTD_generateRecords_Args[3] = {&INTD_generateRecords_Arg0, &INTD_generateRecords_Arg1, &INTD_generateRecords_Arg2};
static const iocshFuncDef    INTD_generateRecords_FuncDef = {"INTD_generateRecords", 3, INTD_generateRecords_Args};
static void  INTD_generateRecords_CallFunc(const iocshArgBuf *args) {INTD_API_genRecord(args[0].sval, args[1].sval, args[2].sval);}

/* INTD_API_genSRReqt */
static const iocshArg        INTD_generateReqFile_Arg0    = {"moduleName",  iocshArgString};
static const iocshArg        INTD_generateReqFile_Arg1    = {"path",        iocshArgString};
static const iocshArg        INTD_generateReqFile_Arg2    = {"reqFileName", iocshArgString};
static const iocshArg        INTD_generateReqFile_Arg3    = {"withMacro",   iocshArgInt};
static const iocshArg        INTD_generateReqFile_Arg4    = {"sel",         iocshArgInt};
static const iocshArg        INTD_generateReqFile_Arg5    = {"destModName", iocshArgString};
static const iocshArg *const INTD_generateReqFile_Args[6] = {&INTD_generateReqFile_Arg0, &INTD_generateReqFile_Arg1, &INTD_generateReqFile_Arg2, &INTD_generateReqFile_Arg3, &INTD_generateReqFile_Arg4, &INTD_generateReqFile_Arg5};
static const iocshFuncDef    INTD_generateReqFile_FuncDef = {"INTD_generateReqFile", 6, INTD_generateReqFile_Args};
static void  INTD_generateReqFile_CallFunc(const iocshArgBuf *args) {INTD_API_genSRReqt(args[0].sval, args[1].sval, args[2].sval, args[3].ival, args[4].ival, args[5].sval);}

/* INTD_API_genArchive */
static const iocshArg        INTD_generateArchCfgFile_Arg0    = {"moduleName",   iocshArgString};
static const iocshArg        INTD_generateArchCfgFile_Arg1    = {"path",         iocshArgString};
static const iocshArg        INTD_generateArchCfgFile_Arg2    = {"confFileName", iocshArgString};
static const iocshArg        INTD_generateArchCfgFile_Arg3    = {"methodStr",    iocshArgString};
static const iocshArg        INTD_generateArchCfgFile_Arg4    = {"withMacro",    iocshArgInt};
static const iocshArg        INTD_generateArchCfgFile_Arg5    = {"sel",          iocshArgInt};
static const iocshArg        INTD_generateArchCfgFile_Arg6    = {"destModName",  iocshArgString};
static const iocshArg *const INTD_generateArchCfgFile_Args[7] = {&INTD_generateArchCfgFile_Arg0, &INTD_generateArchCfgFile_Arg1, &INTD_generateArchCfgFile_Arg2, &INTD_generateArchCfgFile_Arg3, &INTD_generateArchCfgFile_Arg4, &INTD_generateArchCfgFile_Arg5, &INTD_generateArchCfgFile_Arg6};
static const iocshFuncDef    INTD_generateArchCfgFile_FuncDef = {"INTD_generateArchCfgFile", 7, INTD_generateArchCfgFile_Args};
static void  INTD_generateArchCfgFile_CallFunc(const iocshArgBuf *args) {INTD_API_genArchive(args[0].sval, args[1].sval, args[2].sval, args[3].sval, args[4].ival, args[5].ival, args[6].sval);}

/* INTD_API_genDbList */
static const iocshArg        INTD_generateRecList_Arg0    = {"moduleName",  iocshArgString};
static const iocshArg        INTD_generateRecList_Arg1    = {"path",        iocshArgString};
static const iocshArg        INTD_generateRecList_Arg2    = {"lstFileName", iocshArgString};
static const iocshArg *const INTD_generateRecList_Args[3] = {&INTD_generateRecList_Arg0, &INTD_generateRecList_Arg1, &INTD_generateRecList_Arg2};
static const iocshFuncDef    INTD_generateRecList_FuncDef = {"INTD_generateRecList", 3, INTD_generateRecList_Args};
static void  INTD_generateRecList_CallFunc(const iocshArgBuf *args) {INTD_API_genDbList(args[0].sval, args[1].sval, args[2].sval);}

/* INTD_API_syncWithRecords */
static const iocshArg        INTD_syncWithRecords_Arg0    = {"enaCallback", iocshArgInt};
static const iocshArg *const INTD_syncWithRecords_Args[1] = {&INTD_syncWithRecords_Arg0};
static const iocshFuncDef    INTD_syncWithRecords_FuncDef = {"INTD_syncWithRecords", 1, INTD_syncWithRecords_Args};
static void  INTD_syncWithRecords_CallFunc(const iocshArgBuf *args) {INTD_API_syncWithRecords(args[0].ival);}

void INTD_IOCShellRegister(void)
{
    iocshRegister(&INTD_generateRecords_FuncDef,        INTD_generateRecords_CallFunc);
    iocshRegister(&INTD_generateReqFile_FuncDef,        INTD_generateReqFile_CallFunc);
    iocshRegister(&INTD_generateArchCfgFile_FuncDef,    INTD_generateArchCfgFile_CallFunc);
    iocshRegister(&INTD_generateRecList_FuncDef,        INTD_generateRecList_CallFunc);
    iocshRegister(&INTD_syncWithRecords_FuncDef,        INTD_syncWithRecords_CallFunc);
}

epicsExportRegistrar(INTD_IOCShellRegister);

#ifdef __cplusplus
}
#endif






