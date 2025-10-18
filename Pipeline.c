#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "Pipeline.h"
#include "deq.h"
#include "error.h"

typedef struct {
  Deq processes;
  int fg;
} *PipelineRep;

extern Pipeline newPipeline(int fg) {
  PipelineRep r=(PipelineRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  r->processes=deq_new();
  r->fg=fg;
  return r;
}

extern void addPipeline(Pipeline pipeline, Command command) {
  PipelineRep r=(PipelineRep)pipeline;
  deq_tail_put(r->processes,command);
}

extern int sizePipeline(Pipeline pipeline) {
  PipelineRep r=(PipelineRep)pipeline;
  return deq_len(r->processes);
}

static void execute(Pipeline pipeline, Jobs jobs, int *jobbed, int *eof) {
  PipelineRep r=(PipelineRep)pipeline;
  int n = sizePipeline(r);

  // Single command case
  if (n == 1) {
    pid_t pid = execCommand(deq_head_ith(r->processes,0),pipeline,jobs,jobbed,eof,r->fg,-1,-1);
    if (pid > 0){
      setJobPids(jobs, &pid, 1);
    }
    return;
  }

  // Multiple commands - pipeline
  int num_pipes = n - 1;
  int pipes[num_pipes][2];
  
  for (int i = 0; i < num_pipes; i++) {
    if (pipe(pipes[i]) == -1) {
      ERROR("pipe() failed");
    }
  }

  pid_t pids[n];

  for (int i = 0; i < n; i++) {
    Command cmd = deq_head_ith(r->processes, i);

    int pipe_in = (i > 0) ? pipes[i - 1][0] : -1;
    int pipe_out = (i < n - 1) ? pipes[i][1] : -1;

    pids[i] = fork();
    if (pids[i] == -1) {
      ERROR("fork() failed");
    }

    if (pids[i] == 0) {
      // Child - restore signals
      signal(SIGTSTP, SIG_DFL);
      signal(SIGINT, SIG_DFL);
      
      // Close unused pipes
      for (int j = 0; j < num_pipes; j++) {
        if (j != i-1) close(pipes[j][0]);
        if (j != i) close(pipes[j][1]);
      }

      execCommand(cmd, pipeline, jobs, jobbed, eof, r->fg, pipe_in, pipe_out);
      exit(0);
    }
  }

  // Parent - close all pipes
  for (int i = 0; i < num_pipes; i++) {
    close(pipes[i][0]);
    close(pipes[i][1]);
  }

  // Add to jobs
  if (!*jobbed) {
    *jobbed = 1;
    addJobs(jobs, pipeline);
  }

  // Always set PIDs
  setJobPids(jobs, pids, n);

  // Wait if foreground
  if (r->fg) {
    int stopped = 0;
    for (int i = 0; i < n; i++) {
      int status;
      waitpid(pids[i], &status, WUNTRACED);
      if (WIFSTOPPED(status)) {
        stopped = 1;
        break;
      }
    }
    if (stopped) {
      markJobStopped(jobs);
      printf("\n");
    }
  }
}

extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof) {
  int jobbed=0;
  execute(pipeline,jobs,&jobbed,eof);
  if (!jobbed)
    freePipeline(pipeline);
}

extern void freePipeline(Pipeline pipeline) {
  PipelineRep r=(PipelineRep)pipeline;
  deq_del(r->processes,freeCommand);
  free(r);
}
