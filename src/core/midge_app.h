/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

#include <pthread.h>

#include "control/mc_controller.h"
#include "platform/mc_xcb.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

// typedef struct mc_input_event {
//   bool shiftDown, ctrlDown, altDown;
//   window_input_event_type type;
//   window_input_event_detail detail;
//   bool handled;
// } mc_input_event;

typedef struct frame_time {
  long frame_secs, frame_nsecs;
  long app_secs, app_nsecs;
  float frame_secsf, app_secsf;
} frame_time;

typedef struct midge_app_info {
  mc_app_itp_data *itp_data;
  mc_node *global_node;

  struct timespec *app_begin_time;
  bool _exit_requested;

  render_thread_info *render_thread;
  frame_time *elapsed;

  pthread_mutex_t hierarchy_mutex;

  // mcu_ui_state
  mcu_ui_state *ui_state;
  struct {
    unsigned int width, height;
    void *present_image;
    render_color clear_color;
  } screen;

  mci_input_state *input_state;
  bool input_state_requires_update;

} midge_app_info;

// extern "C" {
int midge_initialize_app(struct timespec *app_begin_time);
int midge_run_app();
void midge_cleanup_app();

void mc_obtain_midge_app_info(midge_app_info **p_app_info);
void mc_destroy_midge_app_info();
// }

#endif // MIDGE_APP_H