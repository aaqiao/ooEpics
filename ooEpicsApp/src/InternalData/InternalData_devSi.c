/****************************************************
 * InternalData_devSi.c
 *
 * Device support for stringin record
 *
 * Created by Zheqiao Geng on 10.03.2015
 * Initial creation. Modified from the devSiSoft.c from the EPICS base
 *
 * Modified by Zheqiao Geng on 16.03.2016
 * Use INST_IO link to pass the string for internal data node
 ****************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dbScan.h"
#include "alarm.h"
#include "cvtTable.h"
#include "dbDefs.h"
#include "dbAccess.h"
#include "epicsTime.h"
#include "recGbl.h"
#include "recSup.h"
#include "devSup.h"
#include "link.h"
#include "stringinRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

/* Create the dset for devSiSoft */
static long init_record();
static long read_stringin();
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_stringin;
}devSi_internalData={
	5,
	NULL,
	NULL,
	init_record,
	get_ioint_info,
	read_stringin
};
epicsExportAddress(dset,devSi_internalData);

/* Init the record */
static long init_record(stringinRecord *prec)
{
    char moduleName[128] = "";
    char dataName[128]   = "";

    /* Get the link strings, should be "moduleName.dataName" */
    sscanf(prec -> inp.value.instio.string, "%127[^.].%127c", moduleName, dataName);

    /* Attach to a internal data */
    prec -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)prec);

    if(prec -> dpvt) {        
        return 0;
    } else {
        printf("stringin:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        return -1;
    }   
}

/* For I/O interrupt scanning */
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt)
{
    stringinRecord  *psi      = (stringinRecord *)prec;
    INTD_struc_node *dataNode = (INTD_struc_node *)psi -> dpvt;

    if(!dataNode || !dataNode -> ioIntScan) return -1;

    *ppvt = *dataNode -> ioIntScan;
    return 0;
}

/* Read stringin */
static long read_stringin(stringinRecord *prec)
{
    long status;    
    INTD_struc_node *dataNode = (INTD_struc_node *)prec -> dpvt;

    if(!dataNode) return -1;

    /* Get data */
    status = INTD_API_getData(dataNode, INTD_STR_RECORD_LEN, (void *)(prec -> val));
    prec -> udf = FALSE;

    /* alarm except for UDF */
    if(dataNode -> nsta != UDF_ALARM)
        recGblSetSevr(prec, dataNode -> nsta, dataNode -> nsevr);

    return status;
}



