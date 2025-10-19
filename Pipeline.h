/*
 * File: pipeline.h
 * Description: Header file for pipeline data structure and operations
 * Author(s): Jim Buffenbarger
 * Date: 10/18/25 
 */
#ifndef PIPELINE_H
#define PIPELINE_H

typedef void *Pipeline;

#include "Command.h"
#include "Jobs.h"

// Create a new pipeline
extern Pipeline newPipeline(int fg);
// Add a command to the end of the pipeline
extern void addPipeline(Pipeline pipeline, Command command);
// Get the size of the pipeline
extern int sizePipeline(Pipeline pipeline);
// Execute the pipeline with the given jobs and EOF flag
extern void execPipeline(Pipeline pipeline, Jobs jobs, int *eof);
// Free the resources associated with the pipeline
extern void freePipeline(Pipeline pipeline);

#endif
