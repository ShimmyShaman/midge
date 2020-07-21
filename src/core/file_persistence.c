/* file_persistence.c */
#include "core/midge_core.h"

int find_in_str(char *str, char *seq) { return -1; }

// [_mc_iteration=4]
void save_function_to_file(mc_function_info_v1 *function, char *function_definition)
{
  char *file_text = read_file_text(function->source_filepath);

  int s = find_in_str(file_text, function->name);

  free(file_text);

  // save_function_to_source_file()
  FILE *f = fopen(function->source_filepath, "w");
  // fseek(f, 0, SEEK_SET);

  // printf("here-2\n");
  char buf[128];
  sprintf(buf, "/* %s.c */\n\n#include \"core/midge_core.h\"\n\n", function->name);
  // printf("buf:'%s'\n", buf);
  // printf("here-3a\n");
  int buf_len = strlen(buf);
  size_t written = fwrite(buf, sizeof(char), buf_len, f);
  // printf("here-3b\n");

  sprintf(buf, "// [_mc_iteration=%u]\n", function->latest_iteration);
  buf_len = strlen(buf);
  written += fwrite(buf, sizeof(char), buf_len, f);

  // printf("function_definition:%s\n", function_definition);
  int definition_len = strlen(function_definition);
  written += fwrite(function_definition, sizeof(char), definition_len, f);
  fclose(f);

  printf("saved function to file '%s' (%zu bytes)\n", function->source_filepath, written);
  // printf("here-4\n");
}

// [_mc_iteration=2]
void save_struct_to_file(struct_info *structure, char *structure_definition)
{
  // save_structure_to_source_file()
  FILE *f = fopen(structure->source_filepath, "w");
  if (f == NULL) {
    printf("problem opening file '%s'\n", structure->source_filepath);
    return;
  }
  fseek(f, 0, SEEK_SET);

  // printf("here-2\n");
  char buf[128];
  sprintf(buf, "/* %s.c */\n\n#include \"core/midge_core.h\"\n\n", structure->name);
  printf("buf:'%s'\n", buf);
  // printf("here-3a\n");
  int buf_len = strlen(buf);
  printf("buf_len:'%i'\n", buf_len);
  size_t written = fwrite(buf, sizeof(char), buf_len, f);
  // printf("written:%zu\n", written);
  // printf("here-3b\n");

  sprintf(buf, "// [_mc_version=%u]\n", structure->version);
  buf_len = strlen(buf);
  printf("buf:'%s'\n", buf);
  printf("buf_len:'%i'\n", buf_len);
  written += fwrite(buf, sizeof(char), buf_len, f);
  // printf("written:%zu\n", written);

  printf("structure_definition:%s\n", structure_definition);
  int definition_len = strlen(structure_definition);
  written += fwrite(structure_definition, sizeof(char), definition_len, f);
  // printf("written:%zu\n", written);
  // char eof = '\0';
  // fwrite(&eof, sizeof(char), 1, f);
  fclose(f);

  printf("saved structure to file '%s' (%zu bytes)\n", structure->source_filepath, written);
  // printf("here-4\n");
}