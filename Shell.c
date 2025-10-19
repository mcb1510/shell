/*
 * File: Shell.c
 * Description: Main shell program
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include "Jobs.h"
#include "Parser.h"
#include "Interpreter.h"
#include "error.h"

//clean up finished background jobs
void sigchld_handler(int sig) {
  // wait for all dead processes
  while (waitpid(-1, NULL, WNOHANG) > 0);
}

//setup signal handlers
void setup_signals() {
  signal(SIGTSTP, SIG_IGN);  // Shell ignores ^Z
  signal(SIGINT, SIG_IGN);   // Shell ignores ^C
  signal(SIGCHLD, sigchld_handler);  // clean up zombies processes
}

// Main shell loop
int main() {
  setup_signals();  // Setup signal handlers
  int eof=0;// end-of-file flag
  Jobs jobs=newJobs();// Create jobs structure
  char *prompt=0;// prompt string

  // Setup readline 
  if (isatty(fileno(stdin))) {
    using_history(); // enable history
    read_history(".history"); // read history from file
    prompt="$ "; // set prompt
  } else { // non-interactive mode
    rl_bind_key('\t',rl_insert); // This disable tab completion
    rl_outstream=fopen("/dev/null","w"); // disable output
  }
  
  // Main loop
  while (!eof) {
    char *line=readline(prompt); // read a line
    if (!line)
      break;
    if (*line) // if line is not empty
      add_history(line); // add to history
    Tree tree=parseTree(line); // parse the line
    free(line); // free the line
    interpretTree(tree,&eof,jobs); // interpret the parse tree
    freeTree(tree); // free the parse tree
  }

  // Cleanup before exiting
  if (isatty(fileno(stdin))) { // interactive mode
    write_history(".history"); // write history to file
    rl_clear_history(); // clear history
  } else {
    fclose(rl_outstream); // close disabled output
  }
  freestateCommand(); // free command state
  freeJobs(jobs);  // Free jobs before exiting
  return 0;
}
