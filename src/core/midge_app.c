/* midge_app.c */

#include "core_definitions.h"
#include "render/render_thread.h"
#include <time.h>

int callit(int a, int b, int c, int *result)
{
  *result = a + b * c;
  printf("!!callit-mc:  a:%i + b:%i * c:%i = %i\n", a, b, c, *result);
  return 0;
}

int dothecall(void *something)
{
  int (*rout)() = (int (*)())something;
  printf("dothecall\n");
  rout();
  printf("dothecall-after-rout-call\n");
  return 0;
}

// int begin_silly_thread()
// {
//   printf("begin_silly_thread\n");
//   dothecall(&callit);
//   // dothecall(callit); // TODO -- C implicity converts the function so its the same as the above line, this doesn't
//   // work atm in midge...

//   return 0;
// }

int begin_render_thread()
{
  render_thread_info render_thread;
  render_thread.render_thread_initialized = false;
  {
    // Resource Queue
    pthread_mutex_init(&render_thread.resource_queue.mutex, NULL);
    render_thread.resource_queue.count = 0;
    render_thread.resource_queue.allocated = 0;

    // Render Queue
    pthread_mutex_init(&render_thread.render_queue.mutex, NULL);
    render_thread.render_queue.count = 0;
    render_thread.render_queue.allocated = 0;

    pthread_mutex_init(&render_thread.input_buffer.mutex, NULL);
    render_thread.input_buffer.event_count = 0;
  }

  // -- Start Thread
  dothecall(&callit);
  // printf("callit:%p &callit:%p\n", callit, &callit);

  // begin_mthread(&callit, &render_thread.thread_info, (void *)&render_thread);

  // usleep(1000000);

  // end_mthread(render_thread.thread_info);
}

void midge_initialize_app(struct timespec *app_begin_time)
{
  struct timespec source_load_complete_time;
  clock_gettime(CLOCK_REALTIME, &source_load_complete_time);

  printf("#######################\n  <<<< MIDGE >>>>\n\nCore Compile took %.2f seconds\n",
         source_load_complete_time.tv_sec - app_begin_time->tv_sec +
             1e-9 * (source_load_complete_time.tv_nsec - app_begin_time->tv_nsec));

  // Asynchronously begin the render thread containing vulkan and xcb
  begin_render_thread();
  // begin_silly_thread();
}

void midge_run_app() { printf("~midge_run_app()~\n"); }

void midge_cleanup_app() { printf("~midge_cleanup_app()~\n"); }