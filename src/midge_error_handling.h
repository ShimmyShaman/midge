/* midge_error_handling */

// #include <cstring>
// #include <fstream>
// #include <iostream>
// #include <map>
// #include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
// #include <string>
// #include <time.h>
// #include <unistd.h>
// #include <vector>

#include <signal.h>

#include "cling/Interpreter/Interpreter.h"

#define MCcall(function)                                                              \
  {                                                                                        \
    int mc_error_stack_index;                                                              \
    register_midge_stack_invocation(#function, __FILE__, __LINE__, &mc_error_stack_index); \
    int mc_res = function;                                                                 \
    if (mc_res) {                                                                          \
      printf("--" #function "line:%i:ERR:%i\n", __LINE__, mc_res);                         \
      return mc_res;                                                                       \
    }                                                                                      \
    register_midge_stack_return(mc_error_stack_index);                                     \
  }

#define MCvacall(function)                                                            \
  {                                                                                        \
    int mc_error_stack_index;                                                              \
    register_midge_stack_invocation(#function, __FILE__, __LINE__, &mc_error_stack_index); \
    int mc_res = function;                                                                 \
    if (mc_res) {                                                                          \
      printf("-- line:%d varg-function-call:%i\n", __LINE__, mc_res);                      \
      return mc_res;                                                                       \
    }                                                                                      \
    register_midge_stack_return(mc_error_stack_index);                                     \
  }

#define MCerror(error_code, error_message, ...)                       \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return error_code;

bool IGNORE_MIDGE_ERROR_REPORT = false;

const int MIDGE_ERROR_TAG_MAX_SIZE = 20;
char *MIDGE_ERROR_TAG[MIDGE_ERROR_TAG_MAX_SIZE];
unsigned int MIDGE_ERROR_TAG_STR_LEN[MIDGE_ERROR_TAG_MAX_SIZE];
int MIDGE_ERROR_TAG_INDEX;
bool IGNORE_MIDGE_ERROR_TAG_REPORT = false;

int ensure_cstr_alloc(unsigned int *allocated_size, char **cstr, unsigned int min_alloc)
{
  if (min_alloc >= *allocated_size) {
    unsigned int new_allocated_size = min_alloc + 16 + *allocated_size / 10;
    printf("atc-3 : new_allocated_size:%u\n", new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, *cstr, sizeof(char) * *allocated_size);
    // printf("atc-5\n");
    free(*cstr);
    // printf("atc-6\n");
    *cstr = newptr;
    // printf("atc-7\n");
    *allocated_size = new_allocated_size;
    // printf("atc-8\n");
  }

  return 0;
}

void register_midge_error_tag(const char *fmt, ...)
{
  va_list valist;
  va_start(valist, fmt);

  int index = MIDGE_ERROR_TAG_INDEX % MIDGE_ERROR_TAG_MAX_SIZE;
  char *str = MIDGE_ERROR_TAG[index];
  uint *str_alloc = &MIDGE_ERROR_TAG_STR_LEN[index];
  ++MIDGE_ERROR_TAG_INDEX; // = (MIDGE_ERROR_TAG_INDEX + 1) % MIDGE_ERROR_TAG_MAX_SIZE;

  int si = 0;
  for (int i = 0;; ++i) {
    ensure_cstr_alloc(str_alloc, &str, si + 2);

    str[si++] = fmt[i];
    // printf("'c:'%c'\n", fmt[i]);

    if (fmt[i] == '\0') {
      // printf("atcs-3\n");
      va_end(valist);
      break;
    }

    if (fmt[i] != '%') {
      continue;
    }
    // printf("pass%%\n");

    ++i;
    if (fmt[i] == '%') {
      // Use as an escape character
      // printf("escaped\n");
      continue;
    }

    // Search to replace the fmt
    --si;
    switch (fmt[i]) {
    case 'i': {
      int value = va_arg(valist, int);

      char buf[18];
      sprintf(buf, "%i", value);
      ensure_cstr_alloc(str_alloc, &str, si + 18);
      strcpy(str + si, buf);
      si += strlen(buf);
    } break;
    case 'l': {
      switch (fmt[i + 1]) {
      case 'i': {
        ++i;
        long int value = va_arg(valist, long int);

        char buf[24];
        sprintf(buf, "%li", value);
        ensure_cstr_alloc(str_alloc, &str, si + 24);
        strcpy(str + si, buf);
        si += strlen(buf);
      } break;
      default:
        printf("ERROR register_midge_error_tag::l'%c'", fmt[i + 1]);
        return;
      }
    } break;
    case 'p': {
      void *value = va_arg(valist, void *);

      char buf[18];
      sprintf(buf, "%p", value);
      ensure_cstr_alloc(str_alloc, &str, si + 18);
      strcpy(str + si, buf);
      si += strlen(buf);
    } break;
    case 's': {
      char *value = va_arg(valist, char *);
      // printf("atcs-7a cstrtext:'%s' len:%u end:'%c'\n", cstr->text, cstr->len, cstr->text[cstr->len]);
      // printf("atcs-7a value:'%s'\n", value);
      // cstr->text[i] = '\0';
      // --cstr->len;
      int value_len = strlen(value);
      ensure_cstr_alloc(str_alloc, &str, si + value_len + 1);
      strcpy(str + si, value);
      si += value_len + 0;
      // printf("atcs-7b cstrtext:'%s'\n", cstr->text);
    } break;
    case 'u': {
      unsigned int value = va_arg(valist, unsigned int);

      char buf[18];
      sprintf(buf, "%u", value);
      ensure_cstr_alloc(str_alloc, &str, si + 18);
      strcpy(str + si, buf);
      si += strlen(buf);
    } break;
    default: {
      printf("ERROR register_midge_error_tag::l'%c'", fmt[i + 1]);
      return;
    }
    }
  }
}

struct stack_entry {
  char *function_name;
  char *file_name;
  int line;
};

const int MIDGE_ERROR_STACK_MAX_SIZE = 250;
const int MIDGE_ERROR_STACK_MAX_FUNCTION_NAME_SIZE = 150;
const int MIDGE_ERROR_STACK_MAX_FILE_NAME_SIZE = 80;
struct stack_entry MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_MAX_SIZE];
unsigned int MIDGE_ERROR_STACK_STR_LEN[MIDGE_ERROR_STACK_MAX_SIZE];
int MIDGE_ERROR_STACK_INDEX;
bool IGNORE_MIDGE_ERROR_STACK_TRACE = false;

void register_midge_stack_invocation(const char *function_name, const char *file_name, int line,
                                     int *midge_error_stack_index)
{
  if (MIDGE_ERROR_STACK_INDEX + 1 >= MIDGE_ERROR_STACK_MAX_SIZE) {
    printf("MIDGE_ERROR_STACK: invocation of '%s' exceeded stack index\n", function_name);
    return;
  }
  // printf("MIDGE_ERROR_STACK: function_name:%s added\n", function_name);

  ++MIDGE_ERROR_STACK_INDEX;
  *midge_error_stack_index = MIDGE_ERROR_STACK_INDEX;

  for (int i = 0; i < MIDGE_ERROR_STACK_MAX_FUNCTION_NAME_SIZE; ++i) {
    MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].function_name[i] = function_name[i];
    if (function_name[i] == '\0') {
      break;
    }
  }
  for (int i = 0; i < MIDGE_ERROR_STACK_MAX_FILE_NAME_SIZE; ++i) {
    MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].file_name[i] = file_name[i];
    if (file_name[i] == '\0') {
      break;
    }
  }
  MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].line = line;
}

void register_midge_stack_return(int midge_error_stack_index) { MIDGE_ERROR_STACK_INDEX = midge_error_stack_index - 1; }

void handler(int sig)
{
  if (IGNORE_MIDGE_ERROR_REPORT) {
    return;
  }

  printf("\n===========================================\n"
         "\n===========================================\n"
         "              Illegal Memory Access\n"
         "-------------------------------------------\n\n");
  if (!IGNORE_MIDGE_ERROR_TAG_REPORT) {
    // #ifdef ME_TAG_RECENT_LAST
    printf("\n---------------################------------\n");
    printf("---------------   Tag Report   ------------\n");
    printf("---------------Most Recent Last------------\n\n");
    for (int i = MIDGE_ERROR_TAG_MAX_SIZE; i > 0; --i) {
      // #else
      // printf("---------------Most Recent First------------\n\n");
      // for (int i = 1; i <= MIDGE_ERROR_TAG_MAX_SIZE; ++i) {
      // #endif
      if (MIDGE_ERROR_TAG_INDEX - i < 0) {
        continue;
      }
      int t = (MIDGE_ERROR_TAG_INDEX - i) % MIDGE_ERROR_TAG_MAX_SIZE;
      if (!strlen(MIDGE_ERROR_TAG[t])) {
        continue;
      }

      printf("-[%i]-'%s'\n", MIDGE_ERROR_TAG_INDEX - i, MIDGE_ERROR_TAG[t]);
    }
  }

  if (!IGNORE_MIDGE_ERROR_STACK_TRACE) {
    printf("\n---------------################------------\n");
    printf("---------------  Stack Trace   ------------\n");
    printf("---------------Most Recent Last------------\n\n");
    // printf("size:%i\n", MIDGE_ERROR_STACK_INDEX);
    for (int i = 0; i <= MIDGE_ERROR_STACK_INDEX; ++i) {
      printf("[%i]:>%s :(file='%s'line=%i)\n", i, MIDGE_ERROR_STACK[i].function_name, MIDGE_ERROR_STACK[i].file_name,
             MIDGE_ERROR_STACK[i].line);
    }
  }

  //   // prints array to std error after converting array to
  //   // human-readable strings
  //   // backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(0);
}

void initialize_midge_error_handling(cling::Interpreter *clint)
{
  signal(SIGSEGV, handler); // register our handler
  MIDGE_ERROR_TAG_INDEX = 0;
  for (int i = 0; i < MIDGE_ERROR_TAG_MAX_SIZE; ++i) {
    MIDGE_ERROR_TAG_STR_LEN[i] = 512U;
    MIDGE_ERROR_TAG[i] = (char *)malloc(sizeof(char) * MIDGE_ERROR_TAG_STR_LEN[i]);
    MIDGE_ERROR_TAG[i][0] = '\0';
  }

  // char buf[512];
  // sprintf(buf, "void (*register_midge_error_tag)(const char *, ...) = (void (*)(const char *, ...))%p;",
  //         &register_midge_error_tag);
  // clint->process(buf);

  // sprintf(buf,
  //         "void (*register_midge_stack_invocation)(const char *, int, int, int *) = (void (*)(const char *, int, int, "
  //         "int*))%p;",
  //         &register_midge_stack_invocation);
  // clint->process(buf);
  // sprintf(buf, "void (*register_midge_stack_return)(int) = (void (*)(int))%p;", &register_midge_stack_return);
  // clint->process(buf);

  for (int i = 0; i < MIDGE_ERROR_STACK_MAX_SIZE; ++i) {
    MIDGE_ERROR_STACK[i].function_name = (char *)malloc(sizeof(char) * (MIDGE_ERROR_STACK_MAX_FUNCTION_NAME_SIZE + 1));
    MIDGE_ERROR_STACK[i].function_name[0] = '\0';
    MIDGE_ERROR_STACK[i].file_name = (char *)malloc(sizeof(char) * (MIDGE_ERROR_STACK_MAX_FILE_NAME_SIZE + 1));
    MIDGE_ERROR_STACK[i].file_name[0] = '\0';
  }

  MIDGE_ERROR_STACK_INDEX = 0;
  MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].function_name = (char *)malloc(sizeof(char) * (6 + 1));
  sprintf(MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].function_name, "main()");
  MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].file_name = (char *)malloc(sizeof(char) * (8 + 1));
  sprintf(MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].file_name, "main.cpp");
  MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].line = 0;
}