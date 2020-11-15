/* midge_common.h */

#ifndef MIDGE_COMMON_H
#define MIDGE_COMMON_H

#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true ((unsigned char)0x7F)
#endif
#ifndef false
#define false ((unsigned char)0)
#endif

// int mcc_interpret_and_execute_single_use_code(const char *filename, const char *contents);
// int mcc_interpret_file_contents(const char *filename, const char *contents);
// int mcc_interpret_file_on_disk(const char *filepath);
// int mcc_interpret_files_in_block(const char **files, int nb_files);
// void *mcc_set_global_symbol(const char *symbol_name, void *ptr);
// void *mcc_get_global_symbol(const char *symbol_name);

#define MCerror(error_code, error_message, ...)                          \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return error_code;

#define MC_ASSERT(condition, message)                        \
  if (!(condition)) {                                        \
    printf("MC-ASSERT ERR[" #condition "] :" #message "\n"); \
    return -37373;                                           \
  }

// TEMP will have to do function defines sooner or later
#define VK_CHECK(res, func_name)                               \
  if (res) {                                                   \
    printf("VK-ERR[%i] :%i --" func_name "\n", res, __LINE__); \
    return res;                                                \
  }
#define VK_ASSERT(condition, message)                        \
  if (!(condition)) {                                        \
    printf("VK-ASSERT ERR[" #condition "] :" #message "\n"); \
    return -11111;                                           \
  }

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

// struct bitty;
// struct bitty {
//   int a;
//   struct bitty *b;
// };
// typedef struct bitty bitty;

typedef struct c_str {
  unsigned int alloc;
  unsigned int len;
  char *text;
} c_str;

int init_c_str(c_str **ptr);
int init_c_str_with_specific_capacity(c_str **ptr, unsigned int specific_capacity);
int set_c_str(c_str *cstr, const char *text);
int set_c_strn(c_str *cstr, const char *text, int len);
int release_c_str(c_str *ptr, bool free_char_string_also);
int append_char_to_c_str(c_str *cstr, char c);
int append_to_c_str(c_str *cstr, const char *text);
int append_to_c_strn(c_str *cstr, const char *text, int n);
int append_to_c_strf(c_str *cstr, const char *format, ...);
int insert_into_c_str(c_str *cstr, const char *text, int index);
int restrict_c_str(c_str *cstr, int len);
#endif // MIDGE_COMMON_H