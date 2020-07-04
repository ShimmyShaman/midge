/* midge_common.h */

#ifndef MIDGE_COMMON_H
#define MIDGE_COMMON_H

#include "m_threads.h"

#define COLOR_CORNFLOWER_BLUE (render_color){0.19f, 0.34f, 0.83f, 1.f};
#define COLOR_GREENISH (render_color){0.11f, 0.55f, 0.32f, 1.f};
#define COLOR_TEAL (render_color){0.0f, 0.52f, 0.52f, 1.f};
#define COLOR_PURPLE (render_color){160.f / 255.f, 32.f / 255.f, 240.f / 255.f, 1.f};
#define COLOR_BURLY_WOOD (render_color){0.87f, 0.72f, 0.52f, 1.f};
#define COLOR_DARK_SLATE_GRAY (render_color){0.18f, 0.18f, 0.31f, 1.f};
#define COLOR_GHOST_WHITE (render_color){0.97f, 0.97f, 1.f, 1.f};
#define COLOR_BLACK (render_color){0.f, 0.f, 0.f, 1.f};
#define COLOR_YELLOW (render_color){1.f, 1.f, 0.f, 1.f};

#define MAX_QUEUED_KEY_EVENTS 128
typedef enum window_input_event_type {
  INPUT_EVENT_NONE = 0,
  INPUT_EVENT_KEY_PRESS,
  INPUT_EVENT_KEY_RELEASE,
  INPUT_EVENT_MOUSE_PRESS,
  INPUT_EVENT_MOUSE_RELEASE,
} window_input_event_type;

typedef enum key_event_code {
  KEY_CODE_NONE = 0,

  // From XCB codes I THINK... Poor way of doing it
  KEY_CODE_ESCAPE = 9,
  KEY_CODE_Q = 24,
  KEY_CODE_LEFT_ALT = 64,
  KEY_CODE_RIGHT_ALT = 108,
  KEY_CODE_LEFT_SHIFT = 50,
  KEY_CODE_RIGHT_SHIFT = 62,
  KEY_CODE_LEFT_CTRL = 37,
  KEY_CODE_RIGHT_CTRL = 105,
  KEY_CODE_ENTER = 36,
  KEY_CODE_RETURN = 104,
} key_event_code;

typedef enum mouse_event_code {
  MOUSE_BUTTON_NONE = 0,
  MOUSE_BUTTON_LEFT = 1,
  MOUSE_BUTTON_MIDDLE = 2,
  MOUSE_BUTTON_RIGHT = 3,
  MOUSE_BUTTON_SCROLL_UP = 4,
  MOUSE_BUTTON_SCROLL_DOWN = 5,
  MOUSE_BUTTON_PREVIOUS = 8,
  MOUSE_BUTTON_NEXT = 9,
} mouse_event_code;

int get_key_input_code_char(bool shift, key_event_code code, char *c)
{
  switch (code) {
  case KEY_CODE_Q:
    *c = 'q';
    return 0;

  default:
    return -1;
  }
}

typedef union window_input_event_detail {
  struct {
    key_event_code key;
  } keyboard;
  struct {
    mouse_event_code button;
    uint x, y;
  } mouse;
} window_input_event_detail;

typedef struct window_input_event {
  window_input_event_type type;
  window_input_event_detail detail;
} window_input_event;

typedef struct window_input_buffer {
  pthread_mutex_t mutex;
  window_input_event events[MAX_QUEUED_KEY_EVENTS];
  uint event_count;
} window_input_buffer;

typedef struct frame_time {
  long frame_sec, frame_nsec;
  long app_sec, app_nsec;
} frame_time;

#endif // MIDGE_COMMON_H