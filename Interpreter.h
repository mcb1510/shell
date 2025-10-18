// converts the parse tree into executable objects
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Parser.h"
#include "Tree.h"
#include "Jobs.h"

extern void interpretTree(Tree t, int *eof, Jobs jobs);
extern void i_sequence(T_sequence t, Sequence sequence);

#endif
