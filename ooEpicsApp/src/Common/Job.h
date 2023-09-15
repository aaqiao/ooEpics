//=========================================================
// Job.h
//
// Basic class for a job for applications. This corresponding to a procedure
//
// Created by Zheqiao Geng (zheqiao.geng@psi.ch) on 2014.05.22
//=========================================================
#ifndef JOB_H
#define JOB_H
#include <iostream>
#include <fstream>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

#define JOB_MAX_NUM         256
#define JOB_STRING_LEN      256

//-----------------------------------------------
// base class definition for the Job
//-----------------------------------------------
class Job
{
public:
    Job(const char *modNameIn, const char *jobNameIn);
    virtual ~Job();

    void enableJob(short enabled);                          // enable or disable the job

    virtual void enableJobExt()    = 0;                     // extend the enableJob function by user
    virtual int  execute(int flag) = 0;                     // execute the job

    char modName[JOB_STRING_LEN];
    char jobName[JOB_STRING_LEN];   

    volatile short jobEnabled;                              // 1 - the job is enabled; 0 -disabled
};

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************

#endif

