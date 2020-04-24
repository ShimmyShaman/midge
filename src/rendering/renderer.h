/* renderer.h */

#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    pthread_t threadId;
    bool shouldExit, hasConcluded;
} ThreadInfo;

int beginRenderThread(ThreadInfo **pThreadInfo);
int endRenderThread(ThreadInfo *pThreadInfo);

#endif