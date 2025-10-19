/*
 * File: scanner.h
 * Description: Header file for scanner that breaks a string 
 * into individual words (tokens) separated by whitespace.
 * Author(s): Jim Buffenbarger
 * Date: 10/18/25 
 */
#ifndef SCANNER_H
#define SCANNER_H

typedef void *Scanner;

// Create a new scanner for string
extern Scanner newScanner(char *s);
// Free the resources of the scanner
extern void freeScanner(Scanner scan);
// Get the next token from the scanner
extern char *nextScanner(Scanner scan);
// Get the current token from the scanner
extern char *currScanner(Scanner scan);
// Compare the current token with a string
extern int cmpScanner(Scanner scan, char *s);
// Eat the current token if it matches the string
extern int eatScanner(Scanner scan, char *s);
// Get the current position in the string
extern int posScanner(Scanner scan);

#endif
