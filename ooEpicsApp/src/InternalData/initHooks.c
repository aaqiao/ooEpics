/***************************************************************************
 *  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
 *  All rights reserved.
 *  Authors: Zheqiao Geng
 ***************************************************************************/
/***************************************************************************
 * initHooks.c
 * 
 * ioc initialization hooks
 ***************************************************************************/
#include	<stdio.h>
#include	<initHooks.h>
#include	<epicsPrint.h>
#include    <iocsh.h>
#include    <epicsExport.h>

/**
 * Global variables 
 */
extern int INTD_gvar_iocInitDone;

/**
 * If this function (initHooks) is loaded, iocInit calls this function
 * at certain defined points during IOC initialization 
 */
static void INTD_initHooks(initHookState state)
{
	/* End of iocRun/iocInit commands (restore by autosave should be finished already) */
	if(state == initHookAfterIocRunning)
		INTD_gvar_iocInitDone = 1;
	else
		INTD_gvar_iocInitDone = 0;
}

void INTD_initHooksRegister(void)
{
   initHookRegister(INTD_initHooks);
}

epicsExportRegistrar(INTD_initHooksRegister);
