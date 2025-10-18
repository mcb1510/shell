#ifndef COMMAND_H
#define COMMAND_H

typedef void *Command;

#include "Tree.h"
#include "Jobs.h"
#include "Sequence.h"
#include <sys/types.h> // for pid_t

extern Command newCommand(T_words words, char *infile, char *outfile);

extern pid_t execCommand(Command command, Pipeline pipeline, Jobs jobs,
			int *jobbed, int *eof, int fg, int pipe_in, int pipe_out);

extern void freeCommand(Command command);
extern void freestateCommand();

#endif
