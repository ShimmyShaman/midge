/* midge_app.h */

#ifndef MIDGE_APP_H
#define MIDGE_APP_H

#include <pthread.h>

#include "control/mc_controller.h"
#include "core/core_definitions.h"
#include "platform/mc_xcb.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

#define MIDGE_APP_INFO_ROOT_UID 2760025

typedef struct frame_time {
  long frame_secs, frame_nsecs;
  long app_secs, app_nsecs;
  float frame_secsf, app_secsf;
} frame_time;

typedef struct mc_update_timer {
  struct timespec next_update;
  struct timespec period;
  bool reset_timer_on_update;
  int (*update_delegate)(frame_time *, void *);
  // void *update_delegate;
  void *state;
} mc_update_timer;

typedef struct event_handler_info {
  void *delegate;
  void *state;
} event_handler_info;

typedef struct event_handler_array {
  char *event_code;
  unsigned int count;
  unsigned int capacity;
  event_handler_info **handlers;
} event_handler_array;

typedef struct midge_app_info {
  unsigned int ROOT_UID;
  mc_app_itp_data *itp_data;
  mc_node *global_node;
  
  struct {
    mc_node *module;
    mc_node *module_target;
    mc_node *project;
    mc_node *project_target;
  } focus_status;

  struct timespec *app_begin_time;
  bool _exit_requested;

  unsigned int uid_counter;

  int inotify_fd, wds_size;
  mc_source_file_info **wds;

  render_thread_info *render_thread;
  frame_time *elapsed;

  pthread_mutexattr_t hierarchy_mutex_attr;
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

  struct {
    unsigned int alloc, count;
    mc_update_timer **items;
  } update_timers;
  struct {
    unsigned int capacity, count;
    event_handler_array **items;
  } event_handlers;
  struct {
    mc_project_info *active;
    mc_project_info *items;
    unsigned int capacity, count;
  } projects;

} midge_app_info;

// extern "C" {
int midge_initialize_app(struct timespec *app_begin_time);
int midge_run_app();
void midge_cleanup_app();

int mca_register_update_timer(long usecs_period, bool reset_timer_on_update, void *callback_state,
                              int (*update_callback)(frame_time *, void *));

void mc_obtain_midge_app_info(midge_app_info **p_app_info);
void mc_destroy_midge_app_info();
// }

#endif // MIDGE_APP_H