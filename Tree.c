/*
 * File: Shell.c
 * Description: Main shell program
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Tree.h"
#include "error.h"

#define ALLOC(t) \
  t v=malloc(sizeof(*v)); \
  if (!v) ERROR("malloc() failed"); \
  return memset(v,0,sizeof(*v));

// Create a new sequence and allocate memory for it
extern T_sequence new_sequence() {ALLOC(T_sequence)}
// Create a new pipeline and allocate memory for it
extern T_pipeline new_pipeline() {ALLOC(T_pipeline)}
// Create a new command and allocate memory for it
extern T_command  new_command()  {
  T_command v = malloc(sizeof(*v)); 
  if (!v) ERROR("malloc() failed");
  memset(v,0,sizeof(*v)); //
  v->subshell = -1; 
  return v;
}
// Create a new words list and allocate memory for it
extern T_words    new_words()    {ALLOC(T_words)}
// Create a new word and allocate memory for it
extern T_word     new_word()     {ALLOC(T_word)}
