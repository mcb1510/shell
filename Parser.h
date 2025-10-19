/*
 * File: parser.h
 * Description: Header file for parsing input into a parse tree
 * Author(s): Jim Buffenbarger
 * Date: 10/18/25 
 */
#ifndef PARSER_H
#define PARSER_H

typedef void *Tree;
// Parse the input string into a parse tree
extern Tree parseTree(char *s);
// Free the parse tree
extern void freeTree(Tree t);

#endif
