/****************************************************
 * recordGenerate.c
 * 
 * Source file for generating record from the internal data node
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.06.30
 * Description: Initial creation
 *
 * Modified by Zheqiao Geng on 2014.09.27
 * Introduced sub module name. make the module name also as a substitution
 ****************************************************/
#include "recordGenerate.h"

void INTD_RECORD_AO(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *unitStr, char *outStr)
{
    if(!recName || !scanMethod || !outStr) return;

    strcpy(outStr, "record(ao, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(EGU, \"");
    strcat(outStr, unitStr);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(OUT, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n}");     
}

void INTD_RECORD_AI(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *unitStr, char *outStr)
{
    if(!recName || !scanMethod || !outStr) return;

    strcpy(outStr, "record(ai, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(EGU, \"");
    strcat(outStr, unitStr);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(INP, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");    
    strcat(outStr, datName);
    strcat(outStr, "\")\n}");     
}

void INTD_RECORD_BO(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *supStr, char *outStr)
{
	char strOne[128]  = "";
	char strZero[128] = "";
	char *pfh = NULL;
	char *ped = NULL;

    if(!recName || !scanMethod || !outStr) return;

	/* get the strings for bo */
    pfh = strchr(supStr, ';');
    ped = strchr(supStr, '\0');

	if(pfh && ped) {
	    strncpy(strOne,  supStr,  pfh - supStr);
    	strncpy(strZero, pfh + 1, ped - pfh - 1);
	}

	/* the string for record */
    strcpy(outStr, "record(bo, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(OUT, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n        field(ONAM, \"");
    strcat(outStr, strOne);
    strcat(outStr, "\")\n        field(ZNAM, \"");
    strcat(outStr, strZero);
    strcat(outStr, "\")\n}");
}

void INTD_RECORD_BI(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *supStr, char *outStr)
{
	char strOne[128]  = "";
	char strZero[128] = "";
	char *pfh = NULL;
	char *ped = NULL;

    if(!recName || !scanMethod || !outStr) return;

	/* get the strings for bi */
    pfh = strchr(supStr, ';');
    ped = strchr(supStr, '\0');

	if(pfh && ped) {
	    strncpy(strOne,  supStr,  pfh - supStr);
    	strncpy(strZero, pfh + 1, ped - pfh - 1);
	}

	/* the string for record */
    strcpy(outStr, "record(bi, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(INP, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n        field(ONAM, \"");
    strcat(outStr, strOne);
    strcat(outStr, "\")\n        field(ZNAM, \"");
    strcat(outStr, strZero);
    strcat(outStr, "\")\n}");    
}

void INTD_RECORD_LO(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *unitStr, char *outStr)
{
    if(!recName || !scanMethod || !outStr) return;

    strcpy(outStr, "record(longout, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(EGU, \"");
    strcat(outStr, unitStr);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(OUT, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n}");     
}

void INTD_RECORD_LI(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *unitStr, char *outStr)
{
    if(!recName || !scanMethod || !outStr) return;

    strcpy(outStr, "record(longin, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(EGU, \"");
    strcat(outStr, unitStr);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(INP, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n}");     
}

void INTD_RECORD_MBBO(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *supStr, char *outStr)
{
	int i;
	char strSel[16][128] = {""};
	char subSupStr[1024] = "";
	char *pfh = NULL;

    if(!recName || !scanMethod || !outStr) return;

	/* get the strings for mbbo */
	strcpy(subSupStr, supStr);

	for(i = 0; i < 16; i ++) {
    	pfh = strchr(subSupStr, ';');
		if(pfh) {
			strncpy(strSel[i], subSupStr, pfh - subSupStr);
    		strcpy(subSupStr, pfh + 1);
		} else {
			strcpy(strSel[i], subSupStr);
			break;
		}
	}

	/* build up the string for record */
    strcpy(outStr, "record(mbbo, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(OUT, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n        field(ZRST, \"");
    strcat(outStr, strSel[0]);
    strcat(outStr, "\")\n        field(ONST, \"");
    strcat(outStr, strSel[1]);
    strcat(outStr, "\")\n        field(TWST, \"");
    strcat(outStr, strSel[2]);
    strcat(outStr, "\")\n        field(THST, \"");
    strcat(outStr, strSel[3]);
    strcat(outStr, "\")\n        field(FRST, \"");
    strcat(outStr, strSel[4]);
    strcat(outStr, "\")\n        field(FVST, \"");
    strcat(outStr, strSel[5]);
    strcat(outStr, "\")\n        field(SXST, \"");
    strcat(outStr, strSel[6]);
    strcat(outStr, "\")\n        field(SVST, \"");
    strcat(outStr, strSel[7]);
    strcat(outStr, "\")\n        field(EIST, \"");
    strcat(outStr, strSel[8]);
    strcat(outStr, "\")\n        field(NIST, \"");
    strcat(outStr, strSel[9]);
    strcat(outStr, "\")\n        field(TEST, \"");
    strcat(outStr, strSel[10]);
    strcat(outStr, "\")\n        field(ELST, \"");
    strcat(outStr, strSel[11]);
    strcat(outStr, "\")\n        field(TVST, \"");
    strcat(outStr, strSel[12]);
    strcat(outStr, "\")\n        field(TTST, \"");
    strcat(outStr, strSel[13]);
    strcat(outStr, "\")\n        field(FTST, \"");
    strcat(outStr, strSel[14]);
    strcat(outStr, "\")\n        field(FFST, \"");
    strcat(outStr, strSel[15]);
    strcat(outStr, "\")\n}");     
}

void INTD_RECORD_MBBI(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *supStr, char *outStr)
{
	int i;
	char strSel[16][128] = {""};
	char subSupStr[1024] = "";
	char *pfh = NULL;

    if(!recName || !scanMethod || !outStr) return;

	/* get the strings for mbbi */
	strcpy(subSupStr, supStr);

	for(i = 0; i < 16; i ++) {
    	pfh = strchr(subSupStr, ';');
		if(pfh) {
			strncpy(strSel[i], subSupStr, pfh - subSupStr);
    		strcpy(subSupStr, pfh + 1);
		} else {
			strcpy(strSel[i], subSupStr);
			break;
		}
	}

	/* build up the string for record */
    strcpy(outStr, "record(mbbi, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(INP, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n        field(ZRST, \"");
    strcat(outStr, strSel[0]);
    strcat(outStr, "\")\n        field(ONST, \"");
    strcat(outStr, strSel[1]);
    strcat(outStr, "\")\n        field(TWST, \"");
    strcat(outStr, strSel[2]);
    strcat(outStr, "\")\n        field(THST, \"");
    strcat(outStr, strSel[3]);
    strcat(outStr, "\")\n        field(FRST, \"");
    strcat(outStr, strSel[4]);
    strcat(outStr, "\")\n        field(FVST, \"");
    strcat(outStr, strSel[5]);
    strcat(outStr, "\")\n        field(SXST, \"");
    strcat(outStr, strSel[6]);
    strcat(outStr, "\")\n        field(SVST, \"");
    strcat(outStr, strSel[7]);
    strcat(outStr, "\")\n        field(EIST, \"");
    strcat(outStr, strSel[8]);
    strcat(outStr, "\")\n        field(NIST, \"");
    strcat(outStr, strSel[9]);
    strcat(outStr, "\")\n        field(TEST, \"");
    strcat(outStr, strSel[10]);
    strcat(outStr, "\")\n        field(ELST, \"");
    strcat(outStr, strSel[11]);
    strcat(outStr, "\")\n        field(TVST, \"");
    strcat(outStr, strSel[12]);
    strcat(outStr, "\")\n        field(TTST, \"");
    strcat(outStr, strSel[13]);
    strcat(outStr, "\")\n        field(FTST, \"");
    strcat(outStr, strSel[14]);
    strcat(outStr, "\")\n        field(FFST, \"");
    strcat(outStr, strSel[15]);
    strcat(outStr, "\")\n}");     
}

void INTD_RECORD_WFO(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *pno, const char *dataType, const char *unitStr, char *outStr)
{
    if(!recName || !scanMethod || !pno || !dataType || !outStr) return;

    strcpy(outStr, "record(waveform, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(DESC, \"[W]\")\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(EGU, \"");
    strcat(outStr, unitStr);
    strcat(outStr, "\")\n        field(NELM, \"");
    strcat(outStr, pno);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(FTVL, \"");
    strcat(outStr, dataType);
    strcat(outStr, "\")\n        field(INP, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n}");    
}

void INTD_RECORD_WFI(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *pno, const char *dataType, const char *unitStr, char *outStr)
{
    if(!recName || !scanMethod || !pno || !dataType || !outStr) return;

    strcpy(outStr, "record(waveform, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(EGU, \"");
    strcat(outStr, unitStr);
    strcat(outStr, "\")\n        field(NELM, \"");
    strcat(outStr, pno);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(FTVL, \"");
    strcat(outStr, dataType);
    strcat(outStr, "\")\n        field(INP, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n}");    
}

void INTD_RECORD_SO(const char *datName, const char *subModName, const char *recName, const char *scanMethod, char *outStr)
{
    if(!recName || !scanMethod || !outStr) return;

    strcpy(outStr, "record(stringout, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(OUT, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");
    strcat(outStr, datName);
    strcat(outStr, "\")\n}");     
}

void INTD_RECORD_SI(const char *datName, const char *subModName, const char *recName, const char *scanMethod, char *outStr)
{
    if(!recName || !scanMethod || !outStr) return;

    strcpy(outStr, "record(stringin, $(name_space)");
    strcat(outStr, "$(module_name)");
    strcat(outStr, subModName);
    strcat(outStr, ":");
    strcat(outStr, recName);
    strcat(outStr, ") {\n        field(SCAN, \"");
    strcat(outStr, scanMethod);
    strcat(outStr, "\")\n        field(DTYP, \"InternalData\")\n        field(INP, \"@");
    strcat(outStr, "$(module_name)");
    strcat(outStr, ".");    
    strcat(outStr, datName);
    strcat(outStr, "\")\n}");     
}

