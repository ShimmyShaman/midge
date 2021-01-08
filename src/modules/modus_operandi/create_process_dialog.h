#ifndef CREATE_PROCESS_DIALOG_H
#define CREATE_PROCESS_DIALOG_H

#include "core/core_definitions.h"

#include "modules/collections/hash_table.h"
#include "modules/modus_operandi/mo_types.h"
#include "modules/ui_elements/ui_elements.h"

typedef struct mc_mo_cpd_step_data {
  mo_op_step_action_type type;

  mcu_panel *panel;
  mcu_dropdown *dropdown;
  mcu_textbox *textbox;
  mcu_textblock *textblock, *textblock2;
  mcu_button *continue_button;
} mc_mo_cpd_step_data;

typedef struct mc_create_process_dialog_data {
  mc_node *node;

  render_color shade_color;

  mcu_panel *panel;
  // mcu_textblock *message_textblock;
  // mcu_dropdown *step_type_dropdown;
  // mcu_button *previous_button, *finish_button, *next_button;
  // hash_table_t options_panels;
  // mcu_panel *active_options_panel;

  // mcu_textbox *textbox

  // struct {
  //   unsigned int width, height;
  //   mcr_texture_image *image;
  // } render_target;

  struct {
    unsigned int size;
    mc_mo_cpd_step_data *ary;
  } cells;

  struct {
    void *state;
    void *result_delegate;
  } callback;

} mc_create_process_dialog_data;

int mc_mo_init_create_process_dialog(mc_node *app_root, mc_create_process_dialog_data **p_data);

/*
 * @callback_delegate int (*result_delegate)(void *invoker_state, void *created_process)
 */
int mc_mo_activate_create_process_dialog(mc_create_process_dialog_data *psdd, void *callback_state,
                                         void *callback_delegate);
#endif // CREATE_PROCESS_DIALOG_H
