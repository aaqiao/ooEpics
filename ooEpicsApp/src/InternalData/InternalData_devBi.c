/***************************************************************************
 *  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
 *  All rights reserved.
 *  Authors: Zheqiao Geng
 ***************************************************************************/
/***************************************************************************
 * InternalData_devBi.c
 *
 * Device support for Bi record
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
#include "biRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

/* Create the dset for devBiSoft */
static long init_record();
static long read_bi();
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_bi;
}devBi_internalData={
	5,
	NULL,
	NULL,
	init_record,
	get_ioint_info,
	read_bi
};
epicsExportAddress(dset,devBi_internalData);

/* Init the record (later can be defined as a common function) */
static long init_record(biRecord *pbi)
{
    char moduleName[128] = "";
    char dataName[128]   = "";

    /* Get the link strings, should be "moduleName.dataName" */
    sscanf(pbi -> inp.value.instio.string, "%127[^.].%127c", moduleName, dataName);

    /* Attach to a internal data */
    pbi -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)pbi);

    if(pbi -> dpvt) {
        return 0;
    } else {
        printf("bi:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        return -1;
    }    
}

/* For I/O interrupt scanning */
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt)
{
    biRecord        *pbi      = (biRecord *)prec;
    INTD_struc_node *dataNode = (INTD_struc_node *)pbi -> dpvt;

    if(!dataNode || !dataNode -> ioIntScan) return -1;

    *ppvt = *dataNode -> ioIntScan;
    return 0;
}

/* Process the record */
static long read_bi(biRecord *pbi)
{
    long status;
    INTD_struc_node *dataNode = (INTD_struc_node *)pbi -> dpvt;

    if(!dataNode) return -1;

    status = INTD_API_getData(dataNode, 1, (void *)(&pbi -> val));
    pbi -> udf = FALSE;

    /* alarm except for UDF */
    if(dataNode -> nsta != UDF_ALARM)
        recGblSetSevr(pbi, dataNode -> nsta, dataNode -> nsevr);

    if(status == 0) return 2;
    else return status;
}







