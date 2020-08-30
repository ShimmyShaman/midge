/* midge_common.h */

#ifndef MIDGE_COMMON_H
#define MIDGE_COMMON_H

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "midge.h"
#include "midge_error_handling.h"

#define MCerror(error_code, error_message, ...)                          \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return error_code;

#define allocate_and_copy_cstr(dest, src)                                 \
  if (src == NULL) {                                                      \
    dest = NULL;                                                          \
  }                                                                       \
  else {                                                                  \
    char *mc_tmp_cstr = (char *)malloc(sizeof(char) * (strlen(src) + 1)); \
    strcpy(mc_tmp_cstr, src);                                             \
    mc_tmp_cstr[strlen(src)] = '\0';                                      \
    dest = mc_tmp_cstr;                                                   \
  }

#define allocate_and_copy_cstrn(dest, src, n)                   \
  if (src == NULL) {                                            \
    dest = NULL;                                                \
  }                                                             \
  else {                                                        \
    char *mc_tmp_cstr = (char *)malloc(sizeof(char) * (n + 1)); \
    strncpy(mc_tmp_cstr, src, n);                               \
    mc_tmp_cstr[n] = '\0';                                      \
    dest = mc_tmp_cstr;                                         \
  }

#define cprintf(dest, format, ...)                                       \
  {                                                                      \
    int cprintf_n = snprintf(NULL, 0, format, ##__VA_ARGS__);            \
    char *mc_temp_cstr = (char *)malloc(sizeof(char) * (cprintf_n + 1)); \
    sprintf(mc_temp_cstr, format, ##__VA_ARGS__);                        \
    dest = mc_temp_cstr;                                                 \
  }

#define dprintf(format, ...)       \
  {                                \
    printf(format, ##__VA_ARGS__); \
  }

typedef struct c_str {
  uint alloc;
  uint len;
  char *text;
} c_str;

extern "C" {
int init_c_str(c_str **ptr);
int set_c_str(c_str *cstr, const char *text);
int set_c_strn(c_str *cstr, const char *text, int len);
int release_c_str(c_str *ptr, bool free_char_string_also);
int append_char_to_c_str(c_str *cstr, char c);
int append_to_c_str(c_str *cstr, const char *text);
int append_to_c_strn(c_str *cstr, const char *text, int n);
int append_to_c_strf(c_str *cstr, const char *format, ...);
int insert_into_c_str(c_str *cstr, const char *text, int index);
}

int init_c_str(c_str **ptr)
{
  (*ptr) = (c_str *)malloc(sizeof(c_str));
  (*ptr)->alloc = 2;
  (*ptr)->len = 0;
  (*ptr)->text = (char *)malloc(sizeof(char) * (*ptr)->alloc);
  (*ptr)->text[0] = '\0';

  return 0;
}

int set_c_str(c_str *cstr, const char *src)
{
  cstr->len = 0;
  cstr->text[0] = '\0';
  append_to_c_str(cstr, src);

  return 0;
}

int set_c_strn(c_str *cstr, const char *src, int len)
{
  cstr->len = 0;
  cstr->text[0] = '\0';
  append_to_c_strn(cstr, src, len);

  return 0;
}

int release_c_str(c_str *ptr, bool free_char_string_also)
{
  if (ptr->alloc > 0 && free_char_string_also && ptr->text) {
    free(ptr->text);
  }

  free(ptr);

  return 0;
}

int append_char_to_c_str(c_str *cstr, char c)
{
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  append_to_c_str(cstr, buf);
  return 0;
}

int append_to_c_str(c_str *cstr, const char *text)
{
  // printf("cstr:%p\n", cstr);
  // printf("cstr->len:%u\n", cstr->len);
  // printf("cstr->alloc:%u\n", cstr->alloc);
  // printf("text:%p\n", text);
  // printf("text:'%s'\n", text);

  int len = strlen(text);
  // printf("atc-1\n");
  if (cstr->len + len + 1 >= cstr->alloc) {
    // printf("atc-2\n");
    unsigned int new_allocated_size = cstr->alloc + len + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  // printf("atc-9\n");
  // printf("atcs-a cstrtext:'%s' len:%u end:'%c'\n", cstr->text, cstr->len, cstr->text[cstr->len]);
  // printf("atcs-b text:'%s'\n", text);
  // memcpy(cstr->text + cstr->len, text, sizeof(char) * len);
  strcpy(cstr->text + cstr->len, text);
  // printf("atcs-c cstrtext:'%s' len:%u end:'%c'\n", cstr->text + cstr->len - 2, cstr->len, cstr->text[cstr->len]);
  cstr->len += len;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int append_to_c_strn(c_str *cstr, const char *text, int n)
{
  if (cstr->len + n + 1 >= cstr->alloc) {
    unsigned int new_allocated_size = cstr->alloc + n + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  strncpy(cstr->text + cstr->len, text, n);
  cstr->len += n;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int append_to_c_strf(c_str *cstr, const char *format, ...)
{
  // register_midge_error_tag("append_to_c_strf()");
  // printf("atcs-0\n");
  int chunk_size = 4;
  int i = 0;

  va_list valist;
  va_start(valist, format);

  // printf("atcs-1\n");
  while (1) {
    if (cstr->len + chunk_size + 1 >= cstr->alloc) {
      unsigned int new_allocated_size = chunk_size + cstr->alloc + 16 + (chunk_size + cstr->alloc) / 10;
      // printf("atc-n3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
      char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
      // printf("atc-4\n");
      memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
      // printf("atc-5\n");
      free(cstr->text);
      // printf("atc-6\n");
      cstr->text = newptr;
      // printf("atc-7\n");
      cstr->alloc = new_allocated_size;
      // printf("atc-8\n");
    }
    // printf("atcs-2\n");
    // sleep(1);

    // printf("'%c' chunk_size=%i cstr->len=%u\n", format[i], chunk_size, cstr->len);
    for (int a = 0; a < chunk_size; ++a) {
      cstr->text[cstr->len++] = format[i];
      // cstr->text[cstr->len] = '\0';
      // printf("cstr:'%s'\n", cstr->text);

      if (format[i] == '\0') {
        // printf("atcs-3\n");
        --cstr->len;
        va_end(valist);
        // printf("atcs-3r\n");
        return 0;
      }

      if (format[i] == '%') {
        if (format[i + 1] == '%') {
          // printf("atcs-4\n");
          // Use as an escape character
          ++i;
        }
        else {
          --cstr->len;
          // Search to replace the format
          ++i;
          // printf("atcs-5 i:%i i:'%c'\n", i, format[i]);
          switch (format[i]) {
          case 'i': {
            int value = va_arg(valist, int);

            // printf("atcs-5b value:'%i'\n", value);

            char buf[18];
            sprintf(buf, "%i", value);

            if (!strncmp(cstr->text, "int midge_initial", 17)) {
              printf("atcs-5c cstr:'%s' :%i\n", cstr->text, cstr->len);
            }
            // printf("atcs-5d buf:'%s'\n", buf);
            // printf("atcs-5e\n");
            append_to_c_str(cstr, buf);
            // printf("atcs-5f\n");
          } break;
          // case 'l': {
          //   switch (format[i + 1]) {
          //   case 'i': {
          //     ++i;
          //     long int value = va_arg(valist, long int);

          //     char buf[24];
          //     sprintf(buf, "%li", value);
          //     append_to_c_str(cstr, buf);
          //   } break;
          //   default:
          //     MCerror(99, "TODO:l'%c'", format[i + 1]);
          //   }
          // } break;
          case 'p': {
            void *value = va_arg(valist, void *);

            char buf[18];
            sprintf(buf, "%p", value);
            append_to_c_str(cstr, buf);
          } break;
          case 's': {
            char *value = va_arg(valist, char *);
            // printf("atcs-7a cstrtext:'%s' len:%u end:'%c'\n", cstr->text, cstr->len, cstr->text[cstr->len]);
            // printf("atcs-7b value:'%p'\n", value);
            // printf("atcs-7c value:'%s'\n", value);
            // cstr->text[i] = '\0';
            // --cstr->len;
            append_to_c_str(cstr, value);
            // printf("atcs-7b cstrtext:'%s'\n", cstr->text);
          } break;
          case 'u': {
            unsigned int value = va_arg(valist, unsigned int);

            printf("append_to_c_strf-arg=%u\n", value);

            char buf[18];
            sprintf(buf, "%u", value);
            append_to_c_str(cstr, buf);
          } break;
          default: {
            MCerror(99, "TODO:%c", format[i]);
          }
          }
        }
        ++i;

        // Chunk size is unreliable -- reset
        break;
      }

      ++i;
      // printf("atcs-8 i:%i i:'%c'\n", i, format[i]);
    }

    // printf("atcs-6\n");
    chunk_size = (chunk_size * 5) / 3;
  }
}

int insert_into_c_str(c_str *cstr, const char *text, int index)
{
  if (index > cstr->len) {
    MCerror(667, "TODO");
  }

  int n = strlen(text);
  if (n == 0) {
    return 0;
  }

  if (cstr->len + n + 1 >= cstr->alloc) {
    unsigned int new_allocated_size = cstr->alloc + n + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    if (index) {
      memcpy(newptr, cstr->text, sizeof(char) * index);
    }
    memcpy(newptr + index, text, sizeof(char) * n);
    if (cstr->len - index) {
      memcpy(newptr + index + n, cstr->text + index, sizeof(char) * (cstr->len - index));
    }
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    cstr->len += n;
    cstr->text[cstr->len] = '\0';
    // printf("atc-8\n");
    return 0;
  }

  if (index != cstr->len) {
    memmove(cstr->text + index + n, cstr->text + index, sizeof(char) * (cstr->len - index));
  }
  memcpy(cstr->text + index, text, sizeof(char) * n);
  cstr->len += n;
  cstr->text[cstr->len] = '\0';

  return 0;
}
#endif // MIDGE_COMMON_H