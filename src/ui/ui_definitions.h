/* ui_definitions.h */

#ifndef UI_DEFINITIONS_H
#define UI_DEFINITIONS_H

#include "core/core_definitions.h"
#include "render/render_common.h"

typedef enum ui_element_type {
  UI_ELEMENT_NULL = 0,
  UI_ELEMENT_TEXT_BLOCK,
} ui_element_type;

typedef enum horizontal_alignment {
  HORIZONTAL_ALIGNMENT_NULL = 0,
  HORIZONTAL_ALIGNMENT_LEFT,
  HORIZONTAL_ALIGNMENT_CENTRED,
  HORIZONTAL_ALIGNMENT_RIGHT,
} horizontal_alignment;

typedef enum vertical_alignment {
  VERTICAL_ALIGNMENT_NULL = 0,
  VERTICAL_ALIGNMENT_LEFT,
  VERTICAL_ALIGNMENT_CENTRED,
  VERTICAL_ALIGNMENT_RIGHT,
} vertical_alignment;

typedef struct mui_ui_element {
  mc_node *visual_node;
  ui_element_type type;
  bool requires_update, requires_rerender;
  // unsigned int headless_image_resource_uid;
  void *data;
} mui_ui_element;

typedef struct mui_text_block {
  mui_ui_element *element;
  c_str *text;

  unsigned int font_resource_uid;
} mui_text_block;

typedef struct mui_ui_state {
  bool requires_update;
  mc_node_list *cache_layered_hit_list;
  unsigned int default_font_resource;
} mui_ui_state;

extern "C" {
void mui_initialize_ui_state();
void mui_initialize_core_ui_components();

void mui_update_ui();
void mui_set_element_update(mui_ui_element *element);

void mui_get_ui_elements_at_point(int screen_x, int screen_y, mc_node_list **layered_hit_list);

void mui_handle_mouse_left_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled);
void mui_handle_mouse_right_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled);

void mui_init_text_block(mc_node *parent, mui_text_block **p_text_block);

// Render
void mui_render_ui();
}

#endif // UI_DEFINITIONS_H