/* 
 * File: job.c
 * Description: Implementation of job.h
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */

 #include "Jobs.h"
#include "deq.h"
#include "error.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>

// We create a Job structure to hold information about each job
typedef struct {
  int job_id; // unique job ID
  pid_t *pids; // array of process IDs in the job
  int num_pids; // number of processes
  Pipeline pipeline; // the associated pipeline
  int stopped; // 1 if stopped, 0 if running
} *Job;

static int next_job_id = 1; // To assign unique job IDs

// free job declaration
static void freeJob(Job job);

// Create a new empty Jobs collection
extern Jobs newJobs() {
  //Deque to store jobs
  return deq_new();
}

// Add a Pipeline to the Jobs collection
// arguments:
//   jobs: The Jobs collection
//   pipeline: The Pipeline to add as a new job
extern void addJobs(Jobs jobs, Pipeline pipeline) {
  Job job=malloc(sizeof(*job));// we allocate memory for a new job
  if (!job)// check for malloc failure
    ERROR("malloc() failed");
  job->job_id=next_job_id++; // assign a unique job ID
  job->pids=NULL; // we start with no process IDs
  job->num_pids=0; // we start with zero processes
  job->pipeline=pipeline; // we associate the pipeline
  job->stopped=0; // job starts running, not stopped
  // Add the job to the jobs deque
  deq_tail_put(jobs,job);
}

// Return the number of Pipelines in the Jobs collection
extern int sizeJobs(Jobs jobs) {
  return deq_len(jobs);
}

// Set the process IDs for the jobs
// arguments:
//   jobs: The Jobs collection
//   pids: Array of process IDs to set
//   num_pids: Number of process IDs in the array
extern void setJobPids(Jobs jobs, pid_t *pids, int num_pids){
  if (deq_len(jobs) == 0)
    return; // No jobs to set PIDs for
  
  // Get the job it was added last
  Job job = deq_tail_ith(jobs,0); 
  
  // Allocate memory for the PIDs
  job->pids = malloc(sizeof(pid_t) * num_pids);
  if (!job->pids)
    ERROR("malloc() failed");

  // We copy the PIDs into the job structure
  for (int i = 0; i < num_pids; i++) {
    job->pids[i] = pids[i];
  }
  // We set the number of PIDs
  job->num_pids = num_pids;
}
// Print the list of jobs with their statuses
extern void printJobs(Jobs jobs) {
  int i = 0;
  // we go through each job in the jobs deque
  while (i < deq_len(jobs)) {
    // Get the job at index i
    Job job = deq_head_ith(jobs, i);
    // If the job has no PIDs, we print a message and continue
    if (job->pids == NULL) {
      printf("[%d] Running (no PIDs)\n", job->job_id);
      i++;
      continue;
    }
    
    // Check if all processes are done
    int all_done = 1;
    // We check each PID in the job
    for (int j = 0; j < job->num_pids; j++) {
      int status;
      // We use waitpid to check the status of each PID
      pid_t result = waitpid(job->pids[j], &status, WNOHANG);
      if (result == 0) {  // Still running
        all_done = 0;  // Mark as not done
        break; 
      }
    }
    
    // If all processes are done, we remove the job
    if (all_done) {
      // Remove finished job
      deq_head_rem(jobs, job);
      freeJob(job);
    } else {
      // Print status
      if (job->stopped) {
        // print stopped status
        printf("[%d] Stopped\n", job->job_id);
      } else {
        // print running status
        printf("[%d] Running\n", job->job_id);
      }
      // Move to next job
      i++;
    }
  }
}

// This function brings a job to the foreground
// arguments:
//   jobs: The Jobs collection
//   job_id: The ID of the job to bring to foreground
extern void foregroundJob(Jobs jobs, int job_id) {
  // We search for the job with the given job_id
  for (int i = 0; i < deq_len(jobs); i++) {
    Job job = deq_head_ith(jobs,i); // get the job at index i
    if (job->job_id == job_id) { // if we find the job
      if (job->pids == NULL) {
        ERROR("Job has no PIDs");
        return;
      }
      // If stopped, we send SIGCONT to resume it
      // SIGCONT helps us to continue a stopped process
      if (job->stopped) {
        // We resume each PID in the job
        for (int j = 0; j < job->num_pids; j++) {
          // We send the signal to the process
          kill(job->pids[j], SIGCONT); 
        }
        job->stopped = 0; // mark as running
      }
      // We wait for all processes in the job to finish
      for (int j = 0; j < job->num_pids; j++) {
        int status;
        // We wait for each PID
        // WUNTRACED allows us to detect if the process was stopped
        waitpid(job->pids[j], &status, WUNTRACED);
        
        // Check if stopped again
        // WIFSTOPPED returns true if the process is stopped
        if (WIFSTOPPED(status)) {
          job->stopped = 1;
          printf("\n");
        }
      }
      return;
    }
  }
  // If we don't find the job, we print an error message
  fprintf(stderr, "fg: job %d not found\n", job_id);
}

// Send a job to the background
// arguments:
//   jobs: The Jobs collection
//   job_id: The ID of the job to send to background
extern void backgroundJob(Jobs jobs, int job_id) {
  // We search for the job with the given job_id
  for (int i = 0; i < deq_len(jobs); i++) {
    Job job = deq_head_ith(jobs,i); // get the job at index i
    if (job->job_id == job_id) { // if we find the job
      if (job->pids == NULL) { // if the job has no PIDs we print an error
        ERROR("Job has no PIDs");
        return;
      }

      // We resume stopped job
      if (job->stopped) {
        for (int j = 0; j < job->num_pids; j++) {
          // SIGCONT is used to continue a stopped process
          kill(job->pids[j], SIGCONT); 
        }
        job->stopped = 0; // mark as running
      }
      return; // We return after sending the job to background
    }
  }
  // If we don't find the job, we print an error message
  fprintf(stderr, "bg: job %d not found\n", job_id);
}

// Mark the most recent job as stopped
extern void markJobStopped(Jobs jobs) {
  if (deq_len(jobs) == 0)
    return;

  // We get the most recent job
  Job job = deq_tail_ith(jobs, 0);
  // We mark it as stopped
  job->stopped = 1;
}

// This function frees the Jobs collection and all its Pipelines
static void freeJob(Job job) {
  if (job->pids) // we free the PIDs array if it exists
    free(job->pids);
  freePipeline(job->pipeline); // we free the associated pipeline
  free(job); // we free the job structure itself
}

// Free the Jobs collection and all its Pipelines
extern void freeJobs(Jobs jobs) {
  // We use deq_del to free each job using freeJob
  // We use deqMapF to cast freeJob to the correct function pointer type
  deq_del(jobs, (DeqMapF)freeJob);
}
