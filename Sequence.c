/*
 * File: sequence.c
 * Description: Implementation of sequence.h
 * Author(s): Jim Buffenbarger
 * Date: 10/18/25 
 */
#include "Sequence.h"
#include "deq.h"
#include "error.h"

// We use deque functions to implement sequences
// Create a new empty sequence
extern Sequence newSequence() {
  return deq_new(); // we use a deque to hold the pipelines
}

// Add a pipeline to the end of the sequence
extern void addSequence(Sequence sequence, Pipeline pipeline) {
  deq_tail_put(sequence,pipeline); 
}

// Free all resources of the sequence
extern void freeSequence(Sequence sequence) {
  deq_del(sequence,freePipeline);
}

// execute all pipelines in the sequence on the given jobs
// parameters:
//   sequence: the sequence of pipelines to execute
//   jobs: the jobs to process
//   eof: pointer to an int that indicates end-of-file
extern void execSequence(Sequence sequence, Jobs jobs, int *eof) {
  while (deq_len(sequence) && !*eof) // while there are pipelines and not eof
    execPipeline(deq_head_get(sequence),jobs,eof); // We execute head pipeline
  freeSequence(sequence); // free the sequence when done
}
