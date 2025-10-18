# Code Citations

## License: unknown
https://github.com/BoiseState/CS453-resources/blob/425fd4fea414b61dc190df39c8e789fd3fec620a/buff/classes/452/pub/hw3/Shell.c

```
;
  
  if (isatty(fileno(stdin))) {
    using_history();
    read_history(".history");
    prompt="$ ";
  } else {
    rl_bind_key('\t',rl_insert);
    rl_outstream=fopen("/dev/null","w");
  }
  
  while (!eof) {
    char *line=readline(prompt);
    if (!line)
      break;
    if (*line)
      add_history(line);
    Tree tree=parseTree(line);
    free(line);
    interpretTree(tree,&eof,jobs);
    freeTree(tree);
  }

  if (isatty(fileno(stdin)
```


## License: unknown
https://github.com/BoiseState/CS453-resources/blob/425fd4fea414b61dc190df39c8e789fd3fec620a/buff/classes/452/pub/hw3/Shell.c

```
;
  
  if (isatty(fileno(stdin))) {
    using_history();
    read_history(".history");
    prompt="$ ";
  } else {
    rl_bind_key('\t',rl_insert);
    rl_outstream=fopen("/dev/null","w");
  }
  
  while (!eof) {
    char *line=readline(prompt);
    if (!line)
      break;
    if (*line)
      add_history(line);
    Tree tree=parseTree(line);
    free(line);
    interpretTree(tree,&eof
```


## License: unknown
https://github.com/BoiseState/CS453-resources/blob/425fd4fea414b61dc190df39c8e789fd3fec620a/buff/classes/452/pub/hw3/Shell.c

```
;
  
  if (isatty(fileno(stdin))) {
    using_history();
    read_history(".history");
    prompt="$ ";
  } else {
    rl_bind_key('\t',rl_insert);
    rl_outstream=fopen("/dev/null","w");
  }
  
  while (!eof) {
    char *line=readline(prompt);
    if (!line)
      break;
    if (*line)
      add_history(line);
    Tree tree=parseTree(line);
    free(line);
    interpretTree(tree,&eof,jobs);
    freeTree(tree);
  }

  
```


## License: unknown
https://github.com/BoiseState/CS453-resources/blob/425fd4fea414b61dc190df39c8e789fd3fec620a/buff/classes/452/pub/hw3/Shell.c

```
;
  
  if (isatty(fileno(stdin))) {
    using_history();
    read_history(".history");
    prompt="$ ";
  } else {
    rl_bind_key('\t',rl_insert);
    rl_outstream=fopen("/dev/null","w");
  }
  
  while (!eof) {
    char *line=readline(prompt);
    if (!line)
      break;
    if (*line)
      add_history(line);
    Tree tree=parseTree(line);
    free(line);
    interpretTree(tree,&eof,jobs);
    freeTree(tree);
  }

  if (isatty(fileno(stdin))) {
    write_
```


## License: unknown
https://github.com/BoiseState/CS453-resources/blob/425fd4fea414b61dc190df39c8e789fd3fec620a/buff/classes/452/pub/hw3/Shell.c

```
;
  
  if (isatty(fileno(stdin))) {
    using_history();
    read_history(".history");
    prompt="$ ";
  } else {
    rl_bind_key('\t',rl_insert);
    rl_outstream=fopen("/dev/null","w");
  }
  
  while (!eof) {
    char *line=readline(prompt);
    if (!line)
      break;
    if (*line)
      add_history(line);
    Tree tree=parseTree(line);
    free(line);
    interpretTree(tree,&eof,jobs);
    freeTree(tree);
  }

  if (isatty(fileno(stdin))) {
    write_history(".history");
    rl_clear_history();
  }
```


## License: unknown
https://github.com/BoiseState/CS453-resources/blob/425fd4fea414b61dc190df39c8e789fd3fec620a/buff/classes/452/pub/hw3/Shell.c

```
;
  
  if (isatty(fileno(stdin))) {
    using_history();
    read_history(".history");
    prompt="$ ";
  } else {
    rl_bind_key('\t',rl_insert);
    rl_outstream=fopen("/dev/null","w");
  }
  
  while (!eof) {
    char *line=readline(prompt);
    if (!line)
      break;
    if (*line)
      add_history(line);
    Tree tree=parseTree(line);
    free(line);
    interpretTree(tree,&eof,jobs);
    freeTree(tree);
  }

  if (isatty(fileno(stdin))) {
    write_history(".history");
    rl_clear_history();
  } else {
    fclose(rl_outstream)
```

