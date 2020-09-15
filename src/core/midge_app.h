/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

#include "platform/mc_xcb.h"
#include "render/render_thread.h"

typedef struct update_callback_timer {
  struct timespec next_update;
  struct timespec period;
  bool reset_timer_on_update;
  int (**update_delegate)(int, void **);
  void *state;
} update_callback_timer;

typedef struct mc_input_event {
  bool shiftDown, ctrlDown, altDown;
  window_input_event_type type;
  window_input_event_detail detail;
  bool handled;
} mc_input_event;

typedef struct midge_app_info {
  struct timespec *app_begin_time;
  render_color clear_color;
} midge_app_info;

extern "C" {
void midge_initialize_app(midge_app_info *app_info);
void midge_run_app(midge_app_info *app_info);
void midge_cleanup_app(midge_app_info *app_info);
}

#endif // MIDGE_APP_H