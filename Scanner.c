/*
 * File: scanner.c
 * Description: Implementation of scanner.h
 * Author(s): Jim Buffenbarger
 * Date: 10/18/25 
 */

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Scanner.h"
#include "error.h"

// Representation of a scanner
typedef struct {
  int eos;
  char *str;
  char *pos;
  char *curr;
} *ScannerRep;

// This function creates a new scanner for string
extern Scanner newScanner(char *s) {
  // Allocate scanner structure
  ScannerRep r=(ScannerRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  r->eos=0; //this indicates end of string not reached
  r->str=strdup(s);// this makes a copy of the string
  r->pos=r->str; // this is the current position in the string
  r->curr=0; // this is the current token
  return r;
}

// This function frees the resources of the scanner
extern void freeScanner(Scanner scan) {
  ScannerRep r=scan; 
  free(r->str);
  if (r->curr) // free current token if it exists
    free(r->curr);
  free(r);
}

// These functions help to navigate through the string
static char *thru(char *p, char *q) {
  for (; *p && strchr(q,*p); p++);
  return p;
}
static char *upto(char *p, char *q) {
  for (; *p && !strchr(q,*p); p++);
  return p;
}

static char *wsthru(char *p) { return thru(p," \t"); }
static char *wsupto(char *p) { return upto(p," \t"); }

// This function gets the next token from the scanner
extern char *nextScanner(Scanner scan) {
  ScannerRep r=scan;
  if (r->eos)
    return 0;
  char *old=wsthru(r->pos);
  char *new=wsupto(old);
  int size=new-old;
  if (size==0) {
    r->eos=1;
    return 0;
  }
  if (r->curr)
    free(r->curr);
  r->curr=(char *)malloc(size+1);
  if (!r->curr)
    ERROR("malloc() failed");
  memmove(r->curr,old,size);
  (r->curr)[size]=0;
  r->pos=new;
  return r->curr;
}

// This function gets the current token from the scanner
extern char *currScanner(Scanner scan) {
  ScannerRep r=scan;
  if (r->eos)
    return 0;
  if (r->curr)
    return r->curr;
  return nextScanner(scan);
}

// This function compares the current token with a string
extern int cmpScanner(Scanner scan, char *s) {
  ScannerRep r=scan;
  currScanner(scan);
  if (r->eos)
    return 0;
  if (strcmp(s,r->curr))
    return 0;
  return 1;
}

// This function eats the current token if it matches the string
extern int eatScanner(Scanner scan, char *s) {
  int r=cmpScanner(scan,s);
  if (r)
    nextScanner(scan);
  return r;
}

// This function gets the current position in the string
extern int posScanner(Scanner scan) {
  ScannerRep r=scan;
  return (r->pos)-(r->str);
}
