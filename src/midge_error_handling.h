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

#define MCcall(function)                                                                   \
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

#define MCvacall(function)                                                                 \
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

#define MCerror(error_code, error_message, ...)                          \
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

const int MIDGE_ERROR_MAX_THREAD_COUNT = 50;
const int MIDGE_ERROR_STACK_MAX_SIZE = 250;
const int MIDGE_ERROR_STACK_MAX_FUNCTION_NAME_SIZE = 150;
const int MIDGE_ERROR_STACK_MAX_FILE_NAME_SIZE = 80;
const bool IGNORE_MIDGE_ERROR_STACK_TRACE = false;

struct __mce_stack_entry {
  char *function_name;
  char *file_name;
  int line;
};

struct __mce_thread_entry_stack {
  pthread_t thread_id;
  struct __mce_stack_entry stack[MIDGE_ERROR_STACK_MAX_SIZE];
  int stack_index;
  int stack_activity_line;
};

struct __mce_thread_entry_stack MIDGE_ERROR_THREAD_STACKS[MIDGE_ERROR_MAX_THREAD_COUNT];
unsigned int MIDGE_ERROR_THREAD_INDEX;

void register_midge_stack_invocation(const char *function_name, const char *file_name, int line,
                                     int *midge_error_stack_index)
{
  struct __mce_thread_entry_stack *threades = NULL;
  pthread_t tid = pthread_self();
  for (int t = 0; t < MIDGE_ERROR_MAX_THREAD_COUNT; ++t) {
    if (tid == MIDGE_ERROR_THREAD_STACKS[t].thread_id) {
      threades = &MIDGE_ERROR_THREAD_STACKS[t];
      break;
    }
    else if (tid == (pthread_t)0) {
      break;
    }
  }
  if (!threades) {
    printf("could not find stack for thread with id %lu\n", tid);
    return;
  }

  threades->stack_activity_line = -1;
  if (threades->stack_index + 1 >= MIDGE_ERROR_STACK_MAX_SIZE) {
    printf("MIDGE_ERROR_STACK: invocation of '%s' exceeded stack index\n", function_name);
    return;
  }
  // printf("MIDGE_ERROR_STACK: function_name:%s added\n", function_name);

  ++threades->stack_index;
  *midge_error_stack_index = threades->stack_index;

  for (int i = 0; i < MIDGE_ERROR_STACK_MAX_FUNCTION_NAME_SIZE; ++i) {
    threades->stack[threades->stack_index].function_name[i] = function_name[i];
    if (function_name[i] == '\0') {
      break;
    }
  }
  for (int i = 0; i < MIDGE_ERROR_STACK_MAX_FILE_NAME_SIZE; ++i) {
    threades->stack[threades->stack_index].file_name[i] = file_name[i];
    if (file_name[i] == '\0') {
      break;
    }
  }
  threades->stack[threades->stack_index].line = line;
}

void register_midge_stack_return(int midge_error_stack_index)
{
  struct __mce_thread_entry_stack *threades = NULL;
  pthread_t tid = pthread_self();
  for (int t = 0; t < MIDGE_ERROR_MAX_THREAD_COUNT; ++t) {
    if (tid == MIDGE_ERROR_THREAD_STACKS[t].thread_id) {
      threades = &MIDGE_ERROR_THREAD_STACKS[t];
      break;
    }
    else if (tid == (pthread_t)0) {
      break;
    }
  }
  if (!threades) {
    printf("could not find stack for thread with id %lu\n", tid);
    return;
  }

  threades->stack_activity_line = threades->stack[threades->stack_index].line;
  threades->stack_index = midge_error_stack_index - 1;
}

void register_midge_thread_creation(unsigned int *midge_error_thread_index, const char *base_function_name,
                                    const char *file_name, int line, int *midge_error_stack_index)
{
  if (MIDGE_ERROR_THREAD_INDEX >= MIDGE_ERROR_MAX_THREAD_COUNT) {
    printf("ERROR MIDGETHREAD 206\n");
    return;
  }
  if (MIDGE_ERROR_THREAD_STACKS[MIDGE_ERROR_THREAD_INDEX].thread_id != 0) {
    printf("ERROR MIDGETHREAD 203\n");
    return;
  }

  MIDGE_ERROR_THREAD_STACKS[MIDGE_ERROR_THREAD_INDEX].thread_id = pthread_self();

  printf("thread %lu begun\n", MIDGE_ERROR_THREAD_STACKS[MIDGE_ERROR_THREAD_INDEX].thread_id);

  *midge_error_thread_index = MIDGE_ERROR_THREAD_INDEX;
  ++MIDGE_ERROR_THREAD_INDEX;

  MIDGE_ERROR_THREAD_STACKS[MIDGE_ERROR_THREAD_INDEX].stack_index = -1;
  register_midge_stack_invocation(base_function_name, file_name, line, midge_error_stack_index);
}

void register_midge_thread_conclusion(unsigned int midge_error_thread_index)
{
  if (MIDGE_ERROR_THREAD_INDEX < 1) {
    printf("ERROR MIDGETHREAD 219\n");
    return;
  }

  printf("thread %lu concluded\n", MIDGE_ERROR_THREAD_STACKS[MIDGE_ERROR_THREAD_INDEX - 1].thread_id);

  --MIDGE_ERROR_THREAD_INDEX;
  MIDGE_ERROR_THREAD_STACKS[MIDGE_ERROR_THREAD_INDEX].thread_id = 0;
}

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

    for (int t = 0; t < MIDGE_ERROR_MAX_THREAD_COUNT; ++t) {
      struct __mce_thread_entry_stack *threades = &MIDGE_ERROR_THREAD_STACKS[t];
      if (threades->thread_id == (pthread_t)0) {
        break;
      }
      printf("\nThread-Id:%lu\n", threades->thread_id);

      for (int i = 0; i <= threades->stack_index; ++i) {
        printf("[%i]@%s :(file='%s' :%i)\n", i, threades->stack[i].function_name, threades->stack[i].file_name,
               threades->stack[i].line);
      }
      // if(MIDGE_ERROR_STACK_ACTIVITY_LINE >= 0)
      if (threades->stack_activity_line >= 0 && threades->stack_index + 1 < MIDGE_ERROR_STACK_MAX_SIZE) {

        printf("\nLast Successful Execution (file='%s') :%i\n", threades->stack[threades->stack_index + 1].file_name,
               threades->stack_activity_line);
      }
      else {
        printf("Last Successful Execution :%i\n", threades->stack_activity_line);
      }
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

  // Error Stacks
  for (int t = 0; t < MIDGE_ERROR_MAX_THREAD_COUNT; ++t) {
    MIDGE_ERROR_THREAD_STACKS[t].thread_id = (pthread_t)0;

    MIDGE_ERROR_THREAD_STACKS[t].stack_index = 0;
    for (int i = 0; i < MIDGE_ERROR_STACK_MAX_SIZE; ++i) {
      MIDGE_ERROR_THREAD_STACKS[t].stack[i].function_name =
          (char *)malloc(sizeof(char) * (MIDGE_ERROR_STACK_MAX_FUNCTION_NAME_SIZE + 1));
      MIDGE_ERROR_THREAD_STACKS[t].stack[i].function_name[0] = '\0';
      MIDGE_ERROR_THREAD_STACKS[t].stack[i].file_name =
          (char *)malloc(sizeof(char) * (MIDGE_ERROR_STACK_MAX_FILE_NAME_SIZE + 1));
      MIDGE_ERROR_THREAD_STACKS[t].stack[i].file_name[0] = '\0';
    }
  }
  // MIDGE_ERROR_STACK_INDEX = 0;
  // MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].function_name = (char *)malloc(sizeof(char) * (6 + 1));
  // sprintf(MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].function_name, "main()");
  // MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].file_name = (char *)malloc(sizeof(char) * (8 + 1));
  // sprintf(MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].file_name, "main.cpp");
  // MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX].line = 0;
}