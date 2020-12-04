/* platform_prereq.c */

#include <stdio.h>

struct __mc_test_platform_struct {
  int a;
  char *c;
};

int __mc_test_dummy_function(void) { return 0; }

int __mc_test_platform(void)
{
  if (sizeof(void *) != sizeof(char *) || sizeof(void *) != sizeof(int *) ||
      sizeof(void *) != sizeof(struct __mc_test_platform_struct *) ||
      sizeof(void *) != sizeof(&__mc_test_dummy_function)) {
    puts("ERROR various pointer type sizes are not equal. This is a 'feature' that is utilized heavily in this "
         "application.");
    return 1;
  }

  return 0;
}