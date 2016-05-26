/****************************************************
 * recordGenerate.h
 * 
 * Header file for generating record from the internal data node
 *
 * Created by: Zheqiao Geng, gengzq@slac.stanford.edu
 * Created on: 2011.06.30
 * Description: Initial creation
 ****************************************************/
#ifndef RECORD_GENERATE_H
#define RECORD_GENERATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Routines for record generation
 */
void INTD_RECORD_AO(  const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *unitStr, char *outStr);
void INTD_RECORD_AI(  const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *unitStr, char *outStr);
void INTD_RECORD_BO(  const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *supStr, char *outStr);
void INTD_RECORD_BI(  const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *supStr, char *outStr);
void INTD_RECORD_LO(  const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *unitStr, char *outStr);
void INTD_RECORD_LI(  const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *unitStr, char *outStr);
void INTD_RECORD_MBBO(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *supStr, char *outStr);
void INTD_RECORD_MBBI(const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *supStr, char *outStr);
void INTD_RECORD_WFO( const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *pno, const char *dataType, const char *unitStr, char *outStr);
void INTD_RECORD_WFI( const char *datName, const char *subModName, const char *recName, const char *scanMethod, const char *pno, const char *dataType, const char *unitStr, char *outStr);
void INTD_RECORD_SO(  const char *datName, const char *subModName, const char *recName, const char *scanMethod, char *outStr);
void INTD_RECORD_SI(  const char *datName, const char *subModName, const char *recName, const char *scanMethod, char *outStr);

#ifdef __cplusplus
}
#endif

#endif

