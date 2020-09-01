/* m_threads.h */

#ifndef M_THREADS_H
#define M_THREADS_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct mthread_info {
  pthread_t threadId;
  void *(*start_routine)(void *);
  int should_exit, has_concluded;
  int should_pause, has_paused;
} mthread_info;

// void *mthread_wrapper_delegate(void *ws)
// {
//   void **args = (void **)ws;
//   printf("boop\n");
//   printf("bap %p\n", args[0]);
//   void *(*start_routine)(void *) = (void *(*)(void *))args[0];
//   printf("bop\n");
//   mthread_info *thread_info = (mthread_info *)args[1];
//   printf("boom\n");
//   void *state = args[2];

//   printf("bum\n");
//   void *result;
//   if (!thread_info->should_exit) {
//   printf("bim %p\n", start_routine);
//     result = start_routine(state);
//   }

//   printf("bam\n");
//   thread_info->has_concluded = 1;

//   return result;
// }

// int begin_mthread(void *(*start_routine)(void *), mthread_info **p_thread_info, void *state)
// {
//   printf("bim %p\n", start_routine);
//   *p_thread_info = (mthread_info *)malloc(sizeof *p_thread_info);
//   (*p_thread_info)->start_routine = start_routine;

//   (*p_thread_info)->should_exit = 0;
//   (*p_thread_info)->has_concluded = 0;
//   (*p_thread_info)->should_pause = 0;
//   (*p_thread_info)->has_paused = 0;

//   void *vargs[3];
//   vargs[0] = (void *)start_routine;
//   printf("bap %p\n", vargs[0]);
//   vargs[1] = (void *)(*p_thread_info);
//   vargs[2] = (void *)state;

//   if (pthread_create(&(*p_thread_info)->threadId, NULL, mthread_wrapper_delegate, (void *)vargs)) {
//     return 0;
//   }
//   return -1;
// }

int begin_mthread(void *(*start_routine)(void *), mthread_info **p_thread_info, void *state)
{
  printf("begin_mthread()\n");
  *p_thread_info = (mthread_info *)malloc(sizeof(mthread_info));
  (*p_thread_info)->start_routine = start_routine;

  (*p_thread_info)->should_exit = 0;
  (*p_thread_info)->has_concluded = 0;
  (*p_thread_info)->should_pause = 0;
  (*p_thread_info)->has_paused = 0;
  int result = pthread_create(&(*p_thread_info)->threadId, NULL, (*p_thread_info)->start_routine, state);
  if (result) {
    printf("begin_mthread SUCCESS\n");
    return 0;
  }
  printf("begin_mthread FAILURE TO START THREAD\n");
  return -1;
}

int pause_mthread(mthread_info *p_thread_info, bool blocking)
{
  p_thread_info->should_pause = 1;

  if (blocking) {
    const int MAX_ITERATIONS = 2000000;
    int iterations = 0;
    while (!p_thread_info->has_paused && !p_thread_info->has_concluded) {
      usleep(1);
      ++iterations;
      if (iterations >= MAX_ITERATIONS) {
        printf("TODO -- Thread-Handling for unresponsive thread:: \n");
        return -1;
      }
    }
  }

  return 0;
}

/*
 * Holds the given mthread. Intended to be called by the thread which has been signaled to pause.
 * @returns whether the thread since pausing has been signalled to exit (should_exit).
 */
int hold_mthread(mthread_info *p_thread_info)
{
  p_thread_info->has_paused = 1;
  while (p_thread_info->should_pause)
    usleep(1);
  p_thread_info->has_paused = 0;

  return p_thread_info->should_exit;
}

int unpause_mthread(mthread_info *p_thread_info, bool blocking)
{
  p_thread_info->should_pause = 0;

  if (blocking) {
    const int MAX_ITERATIONS = 2000000;
    int iterations = 0;
    while (p_thread_info->has_paused && !p_thread_info->has_concluded) {
      usleep(1);
      ++iterations;
      if (iterations >= MAX_ITERATIONS) {
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

  printf("end_mthread:0\n");
  const int MAX_ITERATIONS = 20000;
  int iterations = 0;
  while (!p_thread_info->has_concluded) {
    usleep(100);
    ++iterations;
    if (iterations >= MAX_ITERATIONS) {
      printf("TODO -- Thread-Handling for unresponsive thread:: \n");
      return -1;
    }
  }
  printf("end_mthread:1\n");

  free(p_thread_info);

  return 0;
}

#endif