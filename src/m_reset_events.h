#ifndef M_RESET_EVENTS_H
#define M_RESET_EVENTS_H

#include <stdbool.h>

#include <pthread.h>

/* from https://stackoverflow.com/questions/178114/porting-windows-manual-reset-event-to-linux TODO-license */

struct mrevent {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  bool triggered;
};

void mrevent_init(struct mrevent *ev);

void mrevent_trigger(struct mrevent *ev);

void mrevent_reset(struct mrevent *ev);

void mrevent_wait(struct mrevent *ev);
#endif // M_RESET_EVENTS_H
