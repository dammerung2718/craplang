#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* helpers */
#define die(fmt, ...)                                                          \
  do {                                                                         \
    fprintf(stderr, fmt, __VA_ARGS__);                                         \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

struct FileContents {
  char *data;
  int size;
};

/* read a file's contents */
struct FileContents readFileContents(char *path);
