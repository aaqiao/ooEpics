//=========================================================
// ooEpicsMisc.cc
//
// Some common tools
//
// Created by: Zheqiao Geng, gengzq@slac.stanford.edu
// Created on: Nov. 07, 2010
//=========================================================
#include "ooEpicsMisc.h"

#include "stdio.h"
#include "time.h"

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// replate the char of "src" to "des"
//-----------------------------------------------
void stringReplace(char *str, char src, char des)
{    
    char *p;
    while((p = strchr(str, src))) *p = des;
}

//-----------------------------------------------
// get time string as 1/6, 15:06:32
//-----------------------------------------------
void getTimeString(char *timeStr, int withYear, const char *formatStr)
{
    time_t rawTime;
    tm *ptm;

    time(&rawTime);
    ptm = localtime(&rawTime);
    
    if(withYear)
        sprintf(timeStr, formatStr, ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    else
        sprintf(timeStr, formatStr, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

//-----------------------------------------------
// erase the character in the string
//-----------------------------------------------
void eraseChar(string *str, const char *c)
{
    int charPos = -1;
    
    while((charPos = str -> find(c)) != (int)string::npos) {    
        str -> erase(charPos, 1);
    }  	
}

//-----------------------------------------------
// replace the character of c1 with c2 in the string
//-----------------------------------------------
void replaceChar(string *str, const char *c1, const char *c2)
{
    int charPos = -1;
    
    while((charPos = str -> find(c1)) != (int)string::npos) {    
        str -> replace(charPos, 1, c2);
    }  		
}
        
//-----------------------------------------------
// to upper case
//-----------------------------------------------        
void stringToUpper(string *str)
{
    int i;
    char lowChar[30] = "abcdefghijklmnopqrstuvwxyz";
    char capChar[30] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    char t1[2];
    char t2[2];
    
    for(i = 0; i < 26; i ++) {    
        t1[0] = lowChar[i];
        t1[1] = '\0';
        t2[0] = capChar[i];
        t2[1] = '\0';
        
        replaceChar(str, t1, t2);    	
    }            	
}

//-----------------------------------------------
// another way to get the system time
//-----------------------------------------------        
char *getSystemTime()
{
    time_t timer;
    timer = time(0);
    return asctime(localtime(&timer));
}

//-----------------------------------------------
// get the PV time (copied from the seq module "pv.cc")
//-----------------------------------------------
int pvTimeGetCurrentDouble(double *pTime) 
{
    epicsTimeStamp stamp;

    *pTime = 0.0;
    if(epicsTimeGetCurrent(&stamp) == epicsTimeERROR)
	return -1;

    *pTime = (double)stamp.secPastEpoch + ((double)stamp.nsec / 1e9);
    return 0;
}

//-----------------------------------------------
// calculate the remained timeout
//-----------------------------------------------
int calcRemainedTimeOut(double *remainedTimeOut, double startTime, double timeOut)
{
    int status;
    double nowTime, remainTime;

    // init the output
    *remainedTimeOut = 0.0;

    // get the current time
    status = pvTimeGetCurrentDouble(&nowTime);
    if(status != 0)
        return status;

    // calculate the remained timeout
    remainTime = timeOut - (nowTime - startTime);
    if(remainTime > 0)
        *remainedTimeOut = remainTime;

    return status;
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

