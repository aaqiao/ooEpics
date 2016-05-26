/****************************************************
 * InternalData_devSo.c
 *
 * Device support for stringout record
 *
 * Created by Zheqiao Geng on 10.03.2015
 * Initial creation. Modified from the devSoSoft.c from the EPICS base
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
#include "special.h"
#include "stringoutRecord.h"
#include "epicsExport.h"

#include "InternalData.h"

/* added for Channel Access Links */
static long init_record();

/* Create the dset for devSoSoft */
static long write_stringout();

/* Device support entry point */
struct {
    long        number;
    DEVSUPFUN   report;
    DEVSUPFUN   init;
    DEVSUPFUN   init_record;
    DEVSUPFUN   get_ioint_info;
    DEVSUPFUN   write_stringout;
}devSo_internalData={
	5,
	NULL,
	NULL,
	init_record,
	NULL,
	write_stringout
};
epicsExportAddress(dset,devSo_internalData);

/* Init the record */
static long init_record(stringoutRecord *prec)
{
    char fullStr[512]    = "";
    char moduleName[128] = "";
    char dataName[128]   = "";

    char *pat, *pdt, *ped;

    /* Get the OUP link strings, should be "@moduleName.dataName" */
    strncpy(fullStr, prec -> out.text, 512);

    pat = strchr(fullStr, '@');
    pdt = strchr(fullStr, '.');
    ped = strchr(fullStr, '\0');

    strncpy(moduleName, pat + 1, pdt - pat - 1);
    strncpy(dataName,   pdt + 1, ped - pdt - 1);

    /* Attach to a internal data */
    prec -> dpvt = (void *)INTD_API_findDataNode(moduleName, dataName, (dbCommon *)prec);

    if(prec -> dpvt) {
        return 0;
    } else {
        printf("stringout:init_record: Failed to find the internal data of %s for the module of %s\n", dataName, moduleName);
        return -1;
    }    
}

/* Process the record */
static long write_stringout(stringoutRecord *prec)
{    
    long status;

    if(!prec -> dpvt) return -1;

    status = INTD_API_putData((INTD_struc_node *)prec -> dpvt, INTD_STR_RECORD_LEN, (void *)(prec -> val));

    return status;
}


