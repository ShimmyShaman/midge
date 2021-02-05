/* mc_error_handling */

#ifndef MC_ERROR_HANDLING_H
#define MC_ERROR_HANDLING_H

#ifdef MC_TEMP_SOURCE_LOAD
#define MCcall(function)                                                                   \
  {                                                                                        \
    int mc_error_stack_index;                                                              \
    register_midge_stack_invocation(#function, __FILE__, __LINE__, &mc_error_stack_index); \
    int mc_res = function;                                                                 \
    if (mc_res) {                                                                          \
      printf("-]" #function "line:%i:ERR:%i\n", __LINE__, mc_res);                         \
      return mc_res;                                                                       \
    }                                                                                      \
    register_midge_stack_return(mc_error_stack_index);                                     \
  }
#else
#define MCcall(function)                                           \
  {                                                                \
    int mc_res = function;                                         \
    if (mc_res) {                                                  \
      printf("-]" #function "line:%i:ERR:%i\n", __LINE__, mc_res); \
      return mc_res;                                               \
    }                                                              \
  }
#endif

#define MCVerror(error_code, error_message, ...)                         \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return;

#define MCerror(error_code, error_message, ...)                          \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return error_code;

#define MCassert(condition, message)                       \
  if (!(condition)) {                                      \
    printf("ASSERT FAIL[" #condition "] :" #message "\n"); \
    return -37373;                                         \
  }

#define IGNORE_MIDGE_ERROR_REPORT 0

#define IGNORE_MIDGE_ERROR_TAG_REPORT 0
#define MIDGE_ERROR_TAG_MAX_SIZE 20

void register_midge_error_tag(const char *fmt, ...);

#define MIDGE_ERROR_MAX_THREAD_COUNT 50
#define MIDGE_ERROR_STACK_MAX_SIZE 250
#define MIDGE_ERROR_STACK_MAX_FUNCTION_NAME_SIZE 150
#define MIDGE_ERROR_STACK_MAX_FILE_NAME_SIZE 80
#define IGNORE_MIDGE_ERROR_STACK_TRACE 0

void register_midge_stack_function_entry(const char *function_name, const char *file_name, int line,
                                         int *midge_error_stack_index);
void register_midge_stack_invocation(const char *function_name, const char *file_name, int line,
                                     int *midge_error_stack_index);
void register_midge_stack_return(int midge_error_stack_index);
void register_midge_thread_creation(unsigned int *midge_error_thread_index, const char *base_function_name,
                                    const char *file_name, int line, const char *thread_name,
                                    int *midge_error_stack_index);
void register_midge_thread_conclusion(unsigned int midge_error_thread_index);

void midge_error_print_thread_stack_trace();
void midge_error_print_thread_info();
void midge_error_set_thread_name(const char *name);
void initialize_mc_error_handling();

#endif // MC_ERROR_HANDLING_H
