/****************************************************
 * InternalData_devBo.c
 *
 * Device support for Bo record
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.18
 * Description: Initial creation. Modified from the devBoSoft.c from the EPICS base
 *
 * Modified by Zheqiao Geng on 16.03.2016
 * Use INST_IO link to pass the string for internal data node
 ****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "alarm.h"
#include "dbDefs.h"
#include "dbAccess.h"
#include "recGbl.h"
#include "recSup.h"
#include "devSup.h"
#include "boRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

static long init_record();

/* Create the dset for devBoSoft */
static long write_bo();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_bo;
}devBo_internalData={
	5,
	NULL,
	NULL,
	init_record,
	NULL,
	write_bo
};
epicsExportAddress(dset,devBo_internalData);

/* Init the record (later can be defined as a common function) */
static long init_record(boRecord *pbo)
{
    char fullStr[512]    = "";
    char moduleName[128] = "";
    char dataName[128]   = "";

    char *pat, *pdt, *ped;

    /* Get the OUP link strings, should be "@moduleName.dataName" */
    strncpy(fullStr, pbo -> out.text, 512);

    pat = strchr(fullStr, '@');
    pdt = strchr(fullStr, '.');
    ped = strchr(fullStr, '\0');

    strncpy(moduleName, pat + 1, pdt - pat - 1);
    strncpy(dataName,   pdt + 1, ped - pdt - 1);

    /* Attach to a internal data */
    pbo -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)pbo);

    if(pbo -> dpvt) {
        return 0;
    } else {
        printf("bo:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        return -1;
    }    
}

/* Process the record */
static long write_bo(boRecord *pbo)
{
    long status;

    if(!pbo -> dpvt) return -1;

    status = INTD_API_putData((INTD_struc_node *)pbo -> dpvt, 1, (void *)(&pbo -> val));

    return status;
}
