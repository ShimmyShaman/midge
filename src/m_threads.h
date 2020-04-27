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
} mthread_info;

int begin_mthread(void *(*start_routine)(void *), mthread_info **p_thread_info)
{
    *p_thread_info = (mthread_info *)malloc(sizeof *p_thread_info);
    (*p_thread_info)->start_routine = start_routine;

    printf("beginRenderThread\n");
    (*p_thread_info)->should_exit = 0;
    (*p_thread_info)->has_concluded = 0;
    if (pthread_create(&(*p_thread_info)->threadId, NULL, (*p_thread_info)->start_routine, (void *)(*p_thread_info)))
    {
        return 0;
    }
    return -1;
}

int end_mthread(mthread_info *p_thread_info)
{
    p_thread_info->should_exit = 1;

    const int MAX_ITERATIONS = 1000;
    int iterations = 0;
    while (!p_thread_info->has_concluded)
    {
        usleep(1000);
        ++iterations;
        if (iterations >= MAX_ITERATIONS)
        {
            printf("TODO -- Thread-Handling for unresponsive thread:: renderer.c\n");
            return -1;
        }
    }

    free(p_thread_info);

    return 0;
}

#endif