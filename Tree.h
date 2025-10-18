// defines the data structures for the parse commands.
#ifndef TREE_H
#define TREE_H

typedef struct T_sequence *T_sequence;
typedef struct T_pipeline *T_pipeline;
typedef struct T_command  *T_command;
typedef struct T_words    *T_words;
typedef struct T_word     *T_word;

// A sequence is one or more pipelines separated by ; or &
struct T_sequence {
  T_pipeline pipeline;
  char *op;			/* ; or & */
  T_sequence sequence;
};

// A pipeline is one or more commands separated by |
struct T_pipeline {
  T_command command;
  T_pipeline pipeline;
};

// A command is one or more words
struct T_command {
  T_words words;
  char *infile; // input redirection file, Null if none
  char *outfile; // output redirection file, Null if none
  T_sequence block;
  int subshell;
};

// list of words
struct T_words {
  T_word word;
  T_words words;
};

// a single word
struct T_word {
  char *s;
};

extern T_sequence new_sequence();
extern T_pipeline new_pipeline();
extern T_command  new_command();
extern T_words    new_words();
extern T_word     new_word();

#endif
