#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true ((unsigned char)0x7F)
#endif
#ifndef false
#define false ((unsigned char)0)
#endif

int _rmc_read_all_file_text(const char *filepath, char **contents)
{
  // Load the text from the core functions directory
  FILE *f = fopen(filepath, "rb");
  if (!f) {
    return 44;
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  *contents = (char *)malloc(fsize + 1);
  fread(*contents, sizeof(char), fsize, f);
  fclose(f);

  (*contents)[fsize] = '\0';

  return 0;
}

size_t _rmc_save_text_to_file(const char *filepath, char *text)
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

typedef struct _rmc_mc_strr {
  unsigned int alloc;
  unsigned int len;
  char *text;
} _rmc_mc_strr;

int init__rmc_mc_strr(_rmcmc_strtr **ptr, char *text)
{
  (*ptr) = (_rmc_mc_strr *)malloc(sizeof(_rmcmc_strtr));
  (*ptr)->len = strlen(text);
  (*ptr)->alloc = (*ptr)->len;
  (*ptr)->text = text;

  return 0;
}

int remove_from__rmc_mc_strr(_rmcmc_strtr *cstr, int start_index, int len)
{
  if (start_index > cstr->len || len == 0)
    return 0;

  if (start_index + len == cstr->len) {
    cstr->len = start_index;
    return 0;
  }

  int a;
  for (a = 0; start_index + len + a < cstr->len; ++a) {
    cstr->text[start_index + a] = cstr->text[start_index + len + a];
  }
  cstr->len -= len;
  cstr->text[cstr->len] = '\0';

  return 0;
}

const char *_mcl_source_files[] = {
    "src/core/core_definitions.h",
    "src/core/c_parser_lexer.h",
    "src/core/mc_code_transcription.h",
    "src/core/core_definitions.c",
    "src/core/c_parser_lexer.c",
    "src/core/mc_code_transcription.c",
    // And everything here before -------------------------------------------------------------
    "src/core/mc_source.c",
    NULL,
};

void remove_all_MCcalls()
{
  const char *filepath = "src/core/mc_source.c";
  bool write_to_file = true;

  // for(int a = 0 ; _mcl_source_files)
  char *code;
  if (_rmc_read_all_file_text(filepath, &code)) {
    printf("couldn't load text\n");
    return;
  }

  _rmc_mc_strr *src;
  init__rmc_mc_strr(&src, code);

  for (int i = 0; i < src->len; ++i) {
    if (!strncmp(src->text + i, "MCcall(", 6)) {
      remove_from__rmc_mc_strr(src, i, 7);

      // Remove the ')' at the end
      for (int j = i + 1;; ++j) {
        if (src->text[j] == '"') {
          bool escaped = false;
          bool loop = true;
          while (loop) {
            // printf(":%c", src->text[j]);
            ++j;
            switch (src->text[j]) {
            case '\\': {
              escaped = !escaped;
            } break;
            case '\0': {
              printf("unexpected eof\n");
              return;
            }
            case '"': {
              if (escaped) {
                break;
              }
              ++j;
              loop = false;
            } break;
            default: {
              escaped = false;
            } break;
            }
          }
        }

        if (src->text[j] == ';') {
          if (src->text[j - 1] != ')') {
            printf("expected ')'\n");
            return;
          }
          remove_from__rmc_mc_strr(src, j - 1, 1);
          break;
        }
      }
    }
  }

  printf("SUCCESS\n");
  if (write_to_file)
    _rmc_save_text_to_file(filepath, src->text);
  else {
    // printf("src:\n%s||\n", src->text);
  }
}