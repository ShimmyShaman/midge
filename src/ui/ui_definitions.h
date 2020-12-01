/* ui_definitions.h */

#ifndef UI_DEFINITIONS_H
#define UI_DEFINITIONS_H

#include "mc_str.h"

#include "core/core_definitions.h"
#include "render/render_common.h"
#include "env/environment_definitions.h"

typedef enum ui_element_type {
  UI_ELEMENT_NULL = 0,
  UI_ELEMENT_PANEL,
  UI_ELEMENT_TEXT_BLOCK,
  UI_ELEMENT_BUTTON,
  UI_ELEMENT_CONTEXT_MENU,
} ui_element_type;

typedef struct mcu_panel {
  mc_node *node;

  render_color background_color;
} mcu_panel;

// Controls
typedef struct mcu_text_block {
  mc_node *node;

  mc_str *str;
  mcr_font_resource *font;
  render_color font_color;
} mcu_text_block;

// typedef struct mcu_context_menu {
//   mc_node *node;

//   struct {
//     unsigned int alloc, count;
//     mcu_button **items;
//   } _buttons;

//   // void (*option_selected)(const char *option_title);
//   void *option_selected;

//   render_color background_color;
// } mcu_context_menu;

// extern "C" {
// UI Functionality
void mcu_initialize_ui_state(mcu_ui_state **p_ui_state);
// void mcu_initialize_core_ui_components();

void mcu_get_interactive_nodes_at_point(int screen_x, int screen_y, mc_node_list **layered_hit_list);

// void mcu_handle_mouse_left_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled);
// void mcu_handle_mouse_right_click(mc_node *ui_node, int screen_x, int screen_y, bool *handled);

// Render
// void mcu_render_element_headless(mc_node *element_node);
// void mcu_render_element_present(image_render_details *image_render_queue, mc_node *element_node);

// Control Initialization
// void mcu_init_ui_element(mc_node *parent_node, ui_element_type element_type, mcu_ui_element **created_element);
// void mcu_get_hierarchical_children_node_list(mc_node *hierarchy_node, mc_node_list **parent_node_list);

void mcu_init_panel(mc_node *parent, mcu_panel **p_panel);
// void mcu_render_panel(image_render_details *image_render_queue, mc_node *ui_node);

void mcu_init_text_block(mc_node *parent, mcu_text_block **p_text_block);
// void mcu_render_text_block(image_render_details *image_render_queue, mc_node *ui_node);

// void mcu_render_button(image_render_details *image_render_queue, mc_node *ui_node);
// void mca_init_button_context_menu_options();

// void mcu_init_context_menu(mc_node *parent, mcu_context_menu **p_button);
// // void mcu_render_context_menu(image_render_details *image_render_queue, mc_node *ui_node);
// void mcu_context_menu_clear_options(mcu_context_menu *menu);
// void mcu_context_menu_add_option(mcu_context_menu *menu, const char *option_text);
// }

#endif // UI_DEFINITIONS_H