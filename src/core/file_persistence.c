/* file_persistence.c */
#include "core/midge_core.h"

int find_in_str(char *str, char *seq)
{
  printf("fis-0\n");
  printf("str:'%s'\n", str);
  printf("seq:'%s'\n", seq);
  int len = strlen(seq);
  if (len < 1) {
    // ERR(ERROR_ARGUMENT, "seq length must be greater than 0");
    printf("TODO ERROR HANDLING\n");
  }

  printf("fis-1\n");
  for (int a = 0;; ++a) {
    if (str[a] != seq[0]) {
      if (str[a] == '\0') {
        break;
      }
      continue;
    }

    bool found = true;
    for (int b = 1; b < len; ++b) {
      if (str[a + b] != seq[b]) {
        found = false;
        break;
      }
    }
    if (found) {
      // Sequence matches
      printf("fis- return:%i\n", a);
      print_parse_error(str, a, "find_in_str", "successful_return");
      return a;
    }
  }
  printf("fis- return:-1\n");

  return -1;
}

size_t save_text_to_file(char *filepath, char *text)
{
  FILE *f = fopen(filepath, "w");
  if (f == NULL) {
    printf("problem opening file '%s'\n", filepath);
    return 0;
  }
  fseek(f, 0, SEEK_SET);

  int len = strlen(text);

  size_t written = fwrite(text, sizeof(char), len, f);
  printf("written:%zu\n", written);
  fclose(f);

  return written;
}

// [_mc_iteration=4]
void save_function_to_file(mc_function_info_v1 *function)
{
  // printf("sftf-0\n");
  if (!function->source->source_file) {
    // ERR(ERROR_ARGUMENT, "function has no source file to save to.");
    printf("TODO ERROR HANDLING\n");
    return;
  }
  register_midge_error_tag("save_function_to_file-2");

  // TODO -- this somewhere else
  // printf("function->source->code:\n%s||\n", function->source->code);

  c_str *save_text;
  init_c_str(&save_text);

  append_to_c_strf(save_text, "/* %s */\n\n#include \"core/midge_core.h\"\n\n\n",
                   function->source->source_file->filepath);

  register_midge_error_tag("save_function_to_file-4");
  for (int i = 0; i < function->source->source_file->definitions.count; ++i) {
    mc_source_definition_v1 *definition = function->source->source_file->definitions.items[i];
    switch (definition->type) {
    case SOURCE_DEFINITION_FUNCTION: {
      append_to_c_str(save_text, definition->code);
    } break;
    case SOURCE_DEFINITION_STRUCT: {
      printf("sftf-struct:\n%s||\n", definition->code);
      append_to_c_str(save_text, definition->code);
    } break;
    default: {
      printf("ERROR 85\n");
    }
    }

    append_to_c_str(save_text, "\n\n");
  }

  printf("sftf-0\n");
  printf("filepath:'%s'\n", function->source->source_file->filepath);
  printf("save_text:\n%s||\n", save_text->text);
  printf("sftf-0\n");
  size_t written = save_text_to_file(function->source->source_file->filepath, save_text->text);

  release_c_str(save_text, true);

  if (written) {
    printf("saved function to file '%s' (%zu bytes)\n", function->source->source_file->filepath, written);
  }
  else {
    printf("could not save function to file '%s'\n", function->source->source_file->filepath);
  }
  // simpincel
}

// [_mc_iteration=2]
void save_struct_to_file(struct_info *structure, char *structure_definition)
{
  if (!structure->source->source_file) {
    // ERR(ERROR_ARGUMENT, "structure has no source file to save to.");
    printf("TODO ERROR HANDLING\n");
  }

  // save_structure_to_source_file()
  FILE *f = fopen(structure->source->source_file->filepath, "w");
  if (f == NULL) {
    printf("problem opening file '%s'\n", structure->source->source_file->filepath);
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

  printf("saved structure to file '%s' (%zu bytes)\n", structure->source->source_file->filepath, written);
  // printf("here-4\n");
}
