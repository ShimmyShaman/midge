/* dropdown.h */

#ifndef DROPDOWN_H
#define DROPDOWN_H

#include "control/mc_controller.h"
#include "core/core_definitions.h"
#include "mc_str.h"
#include "render/render_common.h"

#include "modules/ui_elements/panel.h"


typedef struct mcu_dropdown {
  mc_node *node;

  void *tag;

  // void (*left_click)(mci_input_event *, mcu_dropdown *); (TODO why can't I declare this with TCC??)
  void *selection;

  mc_str *selected_str;

  struct {
    unsigned int capacity, count;
    char **items;
  } options;
  bool options_extended;

  mcr_font_resource *font;
  render_color font_color;

  render_color background_color;

  mcu_panel *extension_panel;
} mcu_dropdown;

int mcu_init_dropdown(mc_node *parent, mcu_dropdown **p_button);
#endif // DROPDOWN_H
