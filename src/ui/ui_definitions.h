/* ui_definitions.h */

#ifndef UI_DEFINITIONS_H
#define UI_DEFINITIONS_H

#include "core/core_definitions.h"
#include "render/render_common.h"

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

typedef struct mui_text_block {
  node *visual_node;

  c_str *text;
} mui_text_block;

typedef struct mui_ui_state {
  node_list *cache_layered_hit_list;
} mui_ui_state;

extern "C" {
void mui_initialize_ui_state();

void mui_update_ui();

void mui_get_ui_elements_at_point(int screen_x, int screen_y, node_list **layered_hit_list);

void mui_handle_mouse_left_click(node *ui_node, int screen_x, int screen_y, bool *handled) ;
void mui_handle_mouse_right_click(node *ui_node, int screen_x, int screen_y, bool *handled) ;

void mui_init_text_block(node *visual_node, mui_text_block **p_text_block);
}

#endif // UI_DEFINITIONS_H