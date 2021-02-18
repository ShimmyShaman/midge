/* button.h */

#ifndef BUTTON_H
#define BUTTON_H

#include "control/mc_controller.h"
#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "mc_str.h"
#include "render/render_common.h"

typedef struct mcu_button {
  mc_node *node;

  void *tag;

  bool enabled;

  // (TODO why can't I declare this with TCC??)
  // int (*left_click)(mci_input_event *input_event, mcu_button *button);
  // -- its still processed as returning void
  void *left_click;

  /* May be NULL indicating the usage of the default font & size by midge-app */
  mcr_font_resource *font;
  render_color font_color;

  mc_str str;
  struct {
    horizontal_alignment_type horizontal;
    vertical_alignment_type vertical;
    // The margins (if alignments are to one side or another only, not centered) between the text and the button bounds
    float horizontal_margin, vertical_margin;
  } text_align;

  render_color background_color;
  float disabled_multiplier;
  float highlight_multiplier;

  // State Variables -- Not to be directly set, will be set by handler functions
  struct {
    bool highlighted;
    float text_offset_x, text_offset_y;
  } __state;
} mcu_button;

int mcu_alloc_button(mc_node *parent, mcu_button **p_button);
int mcu_init_button(mc_node *parent, mcu_button *p_button);
int mcu_set_button_text(mcu_button *button, const char *text);

#endif // BUTTON_H
