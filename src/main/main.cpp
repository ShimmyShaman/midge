/* main.cpp */

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

#include <signal.h>

#include "cling/Interpreter/Interpreter.h"

using namespace std;

cling::Interpreter *clint;
const int MIDGE_ERROR_STACK_MAX_SIZE = 10;
char *MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_MAX_SIZE];
unsigned int MIDGE_ERROR_STACK_STR_LEN[MIDGE_ERROR_STACK_MAX_SIZE];
int MIDGE_ERROR_STACK_INDEX;

void handler(int sig)
{
  // size_t nStackTraces = 20; //number of backtraces you want at most
  // void *array[nStackTraces];
  // size_t size;
  // fills array and returns actual number of backtraces at the moment
  // size = backtrace(array, nStackTraces);
  printf("\n===========================================\n"
         "\n===========================================\n"
         "              CATASTROPHIC ERROR\n"
         "-------------------------------------------\n\n"
         "---------------Most Recent Last------------\n\n");

  for (int i = 0; i < MIDGE_ERROR_STACK_MAX_SIZE; ++i) {
    int t = (MIDGE_ERROR_STACK_INDEX + i) % MIDGE_ERROR_STACK_MAX_SIZE;
    if (!strlen(MIDGE_ERROR_STACK[t])) {
      continue;
    }

    printf("-%u-'%s'\n", MIDGE_ERROR_STACK_STR_LEN[t], MIDGE_ERROR_STACK[t]);
  }

  // prints array to std error after converting array to
  // human-readable strings
  // backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(0);
}

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

  char *str = MIDGE_ERROR_STACK[MIDGE_ERROR_STACK_INDEX];
  uint *str_alloc = &MIDGE_ERROR_STACK_STR_LEN[MIDGE_ERROR_STACK_INDEX];
  MIDGE_ERROR_STACK_INDEX = (MIDGE_ERROR_STACK_INDEX + 1) % MIDGE_ERROR_STACK_MAX_SIZE;

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

int main(int argc, const char *const *argv)
{
  signal(SIGSEGV, handler); // register our handler
  MIDGE_ERROR_STACK_INDEX = 0;
  for (int i = 0; i < MIDGE_ERROR_STACK_MAX_SIZE; ++i) {
    MIDGE_ERROR_STACK_STR_LEN[i] = 512U;
    MIDGE_ERROR_STACK[i] = (char *)malloc(sizeof(char) * MIDGE_ERROR_STACK_STR_LEN[i]);
    MIDGE_ERROR_STACK[i][0] = '\0';
  }

  // char buffer[200];
  // getcwd(buffer, 200);
  const char *LLVMDIR = "/home/jason/cling/inst";
  clint = new cling::Interpreter(argc, argv, LLVMDIR);

  clint->AddIncludePath("/home/jason/midge/src");
  clint->AddIncludePath("/home/jason/cling/inst/include");

  clint->loadFile("/home/jason/midge/src/midge.h");
  char buf[512];
  sprintf(buf, "clint = (cling::Interpreter *)%p;", (void *)clint);
  clint->process(buf);
  sprintf(buf, "void (*register_midge_error_tag)(const char *, ...) = (void (*)(const char *, ...))%p;",
          &register_midge_error_tag);
  clint->process(buf);

  clint->process("run()");

  delete (clint);

  usleep(100000);
}