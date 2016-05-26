/****************************************************
 * InternalData_devAi.c
 *
 * Device support for Ai record
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.06.08
 * Description: Initial creation. Modified from the devAiSoft.c from the EPICS base
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
#include "recGbl.h"
#include "recSup.h"
#include "devSup.h"
#include "link.h"
#include "aiRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

/* Create the dset for devAiSoft */
static long init_record();
static long read_ai();
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_ai;
	DEVSUPFUN	special_linconv;
}devAi_internalData={
	6,
	NULL,
	NULL,
	init_record,
	get_ioint_info,
	read_ai,
	NULL
};
epicsExportAddress(dset,devAi_internalData);

/* Init the record */
static long init_record(aiRecord *pai)
{
    char fullStr[512]    = "";
    char moduleName[128] = "";
    char dataName[128]   = "";

    char *pat, *pdt, *ped;

    /* Get the INP link strings, should be "@moduleName.dataName" */
    strncpy(fullStr, pai -> inp.text, 512);

    pat = strchr(fullStr, '@');
    pdt = strchr(fullStr, '.');
    ped = strchr(fullStr, '\0');

    strncpy(moduleName, pat + 1, pdt - pat - 1);
    strncpy(dataName,   pdt + 1, ped - pdt - 1);

    /* Make sure record processing routine does not perform any conversion*/
    pai -> linr = 0;

    /* Attach to a internal data */
    pai -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)pai);

    if(pai -> dpvt) {        
        /* Later check the type */

        return 0;
    } else {
        printf("ai:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        return -1;
    }   
}

/* For I/O interrupt scanning */
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt)
{
    aiRecord        *pai      = (aiRecord *)prec;
    INTD_struc_node *dataNode = (INTD_struc_node *)pai -> dpvt;

    if(!dataNode || !dataNode -> ioIntScan) return -1;

    *ppvt = *dataNode -> ioIntScan;
    return 0;
}

/* Read ai */
static long read_ai(aiRecord *pai)
{
    long status;    
    INTD_struc_node *dataNode = (INTD_struc_node *)pai -> dpvt;

    if(!dataNode) return -1;

    /* Get data. Please note that the type of the waveform must be same as the internal data node */
    status = INTD_API_getData(dataNode, 1, (void *)(&pai -> val));
    pai -> udf = FALSE;

    /* alarm except for UDF */
    if(dataNode -> nsta != UDF_ALARM)
        recGblSetSevr(pai, dataNode -> nsta, dataNode -> nsevr);

    if(status == 0) return 2;
    else return status;
}



