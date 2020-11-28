/* m_threads.h */

#ifndef M_THREADS_H
#define M_THREADS_H

#include <stdbool.h>
#include <pthread.h>

typedef struct mthread_info {
  pthread_t threadId;
  void *(*start_routine)(void *);
  int should_exit, has_concluded;
  int should_pause, has_paused;
} mthread_info;

// TODO -- find another home for this method?
void *_mca_thread_entry_wrap(void *state);

int begin_mthread(void *(*start_routine)(void *), mthread_info **p_thread_info, void *state);

int pause_mthread(mthread_info *p_thread_info, bool blocking);

/*
 * Holds the given mthread. Intended to be called by the thread which has been signaled to pause.
 * @returns whether the thread since pausing has been signalled to exit (should_exit).
 */
int hold_mthread(mthread_info *p_thread_info);

int unpause_mthread(mthread_info *p_thread_info, bool blocking);

int end_mthread(mthread_info *p_thread_info);

#endif // M_THREADS_H