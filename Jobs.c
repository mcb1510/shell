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

// Replace the printJobs function with this version:

extern void printJobs(Jobs jobs) {
  int i = 0;
  while (i < deq_len(jobs)) {
    Job job = deq_head_ith(jobs, i);
    
    if (job->pids == NULL) {
      printf("[%d] Running (no PIDs)\n", job->job_id);
      i++;
      continue;
    }
    
    // Check if all processes are done
    int all_done = 1;
    for (int j = 0; j < job->num_pids; j++) {
      int status;
      pid_t result = waitpid(job->pids[j], &status, WNOHANG);
      if (result == 0) {  // Still running
        all_done = 0;
        break;
      }
    }
    
    if (all_done) {
      // Remove finished job
      deq_head_rem(jobs, job);
      freeJob(job);
      // Don't increment i - we removed an element
    } else {
      // Print status
      if (job->stopped) {
        printf("[%d] Stopped\n", job->job_id);
      } else {
        printf("[%d] Running\n", job->job_id);
      }
      i++;
    }
  }
}
// // Print the list of jobs with their statuses
// extern void printJobs(Jobs jobs) {
//   // We go over all jobs and print their details
//   for (int i = 0; i < deq_len(jobs); i++) {
//     Job job = deq_head_ith(jobs,i); // Get the job at index i
//     // If the job has no PIDs, we print a message and continue
//     if (job->pids == NULL){
//       printf("[%d] Running (no PIDs)\n", job->job_id);
//       continue;
//     }
    
//     // We check the status of each PID
//     int still_running = 0; // Flag to check if any process is still running
//     for (int j = 0; j < job->num_pids; j++) {
//       int status; // we store the status of the process
//       // We use WNOHANG to avoid blocking if the process is still running
//       pid_t result = waitpid(job->pids[j], &status, WNOHANG);
//       if (result == 0) { //if the process is still running
//         still_running = 1; // we set the flag
//         break;
//       }
//     }
//     // We print the job status
//     if (still_running) {
//       if (job->stopped) {
//         printf("[%d] Stopped\n", job->job_id);
//       } else {
//         printf("[%d] Running\n", job->job_id);
//       }
//     }
//   }
// }

// Bring a job to the foreground
extern void foregroundJob(Jobs jobs, int job_id) {
  // We search for the job with the given job_id
  for (int i = 0; i < deq_len(jobs); i++) {
    Job job = deq_head_ith(jobs,i);
    if (job->job_id == job_id) {
      if (job->pids == NULL) {
        ERROR("Job has no PIDs");
        return;
      }
      
      // If stopped, send SIGCONT to resume it
      if (job->stopped) {
        for (int j = 0; j < job->num_pids; j++) {
          kill(job->pids[j], SIGCONT);
        }
        job->stopped = 0; // mark as running
      }
      
      // Wait for all PIDs with WUNTRACED to detect if stopped again
      for (int j = 0; j < job->num_pids; j++) {
        int status;
        waitpid(job->pids[j], &status, WUNTRACED);
        
        // Check if stopped again
        if (WIFSTOPPED(status)) {
          job->stopped = 1;
          printf("\n");
        }
      }
      return;
    }
  }
  fprintf(stderr, "fg: job %d not found\n", job_id);
}

// Send a job to the background
extern void backgroundJob(Jobs jobs, int job_id) {
  // We search for the job with the given job_id
  for (int i = 0; i < deq_len(jobs); i++) {
    Job job = deq_head_ith(jobs,i); // get the job at index i
    if (job->job_id == job_id) { // if we find the job
      if (job->pids == NULL) { // if the job has no PIDs we print an error
        ERROR("Job has no PIDs");
        return;
      }
      
      // Send SIGCONT to resume stopped job
      if (job->stopped) {
        for (int j = 0; j < job->num_pids; j++) {
          kill(job->pids[j], SIGCONT); // SIGCONT is used to continue a stopped process
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
  
  Job job = deq_tail_ith(jobs, 0);
  job->stopped = 1;
}

// Free the Jobs collection and all its Pipelines
static void freeJob(Job job) {
  if (job->pids) // we free the PIDs array if it exists
    free(job->pids);
  freePipeline(job->pipeline); // we free the associated pipeline
  free(job); // we free the job structure itself
}

// Free the Jobs collection and all its Pipelines
extern void freeJobs(Jobs jobs) {
  // We use deq_del to free each job using freeJob
  // DeqMapF will call freeJob for each job in the deque
  deq_del(jobs, (DeqMapF)freeJob);
}
