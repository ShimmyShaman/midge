/* midge_common.h */

#ifndef MIDGE_COMMON_H
#define MIDGE_COMMON_H

#include "m_threads.h"

#define MAX_QUEUED_KEY_EVENTS 128
typedef enum {
  INPUT_EVENT_NONE = 0,
  INPUT_EVENT_KEY_PRESS,
  INPUT_EVENT_KEY_RELEASE,
} window_input_event_type;

typedef enum {
  INPUT_EVENT_CODE_NONE = 0,

  // From XCB codes I THINK... Poor way of doing it
  INPUT_EVENT_CODE_ESCAPE = 9,
} window_input_event_code;

typedef struct window_input_buffer {
  pthread_mutex_t mutex;
  struct {
    window_input_event_type type;
    int code;
  } events[MAX_QUEUED_KEY_EVENTS];
  uint event_count;
} window_input_buffer;

#endif // MIDGE_COMMON_H