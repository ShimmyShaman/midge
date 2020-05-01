/* m_threads.h */

#ifndef M_THREADS_H
#define M_THREADS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct
{
  pthread_t threadId;
  void *(*start_routine)(void *);
  bool should_exit, has_concluded;
  bool do_pause, has_paused;
} mthread_info;

int begin_mthread(void *(*start_routine)(void *), mthread_info **p_thread_info, void **vargs)
{
  *p_thread_info = (mthread_info *)malloc(sizeof *p_thread_info);
  (*p_thread_info)->start_routine = start_routine;

  (*p_thread_info)->should_exit = 0;
  (*p_thread_info)->has_concluded = 0;
  (*p_thread_info)->do_pause = 0;
  (*p_thread_info)->has_paused = 0;
  if (pthread_create(&(*p_thread_info)->threadId, NULL, (*p_thread_info)->start_routine, vargs))
  {
    return 0;
  }
  return -1;
}

int pause_mthread(mthread_info *p_thread_info, bool blocking)
{
  p_thread_info->do_pause = 1;

  if (blocking)
  {
    const int MAX_ITERATIONS = 2000000;
    int iterations = 0;
    while (!p_thread_info->has_paused && !p_thread_info->has_concluded)
    {
      usleep(1);
      ++iterations;
      if (iterations >= MAX_ITERATIONS)
      {
        printf("TODO -- Thread-Handling for unresponsive thread:: \n");
        return -1;
      }
    }
  }

  return 0;
}

int unpause_mthread(mthread_info *p_thread_info, bool blocking)
{
  p_thread_info->do_pause = 0;

  if (blocking)
  {
    const int MAX_ITERATIONS = 2000000;
    int iterations = 0;
    while (p_thread_info->has_paused && !p_thread_info->has_concluded)
    {
      usleep(1);
      ++iterations;
      if (iterations >= MAX_ITERATIONS)
      {
        printf("TODO -- Thread-Handling for unresponsive thread:: \n");
        return -1;
      }
    }
  }

  return 0;
}

int end_mthread(mthread_info *p_thread_info)
{
  p_thread_info->should_exit = 1;

  const int MAX_ITERATIONS = 20000;
  int iterations = 0;
  while (!p_thread_info->has_concluded)
  {
    usleep(100);
    ++iterations;
    if (iterations >= MAX_ITERATIONS)
    {
      printf("TODO -- Thread-Handling for unresponsive thread:: \n");
      return -1;
    }
  }

  free(p_thread_info);

  return 0;
}

#endif