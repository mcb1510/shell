#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "Command.h"
#include "error.h"
#include <fcntl.h> // for open()
#include <readline/history.h>
#include <signal.h>
#include "Interpreter.h"
#include "Sequence.h"

typedef struct {
  char *file;
  char **argv;
  char *infile; 
  char *outfile;
  T_sequence block;
  int subshell;
} *CommandRep;

#define BIARGS CommandRep r, int *eof, Jobs jobs
#define BINAME(name) bi_##name
#define BIDEFN(name) static void BINAME(name) (BIARGS)
#define BIENTRY(name) {#name,BINAME(name)}

static char *owd=0;
static char *cwd=0;

static void builtin_args(CommandRep r, int n) {
  if (!r->argv) {
    ERROR("NULL argv in builtin command");
    return;
  }
  char **argv=r->argv;
  for (n++; *argv++; n--);
  if (n)
    ERROR("wrong number of arguments to builtin command"); // warn
}

BIDEFN(exit) {
  builtin_args(r,0);
  *eof=1;
}

BIDEFN(pwd) {
  builtin_args(r,0);
  if (!cwd)
    cwd=getcwd(0,0);
  printf("%s\n",cwd);
}

BIDEFN(cd) {
  builtin_args(r,1);
  if (strcmp(r->argv[1],"-")==0) {
    char *twd=cwd;
    cwd=owd;
    owd=twd;
  } else {
    if (owd) free(owd);
    owd=cwd;
    cwd=strdup(r->argv[1]);
  }
  if (cwd && chdir(cwd))
    ERROR("chdir() failed"); // warn
}

BIDEFN(history) {
  builtin_args(r,0);
  HIST_ENTRY **hist = history_list();
  if (hist) {
    for (int i = 0; hist[i]; i++)
      printf("%5d: %s\n", i + history_base, hist[i]->line);
  }
}

BIDEFN(jobs) {
  builtin_args(r,0);
  printJobs(jobs);
}

BIDEFN(fg) {
  builtin_args(r,1);
  int job_id = atoi(r->argv[1]);
  foregroundJob(jobs, job_id);
}

BIDEFN(bg) {
  builtin_args(r,1);
  int job_id = atoi(r->argv[1]);
  backgroundJob(jobs, job_id);
}

static int builtin(BIARGS) {
  typedef struct {
    char *s;
    void (*f)(BIARGS);
  } Builtin;
  static const Builtin builtins[]={
    BIENTRY(exit),
    BIENTRY(pwd),
    BIENTRY(cd),
    BIENTRY(history),
    BIENTRY(jobs),
    BIENTRY(fg),
    BIENTRY(bg),
    {0,0}
  };
  if (!r->file)
    return 0;
  int i;
  for (i=0; builtins[i].s; i++)
    if (!strcmp(r->file,builtins[i].s)) {
      builtins[i].f(r,eof,jobs);
      return 1;
    }
  return 0;
}

static char **getargs(T_words words) {
  int n=0;
  T_words p=words;
  while (p) {
    p=p->words;
    n++;
  }
  char **argv=(char **)malloc(sizeof(char *)*(n+1));
  if (!argv)
    ERROR("malloc() failed");
  p=words;
  int i=0;
  while (p) {
    argv[i++]=strdup(p->word->s);
    p=p->words;
  }
  argv[i]=0;
  return argv;
}

extern Command newCommand(T_words words, char *infile, char *outfile) {
  CommandRep r=(CommandRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");

  if (words){
  r->argv=getargs(words);
  r->file=r->argv[0];
  }
  else{
    r->argv=NULL;
    r->file=NULL;
  }
  // if infile or outfile is null, set to 0
  // else strdup it
  r->infile=infile ? strdup(infile) : 0;
  r->outfile=outfile ? strdup(outfile) : 0;
  r->block = 0;
  r->subshell = -1; // -1 means not a subshell or compound
  return r;
}

// This function is called in the child process after a fork()
// It handles input/output redirection and executes the command

static void child(CommandRep r, int fg, int pipe_in, int pipe_out) {
  // Restore default signal handlers in child
  signal(SIGTSTP, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  
  int eof=0;
  Jobs jobs=newJobs();

  // Handle pipe input
  if (pipe_in != -1) {
    dup2(pipe_in, STDIN_FILENO);
    close(pipe_in);
  }

  // Handle pipe output
  if (pipe_out != -1) {
    dup2(pipe_out, STDOUT_FILENO);
    close(pipe_out);
  }

  // Handle input redirection (<)
  if (r->infile) {
    int fd = open(r->infile, O_RDONLY);
    if (fd < 0) {
      ERROR("Failed to open input file");
    }
    dup2(fd, STDIN_FILENO);
    close(fd);
  }

  // Handle output redirection (>)
  if (r->outfile) {
    int fd = open(r->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
      ERROR("Failed to open output file");
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
  }

  if (builtin(r,&eof,jobs))
    //return;
    exit(0);
  if (!r->argv || !r->argv[0]) {
    ERROR("NULL command in execvp");
    exit(1);
  }
  execvp(r->argv[0],r->argv);
  ERROR("execvp() failed");
  exit(0);
}

extern pid_t execCommand(Command command, Pipeline pipeline, Jobs jobs,
			int *jobbed, int *eof, int fg, int pipe_in, int pipe_out) {
  CommandRep r=command;
 // ADD ALL THIS CODE HERE ↓↓↓
  
  // Handle block commands ( ) or { }
  if (r->block) {
    if (r->subshell == 0) {
      // { } - no fork, just execute in current process
      Sequence seq=newSequence();
      i_sequence(r->block, seq);
      execSequence(seq, jobs, eof);
      // Don't free seq here - execSequence already frees it
      return 0;
    } 
    else if (r->subshell == 1) {
      // ( ) - fork subshell
      if (!*jobbed) {
        *jobbed=1;
        addJobs(jobs,pipeline);
      }
      
      int pid=fork();
      if (pid==-1)
        ERROR("fork() failed");
      
      if (pid==0) {
        // Child process
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        
        if (pipe_in != -1) {
          dup2(pipe_in, STDIN_FILENO);
          close(pipe_in);
        }
        if (pipe_out != -1) {
          dup2(pipe_out, STDOUT_FILENO);
          close(pipe_out);
        }
        
        Sequence seq=newSequence();
        i_sequence(r->block, seq);
        execSequence(seq, jobs, eof);
        // Don't free seq here - execSequence already frees it
        exit(0);
      }
      
      // Parent process
      if (fg) {
        int status;
        waitpid(pid, &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
          markJobStopped(jobs);
          printf("\n");
        }
      }
      return pid;
    }
  }
  
  // ↑↑↑ END OF ADDED CODE


  if (fg && pipe_in == -1 && pipe_out == -1 && builtin(r,eof,jobs)){
    fflush(stdout);
    return 0;
  }
  if (!*jobbed) {
    *jobbed=1;
    addJobs(jobs,pipeline);
  }
  int pid=fork();
  if (pid==-1)
    ERROR("fork() failed");
  if (pid==0)
    child(r,fg, pipe_in, pipe_out);
  else {
    if (fg) {
      int status;
      waitpid(pid, &status, WUNTRACED);  // Add WUNTRACED flag
      
      // Check if stopped
      if (WIFSTOPPED(status)) {
        markJobStopped(jobs);  // Mark the job as stopped
        printf("\n");
      }
      // If terminated by signal (like SIGINT from ^C), don't mark as stopped
    }
    return pid;
  }
  return 0;
}

extern void freeCommand(Command command) {
  CommandRep r=command;
  if (r->argv) {
    char **argv=r->argv;
    while (*argv)
      free(*argv++);
    free(r->argv);
  }
  if (r->infile) free(r->infile); // free infile if it was allocated
  if (r->outfile) free(r->outfile); // free outfile if it was allocated
  // if (r->block)
  //   freeSequence(r->block);
  free(r);
  
}

extern void freestateCommand() {
  if (cwd) free(cwd);
  if (owd) free(owd);
}
