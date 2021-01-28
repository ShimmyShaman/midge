/* button.h */

#ifndef BUTTON_H
#define BUTTON_H

#include "control/mc_controller.h"
#include "core/core_definitions.h"
#include "mc_str.h"
#include "render/render_common.h"

typedef struct mcu_button {
  mc_node *node;

  void *tag;

  // int (*left_click)(mci_input_event *, mcu_button *); (TODO why can't I declare this with TCC??)
  void *left_click;

  mc_str *str;
  mcr_font_resource *font;
  render_color font_color;

  render_color background_color;
} mcu_button;

int mcu_init_button(mc_node *parent, mcu_button **p_button);

#endif // BUTTON_H
