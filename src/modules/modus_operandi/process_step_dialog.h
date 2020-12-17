#ifndef PROCESS_STEP_DIALOG_H
#define PROCESS_STEP_DIALOG_H

#include "core/core_definitions.h"
#include "modules/ui_elements/ui_elements.h"

typedef struct mc_process_step_dialog_data {
  mc_node *node;

  render_color shade_color;

  mcu_panel *panel;
  mcu_textblock *message_textblock;
  mcu_textbox *textbox;

  // struct {
  //   unsigned int width, height;
  //   mcr_texture_image *image;
  // } render_target;

  struct {
    void *state;
    void *result_delegate;
  } callback;

} mc_process_step_dialog_data;

typedef struct mc_process_step_dialog_result {

} mc_process_step_dialog_result;

int mc_mocsd_init_process_step_dialog(mc_node *app_root, mc_process_step_dialog_data **p_data);
int mc_mocsd_activate_process_step_dialog(mc_process_step_dialog_data *psdd, char *message, void *callback_state,
                                       void *callback_delegate);

#endif // PROCESS_STEP_DIALOG_H
