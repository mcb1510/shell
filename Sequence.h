/*
 * File: sequence.h
 * Description: Header file for sequence module
 * Author(s): Jim Buffenbarger
 * Date: 10/18/25 
 */

#ifndef SEQUENCE_H
#define SEQUENCE_H

typedef void *Sequence;

#include "Jobs.h"
#include "Pipeline.h"

// Create a new empty sequence
extern Sequence newSequence();
// Add a pipeline to the end of the sequence
extern void addSequence(Sequence sequence, Pipeline pipeline);
// Free all resources of the sequence
extern void freeSequence(Sequence sequence);
// execute all pipelines in the sequence on the given jobs
extern void execSequence(Sequence sequence, Jobs jobs, int *eof);

#endif
