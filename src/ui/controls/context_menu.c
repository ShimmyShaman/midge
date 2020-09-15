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

  // Set to out pointer
  *p_context_menu = context_menu;
}

void mui_render_context_menu(image_render_queue *render_queue, mc_node *visual_node)
{
  // printf("mui_render_context_menu\n");
  mui_ui_element *element = (mui_ui_element *)visual_node->data;
  mui_context_menu *context_menu = (mui_context_menu *)element->data;

  // Background
  mcr_issue_render_command_colored_rect(render_queue, (unsigned int)element->bounds.x, (unsigned int)element->bounds.y,
                                        (unsigned int)element->bounds.width, (unsigned int)element->bounds.height,
                                        context_menu->background_color);

  //   // Text
  //   mcr_issue_render_command_text(render_queue, (unsigned int)element->bounds.x, (unsigned int)element->bounds.y,
  //                                 context_menu->str->text, context_menu->font_resource_uid,
  //                                 context_menu->font_color);
}