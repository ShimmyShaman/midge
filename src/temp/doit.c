#include "../dep/cglm/include/cglm/util.h"

#include <stdio.h>

static int dope(void) { return 7; }

int doit(void)
{
  puts("aaaaa");
  float fov = glm_rad(45.0f);
  printf("aaa:%i\n", dope());
  return 0;
};