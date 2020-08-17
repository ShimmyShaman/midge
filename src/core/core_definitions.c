
#include "core/core_definitions.h"
#include <stdio.h>

int read_file_text(char *filepath, char **output)
{
  // Parse
  FILE *f = fopen(filepath, "rb");
  if (f == NULL) {
    MCerror(2263, "File '%s' not found!", filepath);
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char *input = (char *)malloc(fsize + 1);
  fread(input, sizeof(char), fsize, f);
  input[fsize] = '\0';
  fclose(f);

  *output = input;
  return 0;
}