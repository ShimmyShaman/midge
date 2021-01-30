/* file_persistence.c */
#include "core/midge_core.h"

int find_in_str(char *str, char *seq)
{
  printf("fis-0\n");
  printf("str:'%s'\n", str);
  printf("seq:'%s'\n", seq);
  int len = strlen(seq);
  if (len < 1) {
    // ERR(ERROR_ARGUMENT, "seq length must be greater than 0");
    printf("TODO ERROR HANDLING\n");
  }

  printf("fis-1\n");
  for (int a = 0;; ++a) {
    if (str[a] != seq[0]) {
      if (str[a] == '\0') {
        break;
      }
      continue;
    }

    bool found = true;
    for (int b = 1; b < len; ++b) {
      if (str[a + b] != seq[b]) {
        found = false;
        break;
      }
    }
    if (found) {
      // Sequence matches
      printf("fis- return:%i\n", a);
      print_parse_error(str, a, "find_in_str", "successful_return");
      return a;
    }
  }
  printf("fis- return:-1\n");

  return -1;
}