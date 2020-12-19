/****************************************************
 * InternalData.c
 * 
 * Header file for the InternalData module
 * This module acts as the isolation layer between EPICS records and inernal code
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.05.17
 * Description: Initial creation
 *
 * Modified by: Zheqiao Geng
 * Modified on: 2/7/2013
 * Description: allow to specify the path for saving the files
 *
 * Modified by: Zheqiao Geng
 * Modified on: 4/29/2013
 * Description: Change the code to be adaptable to 64bit Linux (use epicsTypes.h)
 * 
 * Modified by Zheqiao Geng (zheqiao.geng@psi.ch) on 2014.04.15
 * Description: In callback functions, pass directly the private pointer, not the whole data node
 *
 * Modified by Zheqiao Geng on 2014.05.20
 * Upgrade the API for database generateion and add the API for request file and list file gen.
 * Input supplementary string into the data node for bi,bo,mbbi,mbbi strings
 *
 * Modified by Zheqiao Geng on 2014.06.02
 * In function of INTD_API_syncWithRecords, use scanOnce to replace the dbProcess for safety
 *
 * Modified by Zheqiao Geng on 2014.09.27
 * Add optional sub-module name to make up the PV names
 *
 * Modified by Zheqiao Geng on 2015.04.22
 * Added the function to clean up the memory when EPICS exits
 *
 * Modified by Zheqiao Geng on 2015.04.27
 * Added the parameter setting limit and alarm limit into the save/restore and archiver file
 *
 * Modified by Andreas Hauff on 16.11.2015
 * Added epicsOldString as data type
 *
 * Modified by Zheqiao Geng on 25.11.2015
 * Extended the length of the buffers for record string in the function to generate database
 *
 * Modified by Zheqiao Geng on 09.03.2016
 * Changed the archiver configuration file "MONITOR" to "Monitor". Later need to pass the string
 * from external world instead of hard coding
 *
 * Modified by Zheqiao Geng on 12.10.2018
 * Updated the output PV archiver method from "Monitor Monitor Monitor" to "None None Monitor"
 ****************************************************/
#include <dbAccess.h>					/* there is conflicts in db_access.h (used by cadef.h) and dbFldTypes.h for the DBR_xxx definitions. So only use dbAccess.h locally */
#include "InternalData.h"
#include "recordGenerate.h"

/*======================================
 * Global variables
 *======================================*/
static ELLLIST INTD_gvar_dataList;                                  /* later we need a mutex to protect this list! */
static int INTD_gvar_dataListInitialized = 0;

static char *INTD_gvar_scanStrs[] = {                               /* string for different scanning */
    "Passive",
    "I/O Intr",
    "10 second",
    "5 second",
    "2 second",
    "1 second",
    ".5 second",
    ".2 second",
    ".1 second"
};

int INTD_gvar_iocInitDone = 0;										/* global variable to show iocInit is finished or not */

/*======================================
 * Private routines
 *======================================*/
static int INTD_func_getDataSize(INTD_enum_dataType dataType)
{
    int var_dataSize = 0;

    switch(dataType) {
        case INTD_CHAR:
        case INTD_UCHAR:
            var_dataSize = sizeof(epicsInt8);
            break;

        case INTD_SHORT:
        case INTD_USHORT:
            var_dataSize = sizeof(epicsInt16);
            break;

        case INTD_INT:
        case INTD_UINT:
            var_dataSize = sizeof(epicsInt32);
            break;

        case INTD_LONG:
        case INTD_ULONG:
            var_dataSize = sizeof(epicsInt32);
            break;

        case INTD_FLOAT:
            var_dataSize = sizeof(epicsFloat32);
            break;

        case INTD_DOUBLE:
            var_dataSize = sizeof(epicsFloat64);
            break;
            
        case INTD_OLDSTRING:
            var_dataSize = sizeof(epicsOldString);
            break;

        default:
            break;
    }

    return var_dataSize;
}

/**
 * Get system time
 */
static char *getSystemTime()
{
    time_t timer;
    timer = time(0);
    return asctime(localtime(&timer));
}

/**
 * Clean the data memory when EPICS exits
 */
static void INTD_func_cleanDataMem()
{
    INTD_struc_node *ptr_dataNode = NULL;

    /* clean the data node in reverse order that they were added */
    while((ptr_dataNode = (INTD_struc_node *)ellLast(&INTD_gvar_dataList))) {
        ellDelete(&INTD_gvar_dataList, &ptr_dataNode -> node);
        free(ptr_dataNode);
    }

    /* Print the message */
    printf("INFO: INTD: All data nodes of Internal Data deleted!\n");    
}

/*======================================
 * Public Routines
 *======================================*/
/**
 * Create a data node. This will be used in the user code to create a internal data node and connect with a data in the user code
 * Input:
 *   Please see the definition of the data structure of INTD_struc_node
 * Return:
 *   Pointer of the data node
 */
INTD_struc_node *INTD_API_createDataNode(const char *moduleName, 
                            const char *dataName, 
						    const char *supStr,
                            const char *unitStr,
                            void *dataPtr, 
                            void *privateData,
                            unsigned int pno, 
                            IOSCANPVT *ioIntScan,
                            INTD_enum_dataType dataType,
                            INTD_CALLBACK readCallback,
                            INTD_CALLBACK writeCallback,
                            epicsMutexId mutexId,
                            epicsEventId eventId,
                            INTD_enum_recordType recordType,
                            INTD_enum_scanType   scanType)
{
    char dataName_convert[128]      = "";
    char dataName_subModPart[128]   = "";
    char dataName_recPart[128]      = "";

    char *posVertLine, *posEnd;

    INTD_struc_node *ptr_dataNode = NULL;

    /* Check the input */
    if(!moduleName || !moduleName[0]) {
        printf("INTD_API_createDataNode: Illegal module name\n");
        return NULL;
    }

    if(!dataName || !dataName[0]) {
        printf("INTD_API_createDataNode: Illegal data name\n");
        return NULL;
    }

    if(!dataPtr) {
        printf("INTD_API_createDataNode: Illegal data assigned\n");
        return NULL;
    }
    
    if(pno <= 0) {
        printf("INTD_API_createDataNode: Number of points must be larger than 0\n");
        return NULL;
    }

    /* Init the list if necessary */
    if(!INTD_gvar_dataListInitialized) {
        ellInit(&INTD_gvar_dataList);
        INTD_gvar_dataListInitialized = 1;

        /* register the exit function */
        epicsAtExit(INTD_func_cleanDataMem, NULL);
    }

    /* Create a new data node */
    ptr_dataNode = (INTD_struc_node *)calloc(1, sizeof(INTD_struc_node));

    if(!ptr_dataNode) {
        printf("INTD_API_createDataNode: Failed to create the data node of %s for the module of %s\n", dataName, moduleName);
        return NULL;
    }

    /* get the sub-module name and the data name. The input dataName may contains of a '|' character to pass in the sub-module name which
       is only used to generate the record names. The '|' will be converted to '-' to get the final data name */
    strncpy(dataName_convert, dataName, 128);
    
    posVertLine = strchr(dataName_convert, '|'); 
    posEnd      = strchr(dataName_convert, '\0');   

    if(posVertLine) {
        dataName_subModPart[0] = '-';
        strncpy(dataName_subModPart + 1, dataName_convert, posVertLine - dataName_convert);    
        strncpy(dataName_recPart,        posVertLine + 1,  posEnd - posVertLine - 1);
        *posVertLine = '-';
    } else {
        strncpy(dataName_recPart, dataName_convert, 128);
    }


    /* Initalize the data node */
    strcpy(ptr_dataNode -> moduleName,      moduleName);
    strcpy(ptr_dataNode -> dataName,        dataName_convert);
    strcpy(ptr_dataNode -> subModuleName,   dataName_subModPart);
    strcpy(ptr_dataNode -> recName,         dataName_recPart);    
    strcpy(ptr_dataNode -> supStr,          supStr);
    strcpy(ptr_dataNode -> unitStr,         unitStr);
    strcpy(ptr_dataNode -> desc,            "");
    strcpy(ptr_dataNode -> alias,           "");
    strcpy(ptr_dataNode -> asg,             "");
    
    ptr_dataNode -> dataPtr         = dataPtr;
    ptr_dataNode -> privateData     = privateData;
    ptr_dataNode -> pno             = pno;
    ptr_dataNode -> prec            = 0;
    ptr_dataNode -> ioIntScan       = ioIntScan;
    ptr_dataNode -> dataType        = dataType;
    ptr_dataNode -> readCallback    = readCallback;
    ptr_dataNode -> writeCallback   = writeCallback;
    ptr_dataNode -> epicsRecord     = NULL;
    ptr_dataNode -> mutexId         = mutexId;
    ptr_dataNode -> eventId         = eventId;
    ptr_dataNode -> recordType      = recordType;
    ptr_dataNode -> scanType        = scanType;
    ptr_dataNode -> enableCallback  = 1;
    ptr_dataNode -> nsta            = UDF_ALARM;
    ptr_dataNode -> nsevr           = INVALID_ALARM;

    /* Add the data node to the list */
    ellAdd(&INTD_gvar_dataList, &ptr_dataNode -> node);

    return ptr_dataNode;
}

/**
 * set missing parameters of the PV: description and display precision,
 * they are optional compared to other fields
 */
int INTD_API_setDescription(INTD_struc_node *dataNode, const char *descStr)
{
    if(dataNode) 
        strcpy(dataNode -> desc, descStr);

    return 0;
}

int INTD_API_setPrecision(INTD_struc_node *dataNode, unsigned int precVal)
{
    if(dataNode)
        dataNode -> prec = precVal;

    return 0;
}

int INTD_API_setAlias(INTD_struc_node *dataNode, const char *aliasStr)
{
    if(dataNode) 
        strcpy(dataNode -> alias, aliasStr);

    return 0;    
}

int INTD_API_setAsg(INTD_struc_node *dataNode, const char *asgStr)
{
    if(dataNode) 
        strcpy(dataNode -> asg, asgStr);

    return 0;    
}

/**
 * Find a data node based on the module name and the data name. This will be called by the record_init function and the
 *   pointer of the record will be also remembered here
 * Input
 *   moduleName     : Module name
 *   dataName       : Data name
 *   epicsRecord    : The pointer to the EPICS recored that is attached to this data
 * Return:
 *   NULL           : Not found
 *   >0             : Address of the data node
 */
INTD_struc_node *INTD_API_findDataNode(const char *moduleName, const char *dataName, dbCommon *epicsRecord)
{
    int              var_nodeFound = 0;
    INTD_struc_node *ptr_dataNode  = NULL;

    /* check the input */
    if(!moduleName || !moduleName[0]) {
        printf("INTD_API_findDataNode: Illegal module name\n");
        return NULL;
    }

    if(!dataName || !dataName[0]) {
        printf("INTD_API_findDataNode: Illegal data name\n");
        return NULL;
    }

    /* find the data node */
    for(ptr_dataNode = (INTD_struc_node *)ellFirst(&INTD_gvar_dataList);
        ptr_dataNode;
        ptr_dataNode = (INTD_struc_node *)ellNext(&ptr_dataNode -> node)) {
        
        if(strcmp(ptr_dataNode -> moduleName, moduleName) == 0 && strcmp(ptr_dataNode -> dataName, dataName) == 0) {
            var_nodeFound = 1;
            break;
        }

    }

    if(var_nodeFound) {
        ptr_dataNode -> epicsRecord = epicsRecord;

        return ptr_dataNode;
    } else {
        return NULL;
    }
}

/**
 * Get data from the data node. This will be called by the record processing routine for reading data
 * Input:
 *   dataNode       : Data node to get data from
 *   pno            : Number of points to get from the internal data node
 * Output:
 *   data           : The data read out from the node, the buffer should be already defined in the caller
 * Return:
 *   0              : Successful
 *  -1              : Failed
 */
int INTD_API_getData(INTD_struc_node *dataNode, unsigned int pno, void *data)
{
    int var_dataSize;

    /* Check the input */
    if(!dataNode || !data || !dataNode -> dataPtr || pno <= 0) 
        return -1;

    /* Execute the call back if defined */
    if(dataNode -> readCallback && dataNode -> enableCallback) 
        (*dataNode -> readCallback)(dataNode -> privateData);
    
    if(!dataNode -> enableCallback)
        dataNode -> enableCallback = 1;

    /* Get the data size */
    var_dataSize = INTD_func_getDataSize(dataNode -> dataType);

    /* Get the data based on the size */
    if(dataNode -> mutexId) epicsMutexMustLock(dataNode -> mutexId);

    if(pno > 1) {                                                   /* array */       
        memcpy(data, dataNode -> dataPtr, pno * var_dataSize);
    } else if(pno == 1) {                                           /* single value (these code to remove the overhead from memcpy) */
        switch(dataNode -> dataType) {
            case INTD_CHAR:     *((epicsInt8    *)data) = *((epicsInt8    *)dataNode -> dataPtr); break;
            case INTD_UCHAR:    *((epicsUInt8   *)data) = *((epicsUInt8   *)dataNode -> dataPtr); break;
            case INTD_SHORT:    *((epicsInt16   *)data) = *((epicsInt16   *)dataNode -> dataPtr); break;
            case INTD_USHORT:   *((epicsUInt16  *)data) = *((epicsUInt16  *)dataNode -> dataPtr); break;
            case INTD_INT:      *((epicsInt32   *)data) = *((epicsInt32   *)dataNode -> dataPtr); break;
            case INTD_UINT:     *((epicsUInt32  *)data) = *((epicsUInt32  *)dataNode -> dataPtr); break;
            case INTD_LONG:     *((epicsInt32   *)data) = *((epicsInt32   *)dataNode -> dataPtr); break;
            case INTD_ULONG:    *((epicsUInt32  *)data) = *((epicsUInt32  *)dataNode -> dataPtr); break;
            case INTD_FLOAT:    *((epicsFloat32 *)data) = *((epicsFloat32 *)dataNode -> dataPtr); break;
            case INTD_DOUBLE:   *((epicsFloat64 *)data) = *((epicsFloat64 *)dataNode -> dataPtr); break;
            case INTD_OLDSTRING: *((char        *)data) = *((char         *)dataNode -> dataPtr); break;
            default: break;
        }
    }

    if(dataNode -> mutexId) epicsMutexUnlock(dataNode -> mutexId);

    /* Signal the event if it is specified */
    if(dataNode -> eventId) epicsEventSignal(dataNode -> eventId);

    return 0;
}

/**
 * Put data from the data node. This will be called by the record processing routine for writing data
 * Input:
 *   dataNode       : Data node to get data from
 *   pno            : Number of points to get from the internal data node
 *   data           : The data write to the node, the buffer should be already defined in the caller
 * Return:
 *   0              : Successful
 *  -1              : Failed
 */
int INTD_API_putData(INTD_struc_node *dataNode, unsigned int pno, void *data)
{
    int var_dataSize;

    /* Check the input */
    if(!dataNode || !data || !dataNode -> dataPtr || pno <= 0) 
        return -1;

    /* Get the data size */
    var_dataSize = INTD_func_getDataSize(dataNode -> dataType);

    /* Put the data based on the size */
    if(dataNode -> mutexId) epicsMutexMustLock(dataNode -> mutexId);

    if(pno > 1) {                                                   /* array */
        memcpy(dataNode -> dataPtr, data, pno * var_dataSize);
    } else if(pno == 1) {                                           /* single value (these code to remove the overhead from memcpy) */
        switch(dataNode -> dataType) {
            case INTD_CHAR:     *((epicsInt8    *)dataNode -> dataPtr) = *((epicsInt8    *)data); break;
            case INTD_UCHAR:    *((epicsUInt8   *)dataNode -> dataPtr) = *((epicsUInt8   *)data); break;
            case INTD_SHORT:    *((epicsInt16   *)dataNode -> dataPtr) = *((epicsInt16   *)data); break;
            case INTD_USHORT:   *((epicsUInt16  *)dataNode -> dataPtr) = *((epicsUInt16  *)data); break;
            case INTD_INT:      *((epicsInt32   *)dataNode -> dataPtr) = *((epicsInt32   *)data); break;
            case INTD_UINT:     *((epicsUInt32  *)dataNode -> dataPtr) = *((epicsUInt32  *)data); break;
            case INTD_LONG:     *((epicsInt32   *)dataNode -> dataPtr) = *((epicsInt32   *)data); break;
            case INTD_ULONG:    *((epicsUInt32  *)dataNode -> dataPtr) = *((epicsUInt32  *)data); break;
            case INTD_FLOAT:    *((epicsFloat32 *)dataNode -> dataPtr) = *((epicsFloat32 *)data); break;
            case INTD_DOUBLE:   *((epicsFloat64 *)dataNode -> dataPtr) = *((epicsFloat64 *)data); break;
            case INTD_OLDSTRING: *((char        *)dataNode -> dataPtr) = *((char         *)data); break;
            default: break;
        }
    }

    if(dataNode -> mutexId) epicsMutexUnlock(dataNode -> mutexId);

    /* Execute the call back if defined */    
    if(dataNode -> writeCallback && dataNode -> enableCallback) 
        (*dataNode -> writeCallback)(dataNode -> privateData);    

    if(!dataNode -> enableCallback)
        dataNode -> enableCallback = 1;

    /* Signal the event if it is specified */
    if(dataNode -> eventId) epicsEventSignal(dataNode -> eventId);

    return 0;
}

/**
 * Generate records for the internal data
 * Input:
 *   moduleName:    the module instance that this file for. Please note that the module name will be hardcoded in the file
 *   path:          the path for the file
 *   dbFileName:    full file name of the output db file
 * Return:
 *   0              : Successful
 *  -1              : Failed
 */
int INTD_API_genRecord(const char *moduleName, const char *path, const char *dbFileName)
{
    char var_fullFileName[256] = "";
    char var_recStr[2048]      = "";                                   /* final string of the record */

    char var_modName[128]      = "";                                   /* module name, identify the record for internal data node */
    char var_datName[128]      = "";                                   /* data name,   identify the record for internal data node */
    char var_subModName[128]   = "";                                   /* sub module name */
    char var_recName[128]      = "";                                   /* record name. The final record "{name_space}{module_name}{sub_module_name}:record_name" */
    char var_scanMethod[128]   = "";
    char var_pno[128]          = "";
    char var_dataType[128]     = "";
    char var_supStr[1024]      = "";
    char var_unitStr[16]       = "";
    char var_descStr[128]      = "";
    char var_aliasStr[128]     = "";
    char var_asgStr[128]       = "";
    unsigned int var_prec      = 0;

    FILE *var_outFile = NULL;

    INTD_struc_node *ptr_dataNode  = NULL;
    
    /* check the input */
    if(!dbFileName || !dbFileName[0]) return -1;

    /* generate the full file name and open the file */
    if(path && path[0]) {
        strcpy(var_fullFileName, path);
        strcat(var_fullFileName, "/");
        strcat(var_fullFileName, dbFileName);
    } else {
        strcpy(var_fullFileName, dbFileName);
    }

    var_outFile = fopen(var_fullFileName, "w");

    if(!var_outFile) {
        printf("INTD_API_genRecord: Failed to create file of %s\n", var_fullFileName);
        return -1;
    }

    /* write to file */

    fprintf(var_outFile, "#-------------------------------------------------------------------------------------\n");
    fprintf(var_outFile, "# %s\n", dbFileName);
    fprintf(var_outFile, "# EPICS database file for the module instance of %s\n", moduleName);
    fprintf(var_outFile, "# Auto generated by InternalData module! Do not modify...\n");
    fprintf(var_outFile, "#-------------------------------------------------------------------------------------\n");
    fprintf(var_outFile, "\n");

    for(ptr_dataNode = (INTD_struc_node *)ellFirst(&INTD_gvar_dataList);
        ptr_dataNode;
        ptr_dataNode = (INTD_struc_node *)ellNext(&ptr_dataNode -> node)) {
        
        /* get the necessary information */
        strncpy(var_modName,    ptr_dataNode -> moduleName,                         128);   /* module name */
        strncpy(var_datName,    ptr_dataNode -> dataName,                           128);   /* data name */
        strncpy(var_subModName, ptr_dataNode -> subModuleName,                      128);   /* sub module name */
        strncpy(var_recName,    ptr_dataNode -> recName,                            128);   /* record name */
        strncpy(var_supStr,     ptr_dataNode -> supStr,                             256);	/* extra string */
        strncpy(var_unitStr,    ptr_dataNode -> unitStr,                            16);    /* unit string */
        strncpy(var_descStr,    ptr_dataNode -> desc,                               128);
        strncpy(var_aliasStr,   ptr_dataNode -> alias,                              128);
        strncpy(var_asgStr,     ptr_dataNode -> asg,                                128);
        strncpy(var_scanMethod, INTD_gvar_scanStrs[(int)ptr_dataNode -> scanType],  128);   /* scan method */
        sprintf(var_pno, "%d", ptr_dataNode -> pno);                                        /* point number (only used for waveform) */
        var_prec = ptr_dataNode -> prec;
        
        switch(ptr_dataNode -> dataType) {                                                  /* field data type (only used for waveform) */
            case INTD_CHAR:     strcpy(var_dataType, "CHAR");   break;
            case INTD_UCHAR:    strcpy(var_dataType, "UCHAR");  break;
            case INTD_SHORT:    strcpy(var_dataType, "SHORT");  break;
            case INTD_USHORT:   strcpy(var_dataType, "USHORT"); break;
            case INTD_INT:      strcpy(var_dataType, "LONG");   break;
            case INTD_UINT:     strcpy(var_dataType, "ULONG");  break;
            case INTD_LONG:     strcpy(var_dataType, "LONG");   break;
            case INTD_ULONG:    strcpy(var_dataType, "ULONG");  break;
            case INTD_FLOAT:    strcpy(var_dataType, "FLOAT");  break;
            case INTD_DOUBLE:   strcpy(var_dataType, "DOUBLE"); break;
            case INTD_OLDSTRING: strcpy(var_dataType, "STRING"); break;
            default:            strcpy(var_dataType, "");       break;
        }
        
        /* generate the string for record */
        switch(ptr_dataNode -> recordType) {
            case INTD_AO:       INTD_RECORD_AO(  var_datName, var_subModName, var_recName, var_scanMethod, var_unitStr, var_descStr, var_prec, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_AI:       INTD_RECORD_AI(  var_datName, var_subModName, var_recName, var_scanMethod, var_unitStr, var_descStr, var_prec, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_BO:       INTD_RECORD_BO(  var_datName, var_subModName, var_recName, var_scanMethod, var_supStr,  var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_BI:       INTD_RECORD_BI(  var_datName, var_subModName, var_recName, var_scanMethod, var_supStr,  var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_LO:       INTD_RECORD_LO(  var_datName, var_subModName, var_recName, var_scanMethod, var_unitStr, var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_LI:       INTD_RECORD_LI(  var_datName, var_subModName, var_recName, var_scanMethod, var_unitStr, var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_MBBO:     INTD_RECORD_MBBO(var_datName, var_subModName, var_recName, var_scanMethod, var_supStr,  var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_MBBI:     INTD_RECORD_MBBI(var_datName, var_subModName, var_recName, var_scanMethod, var_supStr,  var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_WFO:      INTD_RECORD_WFO( var_datName, var_subModName, var_recName, var_scanMethod, var_pno, var_dataType, var_unitStr, var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_WFI:      INTD_RECORD_WFI( var_datName, var_subModName, var_recName, var_scanMethod, var_pno, var_dataType, var_unitStr, var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_SO:       INTD_RECORD_SO(  var_datName, var_subModName, var_recName, var_scanMethod, var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            case INTD_SI:       INTD_RECORD_SI(  var_datName, var_subModName, var_recName, var_scanMethod, var_descStr, var_aliasStr, var_asgStr, var_recStr); break;
            default: strcpy(var_recStr, "");
        }
            
        /* save to file only for this module */
		if(strcmp(moduleName, "MOD_ALL") == 0 || strcmp(moduleName, var_modName) == 0)
        	fprintf(var_outFile, "%s\n\n", var_recStr);

    }    

    /* close to file */
    fflush(var_outFile);
    fclose(var_outFile);

    return 0;
}

/**
 * Generate save/restore request file for the internal data
 * Input:
 *   moduleName:    the module instance that this file for. Please note that the module name will be hardcoded in the file
 *   path:          the path for the file
 *   reqFileName:   full file name of the output req file
 *   withMacro:     chose to use macro or hardcoded PV names (0 - same module name; 1 - use macro; 2 - use destModuleName)
 *   sel:           0 - scalar output PVs; 1 - waveform output PVs; 2 - all output PVs
 *   destModuleName:generate a file for other module with the same type
 * Return:
 *   0              : Successful
 *  -1              : Failed
 */
int INTD_API_genSRReqt(const char *moduleName, const char *path, const char *reqFileName, int withMacro, int sel, const char *destModuleName)
{
    char var_fullSRFileName[256]    = "";
    char var_modName[128]           = "";
    char var_destModName[128]       = "";
    char var_subModName[128]        = ""; 
    char var_recName[128]           = "";                                   /* record name */
    char var_scanMethod[128]        = "";
    char var_pno[128]               = "";

    FILE *var_outFile_sr = NULL;
    INTD_struc_node *ptr_dataNode = NULL;
    
    /* check the input */
    if( !moduleName     || !moduleName[0]   ||
        !reqFileName    || !reqFileName[0]) 
        return -1;

    /* generate the full file names */
    if(path && path[0]) {
        strcpy(var_fullSRFileName,   path);
        strcat(var_fullSRFileName,   "/");
        strcat(var_fullSRFileName,   reqFileName);
    } else {
        strcpy(var_fullSRFileName,   reqFileName);
    }

    /* other module name with the same type */
    if(withMacro == 2 && destModuleName && destModuleName[0])
        strncpy(var_destModName, destModuleName, 128);                                      

    /* open the file */
    var_outFile_sr  = fopen(var_fullSRFileName, "w");

    if(!var_outFile_sr) {
		printf("INTD_API_genSRReqt: Failed to create file of %s\n", var_fullSRFileName);
        return -1;
    }

    /* write to file */

    fprintf(var_outFile_sr, "#//UPDATE-FREQ=60\n");
    fprintf(var_outFile_sr, "#ENABLE-PASS=1\n");
    fprintf(var_outFile_sr, "#-------------------------------------------------------------------------------------\n");
    fprintf(var_outFile_sr, "# %s\n", reqFileName);
    fprintf(var_outFile_sr, "# Autosave request file for the module instance of %s\n", moduleName); 
    fprintf(var_outFile_sr, "# Auto generated by InternalData module!\n");
    fprintf(var_outFile_sr, "#-------------------------------------------------------------------------------------\n");
    fprintf(var_outFile_sr, "\n");

    for(ptr_dataNode = (INTD_struc_node *)ellFirst(&INTD_gvar_dataList);
        ptr_dataNode;
        ptr_dataNode = (INTD_struc_node *)ellNext(&ptr_dataNode -> node)) {
        
        /* get the necessary information */
        strncpy(var_modName,        ptr_dataNode -> moduleName,                         128);   /* module name */
        strncpy(var_subModName,     ptr_dataNode -> subModuleName,                      128);   /* sub module name */
        strncpy(var_recName,        ptr_dataNode -> recName,                            128);   /* record name */
        strncpy(var_scanMethod,     INTD_gvar_scanStrs[(int)ptr_dataNode -> scanType],  128);   /* scan method */
        sprintf(var_pno, "%d", ptr_dataNode -> pno);                                            /* point number (only used for waveform) */      
     
        /* generate the string for record and save to file for this module instance */
		if(strcmp(moduleName, "MOD_ALL") == 0 || strcmp(moduleName, var_modName) == 0) {

            switch(ptr_dataNode -> recordType) {
                case INTD_AO:
                case INTD_LO:
                    if(sel == 0 || sel == 2) {   
                        if(withMacro == 1) {
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s\n",      var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.HIHI\n", var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.HIGH\n", var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.LOW\n",  var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.LOLO\n", var_subModName, var_recName);
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.DRVL\n", var_subModName, var_recName);
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.DRVH\n", var_subModName, var_recName);
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.LSV\n",  var_subModName, var_recName);
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.HSV\n",  var_subModName, var_recName);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_sr, "%s%s:%s\n",      var_destModName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.HIHI\n", var_destModName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.HIGH\n", var_destModName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.LOW\n",  var_destModName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.LOLO\n", var_destModName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.DRVL\n", var_destModName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.DRVH\n", var_destModName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.LSV\n",  var_destModName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.HSV\n",  var_destModName, var_subModName, var_recName);
                        } else {
                            fprintf(var_outFile_sr, "%s%s:%s\n",      var_modName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.HIHI\n", var_modName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.HIGH\n", var_modName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.LOW\n",  var_modName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.LOLO\n", var_modName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.DRVL\n", var_modName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.DRVH\n", var_modName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.LSV\n",  var_modName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.HSV\n",  var_modName, var_subModName, var_recName);
                        } 
                    }
                    break;

                case INTD_AI:
                case INTD_LI:
                    if(sel == 0 || sel == 2) {   
                        if(withMacro == 1) {
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.HIHI\n", var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.HIGH\n", var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.LOW\n",  var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.LOLO\n", var_subModName, var_recName);
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.LSV\n",  var_subModName, var_recName);
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s.HSV\n",  var_subModName, var_recName);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_sr, "%s%s:%s.HIHI\n", var_destModName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.HIGH\n", var_destModName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.LOW\n",  var_destModName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.LOLO\n", var_destModName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.LSV\n",  var_destModName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.HSV\n",  var_destModName, var_subModName, var_recName);
                        } else {
                            fprintf(var_outFile_sr, "%s%s:%s.HIHI\n", var_modName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.HIGH\n", var_modName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.LOW\n",  var_modName, var_subModName, var_recName);  
                            fprintf(var_outFile_sr, "%s%s:%s.LOLO\n", var_modName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.LSV\n",  var_modName, var_subModName, var_recName);
                            fprintf(var_outFile_sr, "%s%s:%s.HSV\n",  var_modName, var_subModName, var_recName);
                        }
                    }
                    break;

	            case INTD_BO:  
            	case INTD_MBBO:  
            	case INTD_WFO:  
            	case INTD_SO:  
                    if(sel == 0 || sel == 2) {     
                        if(withMacro == 1) {
                            fprintf(var_outFile_sr, "$(name_space)$(module_name)%s:%s\n", var_subModName, var_recName);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_sr, "%s%s:%s\n", var_destModName, var_subModName, var_recName);
                        } else {
                            fprintf(var_outFile_sr, "%s%s:%s\n", var_modName, var_subModName, var_recName);
                        }
                    }
                    break;

            	default: break;
        	}        
		}        }    

    /* close to file */
    fflush(var_outFile_sr);
    fclose(var_outFile_sr);

    return 0;
}

/** * Generate archiver configuration file
 * Input:
 *   moduleName:    the module instance that this file for. Please note that the module name will be hardcoded in the file
 *   path:          the path for the file
 *   confFileName:  full file name of the output config file
 *   methodStr:     string for archive method for short/medium/long term. These apply only on the results PVs (IPV) with periodic scanning
 *   withMacro:     chose to use macro or hardcoded PV names (0 - same module name; 1 - use macro; 2 - use destModuleName)
 *   sel:           0 - short term; 1 - medium term; 2 - long term; 3 - combined
 *   destModuleName:generate a file for other module with the same type
 * Return:
 *   0              : Successful
 *  -1              : Failed
 */
int INTD_API_genArchive(const char *moduleName, const char *path, const char *confFileName, const char *methodStr, int withMacro, int sel, const char *destModuleName)
{
    char var_fullFileName[256]      = "";
    char var_modName[128]           = "";
    char var_destModName[128]       = "";
    char var_subModName[128]        = ""; 
    char var_recName[128]           = "";                                   /* record name */
    char var_OPVArchiveMethod[128]  = "";
    char var_IPVArchiveMethod[128]  = "";

    FILE *var_outFile_conf = NULL;
    INTD_struc_node *ptr_dataNode = NULL;
    
    /* check the input */
    if(!moduleName || !moduleName[0] || !confFileName || !confFileName[0]) return -1;

    /* generate the full file names */
    if(path && path[0]) {
        strcpy(var_fullFileName, path);
        strcat(var_fullFileName, "/");
        strcat(var_fullFileName, confFileName);
    } else {
        strcpy(var_fullFileName, confFileName);
    }

    /* other module name with the same type */
    if(withMacro == 2 && destModuleName && destModuleName[0])
        strncpy(var_destModName, destModuleName, 128);

    /* generate the archive string:
       - for output PVs for parameters setting, always use monitor method
       - for input PVs for results, if it is event driven, always use monitor method
       - for input PVs for results, if it is updated periodically, use the assigned method
    */
    if(sel == 3) {                                                  /* combined */
        strcpy(var_OPVArchiveMethod, "None None Monitor");
        strcpy(var_IPVArchiveMethod, methodStr);                    /* here method string should have 3 items seperated with space */
    } else {
        strcpy(var_OPVArchiveMethod, "Monitor");
        strcpy(var_IPVArchiveMethod, methodStr);                    /* here method string only have one */
    }
 
    /* open the file */
    var_outFile_conf  = fopen(var_fullFileName, "w");

    if(!var_outFile_conf) {
		printf("INTD_API_genArchive: Failed to create file of %s\n", var_fullFileName);
        return -1;
    }

    /* write to file */

    fprintf(var_outFile_conf, "#-------------------------------------------------------------------------------------\n");
    fprintf(var_outFile_conf, "# %s\n", confFileName);
    fprintf(var_outFile_conf, "# Archiver configure file for the module instance of %s\n", moduleName); 
    fprintf(var_outFile_conf, "# Auto generated by InternalData module!\n");
    fprintf(var_outFile_conf, "#-------------------------------------------------------------------------------------\n");
    fprintf(var_outFile_conf, "\n");

    for(ptr_dataNode = (INTD_struc_node *)ellFirst(&INTD_gvar_dataList);
        ptr_dataNode;
        ptr_dataNode = (INTD_struc_node *)ellNext(&ptr_dataNode -> node)) {
        
        /* get the necessary information */
        strncpy(var_modName,    ptr_dataNode -> moduleName,    128);   /* module name */
        strncpy(var_subModName, ptr_dataNode -> subModuleName, 128);   /* sub module name */
        strncpy(var_recName,    ptr_dataNode -> recName,       128);   /* record name */
               
        /* generate the string for record and save to file for this module instance */
		if(strcmp(moduleName, "MOD_ALL") == 0 || strcmp(moduleName, var_modName) == 0) {

            switch(ptr_dataNode -> recordType) {
                case INTD_AO:
        	    case INTD_LO:
                    if(ptr_dataNode -> scanType == INTD_IOINT || ptr_dataNode -> scanType == INTD_PASSIVE) {    /* means event driven scanning for results, passive scanning for settings */
                        if(withMacro == 1) {
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s %s\n",      var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HIHI %s\n", var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HIGH %s\n", var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LOW %s\n",  var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LOLO %s\n", var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.DRVL %s\n", var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.DRVH %s\n", var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LSV %s\n",  var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HSV %s\n",  var_subModName, var_recName, var_OPVArchiveMethod);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n",      var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIHI %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIGH %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOW %s\n",  var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOLO %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.DRVL %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.DRVH %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.LSV %s\n",  var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.HSV %s\n",  var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                        } else {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n",      var_modName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIHI %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIGH %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOW %s\n",  var_modName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOLO %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.DRVL %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.DRVH %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.LSV %s\n",  var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.HSV %s\n",  var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                        }
                    } else {                                                                                    /* means periodic scanning for results */
                        if(withMacro == 1) {
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s %s\n",      var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HIHI %s\n", var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HIGH %s\n", var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LOW %s\n",  var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LOLO %s\n", var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.DRVL %s\n", var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.DRVH %s\n", var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LSV %s\n",  var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HSV %s\n",  var_subModName, var_recName, var_IPVArchiveMethod);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n",      var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIHI %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIGH %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOW %s\n",  var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOLO %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.DRVL %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.DRVH %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.LSV %s\n",  var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.HSV %s\n",  var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                        } else {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n",      var_modName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIHI %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIGH %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOW %s\n",  var_modName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOLO %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.DRVL %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.DRVH %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.LSV %s\n",  var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.HSV %s\n",  var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                        }
                    }
                    break;

                case INTD_AI:
                case INTD_LI:
                    if(ptr_dataNode -> scanType == INTD_IOINT || ptr_dataNode -> scanType == INTD_PASSIVE) {    /* means event driven scanning for results, passive scanning for settings */
                        if(withMacro == 1) {
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s %s\n",      var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HIHI %s\n", var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HIGH %s\n", var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LOW %s\n",  var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LOLO %s\n", var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LSV %s\n",  var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HSV %s\n",  var_subModName, var_recName, var_OPVArchiveMethod);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n",      var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIHI %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIGH %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOW %s\n",  var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOLO %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.LSV %s\n",  var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.HSV %s\n",  var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                        } else {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n",      var_modName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIHI %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIGH %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOW %s\n",  var_modName, var_subModName, var_recName, var_OPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOLO %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.LSV %s\n",  var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.HSV %s\n",  var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                        }
                    } else {                                                                                    /* means periodic scanning for results */
                        if(withMacro == 1) {
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s %s\n",      var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HIHI %s\n", var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HIGH %s\n", var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LOW %s\n",  var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LOLO %s\n", var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.LSV %s\n",  var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s.HSV %s\n",  var_subModName, var_recName, var_IPVArchiveMethod);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n",      var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIHI %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIGH %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOW %s\n",  var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOLO %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.LSV %s\n",  var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.HSV %s\n",  var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                        } else {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n",      var_modName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIHI %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.HIGH %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOW %s\n",  var_modName, var_subModName, var_recName, var_IPVArchiveMethod);  
                            fprintf(var_outFile_conf, "%s%s:%s.LOLO %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.LSV %s\n",  var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                            fprintf(var_outFile_conf, "%s%s:%s.HSV %s\n",  var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                        }
                    }
                    break;

	            case INTD_BO:
	            case INTD_BI:
	            case INTD_MBBO:
	            case INTD_MBBI:
	            case INTD_WFO:
                    if(ptr_dataNode -> scanType == INTD_IOINT || ptr_dataNode -> scanType == INTD_PASSIVE) {
                        if(withMacro == 1) {
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s %s\n", var_subModName, var_recName, var_OPVArchiveMethod);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n", var_destModName, var_subModName, var_recName, var_OPVArchiveMethod);
                        } else {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n", var_modName, var_subModName, var_recName, var_OPVArchiveMethod);
                        }
                    } else {
                        if(withMacro == 1) {
                            fprintf(var_outFile_conf, "$(name_space)$(module_name)%s:%s %s\n", var_subModName, var_recName, var_IPVArchiveMethod);
                        } else if(withMacro == 2) {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n", var_destModName, var_subModName, var_recName, var_IPVArchiveMethod);
                        } else {
                            fprintf(var_outFile_conf, "%s%s:%s %s\n", var_modName, var_subModName, var_recName, var_IPVArchiveMethod);
                        }
                    }
                    break;

            	default: break;
        	}        
		}        }    

    /* close to file */
    fflush(var_outFile_conf);
    fclose(var_outFile_conf);

    return 0;
}

/**
 * Generate a list file for the internal data
 * Input:
 *   moduleName:    the module instance that this file for. Please note that the module name will be hardcoded in the file
 *   path:          the path for the file
 *   lstFileName:   full file name of the output list file
 * Return:
 *   0              : Successful
 *  -1              : Failed
 */
int INTD_API_genDbList(const char *moduleName, const char *path, const char *lstFileName)
{
    char var_fullListFileName[256] = "";
    char var_modName[128]    = "";
    char var_subModName[128] = ""; 
    char var_recName[128]    = "";                                   /* record name */
    char var_scanMethod[128] = "";
    char var_pno[128]        = "";
    char var_dataType[128]   = "";


    FILE *var_outFile       = NULL;

    INTD_struc_node *ptr_dataNode  = NULL;
    
    /* check the input */
    if(!lstFileName || !lstFileName[0]) return -1;

    /* generate the full file names */
    if(path && path[0]) {
        strcpy(var_fullListFileName, path);
        strcat(var_fullListFileName, "/");
        strcat(var_fullListFileName, lstFileName);
    } else {
        strcpy(var_fullListFileName, lstFileName);
    }

    /* open the file */
    var_outFile     = fopen(var_fullListFileName, "w");

    if(!var_outFile) {
		printf("INTD_API_genDbList: Failed to create file of %s\n", var_fullListFileName);
        return -1;
    }

    /* write to file */

    fprintf(var_outFile, "#-------------------------------------------------------------------------------------\n");
    fprintf(var_outFile, "# %s\n", lstFileName);
    fprintf(var_outFile, "# Generate the list of records for the module instance of %s\n", moduleName);
    fprintf(var_outFile, "# Auto generated by InternalData module!\n");
    fprintf(var_outFile, "#\n");
    fprintf(var_outFile, "# variable-id string, record type, scan type, data type, num of point\n"); 
    fprintf(var_outFile, "#-------------------------------------------------------------------------------------\n");
    fprintf(var_outFile, "\n");

    for(ptr_dataNode = (INTD_struc_node *)ellFirst(&INTD_gvar_dataList);
        ptr_dataNode;
        ptr_dataNode = (INTD_struc_node *)ellNext(&ptr_dataNode -> node)) {
        
        /* get the necessary information */
        strncpy(var_modName,    ptr_dataNode -> moduleName,                         128);   /* module name */
        strncpy(var_subModName, ptr_dataNode -> subModuleName,                      128);   /* sub module name */
        strncpy(var_recName,    ptr_dataNode -> recName,                            128);   /* record name */
        strncpy(var_scanMethod, INTD_gvar_scanStrs[(int)ptr_dataNode -> scanType],  128);   /* scan method */
        sprintf(var_pno, "%d", ptr_dataNode -> pno);                                        /* point number (only used for waveform) */
        
        switch(ptr_dataNode -> dataType) {                                                  /* field data type (only used for waveform) */
            case INTD_CHAR:     strcpy(var_dataType, "CHAR");   break;
            case INTD_UCHAR:    strcpy(var_dataType, "UCHAR");  break;
            case INTD_SHORT:    strcpy(var_dataType, "SHORT");  break;
            case INTD_USHORT:   strcpy(var_dataType, "USHORT"); break;
            case INTD_INT:      strcpy(var_dataType, "LONG");   break;
            case INTD_UINT:     strcpy(var_dataType, "ULONG");  break;
            case INTD_LONG:     strcpy(var_dataType, "LONG");   break;
            case INTD_ULONG:    strcpy(var_dataType, "ULONG");  break;
            case INTD_FLOAT:    strcpy(var_dataType, "FLOAT");  break;
            case INTD_DOUBLE:   strcpy(var_dataType, "DOUBLE"); break;
            case INTD_OLDSTRING:   strcpy(var_dataType, "STRING"); break;
            default:            strcpy(var_dataType, "");       break;
        }
        
        /* generate the string for record and save to file for this module instance */
		if(strcmp(moduleName, "MOD_ALL") == 0 || strcmp(moduleName, var_modName) == 0) {
        	switch(ptr_dataNode -> recordType) {
            	case INTD_AO:       fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tao\t%s\tDOUBLE\t1\n",    var_subModName, var_recName, var_scanMethod); break;
            	case INTD_AI:       fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tai\t%s\tDOUBLE\t1\n",    var_subModName, var_recName, var_scanMethod); break;
            	case INTD_BO:       fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tbo\t%s\tUSHORT\t1\n",    var_subModName, var_recName, var_scanMethod); break;
            	case INTD_BI:       fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tbi\t%s\tUSHORT\t1\n",    var_subModName, var_recName, var_scanMethod); break;
            	case INTD_LO:       fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tlongout\t%s\tLONG\t1\n", var_subModName, var_recName, var_scanMethod); break;
            	case INTD_LI:       fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tlongin\t%s\tLONG\t1\n",  var_subModName, var_recName, var_scanMethod); break;
            	case INTD_MBBO:     fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tmbbo\t%s\tULONG\t1\n",   var_subModName, var_recName, var_scanMethod); break;
            	case INTD_MBBI:     fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tmbbi\t%s\tULONG\t1\n",   var_subModName, var_recName, var_scanMethod); break;
            	case INTD_WFO:      fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\twaveform(out)\t%s\t%s\t%s\n", var_subModName, var_recName, var_scanMethod, var_dataType, var_pno); break;
            	case INTD_WFI:      fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\twaveform(in)\t%s\t%s\t%s\n",  var_subModName, var_recName, var_scanMethod, var_dataType, var_pno); break;
            	case INTD_SO:       fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tstringout\t%s\tSTRING\t1\n",  var_subModName, var_recName, var_scanMethod); break;
            	case INTD_SI:       fprintf(var_outFile, "$(name_space)$(module_name)%s:%s\tstringin\t%s\tSTRING\t1\n",   var_subModName, var_recName, var_scanMethod); break;
            	default: break;
        	}            
		}
    }    

    /* close to file */
    fflush(var_outFile);
    fclose(var_outFile);

    return 0;
}

/**
 * Synchronize the internal data with the records
 * senario 1: put the record data into the internal data and read the internal data to the record, without executing the call back functions
 * senario 2: put the record data into the internal data and read the internal data to the record, wit executing the call back functions
 */
int INTD_API_syncWithRecords(int enaCallback)
{
    INTD_struc_node *ptr_dataNode  = NULL;

    /* process the records */
    for(ptr_dataNode = (INTD_struc_node *)ellFirst(&INTD_gvar_dataList);
        ptr_dataNode;
        ptr_dataNode = (INTD_struc_node *)ellNext(&ptr_dataNode -> node)) {
        
        if(ptr_dataNode -> epicsRecord) {
            if( ptr_dataNode -> recordType == INTD_AO    ||
                ptr_dataNode -> recordType == INTD_BO    ||
                ptr_dataNode -> recordType == INTD_LO    ||
                ptr_dataNode -> recordType == INTD_MBBO  ||
                ptr_dataNode -> recordType == INTD_WFO   ||
                ptr_dataNode -> recordType == INTD_SO) {    
                ptr_dataNode -> enableCallback = enaCallback;
                scanOnce(ptr_dataNode -> epicsRecord);
            }
        }
    }    

    return 0;
}

/**
 * Force to set the value of the output PVs (AO, BO, LO, MBBO, WFO, SO) and process the PV.
 * This routine offers a way to synch the setting PVs with the internal changes of the variables.
 * Note: the dbPutField can also be used to update other fields of the record.
 */
int INTD_API_forceOPVValue(INTD_struc_node *dataNode, void *dataPtr)
{
	struct dbAddr	paddr;
	short         	dbrType;
	long 		  	nRequest;

	// check the input
	if(!dataNode || !dataPtr || !dataNode -> epicsRecord) return -1;

	// use dbPutField
	dbNameToAddr(dataNode -> epicsRecord -> name, &paddr);
	nRequest = (long)dataNode -> pno;

    switch(dataNode -> dataType) {
        case INTD_CHAR:		dbrType = DBR_CHAR; 	break;		
        case INTD_UCHAR:	dbrType = DBR_UCHAR; 	break;		
        case INTD_SHORT:	dbrType = DBR_SHORT; 	break;		
        case INTD_USHORT:	dbrType = DBR_USHORT; 	break;		
        case INTD_INT:		dbrType = DBR_LONG; 	break;		
        case INTD_UINT:		dbrType = DBR_ULONG; 	break;		
        case INTD_LONG:		dbrType = DBR_LONG; 	break;		
        case INTD_ULONG:	dbrType = DBR_ULONG; 	break;		
        case INTD_FLOAT:	dbrType = DBR_FLOAT; 	break;		
        case INTD_DOUBLE:	dbrType = DBR_DOUBLE; 	break;
        case INTD_OLDSTRING:	dbrType = DBR_STRING; 	break;		
        default:                dbrType = DBR_DOUBLE;   break;
    }

	return dbPutField(&paddr, dbrType, dataPtr, nRequest);
}

/** 
 * Force the PV to be processed
 */
int INTD_API_forcePVProcess(INTD_struc_node *dataNode)
{
	/* check the input */
	if(!dataNode || !dataNode -> epicsRecord)
		return -1;

	/* scan it */
	scanOnce(dataNode -> epicsRecord);

    return 0;
}

/** 
 * Raise an alarm
 */
int INTD_API_raiseAlarm(INTD_struc_node *dataNode, epicsEnum16 nsta, epicsEnum16 nsevr)
{
	/* check the input */
	if(!dataNode)
		return -1;

    /* remember the setting */
    dataNode -> nsta    = nsta;
    dataNode -> nsevr   = nsevr;

    return 0;
}

/**
 * Get the IOC init status. 1 means done
 */
int INTD_API_getIocInitStatus()
{
	return INTD_gvar_iocInitDone;
}

/**
 * Run time function: Get the value of a field
 */
int INTD_API_getFieldInfo(INTD_struc_node *dataNode, const char *fieldName, short *dbrType, long *nelm)
{
	struct dbAddr paddr;    
    char  fullName[128] = "";

	// check the input
	if(!dataNode || !fieldName || !dataNode -> epicsRecord) return -1;

    // get the full name
    strcpy(fullName, dataNode -> epicsRecord -> name);
    strcat(fullName, ".");
    strcat(fullName, fieldName);

	// use dbGetField
	dbNameToAddr(fullName, &paddr);

    if(dbrType) *dbrType = paddr.dbr_field_type;
    if(nelm)    *nelm    = paddr.no_elements;

    return 0;
}

int INTD_API_getFieldData(INTD_struc_node *dataNode, const char *fieldName, void *data, long pno)
{
	struct dbAddr paddr;    
    long  options       = 0;
    long  nelm          = 0;
    char  fullName[128] = "";

	// check the input
	if(!dataNode || !fieldName || !data || !dataNode -> epicsRecord || pno <= 0) return -1;

    // get the full name
    strcpy(fullName, dataNode -> epicsRecord -> name);
    strcat(fullName, ".");
    strcat(fullName, fieldName);

	// use dbGetField
	dbNameToAddr(fullName, &paddr);

    nelm = paddr.no_elements;
    if(nelm > pno) nelm = pno;

    return dbGetField(&paddr, paddr.dbr_field_type, data, &options, &nelm, NULL);
}

int INTD_API_putFieldData(INTD_struc_node *dataNode, const char *fieldName, double *data, long pno)
{
	struct dbAddr paddr;    
    long  nelm          = 0;
    char  fullName[128] = "";

	// check the input
	if(!dataNode || !fieldName || !data || !dataNode -> epicsRecord || pno <= 0) return -1;

    // get the full name
    strcpy(fullName, dataNode -> epicsRecord -> name);
    strcat(fullName, ".");
    strcat(fullName, fieldName);

	// use dbPutField
	dbNameToAddr(fullName, &paddr);

    nelm = paddr.no_elements;
    if(nelm > pno) nelm = pno;

    return dbPutField(&paddr, DBR_DOUBLE, data, nelm);
}

















