#include "env/environment_definitions.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"
#include <string.h>

void mui_init_context_menu(mc_node *parent, mui_context_menu **p_context_menu)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // UI Element
  mui_ui_element *element;
  mui_init_ui_element(parent, UI_ELEMENT_CONTEXT_MENU, &element);

  // Text Block
  mui_context_menu *context_menu = (mui_context_menu *)malloc(sizeof(mui_context_menu));
  context_menu->element = element;
  element->data = context_menu;

  context_menu->background_color = COLOR_BURLY_WOOD;

  context_menu->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  context_menu->children->alloc = 0;
  context_menu->children->count = 0;

  context_menu->_buttons.alloc = 0;
  context_menu->_buttons.count = 0;

  // Set to out pointer
  *p_context_menu = context_menu;
}

// void mui_layout_context_menu_extents(mc_node *node, mc_rectf *available_area, layout_extent_restraints restraints) {

//   mui_ui_element *element = (mui_ui_element *)node->data;
//   mca_node_layout *layout = node->
// }

void mui_render_context_menu(image_render_queue *render_queue, mc_node *visual_node)
{
  // printf("mui_render_context_menu\n");
  mui_ui_element *element = (mui_ui_element *)visual_node->data;
  mui_context_menu *context_menu = (mui_context_menu *)element->data;

  // Background
  mcr_issue_render_command_colored_rect(render_queue, (unsigned int)element->layout->__bounds.x,
                                        (unsigned int)element->layout->__bounds.y,
                                        (unsigned int)element->layout->__bounds.width,
                                        (unsigned int)element->layout->__bounds.height, context_menu->background_color);

  // Children
  for (int a = 0; a < context_menu->children->count; ++a) {
    if (context_menu->children->items[a]->visible)
      mui_render_element_present(render_queue, context_menu->children->items[a]);
  }
}

void mui_context_menu_clear_options(mui_ui_element *menu_element)
{
  mui_context_menu *context_menu = (mui_context_menu *)menu_element->data;

  for (int i = 0; i < context_menu->_buttons.count; ++i) {
    set_c_str(context_menu->_buttons.items[i]->str, "");
    context_menu->_buttons.items[i]->element->visual_node->visible = false;
  }
  context_menu->_buttons.count = 0;
}

void mui_context_menu_option_clicked(mui_button *button, mc_point click_point)
{
  printf("Button clicked at %i, %i\n", click_point.x, click_point.y);
}

void mui_temp_TODO(int a) { printf("you entered the number %i!!!!~!~~~\n", a); }

void mui_context_menu_add_option(mui_ui_element *menu_element, const char *option_text)
{
  mui_context_menu *menu = (mui_context_menu *)menu_element->data;

  if (menu->_buttons.count + 1 > menu->_buttons.alloc) {

    int prev_alloc = menu->_buttons.alloc;
    reallocate_collection((void ***)&menu->_buttons.items, &menu->_buttons.alloc, 0, 0);

    for (int a = prev_alloc; a < menu->_buttons.alloc; ++a) {
      mui_init_button(menu_element->visual_node, &menu->_buttons.items[a]);
      menu->_buttons.items[a]->element->visual_node->visible = false;

      menu->_buttons.items[a]->element->layout->padding = {2, 2, 2, 2};

      // printf("mui_temp_TODO:%p\n", *((void **)mui_temp_TODO));
      // printf("mui_temp_TODO:%p\n", mui_temp_TODO);
      // printf("mui_temp_TODO:%p\n", &mui_temp_TODO);
      // void *bb = (void *)&mui_temp_TODO;
      // printf("mui_temp_TODO:%p\n", (void *)&bb);

      // DEBUG TODO (has to be set with void for now)
      menu->_buttons.items[a]->left_click = (void *)&mui_context_menu_option_clicked;

      // menu->_buttons.items[a]->element->layout->preferred_width = 118.f;
      // menu->_buttons.items[a]->element->layout->preferred_height = 28.f;
    }
  }

  set_c_str(menu->_buttons.items[menu->_buttons.count]->str, option_text);
  menu->_buttons.items[menu->_buttons.count]->element->visual_node->visible = true;
  ++menu->_buttons.count;
}