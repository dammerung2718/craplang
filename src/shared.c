#include "shared.h"

struct FileContents readFileContents(char *path) {
  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    die("failed to open file! %s", path);
  }

  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  rewind(fp);

  char *buffer = (char*)malloc(size + 1);
  memset(buffer, 0, size + 1);

  fread(buffer, 1, size, fp);
  fclose(fp);

  return (struct FileContents){
      .data = buffer,
      .size = size,
  };
}
