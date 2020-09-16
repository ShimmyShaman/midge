#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void mui_init_panel(mc_node *parent, mui_panel **p_panel)
{
  mui_ui_element *element;
  mui_init_ui_element(parent, UI_ELEMENT_PANEL, &element);

  // Text Block
  mui_panel *panel = (mui_panel *)malloc(sizeof(mui_panel));
  panel->element = element;
  element->data = panel;

  panel->background_color = COLOR_CORNFLOWER_BLUE;
  panel->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  panel->children->alloc = 0;
  panel->children->count = 0;

  // Set to out pointer
  *p_panel = panel;
}

void mui_render_panel(image_render_queue *render_queue, mc_node *visual_node)
{
  mui_ui_element *element = (mui_ui_element *)visual_node->data;
  mui_panel *panel = (mui_panel *)element->data;

  mcr_issue_render_command_colored_rect(render_queue, (unsigned int)element->layout->__bounds.x,
                                        (unsigned int)element->layout->__bounds.y,
                                        (unsigned int)element->layout->__bounds.width,
                                        (unsigned int)element->layout->__bounds.height, panel->background_color);

  // Children
  for (int a = 0; a < panel->children->count; ++a) {
    if (panel->children->items[a]->visible)
      mui_render_element_present(render_queue, panel->children->items[a]);
  }
}