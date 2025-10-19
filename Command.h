/* 
 * File: Command.h
 * Description: Header file for command execution
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */

 #ifndef COMMAND_H
#define COMMAND_H

typedef void *Command;

#include "Tree.h"
#include "Jobs.h"
#include "Sequence.h"
#include <sys/types.h> // for pid_t

// Create a new Command
extern Command newCommand(T_words words, char *infile, char *outfile);

// Execute a Command
extern pid_t execCommand(Command command, Pipeline pipeline, Jobs jobs,
			int *jobbed, int *eof, int fg, int pipe_in, int pipe_out);

// Free a Command			
extern void freeCommand(Command command);
// Free state used by built-in commands
extern void freestateCommand();

#endif
