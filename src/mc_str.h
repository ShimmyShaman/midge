#ifndef MC_STR_H
#define MC_STR_H

#include <stdbool.h>

typedef struct mc_str {
  unsigned int alloc;
  unsigned int len;
  char *text;
} mc_str;

int mc_init_str(mc_str *ptr, int specific_capacity);
int mc_alloc_str(mc_str **ptr);
int mc_alloc_str_with_specific_capacity(mc_str **ptr, int specific_capacity);
int mc_set_str(mc_str *str, const char *text);
int mc_set_strf(mc_str *str, const char *fmt, ...);
int mc_set_strn(mc_str *str, const char *text, int len);
int mc_append_char_to_str(mc_str *str, char c);
int mc_append_to_str(mc_str *str, const char *text);
int mc_append_to_strn(mc_str *str, const char *text, int n);
int mc_append_to_strf(mc_str *str, const char *fmt, ...);
int mc_insert_into_str(mc_str *str, const char *text, int index);
int mc_remove_from_str(mc_str *str, int index, int len);
void mc_release_str(mc_str *ptr, bool free_char_string_also);

/* Reduce a string to a certain length. Nothing occurs if string is already at or smaller than len */
int mc_restrict_str(mc_str *str, int len);
int mc_append_uppercase_to_str(mc_str *str, const char *text);
int mc_append_lowercase_to_str(mc_str *str, const char *text);

#endif // MC_STR_H
