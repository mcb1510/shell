/* 
 * File: Command.c
 * Description: Implementation of Command.h
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */

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

// This structure represents a command
typedef struct {
  char *file;
  char **argv;
  char *infile; 
  char *outfile;
  T_sequence block;
  int subshell;
} *CommandRep;

// Macros to define built-in commands
#define BIARGS CommandRep r, int *eof, Jobs jobs
#define BINAME(name) bi_##name
#define BIDEFN(name) static void BINAME(name) (BIARGS)
#define BIENTRY(name) {#name,BINAME(name)}

static char *owd=0;
static char *cwd=0;

// Validate the number of arguments for built-in commands
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

// Exit the shell
BIDEFN(exit) {
  builtin_args(r,0);
  *eof=1;
}

// Print the current working directory
BIDEFN(pwd) {
  builtin_args(r,0);
  if (!cwd)
    cwd=getcwd(0,0);
  printf("%s\n",cwd);
}

// Change the current working directory
BIDEFN(cd) {
  builtin_args(r,1);
  // change to old working directory
  if (strcmp(r->argv[1],"-")==0) { 
    char *twd=cwd;
    cwd=owd;
    owd=twd;
  } else { // change to specified directory
    if (owd) free(owd);
    owd=cwd;
    cwd=strdup(r->argv[1]);
  }
  if (cwd && chdir(cwd))
    ERROR("chdir() failed"); // warning
}

// Print command history
BIDEFN(history) {
  builtin_args(r,0);
  // Print the history list
  HIST_ENTRY **hist = history_list();
  if (hist) { // if history exists
    for (int i = 0; hist[i]; i++)
      printf("%5d: %s\n", i + history_base, hist[i]->line);
  }
}

// Print the list of jobs
BIDEFN(jobs) {
  builtin_args(r,0); // Validate arguments
  printJobs(jobs); // Call the printJobs function
}

// Bring a job to the foreground
BIDEFN(fg) {
  builtin_args(r,1); // Validate arguments
  int job_id = atoi(r->argv[1]); // Convert job ID from string to integer
  foregroundJob(jobs, job_id); // Call the foregroundJob function
}
// Send a job to the background
BIDEFN(bg) { 
  builtin_args(r,1); // Validate arguments
  int job_id = atoi(r->argv[1]); // Convert job ID from string to integer
  backgroundJob(jobs, job_id);// Call the backgroundJob function
}

// Check and execute built-in commands
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

// Convert T_words to argv array
static char **getargs(T_words words) {
  int n=0; 
  T_words p=words; // count words
  while (p) { // while there are words
    p=p->words; // move to next word
    n++; // increment count
  }
  // Allocate argv array
  char **argv=(char **)malloc(sizeof(char *)*(n+1));
  if (!argv)
    ERROR("malloc() failed");
  p=words; // reset p to the start of the list
  int i=0;
  while (p) {
    argv[i++]=strdup(p->word->s); // duplicate the string
    p=p->words; // move to next word
  }
  argv[i]=0; //
  return argv; // Return the argv array
}

// Create a new Command
// args: words: T_words representing command and arguments
//       infile: input redirection file (or NULL)
//       outfile: output redirection file (or NULL)
extern Command newCommand(T_words words, char *infile, char *outfile) {
  CommandRep r=(CommandRep)malloc(sizeof(*r)); //allocate memory for CommandRep
  if (!r)
    ERROR("malloc() failed");

  if (words){ // if words is not null
    r->argv=getargs(words); // convert T_words to argv array
    r->file=r->argv[0]; // first argument is the command name
  }
  else{ // if words is null
    r->argv=NULL;
    r->file=NULL;
  }
  // if infile or outfile is null, set to 0
  // else strdup it
  r->infile=infile ? strdup(infile) : 0; 
  r->outfile=outfile ? strdup(outfile) : 0;
  r->block = 0; 
  r->subshell = -1; // -1 = not a subshell or compound
  return r; // return the new Command
}

// This function handles the execution of a command in a child process
// It sets up input/output redirection and executes the command
//  arguments:
//   r: CommandRep representing the command to execute
//   fg: int indicating if the command is in the foreground (1) or background (0)
//   pipe_in: file descriptor for input pipe 
//   pipe_out: file descriptor for output pipe
static void child(CommandRep r, int fg, int pipe_in, int pipe_out) {
  // Restore default signal handlers in child
  signal(SIGTSTP, SIG_DFL); // Restore default handler for SIGTSTP which is Ctrl+Z
  signal(SIGINT, SIG_DFL); // Restore default handler for SIGINT which is Ctrl+C
  
  int eof=0; // Initialize eof variable
  Jobs jobs=newJobs(); // Create a new Jobs collection

  // Handle pipe input
  if (pipe_in != -1) { // If there is a pipe input
    dup2(pipe_in, STDIN_FILENO); // Redirect standard input to pipe input
    close(pipe_in); // We close the original pipe input descriptor
  }

  // Handle pipe output
  if (pipe_out != -1) { // If there is a pipe output
    dup2(pipe_out, STDOUT_FILENO); // Redirect standard output to pipe output
    close(pipe_out); // We close the original pipe output descriptor
  }

  // Handle input redirection (<)
  if (r->infile) { // If there is an input file
    int fd = open(r->infile, O_RDONLY); // Open the input file for reading
    if (fd < 0) { 
      ERROR("Failed to open input file");
    }
    dup2(fd, STDIN_FILENO); // Redirect standard input to the input file
    close(fd); // Close the original file descriptor
  }

  // Handle output redirection (>)
  if (r->outfile) { // If there is an output file
    // Open the output file for writing
    int fd = open(r->outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) { 
      ERROR("Failed to open output file");
    }
    dup2(fd, STDOUT_FILENO); // Redirect standard output to the output file
    close(fd); // we close the original file descriptor
  }
  // Handle built-in commands
  if (builtin(r,&eof,jobs)) // If the command is a built-in
    //return;
    exit(0); // Exit child process after executing built-in command
  if (!r->argv || !r->argv[0]) { // Check if command is NULL
    ERROR("NULL command in execvp");
    exit(1); // Exit with error
  }
  execvp(r->argv[0],r->argv); // Execute the command
  ERROR("execvp() failed"); 
  exit(0);
}

// Execute a command
// Arguments:
//   command: The command to execute
//   pipeline: The pipeline the command belongs to
//   jobs: The jobs collection
//   jobbed: pointer to int indicating if the job has been added to jobs
//   eof: pointer to int indicating end-of-file
//   fg: int indicating if the command is in the foreground (1) or background (0)
//   pipe_in: file descriptor for input pipe
//   pipe_out: file descriptor for output pipe
extern pid_t execCommand(Command command, Pipeline pipeline, Jobs jobs,
			int *jobbed, int *eof, int fg, int pipe_in, int pipe_out) {
  CommandRep r=command; // cast to CommandRep  
  // Handle block commands either ( ) or { }
  if (r->block) {
    if (r->subshell == 0) {  // { } - no subshell
      // Execute block in current process
      Sequence seq=newSequence(); // create new Sequence
      i_sequence(r->block, seq); // interpret T_sequence into Sequence
      execSequence(seq, jobs, eof); // execute the sequence
      return 0;
    } 
    else if (r->subshell == 1) { // ( ) - fork subshell
      if (!*jobbed) { // if job not yet added
        *jobbed=1; // mark as added
        addJobs(jobs,pipeline); // add pipeline to jobs
      }
      
      // Fork a new process for the subshell
      int pid=fork(); 
      if (pid==-1) 
        ERROR("fork() failed");
      
      // Child process
      if (pid==0) {
        signal(SIGTSTP, SIG_DFL); // Restore default handler for SIGTSTP for Ctrl+Z
        signal(SIGINT, SIG_DFL); // Restore default handler for SIGINT for Ctrl+C
        
        // Handle pipe input
        if (pipe_in != -1) { // If there is a pipe input
          dup2(pipe_in, STDIN_FILENO); // Redirect standard input to pipe input
          close(pipe_in); // We close original pipe input descriptor
        }
        if (pipe_out != -1) { // If there is a pipe output
          dup2(pipe_out, STDOUT_FILENO); // Redirect standard output to pipe output
          close(pipe_out); // we close original pipe output descriptor
        }
        
        // Execute the block in the subshell
        Sequence seq=newSequence(); // create new Sequence
        i_sequence(r->block, seq); // interpret T_sequence into Sequence
        execSequence(seq, jobs, eof); // execute the sequence
        exit(0);
      }
      
      // Parent process
      if (fg) { // If foreground
        int status; // we store the status of the process
        // Add WUNTRACED flag for waitpid
        // We use WUNTRACED to detect if the process is stopped
        waitpid(pid, &status, WUNTRACED); 
        // Check if stopped
        if (WIFSTOPPED(status)) {
          markJobStopped(jobs); // mark the job as stopped
          printf("\n");
        }
      }
      return pid; //we return the pid of the subshell
    }
  }

  // Handle non-block commands
  // if foreground and no pipes and built-in command
  if (fg && pipe_in == -1 && pipe_out == -1 && builtin(r,eof,jobs)){
    fflush(stdout); // flush stdout for correct output order
    return 0;
  }
  // For other commands
  if (!*jobbed) { // if job not yet added
    *jobbed=1; // mark as added
    addJobs(jobs,pipeline); // add pipeline to jobs
  }
  // Fork a new process to execute the command
  int pid=fork(); // create a new process
  if (pid==-1)
    ERROR("fork() failed");
  // Child process
  if (pid==0)
    child(r,fg, pipe_in, pipe_out); // execute the command in child 
  else { // Parent process
    if (fg) { // If foreground
      int status;// we store the status of the process
      waitpid(pid, &status, WUNTRACED);  // Add WUNTRACED flag for waitpid
      
      // Check if stopped
      if (WIFSTOPPED(status)) {
        markJobStopped(jobs);  // Mark the job as stopped
        printf("\n");
      }
    }
    return pid; // return the pid of the command
  }
  return 0; 
}

// Free a Command
// Arguments:
//   command: The command to free
extern void freeCommand(Command command) {
  CommandRep r=command; // cast to CommandRep
  if (r->argv) { // if argv is not null
    char **argv=r->argv; // temporary pointer to argv for freeing
    while (*argv) 
      free(*argv++); // we free each argument string
    free(r->argv);// we free the argv array
  }
  if (r->infile) free(r->infile); // free infile if it was allocated
  if (r->outfile) free(r->outfile); // free outfile if it was allocated
  free(r); // free the CommandRep structure
  
}

// This function frees state used by built-in commands
extern void freestateCommand() {
  if (cwd) free(cwd); // free current working directory
  if (owd) free(owd); // free old working directory
}
