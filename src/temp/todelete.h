#include "midge_error_handling.h"

/* m_threads.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "midge_error_handling.h"

#include "m_threads.h"

void *_mca_thread_entry_wrap(void *state) {
  unsigned int  mc_error_thread_index;
  int  base_error_stack_index;
  register_midge_thread_creation(&mc_error_thread_index, "_mca_thread_entry_wrap", "m_threads.c", 13, &base_error_stack_index);

  void ** state_args = (void **)state;
  void *(*mc_routine)(void *) = (/*!*/void *(*)(void *))state_args[0];
  void * wrapped_state = state_args[1];

  // printf("mc_routine ptr:%p\n", mc_routine);
  // printf("mc_routine deref ptr:%p\n", *mc_routine);

  const char * fn_name = "TODO-find-by-ptr?";
  void * thread_res;
{
    int  mc_error_stack_index;
    register_midge_stack_invocation(fn_name, __FILE__, __LINE__ + 1, &mc_error_stack_index);
    thread_res = mc_routine(wrapped_state);
    // if (mc_res) {
    //   printf("--unknown-thread-start-function: line:%i: ERR:%i\n", __LINE__ - 2, mc_res);
    //   return NULL;
    // }
    register_midge_stack_return(mc_error_stack_index);
  }

  // printf("routine called\n");

  register_midge_thread_conclusion(mc_error_thread_index);
  // return routine_result;

  {
    // Return
    return thread_res;
  }

}


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

int begin_mthread(void *(*start_routine)(void *), mthread_info **p_thread_info, void *state) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("begin_mthread", __FILE__, __LINE__, &midge_error_stack_index);

  *p_thread_info = (mthread_info *)malloc(sizeof(mthread_info));
  (*p_thread_info)->start_routine = start_routine;

  (*p_thread_info)->should_exit = 0;
  (*p_thread_info)->has_concluded = 0;
  (*p_thread_info)->should_pause = 0;
  (*p_thread_info)->has_paused = 0;
  int result;
  {
    void **mcti_wrapper_state = (void **)malloc(sizeof(void *) * 2);
    mcti_wrapper_state[0] = (void *)(*p_thread_info)->start_routine;
    mcti_wrapper_state[1] = (void *)state;
    result = pthread_create(&(*p_thread_info)->threadId, NULL, _mca_thread_entry_wrap, (void *)mcti_wrapper_state);
  }

  if (!result) {

    {
      // Return
      register_midge_stack_return(midge_error_stack_index);
      return 0;
    }

  }

  printf("begin_mthread FAILURE TO START THREAD pthread_create ERR:%i\n", result);

  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


int pause_mthread(mthread_info *p_thread_info, bool blocking) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("pause_mthread", __FILE__, __LINE__, &midge_error_stack_index);

  p_thread_info->should_pause = 1;

  if (blocking) {
    const int  MAX_ITERATIONS = 2000000;
    int  iterations = 0;
    while (!p_thread_info->has_paused&&!p_thread_info->has_concluded    ) {
      usleep(1);
      ++iterations;
      if (iterations>=MAX_ITERATIONS) {
        printf("TODO -- Thread-Handling for unresponsive thread:: \n");

        {
          // Return
          register_midge_stack_return(midge_error_stack_index);
          return -1;
        }

      }

    }

  }



  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


/*
 * Holds the given mthread. Intended to be called by the thread which has been signaled to pause.
 * @returns whether the thread since pausing has been signalled to exit (should_exit).
 */
int hold_mthread(mthread_info *p_thread_info) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("hold_mthread", __FILE__, __LINE__, &midge_error_stack_index);

  p_thread_info->has_paused = 1;
  while (p_thread_info->should_pause  ) 
    usleep(1);
  p_thread_info->has_paused = 0;


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return p_thread_info->should_exit;
  }

}


int unpause_mthread(mthread_info *p_thread_info, bool blocking) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("unpause_mthread", __FILE__, __LINE__, &midge_error_stack_index);

  p_thread_info->should_pause = 0;

  if (blocking) {
    const int  MAX_ITERATIONS = 2000000;
    int  iterations = 0;
    while (p_thread_info->has_paused&&!p_thread_info->has_concluded    ) {
      usleep(1);
      ++iterations;
      if (iterations>=MAX_ITERATIONS) {
        printf("TODO -- Thread-Handling for unresponsive thread:: \n");

        {
          // Return
          register_midge_stack_return(midge_error_stack_index);
          return -1;
        }

      }

    }

  }



  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


int end_mthread(mthread_info *p_thread_info) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("end_mthread", __FILE__, __LINE__, &midge_error_stack_index);

  p_thread_info->should_exit = 1;

  // printf("end_mthread:0\n");
  const int  MAX_ITERATIONS = 20000;
  int  iterations = 0;
  while (!p_thread_info->has_concluded  ) {
    usleep(100);
    ++iterations;
    if (iterations>=MAX_ITERATIONS) {
      printf("TODO -- Thread-Handling for unresponsive thread:: \n");

      {
        // Return
        register_midge_stack_return(midge_error_stack_index);
        return -1;
      }

    }

  }

  // printf("end_mthread:1\n");

  free(p_thread_info);


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}
