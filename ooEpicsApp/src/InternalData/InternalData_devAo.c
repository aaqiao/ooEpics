/***************************************************************************
 *  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
 *  All rights reserved.
 *  Authors: Zheqiao Geng
 ***************************************************************************/
/***************************************************************************
 * InternalData_devAo.c
 *
 * Device support for Ao record
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
#include "link.h"
#include "special.h"
#include "aoRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

/* added for Channel Access Links */
static long init_record();

/* Create the dset for devAoSoft */
static long write_ao();

/* Device support entry point */
struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	write_ao;
	DEVSUPFUN	special_linconv;
}devAo_internalData={
	6,
	NULL,
	NULL,
	init_record,
	NULL,
	write_ao,
	NULL};
epicsExportAddress(dset,devAo_internalData);

/* Init the record */
static long init_record(aoRecord *pao)
{
    char moduleName[128] = "";
    char dataName[128]   = "";

    /* Get the link strings, should be "moduleName.dataName" */
    sscanf(pao -> out.value.instio.string, "%127[^.].%127c", moduleName, dataName);

    /* Attach to a internal data */
    pao -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)pao);

    if(pao -> dpvt) {
        return 0;
    } else {
        printf("ao:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        return -1;
    }    
}

/* Process the record */
static long write_ao(aoRecord *pao)
{    
    long status;

    if(!pao -> dpvt) return -1;

    status = INTD_API_putData((INTD_struc_node *)pao -> dpvt, 1, (void *)(&pao -> oval));
    pao -> val = pao -> oval;

    return status;
}


