#ifndef JOBS_H
#define JOBS_H

typedef void *Jobs;

#include "Pipeline.h"
#include <sys/types.h> // for pid_t

// Create a new empty Jobs collection
extern Jobs newJobs();
// Add a Pipeline to the Jobs collection
extern void addJobs(Jobs jobs, Pipeline pipeline);
// Return the number of Pipelines in the Jobs collection
extern int sizeJobs(Jobs jobs);
// Free the Jobs collection and all its Pipelines
extern void freeJobs(Jobs jobs);

// Set the process IDs for the jobs
extern void setJobPids(Jobs jobs, pid_t *pids, int num_pids);
// Print the list of jobs with their statuses
extern void printJobs(Jobs jobs);
// Bring a job to the foreground
extern void foregroundJob(Jobs jobs, int job_id);
// Send a job to the background
extern void backgroundJob(Jobs jobs, int job_id);
// Mark the most recent job as stopped
extern void markJobStopped(Jobs jobs);

#endif
