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
  printf("written %zu bytes to %s\n", written, filepath);
  fclose(f);

  return written;
}

// [_mc_iteration=4]
void save_source_to_file(mc_source_definition_v1 *source_definition)
{
  // printf("sftf-0\n");
  if (!source_definition->source_file) {
    // ERR(ERROR_ARGUMENT, "function has no source file to save to.");
    printf("TODO ERROR HANDLING\n");
    return;
  }
  register_midge_error_tag("save_function_to_file-2");

  // TODO -- this somewhere else
  // printf("function->source->code:\n%s||\n", function->source->code);

  c_str *save_text;
  init_c_str(&save_text);

  append_to_c_strf(
      save_text,
      "/* %s\n   Copyright 2020, Adam Rasburn, All Rights Reserved.\n*/\n\n#include \"core/midge_core.h\"\n\n\n",
      source_definition->source_file->filepath);

  register_midge_error_tag("save_function_to_file-4");
  for (int i = 0; i < source_definition->source_file->definitions.count; ++i) {
    mc_source_definition_v1 *definition = source_definition->source_file->definitions.items[i];
    switch (definition->type) {
    case SOURCE_DEFINITION_FUNCTION: {
      append_to_c_str(save_text, definition->code);
    } break;
    case SOURCE_DEFINITION_STRUCTURE: {
      printf("sftf-struct:\n%s||\n", definition->code);
      append_to_c_str(save_text, definition->code);
    } break;
    default: {
      printf("ERROR 85\n");
    }
    }

    append_to_c_str(save_text, "\n\n");
  }

  // printf("sftf-0\n");
  // printf("filepath:'%s'\n", function->source->source_file->filepath);
  // printf("save_text:\n%s||\n", save_text->text);
  // printf("sftf-0\n");
  size_t written = save_text_to_file(source_definition->source_file->filepath, save_text->text);

  release_c_str(save_text, true);

  if (written) {
    printf("saved function to file '%s' (%zu bytes)\n", source_definition->source_file->filepath, written);
  }
  else {
    printf("could not save function to file '%s'\n", source_definition->source_file->filepath);
  }
  // simpincel
}