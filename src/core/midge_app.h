/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

#include "platform/mc_xcb.h"
#include "render/render_thread.h"

// typedef struct mc_input_event {
//   bool shiftDown, ctrlDown, altDown;
//   window_input_event_type type;
//   window_input_event_detail detail;
//   bool handled;
// } mc_input_event;

typedef struct midge_app_info {
  struct timespec *app_begin_time;
  render_color clear_color;
} midge_app_info;

// extern "C" {
void midge_initialize_app(struct timespec *app_begin_time);
void midge_run_app();
void midge_cleanup_app();
// }

#endif // MIDGE_APP_H