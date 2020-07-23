/* file_persistence.c */
#include "core/midge_core.h"

int find_in_str(char *str, char *seq)
{
  printf("fis-0\n");
  printf("str:'%s'\n", str);
  printf("seq:'%s'\n", seq);
  int len = strlen(seq);
  if (len < 1) {
    ERR(ERROR_ARGUMENT, "seq length must be greater than 0");
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
  fclose(f);

  return written;
}

// [_mc_iteration=4]
void save_function_to_file(mc_function_info_v1 *function, char *function_definition)
{
  printf("sftf-0\n");
  if (!function->source_file) {
    ERR(ERROR_ARGUMENT, "function has no source file to save to.");
  }

  char *file_text = read_file_text(function->source_file->filepath);

  c_str *save_text;
  init_c_str(&save_text);

  int s = find_in_str(file_text, function->name);
    printf("sftf-5 s:%i\n", s);
  if (s >= 0) {
    // Move to before the return type
    --s;
    while (file_text[s] == '*' || file_text[s] == ' ' || file_text[s] == '\n' || file_text[s] == '\t') {
      --s;
    }
    while (isalnum(file_text[s]) || file_text[s] == '_') {
      --s;
    }
    ++s;

    printf("sftf-6 s:%i\n", s);
    char *tempfs = file_text + s;
    int i = find_in_str(tempfs, "{");
    if (i < 0) {
      ERR(ERROR_FILE_FORMAT_ERROR1, "can't replace the function that was found. TODO safely deal with this");
    }
    printf("sftf-7\n");
    {
      bool eof = false;
      int bracket_count = 1;
      while (bracket_count) {
        if (file_text[i] == '\0') {
          eof = true;
          break;
        }
        else if (file_text[i] == '{') {
          ++bracket_count;
        }
        else if (file_text[i] == '}') {
          --bracket_count;
        }
        ++i;
      }
      if (eof) {
        ERR(ERROR_FILE_FORMAT_ERROR2, "can't replace the function that was found. TODO safely deal with this");
      }
    }

    printf("sftf-0\n");
    append_to_c_strn(save_text, file_text, s);
    append_to_c_str(save_text, function_definition);

    printf("sftf-0\n");
    char *tempfi = file_text + i;
    append_to_c_str(save_text, tempfi);
  }
  else {
    // Place it according to its position or dependencies?? donno TODO
    // Place it at the end
    MCerror(50, "TODO");
  }

  free(file_text);

  // // printf("here-2\n");
  // char buf[128];
  // sprintf(buf, "/* %s.c */\n\n#include \"core/midge_core.h\"\n\n", function->name);
  // // printf("buf:'%s'\n", buf);
  // // printf("here-3a\n");
  // int buf_len = strlen(buf);
  // size_t written = fwrite(buf, sizeof(char), buf_len, f);
  // // printf("here-3b\n");

  // sprintf(buf, "// [_mc_iteration=%u]\n", function->latest_iteration);
  // buf_len = strlen(buf);
  // written += fwrite(buf, sizeof(char), buf_len, f);

  printf("sftf-0\n");
  size_t written = save_text_to_file(function->source_file->filepath, save_text->text);

  release_c_str(save_text);

  if (written) {
    printf("saved function to file '%s' (%zu bytes)\n", function->source_file->filepath, written);
  }
  else {
    printf("could not save function to file '%s'\n", function->source_file->filepath);
  }
  // printf("here-4\n");
}

// [_mc_iteration=2]
void save_struct_to_file(struct_info *structure, char *structure_definition)
{
  if (!structure->source_file) {
    ERR(ERROR_ARGUMENT, "structure has no source file to save to.");
  }

  // save_structure_to_source_file()
  FILE *f = fopen(structure->source_file->filepath, "w");
  if (f == NULL) {
    printf("problem opening file '%s'\n", structure->source_file->filepath);
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

  printf("saved structure to file '%s' (%zu bytes)\n", structure->source_file->filepath, written);
  // printf("here-4\n");
}