#ifndef MC_STR_H
#define MC_STR_H

#include <stdbool.h>

typedef struct mc_str {
  unsigned int alloc;
  unsigned int len;
  char *text;
} mc_str;

int init_mc_str(mc_str **ptr);
int init_mc_str_with_specific_capacity(mc_str **ptr, unsigned int specific_capacity);
int set_mc_str(mc_str *str, const char *text);
int set_mc_strf(mc_str *str, const char *fmt, ...);
int set_mc_strn(mc_str *str, const char *text, int len);
int append_char_to_mc_str(mc_str *str, char c);
int append_to_mc_str(mc_str *str, const char *text);
int append_to_mc_strn(mc_str *str, const char *text, int n);
int append_to_mc_strf(mc_str *str, const char *fmt, ...);
int insert_into_mc_str(mc_str *str, const char *text, int index);
int remove_from_mc_str(mc_str *str, int index, int len);
void release_mc_str(mc_str *ptr, bool free_char_string_also);

/* Reduce a string to a certain length. Nothing occurs if string is already at or smaller than len */
int restrict_mc_str(mc_str *str, int len);
int append_uppercase_to_mc_str(mc_str *str, const char *text);
int append_lowercase_to_mc_str(mc_str *str, const char *text);

#endif // MC_STR_H
