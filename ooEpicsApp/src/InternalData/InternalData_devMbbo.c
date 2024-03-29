/***************************************************************************
 *  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
 *  All rights reserved.
 *  Authors: Zheqiao Geng
 ***************************************************************************/
/***************************************************************************
 * InternalData_devMbbo.c
 *
 * Device support for Mbbo record
 ***************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "alarm.h"
#include "dbDefs.h"
#include "dbAccess.h"
#include "recGbl.h"
#include "recSup.h"
#include "devSup.h"
#include "mbboRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

/* Create the dset for devMbboSoftRaw */
static long init_record();
static long write_mbbo();

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_mbbo;
}devMbbo_internalData={
	5,
	NULL,
	NULL,
	init_record,
    NULL,
	write_mbbo
};
epicsExportAddress(dset,devMbbo_internalData);

/* Init the record */
static long init_record(mbboRecord *pmbbo)
{ 
    long status;

    char moduleName[128] = "";
    char dataName[128]   = "";

    /* Get the link strings, should be "moduleName.dataName" */
    sscanf(pmbbo -> out.value.instio.string, "%127[^.].%127c", moduleName, dataName);

    /* Attach to a internal data */
    pmbbo -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)pmbbo);

    if(pmbbo -> dpvt) {
        status = 2;   /* do not convert */
    } else {
        printf("mbbo:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        status = -1;
    }    

    /*to preserve old functionality*/
    if(pmbbo->nobt == 0) pmbbo->mask = 0xffffffff;
    pmbbo->mask <<= pmbbo->shft;

    return status; 
} 

/* write the mbbo data */
static long write_mbbo(mbboRecord *pmbbo)
{
    long status;
    unsigned short data;

    if(!pmbbo -> dpvt) return -1;

    data = pmbbo->val;
    status = INTD_API_putData((INTD_struc_node *)pmbbo -> dpvt, 1, (void *)(&data));

    return status;
}








