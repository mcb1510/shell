/*
 * File: parser.c
 * Description: Implementation of parser.h
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"
#include "Tree.h"
#include "Scanner.h"
#include "error.h"

static Scanner scan;

#undef ERROR
#define ERROR(s) ERRORLOC(__FILE__,__LINE__,"error","%s (pos: %d)",s,posScanner(scan))

static char *next()       { return nextScanner(scan); } 
static char *curr()       { return currScanner(scan); } 
static int   cmp(char *s) { return cmpScanner(scan,s); } 
static int   eat(char *s) { return eatScanner(scan,s); }

static T_word p_word();
static T_words p_words();
static T_command p_command();
static T_pipeline p_pipeline();
static T_sequence p_sequence();

// This function parses a single word from the input
static T_word p_word() {
  char *s=curr(); 
  if (!s)
    return 0;
  T_word word=new_word();
  word->s=strdup(s);
  next();
  return word;
}

// This function parses a list of words until it hits a special token
static T_words p_words() {
  T_word word=p_word(); 
  if (!word)
    return 0;
  T_words words=new_words();
  words->word=word;
  // Stop parsing words when we hit a special token
  if (cmp("|") || cmp("&") || cmp(";") || cmp("<") || cmp(">") ||
      cmp("{") || cmp("}") || cmp("(") || cmp(")"))
    return words;
  words->words=p_words();
  return words;
}

// handle input/output redirection
static void p_redir(T_command command) {
  if (eat("<")) { // input redirection
    char *s=curr(); // get current token, should be filename
    if (!s) // if no token, error
      ERROR("expected filename after <");
    // We copy the filename into the command structure
    // strdup allocates memory for the string
    command->infile=strdup(s); 
    next(); // move to next token
  }
  if (eat(">")) { // output redirection
    char *s=curr(); // get current token, should be filename
    if (!s) // if no token, error
      ERROR("expected filename after >");
    // We copy the filename into the command structure
    // strdup allocates memory for the string
    command->outfile=strdup(s);
    next(); // move to next token
  }
}

// This function parses a command, which can be a simple command or a block
static T_command p_command() {
  // Check for ( sequence )
  if (cmp("(")) { // if current token is (
    next();
    T_command command=new_command(); // create new command
    command->block=p_sequence(); // parse the sequence inside the parenstheses
    command->subshell=1; // mark as subshell command
    if (!eat(")")) // expect closing )
      ERROR("expected )");
    p_redir(command); // check for input/output redirection
    return command;
  }
  
  // Check for { sequence }
  if (cmp("{")) {
    next();
    T_command command=new_command();
    command->block=p_sequence();
    command->subshell=0; // mark as block command
    if (!eat("}"))
      ERROR("expected }");
    p_redir(command);
    return command;
  }
  // Simple command
  T_words words=0;
  words=p_words();
  if (!words)
    return 0;
  T_command command=new_command();
  command->words=words; // set the words
  command->infile=0; // no input redirection by default
  command->outfile=0; // no output redirection by default
  p_redir(command); // check for input/output redirection
  return command;
}

// This function parses a pipeline of commands separated by |
static T_pipeline p_pipeline() {
  T_command command=p_command(); 
  if (!command)
    return 0;
  T_pipeline pipeline=new_pipeline();
  pipeline->command=command;
  if (eat("|")) // if we see a pipe
    pipeline->pipeline=p_pipeline(); // parse the next command in the pipeline
  return pipeline;
}

// This function parses a sequence of pipelines separated by & or ;
static T_sequence p_sequence() {
  // Stop if we hit a closing brace or paren
  if (cmp("}") || cmp(")"))
    return 0;
  T_pipeline pipeline=p_pipeline();
  if (!pipeline)
    return 0;
  T_sequence sequence=new_sequence();
  sequence->pipeline=pipeline; 
  // Check for & or ; to continue the sequence
  if (eat("&")) {
    sequence->op="&";
    sequence->sequence=p_sequence();
  }
  if (eat(";")) {
    sequence->op=";";
    sequence->sequence=p_sequence();
  }
  return sequence;
}

// This function executes the parsing process
extern Tree parseTree(char *s) {
  scan=newScanner(s); // create a new scanner
  Tree tree=p_sequence(); // parse the sequence
  if (curr())
    ERROR("extra characters at end of input");
  freeScanner(scan); // free the scanner
  return tree; // return the parse tree
}

static void f_word(T_word t);
static void f_words(T_words t);
static void f_command(T_command t);
static void f_pipeline(T_pipeline t);
static void f_sequence(T_sequence t);

// free the word structure
static void f_word(T_word t) {
  if (!t)
    return;
  if (t->s)
    free(t->s); // free the string
  free(t);
}

// free the words structure
static void f_words(T_words t) {
  if (!t)
    return;
  f_word(t->word); // free the single word
  f_words(t->words); // free the rest of the words
  free(t);
}

// free the command structure: words and redirection strings
static void f_command(T_command t) {
  if (!t)
    return;
  f_words(t->words); // free the words
  if (t->block)
    f_sequence(t->block); // free the block if it exists
  // free input/output redirection strings if they exist
  if (t->infile)
    free(t->infile);
  if (t->outfile)
    free(t->outfile);
  free(t);
}

// free the pipeline structure
static void f_pipeline(T_pipeline t) {
  if (!t)
    return;
  f_command(t->command); // free the command
  f_pipeline(t->pipeline); // free the next pipeline
  free(t);
}

// free the sequence structure
static void f_sequence(T_sequence t) {
  if (!t)
    return;
  f_pipeline(t->pipeline); // free the pipeline
  f_sequence(t->sequence); // free the next sequence
  free(t);
}

// free the entire parse tree
extern void freeTree(Tree t) {
  f_sequence(t); 
}
