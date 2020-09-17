/* ui_definitions.h */

#ifndef UI_DEFINITIONS_H
#define UI_DEFINITIONS_H

#include "core/core_definitions.h"
#include "render/render_common.h"

typedef enum ui_element_type {
  UI_ELEMENT_NULL = 0,
  UI_ELEMENT_PANEL,
  UI_ELEMENT_TEXT_BLOCK,
  UI_ELEMENT_BUTTON,
  UI_ELEMENT_CONTEXT_MENU,
} ui_element_type;

typedef struct mui_ui_element {
  mc_node *visual_node;
  ui_element_type type;

  bool requires_layout_update, requires_rerender;
  // unsigned int headless_image_resource_uid;

  mca_node_layout *layout;

  void *data;
} mui_ui_element;

typedef struct mui_panel {
  mui_ui_element *element;

  mc_node_list *children;

  render_color background_color;
} mui_panel;

// Controls
typedef struct mui_text_block {
  mui_ui_element *element;
  c_str *str;

  unsigned int font_resource_uid;
  render_color font_color;
} mui_text_block;

// Controls
typedef struct mui_button {
  mui_ui_element *element;
  c_str *str;

  void *tag;

  unsigned int font_resource_uid;
  render_color font_color;

  render_color background_color;
} mui_button;

typedef struct mui_context_menu {
  mui_ui_element *element;

  mc_node_list *children;

  struct {
    unsigned int alloc, count;
    mui_button **items;
  } _buttons;

  render_color background_color;
} mui_context_menu;

extern "C" {
// UI Functionality
void mui_initialize_ui_state(mui_ui_state **p_ui_state);
void mui_initialize_core_ui_components();

void mui_update_element_layout(mui_ui_element *element);

void mui_get_interactive_nodes_at_point(int screen_x, int screen_y, mc_node_list **layered_hit_list);

void mui_handle_mouse_left_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled);
void mui_handle_mouse_right_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled);

// Render
void mui_render_element_headless(mc_node *element_node);
void mui_render_element_present(image_render_queue *render_queue, mc_node *element_node);

// Control Initialization
void mui_init_ui_element(mc_node *parent_node, ui_element_type element_type, mui_ui_element **created_element);

void mui_init_panel(mc_node *parent, mui_panel **p_panel);
void mui_render_panel(image_render_queue *render_queue, mc_node *ui_node);

void mui_init_text_block(mc_node *parent, mui_text_block **p_text_block);
void mui_render_text_block(image_render_queue *render_queue, mc_node *ui_node);

void mui_init_button(mc_node *parent, mui_button **p_button);
void mui_render_button(image_render_queue *render_queue, mc_node *ui_node);

void mui_init_context_menu(mc_node *parent, mui_context_menu **p_button);
void mui_render_context_menu(image_render_queue *render_queue, mc_node *ui_node);
void mui_context_menu_clear_options(mui_ui_element *menu_element);
void mui_context_menu_add_option(mui_ui_element *menu_element, const char *option_text);
}

#endif // UI_DEFINITIONS_H