#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Interpreter.h"
#include "Sequence.h"
#include "Pipeline.h"
#include "Command.h"

typedef struct {
  char *file;
  char **argv;
  char *infile; 
  char *outfile;
  T_sequence block;
  int subshell;
} *CommandRep;

static Command i_command(T_command t);
static void i_pipeline(T_pipeline t, Pipeline pipeline);
//static void i_sequence(T_sequence t, Sequence sequence);


static Command i_command(T_command t) {
  if (!t)
    return 0;
  Command command=0;
  if (t->words)
    command=newCommand(t->words, t->infile, t->outfile);
  else
    command=newCommand(0, t->infile, t->outfile);

  if (command){
    CommandRep r=(CommandRep)command;
    r->block = t->block;
    r->subshell = t->subshell;
  }
  return command;
}

static void i_pipeline(T_pipeline t, Pipeline pipeline) {
  if (!t)
    return;
  addPipeline(pipeline,i_command(t->command));
  i_pipeline(t->pipeline,pipeline);
}

void i_sequence(T_sequence t, Sequence sequence) {
  if (!t)
    return;
  
  int foreground = 1; // default to foreground
  // check if the last operator is "&"
  if (t->op && strcmp(t->op, "&") == 0) {
    foreground = 0; // background job
  }
  Pipeline pipeline=newPipeline(foreground);
  i_pipeline(t->pipeline,pipeline);
  addSequence(sequence,pipeline);
  i_sequence(t->sequence,sequence);
}

extern void interpretTree(Tree t, int *eof, Jobs jobs) {
  if (!t)
    return;
  Sequence sequence=newSequence();
  i_sequence(t,sequence);
  execSequence(sequence,jobs,eof);
}
