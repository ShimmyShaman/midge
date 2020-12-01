#ifndef mc_str_H
#define mc_str_H

#include <stdbool.h>

typedef struct mc_str {
  unsigned int alloc;
  unsigned int len;
  char *text;
} mc_str;

int init_mc_str(mc_str **ptr);
int init_mc_str_with_specific_capacity(mc_str **ptr, unsigned int specific_capacity);
int set_mc_str(mc_str *cstr, const char *text);
int set_mc_strn(mc_str *cstr, const char *text, int len);
int release_mc_str(mc_str *ptr, bool free_char_string_also);
int append_char_to_mc_str(mc_str *cstr, char c);
int append_to_mc_str(mc_str *cstr, const char *text);
int append_to_mc_strn(mc_str *cstr, const char *text, int n);
int append_to_mc_strf(mc_str *cstr, const char *fmt, ...);
int insert_into_mc_str(mc_str *cstr, const char *text, int index);
int remove_from_mc_str(mc_str *cstr, int index, int len);

/* Reduce a string to a certain length. Nothing occurs if string is already at or smaller than len */
int restrict_mc_str(mc_str *cstr, int len);

#endif // mc_str_H
