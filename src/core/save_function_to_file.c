
#include "core/midge_core.h"

void save_function_to_file(mc_function_info_v1 *function, char *function_definition)
{
  // printf("here-1\n");
  // save_function_to_source_file()
  FILE *f = fopen(function->source_filepath, "w");
  // fseek(f, 0, SEEK_SET);

  // printf("here-2\n");
  char buf[128];
  sprintf(buf, "/* %s.c */\n\n#include \"core/midge_core.h\"\n\n", function->name);
  // printf("buf:'%s'\n", buf);
  // printf("here-3a\n");
  int buf_len = strlen(buf);
  fwrite(buf, sizeof(char), buf_len, f);
  // printf("here-3b\n");

  sprintf(buf, "// [_mc_iteration=%u]\n", function->latest_iteration);
  buf_len = strlen(buf);
  fwrite(buf, sizeof(char), buf_len, f);

  // printf("function_definition:%s\n", function_definition);
  int definition_len = strlen(function_definition);
  fwrite(function_definition, sizeof(char), definition_len, f);
  fclose(f);

  printf("saved function to file '%s'\n", function->source_filepath);
  // printf("here-4\n");
}