/* panel.h */

#ifndef PANEL_H
#define PANEL_H

#include "control/mc_controller.h"
#include "core/core_definitions.h"
#include "mc_str.h"
#include "render/render_common.h"

typedef struct mcu_panel {
  mc_node *node;

  void *tag;

  render_color background_color;
} mcu_panel;

int mcu_init_panel(mc_node *parent, mcu_panel **p_button);

#endif // PANEL_H
