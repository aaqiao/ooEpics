/****************************************************
 * InternalData_devMbbi.c
 *
 * Device support for Mbbi record
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.06.22
 * Description: Initial creation. Modified from the devMbbiSoftRaw.c from the EPICS base
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
#include "mbbiRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

/* Create the dset for devMbbiSoftRaw */
static long init_record();
static long read_mbbi();
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_mbbi;
}devMbbi_internalData={
	5,
	NULL,
	NULL,
	init_record,
	get_ioint_info,
	read_mbbi
};
epicsExportAddress(dset,devMbbi_internalData);

/* Init the record */
static long init_record(mbbiRecord *pmbbi)
{
    char moduleName[128] = "";
    char dataName[128]   = "";

    /* Get the link strings, should be "moduleName.dataName" */
    sscanf(pmbbi -> inp.value.instio.string, "%127[^.].%127c", moduleName, dataName);

    /* Attach to a internal data */
    pmbbi -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)pmbbi);

    /*to preserve old functionality*/
    if(pmbbi->nobt == 0) pmbbi->mask = 0xffffffff;
    pmbbi->mask <<= pmbbi->shft;

    if(pmbbi -> dpvt) {
        return 0;
    } else {
        printf("mbbi:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        return -1;
    }    
}

/* For I/O interrupt scanning */
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt)
{
    mbbiRecord      *pmbbi    = (mbbiRecord *)prec;
    INTD_struc_node *dataNode = (INTD_struc_node *)pmbbi -> dpvt;

    if(!dataNode || !dataNode -> ioIntScan) return -1;

    *ppvt = *dataNode -> ioIntScan;
    return 0;
}

/* Read the mbbi data */
static long read_mbbi(mbbiRecord *pmbbi)
{
    long status;
    INTD_struc_node *dataNode = (INTD_struc_node *)pmbbi -> dpvt;

    if(!dataNode) return -1;

    status = INTD_API_getData(dataNode, 1, (void *)(&pmbbi -> val));
    pmbbi -> udf = FALSE;

    /* alarm except for UDF */
    if(dataNode -> nsta != UDF_ALARM)
        recGblSetSevr(pmbbi, dataNode -> nsta, dataNode -> nsevr);

    if(status == 0) return 2;
    else return status;
}


