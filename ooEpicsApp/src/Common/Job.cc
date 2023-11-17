//===============================================================
//  Copyright (c) 2023 by Paul Scherrer Institute, Switzerland
//  All rights reserved.
//  Authors: Zheqiao Geng
//===============================================================
//===============================================================
// Job.cc
//
// Basic class for a job for applications. This corresponding to a procedure
//===============================================================
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
    strcpy(modName, modNameIn);
    strcpy(jobName, jobNameIn);

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




