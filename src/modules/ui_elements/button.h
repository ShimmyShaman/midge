/* button.h */

#ifndef BUTTON_H
#define BUTTON_H

#include "core/core_definitions.h"
#include "mc_str.h"
#include "render/render_common.h"

typedef struct mcu_button {
  mc_node *node;

  void *tag;

  // void (*left_click)(mcu_button *button, mc_point click_location);
  void *left_click;

  mc_str *str;
  mcr_font_resource *font;
  render_color font_color;

  render_color background_color;
} mcu_button;

int mcu_init_button(mc_node *parent, mcu_button **p_button);

#endif // BUTTON_H
