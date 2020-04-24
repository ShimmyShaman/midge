/* renderer.c */

#include "renderer.h"

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *renderThread(void *vargp)
{
  ThreadInfo *info = (ThreadInfo *)vargp;

  while (!info->shouldExit)
    usleep(1);

  printf("Thread Exit\n");
  info->hasConcluded = 1;
  return 0;
}

int beginRenderThread(ThreadInfo **pThreadInfo)
{
  if ((*pThreadInfo = (ThreadInfo *)malloc(sizeof *pThreadInfo)) != NULL)
  {
    (*pThreadInfo)->shouldExit = 0;
    (*pThreadInfo)->hasConcluded = 0;

    pthread_create(&(*pThreadInfo)->threadId, NULL, renderThread, (void *)*pThreadInfo);

    return 0;
  }
  return -1;
}

int endRenderThread(ThreadInfo *pThreadInfo)
{
  pThreadInfo->shouldExit = 1;

  const int MAX_ITERATIONS = 500;
  int iterations = 0;
  while (!pThreadInfo->hasConcluded)
  {
    usleep(1);
    ++iterations;
    if (iterations >= MAX_ITERATIONS)
    {
      printf("TODO -- Thread-Handling for unresponsive thread:: renderer.c");
      return -1;
    }
  }

  free(pThreadInfo);
  return 0;
}