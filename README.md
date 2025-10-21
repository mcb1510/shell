# CS 552 - Homework 3: Execution Integrity and Do Process

**Author:** Jim Buffenbarger, Miguel Carrasco Belmar 
**Date:** October 21, 2025  
**Course:** CS 552  
**Assignment:** HW3

## Overview
This is an implementation of a simple Unix shell. This assignment's purpuse is to learn
and practice multipprocess programming. The shell will support sequences, pipelines, 
I/O redirection, compound commands, subshells, and job control (e.g, fg and bg).

## Grammar of this Shell 
 sequence ::=
    pipeline
    pipeline &
    pipeline ;
    pipeline & sequence
    pipeline ; sequence
pipeline ::=
     command
    command | pipeline
command ::=
     words redir
    ( sequence ) redir
    { sequence } redir
 words ::=
    word
    words word
 redir ::=
    ^
    < word
    > word
    < word > word
 
## Files Included
- `Command.h` - Command Execution interface
- `Command.c` - Command interface implementation
- `Interpreter.h` - Interpreting parse trees interface
- `Interpreter.c` - Interpreting parse trees implementation
- `Jobs.h` - Job control interface
- `Jobs.c` - Job control implementation
- `Parser.h` - Parsing input into a parse tree interface
- `Parser.c` - Parsing input into a parse tree implementation
- `Pipeline.h` - Pipeline data structure and operations interface
- `Pipeline.c` - Pipeline data structure and operations implementation
- `Scanner.h`- Scanner module interface
- `Scanner.c` - Scanner module implementation
- `deq.c` - Main implementation of the deque data structure from hw1
- `deq.h` - Header file with program interface hw1
- `error.h` - Error handling hw1
- `valgrind_results.txt` - Output of test function showing valgrind output
- `Sequence.h` - Sequence Module interface
- `Sequence.c` - Sequence Module implementation
- `Shell.c` - Main function 
- `Tree.h` - Tree data structure interface
- `Tree.c` - Tree data structure implementation
- `Test Directory` - Test files for testsuite 
- `test_output.txt` -  Test results
- `manual_job_control_test.txt` - proof of working job module with fg and bg

## How to Run
make
./shell

## To test
test/run

## Resources
-Starter code provided by Professor Jim Buffenbarger.
-GitHub Copilot
-Chat GPT5