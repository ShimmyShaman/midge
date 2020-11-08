/* midge_error_handling */

#ifndef MIDGE_ERROR_HANDLING_H
#define MIDGE_ERROR_HANDLING_H

// #include <pthread.h>
// #include <signal.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
// // #include "p_threa"
// #include "midge_common.h"

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

#define IGNORE_MIDGE_ERROR_REPORT 0

#define IGNORE_MIDGE_ERROR_TAG_REPORT false
#define MIDGE_ERROR_TAG_MAX_SIZE 20

void register_midge_error_tag(const char *fmt, ...);

#define MIDGE_ERROR_MAX_THREAD_COUNT 50
#define MIDGE_ERROR_STACK_MAX_SIZE 250
#define MIDGE_ERROR_STACK_MAX_FUNCTION_NAME_SIZE 150
#define MIDGE_ERROR_STACK_MAX_FILE_NAME_SIZE 80
#define IGNORE_MIDGE_ERROR_STACK_TRACE false

void register_midge_stack_invocation(const char *function_name, const char *file_name, int line,
                                     int *midge_error_stack_index);

void register_midge_stack_return(int midge_error_stack_index);

void register_midge_thread_creation(unsigned int *midge_error_thread_index, const char *base_function_name,
                                    const char *file_name, int line, int *midge_error_stack_index);

void register_midge_thread_conclusion(unsigned int midge_error_thread_index);

void midge_error_print_thread_stack_trace();

void initialize_midge_error_handling();
#endif // MIDGE_ERROR_HANDLING_H
