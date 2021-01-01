/* m_reset_events.c */

#include "m_reset_events.h"

/* modified from https://stackoverflow.com/questions/178114/porting-windows-manual-reset-event-to-linux TODO-license */

void mrevent_init(struct mrevent *ev)
{
  pthread_mutex_init(&ev->mutex, 0);
  pthread_cond_init(&ev->cond, 0);
  ev->triggered = false;
}

void mrevent_trigger(struct mrevent *ev)
{
  pthread_mutex_lock(&ev->mutex);
  ev->triggered = true;
  pthread_cond_signal(&ev->cond);
  pthread_mutex_unlock(&ev->mutex);
}

void mrevent_reset(struct mrevent *ev)
{
  pthread_mutex_lock(&ev->mutex);
  ev->triggered = false;
  pthread_mutex_unlock(&ev->mutex);
}

void mrevent_wait(struct mrevent *ev)
{
  pthread_mutex_lock(&ev->mutex);
  while (!ev->triggered)
    pthread_cond_wait(&ev->cond, &ev->mutex);
  pthread_mutex_unlock(&ev->mutex);
}