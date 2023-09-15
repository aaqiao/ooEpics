/****************************************************
 * InternalData_devWf.c
 *
 * Device support for waveform record
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.20
 * Description: Initial creation. Modified from the devWfSoft.c from the EPICS base
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2011.06.10
 * Description: Add the functionality to write waveform
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
#include "link.h"
#include "waveformRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

/* Create the dset for devWfSoft */
static long init_record();
static long process_wf();
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt);

struct {
	long		number;
	DEVSUPFUN	report;
	DEVSUPFUN	init;
	DEVSUPFUN	init_record;
	DEVSUPFUN	get_ioint_info;
	DEVSUPFUN	read_wf;
}devWf_internalData={
	5,
	NULL,
	NULL,
	init_record,
	get_ioint_info,
	process_wf
};
epicsExportAddress(dset,devWf_internalData);

/* Init the record */
static long init_record(waveformRecord *pwf)
{
    char moduleName[128] = "";
    char dataName[128]   = "";

    /* Get the link strings, should be "moduleName.dataName" */
    sscanf(pwf -> inp.value.instio.string, "%127[^.].%127c", moduleName, dataName);

    /* Init value */
    pwf -> nord = 0;

    /* Attach to a internal data */
    pwf -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)pwf);

    if(pwf -> dpvt) {        
        /* Later check the type */

        return 0;
    } else {
        printf("waveform:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        return -1;
    }   
}

/* For I/O interrupt scanning */
static long get_ioint_info(int delFrom, dbCommon *prec, IOSCANPVT *ppvt)
{
    waveformRecord  *pwf      = (waveformRecord *)prec;
    INTD_struc_node *dataNode = (INTD_struc_node *)pwf -> dpvt;

    if(!dataNode || !dataNode -> ioIntScan) return -1;

    *ppvt = *dataNode -> ioIntScan;
    return 0;
}

/* Process the record */
static long process_wf(waveformRecord *pwf)
{
    long status;  
    unsigned int pno;      
    INTD_struc_node *dataNode = (INTD_struc_node *)pwf -> dpvt;                             /* get the data node */

    if(!dataNode) return -1;

    pno = (pwf->nelm < dataNode->pno)?pwf->nelm:dataNode->pno;                              /* find the minimum as point number to be processed */

    /* Get/put data. Please note that the type of the waveform must be same as the internal data node */
    if(strstr(pwf -> desc, "[W]") || strstr(pwf -> desc, "[w]")) {                                  /* [W] or [w] in desc field indicate to write the waveform */
        status = INTD_API_putData(dataNode, pno, pwf -> bptr);
    } else {
        /*printf("Read waveform for record of %s\n", pwf->name);      */  
        status = INTD_API_getData(dataNode, pno, pwf -> bptr);
    }

    if(pno > 0) pwf -> nord = pno;

    return status;
}

