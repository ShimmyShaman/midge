/* save_structure_to_file.c */

#include "core/midge_core.h"

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