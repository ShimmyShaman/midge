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

  element_render_command *render_cmd;
  obtain_element_render_command(render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_COLORED_QUAD;
  render_cmd->x = element->bounds.x;
  render_cmd->y = element->bounds.y;
  render_cmd->data.colored_rect_info.width = element->bounds.width;
  render_cmd->data.colored_rect_info.height = element->bounds.height;
  render_cmd->data.colored_rect_info.color = panel->background_color;

  // Children
  for (int a = 0; a < panel->children->count; ++a) {
    printf("rendering something child\n");
    mui_render_ui_node(render_queue, panel->children->items[a]);
  }
}