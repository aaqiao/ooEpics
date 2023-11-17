//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// ModuleManager.cc
//
// Realization of the class for device manager
//===============================================================
#include "epicsTypes.h"
#include "epicsExport.h"
#include "iocsh.h"

#include "ModuleConfig.h"
#include "ControlDevice.h"
#include "DomainDevice.h"
#include "Application.h"

#include "ModuleManager.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// global objects and variables
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ModuleManager gobj_moduleManager;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// module type registeration (C type codes)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
int     gvar_moduleTypeListInitialized = 0;                // to show if the module type list initalized or not
ELLLIST gvar_moduleTypeList;                               // a list of the module types

extern "C" {
//-----------------------------------------------
// EPICS exit clean up
//-----------------------------------------------
static void epicsExitDelete(void *ptr) 
{
    ModuleInstance *mod = NULL;
    ModuleType *modType = NULL;

    // delete all modules and the module instances
    while((mod = (ModuleInstance *)ellLast(&gobj_moduleManager.moduleInstanceList))) {

        ellDelete(&gobj_moduleManager.moduleInstanceList, &mod -> node);
        
        if(mod -> ctlDevice)    delete mod -> ctlDevice;
        if(mod -> domDevice)    delete mod -> domDevice;
        if(mod -> app)          delete mod -> app;

        cout << "INFO: ModuleManager: module " << mod -> moduleName << " deleted!" << endl;
        
        delete mod;   
    }

    // delete the module type
    while((modType = (ModuleType *)ellLast(&gvar_moduleTypeList))) {

        ellDelete(&gvar_moduleTypeList, &modType -> node);
        cout << "INFO: ModuleManager: module type " << modType -> moduleTypeName << " deleted!" << endl;       
        delete modType;   
    }
}
}

//-----------------------------------------------
// register the module type in the list
// the module configuration API object will be a global object, when
// it is constructed, the module type will be automatically registered
//
// Note: we do this in C type code because the C type global variables
//       will be created prior to the global C++ object!
//       (if we also put this list as a member of the ModuleManager, we can
//       not guarantee that the ModuleManager object is created prior to the global
//       object of the ModuleConfig!)module
//-----------------------------------------------
void moduleTypeRegister(const char *moduleTypeName, ModuleConfig *modConfig)
{
    // check the input parameters
    if(!moduleTypeName || !moduleTypeName[0]) {
        cout << "ERROR: moduleTypeRegister: Invalid module type name!\n";
        return;
    }

    if(!modConfig) {
        cout << "ERROR: moduleTypeRegister: Invalid module config object for the type of " << moduleTypeName << "!\n";
        return;
    }

    // init the list if have not
    if(!gvar_moduleTypeListInitialized) {
        ellInit(&gvar_moduleTypeList);
        gvar_moduleTypeListInitialized = 1;

        // register the EPICS exit function
        epicsAtExit(epicsExitDelete, NULL);
    }

    // create the devicey type node
    ModuleType *modtype = new ModuleType;                       // create a new module type object

    if(modtype) {
        strcpy(modtype -> moduleTypeName, moduleTypeName);      // get the name
        modtype -> modConfig = modConfig;                       // get the configure API object    
        ellAdd(&gvar_moduleTypeList, &modtype -> node);
    } else {
        cout << "ERROR: moduleTypeRegister: Failed to create module type node for " << moduleTypeName << "!\n";
    }
}
 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// class methods realization
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------
// construction
//-----------------------------------------------
ModuleManager::ModuleManager()
{
    // init the link lists
    ellInit(&moduleInstanceList);
}

//-----------------------------------------------
// distruction
//-----------------------------------------------
ModuleManager::~ModuleManager()
{
/* It seemed ctrl+X will not delete this object, so put the clean up in the epicsExit function

    ModuleInstance *mod = NULL;

    // delete all modules and the module instances
    while((mod = (ModuleInstance *)ellLast(&moduleInstanceList))) {

        ellDelete(&moduleInstanceList, &mod -> node);
        
        if(mod -> ctlDevice)    delete mod -> ctlDevice;
        if(mod -> domDevice)    delete mod -> domDevice;
        if(mod -> app)          delete mod -> app;

        cout << "INFO:ModuleManager: module " << mod -> moduleName << " deleted!" << endl;
        
        delete mod;   
    }
*/

    cout << "INFO: ModuleManager: Module Manager object deleted!" << endl;
}

//-----------------------------------------------
// register the module instance in the list
//-----------------------------------------------
void ModuleManager::moduleInstanceRegister(const char *moduleTypeName, ModuleConfig *modConfig, const char *moduleName, void *module, int modCategory)
{
    ModuleInstance *mod = new ModuleInstance;

    if(mod) {
        strcpy(mod -> moduleTypeName, moduleTypeName);
        strcpy(mod -> moduleName, moduleName);

        mod -> modConfig        = modConfig;
        mod -> moduleCategory   = modCategory;

        if(modCategory == OOEPICS_DOM_DEVICE) {
            mod -> ctlDevice    = NULL;
            mod -> domDevice    = (DomainDevice *)module;
            mod -> app          = NULL;
        } else if(modCategory == OOEPICS_CTL_DEVICE) {
            mod -> ctlDevice    = (ControlDevice *)module;
            mod -> domDevice    = NULL;
            mod -> app          = NULL;        
        } else if(modCategory == OOEPICS_APPLICATION) {
            mod -> ctlDevice    = NULL;
            mod -> domDevice    = NULL;
            mod -> app          = (Application *)module;        
        }

        moduleInstanceListMutex.lock();
        ellAdd(&moduleInstanceList, &mod -> node);
        moduleInstanceListMutex.unlock();
    } else {
        cout << "ERROR: ModuleManager::moduleInstanceRegister: Failed to register the module " << moduleName << " with type of " << moduleTypeName <<"!" << endl;
    }
}

//-----------------------------------------------
// create a new module
//-----------------------------------------------
void ModuleManager::createModule(const char *moduleTypeName, const char *moduleName, const char *moduleInfo, int priority)
{
    // ---- find the module configuration API with the module type name ----
    int mtypeFound = 0;
    ModuleType *pmtype = 0;
    
    for(pmtype = (ModuleType *)ellFirst(&gvar_moduleTypeList);
        pmtype;
        pmtype = (ModuleType *)ellNext(&pmtype -> node)) {
        if(strcmp(pmtype -> moduleTypeName, moduleTypeName) == 0) {
            mtypeFound = 1;
            break;
        }
    }

    // ---- create new module instance with the API ----
    if(mtypeFound) pmtype -> modConfig -> moduleCreate(moduleName, moduleInfo, priority);
    else cout << "ERROR: ModuleManager::createModule: Module type of " << moduleTypeName << " not found!" << endl;
}

//-----------------------------------------------
// init the module
//-----------------------------------------------
void ModuleManager::initModule(const char *moduleName)
{
    // get the module instance
    ModuleInstance *mod = getModuleInstance(moduleName);

    // call the routine of the module config
    if(mod)
        mod -> modConfig -> moduleInit(mod);
    else 
        cout << "ERROR: ModuleManager::initModule: Cannot find the module of " << moduleName << "!" << endl; 
}

//-----------------------------------------------
// set one parameter of the module
//-----------------------------------------------
void ModuleManager::setModule(const char *moduleName, const char *cmd, char **data)
{
    // get the module instance
    ModuleInstance *mod     = getModuleInstance(moduleName);

    // call the routine of the module config
    if(mod)
        mod -> modConfig -> moduleSet(mod, cmd, data);
    else 
        cout << "ERROR: ModuleManager::setModule: Cannot find the module of " << moduleName << "!" << endl;  
}

//-----------------------------------------------
// get the pointer of the module instance
//-----------------------------------------------
ModuleInstance *ModuleManager::getModuleInstance(const char *moduleName)
{
    int modFound = 0;

    ModuleInstance *mod = 0;

    for(mod = (ModuleInstance *)ellFirst(&moduleInstanceList);
        mod;
        mod = (ModuleInstance *)ellNext(&mod -> node)) {
        if(strcmp(mod -> moduleName, moduleName) == 0) {
            modFound = 1;
            break;
        }
    }
    
    if(modFound) return mod;
    else return 0;
}

//-----------------------------------------------
// print all module type / module instances for debugging purpose
//-----------------------------------------------
void ModuleManager::printModuleType()
{
    ModuleType *pmtype = 0;
    
    cout << "----------------------------------------" << endl;
    cout << "Module Types:" << endl;    
    for(pmtype = (ModuleType *)ellFirst(&gvar_moduleTypeList);
        pmtype;
        pmtype = (ModuleType *)ellNext(&pmtype -> node)) {
        cout << "    " << pmtype -> moduleTypeName << endl;
    }
    cout << "----------------------------------------" << endl;
}

void ModuleManager::printModuleList()
{
    ModuleInstance *pmod = 0;
    
    cout << "----------------------------------------" << endl;
    cout << "Module Instances:" << endl;    
    for(pmod = (ModuleInstance *)ellFirst(&moduleInstanceList);
        pmod;
        pmod = (ModuleInstance *)ellNext(&pmod -> node)) {
        cout << "    " << pmod->moduleName << "\t<<withTypeOf>>    " << pmod->moduleTypeName << endl;
    }
    cout << "----------------------------------------" << endl;
}

} // namespace OOEPICS
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// global interfaces
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern "C" {

//-----------------------------------------------
// C interface to the class methods
//-----------------------------------------------
void createModule(const char *moduleTypeName, const char *moduleName, const char *moduleInfo, int priority)
{
    OOEPICS::gobj_moduleManager.createModule(moduleTypeName, moduleName, moduleInfo, priority);
}

void initModule(const char *moduleName)
{
    OOEPICS::gobj_moduleManager.initModule(moduleName);
}

void setModule(const char *moduleName, const char *cmd, char **data)
{
    OOEPICS::gobj_moduleManager.setModule(moduleName, cmd, data);
}

void printModuleType()       {OOEPICS::gobj_moduleManager.printModuleType();}
void printModuleList()       {OOEPICS::gobj_moduleManager.printModuleList();}

//-----------------------------------------------
// EPICS shell functions
//-----------------------------------------------
static const iocshArg        createModule_Arg0    = {"moduleTypeName", iocshArgString};
static const iocshArg        createModule_Arg1    = {"moduleName",     iocshArgString};
static const iocshArg        createModule_Arg2    = {"moduleInfo",     iocshArgString};
static const iocshArg        createModule_Arg3    = {"priority",       iocshArgInt};
static const iocshArg *const createModule_Args[4] = {&createModule_Arg0, &createModule_Arg1, &createModule_Arg2, &createModule_Arg3};
static const iocshFuncDef    createModule_FuncDef = {"createModule", 4, createModule_Args};
static void  createModule_CallFunc(const iocshArgBuf *args) {createModule(args[0].sval, args[1].sval, args[2].sval, args[3].ival);}

static const iocshArg        initModule_Arg0    = {"moduleName", iocshArgString};
static const iocshArg *const initModule_Args[1] = {&initModule_Arg0};
static const iocshFuncDef    initModule_FuncDef = {"initModule", 1, initModule_Args};
static void  initModule_CallFunc(const iocshArgBuf *args) {initModule(args[0].sval);}

static const iocshArg        setModule_Arg0    = {"moduleName", iocshArgString};
static const iocshArg        setModule_Arg1    = {"cmd",    iocshArgString};
static const iocshArg        setModule_Arg2    = {"data1",  iocshArgString};
static const iocshArg        setModule_Arg3    = {"data2",  iocshArgString};
static const iocshArg        setModule_Arg4    = {"data3",  iocshArgString};
static const iocshArg        setModule_Arg5    = {"data4",  iocshArgString};
static const iocshArg        setModule_Arg6    = {"data5",  iocshArgString};
static const iocshArg        setModule_Arg7    = {"data6",  iocshArgString};
static const iocshArg        setModule_Arg8    = {"data7",  iocshArgString};
static const iocshArg        setModule_Arg9    = {"data8",  iocshArgString};
static const iocshArg        setModule_Arg10   = {"data9",  iocshArgString};
static const iocshArg        setModule_Arg11   = {"data10", iocshArgString};
static const iocshArg        setModule_Arg12   = {"data11", iocshArgString};
static const iocshArg        setModule_Arg13   = {"data12", iocshArgString};
static const iocshArg        setModule_Arg14   = {"data13", iocshArgString};
static const iocshArg        setModule_Arg15   = {"data14", iocshArgString};
static const iocshArg        setModule_Arg16   = {"data15", iocshArgString};
static const iocshArg        setModule_Arg17   = {"data16", iocshArgString};
static const iocshArg *const setModule_Args[18] = {&setModule_Arg0,  &setModule_Arg1,  &setModule_Arg2,  &setModule_Arg3,  &setModule_Arg4,  &setModule_Arg5,
												   &setModule_Arg6,  &setModule_Arg7,  &setModule_Arg8,  &setModule_Arg9,  &setModule_Arg10, &setModule_Arg11,
												   &setModule_Arg12, &setModule_Arg13, &setModule_Arg14, &setModule_Arg15, &setModule_Arg16, &setModule_Arg17};
static const iocshFuncDef    setModule_FuncDef  = {"setModule", 18, setModule_Args};
static void  setModule_CallFunc(const iocshArgBuf *args) 
{
	int i, arg_num;
	char *data[OOEPICS_MAX_CMD_DATA_NUM];

	if(OOEPICS_MAX_CMD_DATA_NUM < 16) arg_num = OOEPICS_MAX_CMD_DATA_NUM;
	else 					  	      arg_num = 16;

	for(i = 0; i < arg_num; i ++)
		data[i] = args[i+2].sval;
	
	setModule(args[0].sval, args[1].sval, data);
}

static const iocshFuncDef   printModuleType_FuncDef = {"printModuleType", 0, NULL};
static void  printModuleType_CallFunc(const iocshArgBuf *args) {printModuleType();}

static const iocshFuncDef   printModuleList_FuncDef = {"printModuleList", 0, NULL};
static void  printModuleList_CallFunc(const iocshArgBuf *args) {printModuleList();}

void moduleManagerRegister(void)
{
    iocshRegister(&createModule_FuncDef,    createModule_CallFunc);
    iocshRegister(&initModule_FuncDef,      initModule_CallFunc);
    iocshRegister(&setModule_FuncDef,       setModule_CallFunc);
    iocshRegister(&printModuleType_FuncDef, printModuleType_CallFunc);
    iocshRegister(&printModuleList_FuncDef, printModuleList_CallFunc);
}

epicsExportRegistrar(moduleManagerRegister);

}










