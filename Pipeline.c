/*
 * File: pipeline.c
 * Description: Implementation of pipeline.h
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "Pipeline.h"
#include "deq.h"
#include "error.h"

// Representation of a pipeline
typedef struct {
  Deq processes;
  int fg;
} *PipelineRep;

// This function creates a new pipeline
// with a specified foreground/background flag
extern Pipeline newPipeline(int fg) {
  PipelineRep r=(PipelineRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  // Initialize the deque to hold commands
  r->processes=deq_new();
  // Set foreground/background flag
  r->fg=fg;
  return r;
}

// This function adds a command to the end of the pipeline
// arguments:
//   pipeline - the pipeline to which the command is added
//   command - the command to add
extern void addPipeline(Pipeline pipeline, Command command) {
  PipelineRep r=(PipelineRep)pipeline;
  deq_tail_put(r->processes,command);
}

// This function returns the size of the pipeline
// arguments:
//   pipeline - the pipeline whose size is needed
extern int sizePipeline(Pipeline pipeline) {
  PipelineRep r=(PipelineRep)pipeline;
  return deq_len(r->processes);
}

// This function executes the pipeline
// arguments:
//   pipeline - the pipeline to execute
//   jobs - the jobs structure to manage background/foreground jobs
//   jobbed - pointer to jobbed flag
//   eof - pointer to EOF flag  
static void execute(Pipeline pipeline, Jobs jobs, int *jobbed, int *eof) {
  // Get pipeline representation and size
  PipelineRep r=(PipelineRep)pipeline; 
  int n = sizePipeline(r);

  // Single command case
  if (n == 1) {
    // Execute single command 
    pid_t pid = execCommand(deq_head_ith(r->processes,0),pipeline,jobs,jobbed,eof,r->fg,-1,-1);
    if (pid > 0){ // If a valid PID is returned we set it in jobs
      setJobPids(jobs, &pid, 1);
    }
    return;
  }

  // Multiple commands - pipeline
  int num_pipes = n - 1; // Number of pipes needed
  int pipes[num_pipes][2]; // Array to hold the pipes
  
  // Create the pipes
  for (int i = 0; i < num_pipes; i++) {
    if (pipe(pipes[i]) == -1) {
      ERROR("pipe() failed");
    }
  }

  pid_t pids[n]; // Array to hold child PIDs

  // Fork and execute each command
  for (int i = 0; i < n; i++) {
    // we get the command
    Command cmd = deq_head_ith(r->processes, i);
    // we determine pipe_in and pipe_out
    int pipe_in = (i > 0) ? pipes[i - 1][0] : -1;
    int pipe_out = (i < n - 1) ? pipes[i][1] : -1;

    // Fork the process
    pids[i] = fork();
    if (pids[i] == -1) {
      ERROR("fork() failed");
    }
    // Child process
    if (pids[i] == 0) {
      // Child - restore signals
      setpgid(0, 0);  // we create a new process group
      signal(SIGTSTP, SIG_DFL); // This allows child processes to be stopped
      signal(SIGINT, SIG_DFL); // This allows child processes to be interrupted 
      
      // Close unused pipes
      for (int j = 0; j < num_pipes; j++) {
        if (j != i-1) close(pipes[j][0]);
        if (j != i) close(pipes[j][1]);
      }
      // Execute the command
      execCommand(cmd, pipeline, jobs, jobbed, eof, r->fg, pipe_in, pipe_out);
      exit(0); // Exit child process
    }
    else {
      setpgid(pids[i], pids[0]);  // Set all children to the same process group
    }
  }

  // Parent - close all pipes
  for (int i = 0; i < num_pipes; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }

  // Add to jobs
  if (!*jobbed) { 
    *jobbed = 1; // mark as jobbed
    addJobs(jobs, pipeline); // add pipeline to jobs
  }

  // Set job PIDs
  setJobPids(jobs, pids, n); 

  // Wait if foreground
  if (r->fg) {
    int stopped = 0;
    // We wait for all child processes
    for (int i = 0; i < n; i++) {
      int status;
      waitpid(pids[i], &status, WUNTRACED);
      // Check if the process was stopped
      if (WIFSTOPPED(status)) {
        stopped = 1;
        break;
      }
    }
    // if pipeline was stopped we mark job as stopped
    if (stopped) {
      markJobStopped(jobs);
      printf("\n");
    }
  }
}

// This function executes the pipeline
// arguments:
//   pipeline - the pipeline to execute
//   jobs - the jobs structure to manage background/foreground jobs
//   eof - pointer to EOF flag
extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof) {
  int jobbed=0; 
  execute(pipeline,jobs,&jobbed,eof); // execute the pipeline
  if (!jobbed) // if not jobbed, free the pipeline
    freePipeline(pipeline);
}

// This function frees the resources of a pipeline
extern void freePipeline(Pipeline pipeline) {
  // Free each command in the pipeline and the pipeline itself
  PipelineRep r=(PipelineRep)pipeline; 
  deq_del(r->processes,freeCommand); // free the deque and its commands
  free(r);
}
