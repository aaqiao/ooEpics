//=========================================================
// Job.cc
//
// Basic class for a job for applications. This corresponding to a procedure
//
// Created by Zheqiao Geng (zheqiao.geng@psi.ch) on 2014.05.22
//=========================================================
#include "Job.h"

using namespace std;

//******************************************************
// NAME SPACE OOEPICS
//******************************************************
namespace OOEPICS {

//-----------------------------------------------
// construction and distruction for Job
//-----------------------------------------------
Job::Job(const char *modNameIn, const char *jobNameIn)
{
    // remember the input
    strncpy(modName, modNameIn, JOB_STRING_LEN);
    strncpy(jobName, jobNameIn, JOB_STRING_LEN);

    cout << "INFO: Job::Job: Object " << jobName << " for module " << modName << " created." << endl;
}

Job::~Job() 
{
    cout << "INFO: Job::~Job: Object " << jobName << " for module " << modName << " deleted!" << endl;
}

//-----------------------------------------------
// implement the local functions
//-----------------------------------------------
void Job::enableJob(short enabled) 
{
    jobEnabled = enabled;
    enableJobExt();
}

}
//******************************************************
// NAME SPACE OOEPICS
//******************************************************




