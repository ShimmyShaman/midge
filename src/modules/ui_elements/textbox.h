/* button.h */

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "core/core_definitions.h"
#include "mc_str.h"
#include "render/render_common.h"

struct mcu_textbox;
typedef struct mcu_textbox {
  mc_node *node;

  void *tag;

  //   // void (*left_click)(mcu_button *button, mc_point click_location);
  //   void *left_click;

  mc_str *contents;
  mcr_font_resource *font;
  render_color font_color;

  struct {
    float left, top;
  } content_padding;

  float font_horizontal_stride;

  struct {
    unsigned int col;
    bool visible;
  } cursor;

  // void (*submit)(mci_input_event *, struct mcu_textbox *);
  void *submit;

  render_color background_color;
} mcu_textbox;

int mcu_init_textbox(mc_node *parent, mcu_textbox **p_button);
#endif // TEXTBOX_H
