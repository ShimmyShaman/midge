/* textblock.h */
#ifndef TEXTBLOCK_H
#define TEXTBLOCK_H

#include "control/mc_controller.h"
#include "core/core_definitions.h"
#include "mc_str.h"
#include "render/render_common.h"

typedef struct mcu_textblock {
  mc_node *node;

  void *tag;

  // void (*left_click)(mci_input_event *, mcu_textblock *); (TODO why can't I declare this with TCC??)
  void *left_click;

  mc_str *str;
  mcr_font_resource *font;
  render_color font_color;

  render_color background_color;
  bool clip_text_to_bounds;
} mcu_textblock;

int mcu_init_textblock(mc_node *parent, mcu_textblock **p_textblock);

#endif // TEXTBLOCK_H
