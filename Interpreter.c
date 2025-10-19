/* 
 * File: interpreter.c
 * Description: Implementation of interpreter.h
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Interpreter.h"
#include "Sequence.h"
#include "Pipeline.h"
#include "Command.h"

// This structure represents a command
typedef struct {
  char *file;
  char **argv;
  char *infile; 
  char *outfile;
  T_sequence block;
  int subshell;
} *CommandRep;

// Helper functions to interpret components of the parse tree
static Command i_command(T_command t);
static void i_pipeline(T_pipeline t, Pipeline pipeline);

// Interpret a command from the parse tree
static Command i_command(T_command t) {
  if (!t) // we check if the command is null
    return 0;
  Command command=0; // Initialize command to null
  // If there are words in the command
  // we create a new command with words, infile, and outfile
  if (t->words) 
    command=newCommand(t->words, t->infile, t->outfile);
  // If there are no words
  // we create a new command with null words, infile, and outfile
  else 
    command=newCommand(0, t->infile, t->outfile);

  // If the command was created successfully
  if (command){ 
    // We cast to CommandRep to set block and subshell
    CommandRep r=(CommandRep)command;
    r->block = t->block;
    r->subshell = t->subshell;
  }
  // we return the command
  return command;
}

// Interpret a pipeline from the parse tree
// Arguments:
//   t: T_pipeline parse tree node
//   pipeline: Pipeline object to populate
static void i_pipeline(T_pipeline t, Pipeline pipeline) {
  if (!t) 
    return;
  // We add the command to the pipeline
  addPipeline(pipeline,i_command(t->command));
  // We recursively interpret the next pipeline
  i_pipeline(t->pipeline,pipeline);
}

// Interpret a sequence from the parse tree
// Arguments:
//   t: T_sequence parse tree node
//   sequence: Sequence object to populate
void i_sequence(T_sequence t, Sequence sequence) {
  if (!t)
    return;
  int foreground = 1; // default to foreground
  // check if the last operator is "&"
  if (t->op && strcmp(t->op, "&") == 0) {
    foreground = 0; // background job
  }
  // Create a new pipeline with the foreground/background setting
  Pipeline pipeline=newPipeline(foreground);
  i_pipeline(t->pipeline,pipeline);
  addSequence(sequence,pipeline); // add the pipeline to the sequence
  // Recursively interpret the next sequence 
  i_sequence(t->sequence,sequence);
}

// Interpret the parse tree into executable objects
// Arguments:
//   t: The parse tree
//   eof: pointer to int indicating end-of-file
//   jobs: The jobs collection
extern void interpretTree(Tree t, int *eof, Jobs jobs) {
  if (!t)
    return;
  Sequence sequence=newSequence(); // create a new sequence
  i_sequence(t,sequence); // interpret the T_sequence into Sequence
  execSequence(sequence,jobs,eof); // execute the sequence
}
