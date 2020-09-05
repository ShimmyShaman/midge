
#include "midge_common.h"

void *midge_render_thread(void *vargp)
{
  render_thread_info *render_thread = (render_thread_info *)vargp;

  printf("~~midge_render_thread called!~~\n");

  render_thread->thread_info->has_concluded = 1;
  return NULL;
}