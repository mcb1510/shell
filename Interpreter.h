/* 
 * File: interpreter.h
 * Description: Header file for interpreting parse trees
 * Author(s): Jim Buffenbarger - Miguel Carrasco
 * Date: 10/18/25 
 */
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Parser.h"
#include "Tree.h"
#include "Jobs.h"

// Interpret the parse tree into executable objects
extern void interpretTree(Tree t, int *eof, Jobs jobs);
// Interpret a sequence from the parse tree
extern void i_sequence(T_sequence t, Sequence sequence);

#endif
