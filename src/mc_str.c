/* mc_str.c */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midge_error_handling.h"

#include "mc_str.h"

int init_mc_str(mc_str **ptr)
{
  (*ptr) = (mc_str *)malloc(sizeof(mc_str));
  (*ptr)->alloc = 2;
  (*ptr)->len = 0;
  (*ptr)->text = (char *)malloc(sizeof(char) * (*ptr)->alloc);
  (*ptr)->text[0] = '\0';

  return 0;
}

int init_mc_str_with_specific_capacity(mc_str **ptr, unsigned int specific_capacity)
{
  (*ptr) = (mc_str *)malloc(sizeof(mc_str));
  (*ptr)->alloc = specific_capacity;
  (*ptr)->len = 0;
  (*ptr)->text = (char *)malloc(sizeof(char) * (*ptr)->alloc);
  (*ptr)->text[0] = '\0';

  return 0;
}

int set_mc_str(mc_str *str, const char *src)
{
  str->len = 0;
  str->text[0] = '\0';
  append_to_mc_str(str, src);

  return 0;
}

int set_mc_strn(mc_str *str, const char *src, int len)
{
  str->len = 0;
  str->text[0] = '\0';
  append_to_mc_strn(str, src, len);

  return 0;
}

void release_mc_str(mc_str *ptr, bool free_char_string_also)
{
  if (ptr->alloc > 0 && free_char_string_also && ptr->text) {
    free(ptr->text);
  }

  free(ptr);
}

// TODO -- this should be faster then append_to_mc_str
int append_char_to_mc_str(mc_str *str, char c)
{
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  append_to_mc_str(str, buf);
  return 0;
}

int append_to_mc_str(mc_str *str, const char *text)
{
  // printf("str:%p\n", str);
  // printf("str->len:%u\n", str->len);
  // printf("str->alloc:%u\n", str->alloc);
  // printf("text:%p\n", text);
  // printf("text:'%s'\n", text);

  int len = strlen(text);
  // printf("atc-1 %p\n", str);
  if (str->len + len + 1 >= str->alloc) {
    // printf("atc-2\n");
    unsigned int new_allocated_size = str->alloc + len + 16 + (str->alloc) / 2;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", str->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, str->text, sizeof(char) * str->alloc);
    // printf("atc-5\n");
    free(str->text);
    // printf("atc-6\n");
    str->text = newptr;
    // printf("atc-7\n");
    str->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  // printf("atc-9\n");
  // printf("atcs-a cstrtext:'%s'\n", str->text);
  // printf("atcs-b len:%u\n",  str->len);
  // printf("atcs-c end:'%c'\n",  str->text[str->len]);
  // printf("atcs-d text:'%s'\n", text);
  // memcpy(str->text + str->len, text, sizeof(char) * len);
  strcpy(str->text + str->len, text);
  // printf("atcs-e cstrtext:'%s' len:%u end:'%c'\n", str->text + str->len - 2, str->len, str->text[str->len]);
  str->len += len;
  str->text[str->len] = '\0';

  return 0;
}

int append_to_mc_strn(mc_str *str, const char *text, int n)
{
  if (str->len + n + 1 >= str->alloc) {
    unsigned int new_allocated_size = str->alloc + n + 16 + (str->alloc) / 2;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", str->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, str->text, sizeof(char) * str->alloc);
    // printf("atc-5\n");
    free(str->text);
    // printf("atc-6\n");
    str->text = newptr;
    // printf("atc-7\n");
    str->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  strncpy(str->text + str->len, text, n);
  str->len += n;
  str->text[str->len] = '\0';

  return 0;
}

int append_to_mc_strf(mc_str *str, const char *fmt, ...)
{
  // register_midge_error_tag("append_to_mc_strf()");
  // printf("atcs-0\n");
  int chunk_size = 4;
  int i = 0;

  va_list valist;
  va_start(valist, fmt);

  // printf("atcs-1\n");
  while (1) {
    if (str->len + chunk_size + 1 >= str->alloc) {
      unsigned int new_allocated_size = chunk_size + str->alloc + 16 + (chunk_size + str->alloc) / 2;
      // printf("atc-n3 : len:%u new_allocated_size:%u\n", str->len, new_allocated_size);
      char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
      // printf("atc-4\n");
      memcpy(newptr, str->text, sizeof(char) * str->alloc);
      // printf("atc-5\n");
      free(str->text);
      // printf("atc-6\n");
      str->text = newptr;
      // printf("atc-7\n");
      str->alloc = new_allocated_size;
      // printf("atc-8\n");
    }
    // printf("atcs-2\n");
    // sleep(1);

    // printf("'%c' chunk_size=%i str->len=%u\n", format[i], chunk_size, str->len);
    for (int a = 0; a < chunk_size; ++a) {
      str->text[str->len++] = fmt[i];
      // str->text[str->len] = '\0';
      // printf("str:'%s'\n", str->text);

      if (fmt[i] == '\0') {
        // printf("atcs-3\n");
        --str->len;
        va_end(valist);
        // printf("atcs-3r\n");
        return 0;
      }

      if (fmt[i] == '%') {
        if (fmt[i + 1] == '%') {
          // printf("atcs-4\n");
          // Use as an escape character
          ++i;
        }
        else {
          --str->len;
          // Search to replace the format
          ++i;
          // printf("atcs-5 i:%i i:'%c'\n", i, fmt[i]);
          switch (fmt[i]) {
          case 'i': {
            int value = va_arg(valist, int);

            // printf("atcs-5b value:'%i'\n", value);

            char buf[18];
            sprintf(buf, "%i", value);

            if (!strncmp(str->text, "int midge_initial", 17)) {
              // printf("atcs-5c str:'%s' :%i\n", str->text, str->len);
            }
            // printf("atcs-5d buf:'%s'\n", buf);
            // printf("atcs-5e\n");
            append_to_mc_str(str, buf);
            // printf("atcs-5f\n");
          } break;
          // case 'l': {
          //   switch (format[i + 1]) {
          //   case 'i': {
          //     ++i;
          //     long int value = va_arg(valist, long int);

          //     char buf[24];
          //     sprintf(buf, "%li", value);
          //     append_to_mc_str(str, buf);
          //   } break;
          //   default:
          //     MCerror(99, "TODO:l'%c'", format[i + 1]);
          //   }
          // } break;
          case 'p': {
            void *value = va_arg(valist, void *);

            char buf[18];
            sprintf(buf, "%p", value);
            append_to_mc_str(str, buf);
          } break;
          case 's': {
            char *value = va_arg(valist, char *);
            // printf("atcs-7a cstrtext:'%s' len:%u end:'%c'\n", str->text, str->len, str->text[str->len]);
            // printf("atcs-7b value:'%p'\n", value);
            // printf("atcs-7c value:'%s'\n", value);
            // str->text[i] = '\0';
            // --str->len;
            append_to_mc_str(str, value);
            // printf("atcs-7b cstrtext:'%s'\n", str->text);
          } break;
          case 'u': {
            unsigned int value = va_arg(valist, unsigned int);

            // printf("append_to_mc_strf-arg=%u\n", value);

            char buf[18];
            sprintf(buf, "%u", value);
            append_to_mc_str(str, buf);
          } break;
          default: {
            MCerror(99, "TODO:%c", fmt[i]);
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

int insert_into_mc_str(mc_str *str, const char *text, int index)
{
  if (index > str->len) {
    midge_error_print_thread_stack_trace();
    MCerror(4331, "TODO");
  }

  int n = strlen(text);
  if (n == 0) {
    return 0;
  }

  if (str->len + n + 1 >= str->alloc) {
    unsigned int new_allocated_size = str->alloc + n + 16 + (str->alloc) / 2;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", str->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    if (index) {
      memcpy(newptr, str->text, sizeof(char) * index);
    }
    memcpy(newptr + index, text, sizeof(char) * n);
    if (str->len - index) {
      memcpy(newptr + index + n, str->text + index, sizeof(char) * (str->len - index));
    }
    // printf("atc-5\n");
    free(str->text);
    // printf("atc-6\n");
    str->text = newptr;
    // printf("atc-7\n");
    str->alloc = new_allocated_size;
    str->len += n;
    str->text[str->len] = '\0';
    // printf("atc-8\n");
    return 0;
  }

  if (index != str->len) {
    memmove(str->text + index + n, str->text + index, sizeof(char) * (str->len - index));
  }
  memcpy(str->text + index, text, sizeof(char) * n);
  str->len += n;
  str->text[str->len] = '\0';

  return 0;
}

int remove_from_mc_str(mc_str *str, int index, int len)
{
  if (index < 0 || index >= str->len)
    return 0;

  if (index + len >= str->len) {
    str->len = index;
    str->text[str->len] = '\0';
    return 0;
  }

  for (int a = index + len; a < str->len; ++a) {
    str->text[a - len] = str->text[a];
  }
  str->len -= len;
  str->text[str->len] = '\0';

  return 0;
}

int restrict_mc_str(mc_str *str, int len)
{
  if (len > str->len)
    return 0;

  str->len = len;
  str->text[len] = '\0';

  return 0;
}

// TODO -- non-essential core method in a core file
int append_uppercase_to_mc_str(mc_str *str, const char *text)
{
  int len = strlen(text);
  if (str->len + len + 1 >= str->alloc) {
    // printf("atc-2\n");
    unsigned int new_allocated_size = str->alloc + len + 16 + (str->alloc) / 2;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", str->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, str->text, sizeof(char) * str->alloc);
    // printf("atc-5\n");
    free(str->text);
    // printf("atc-6\n");
    str->text = newptr;
    // printf("atc-7\n");
    str->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  char c;
  for (int i = 0; i < len; ++i) {
    c = text[i];
    // printf("%c-%i:", c, c); TODO -- strange bug when using the '_' character
    if (c >= 'a' || c <= 'z') {
      c -= 'a' - 'A';
    }
    str->text[str->len + i] = c;
  }
  str->len += len;
  str->text[str->len] = '\0';

  return 0;
}

// TODO -- non-essential core method in a core file
int append_lowercase_to_mc_str(mc_str *str, const char *text)
{
  int len = strlen(text);
  if (str->len + len + 1 >= str->alloc) {
    // printf("atc-2\n");
    unsigned int new_allocated_size = str->alloc + len + 16 + (str->alloc) / 2;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", str->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, str->text, sizeof(char) * str->alloc);
    // printf("atc-5\n");
    free(str->text);
    // printf("atc-6\n");
    str->text = newptr;
    // printf("atc-7\n");
    str->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  char c;
  for (int i = 0; i < len; ++i) {
    c = text[i];
    if (c >= 'A' || c <= 'Z') {
      c += 'a' - 'A';
    }
    str->text[str->len + i] = c;
  }
  str->len += len;
  str->text[str->len] = '\0';

  return 0;
}