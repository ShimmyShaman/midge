#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void mui_init_panel(mc_node *parent, mui_panel **p_panel)
{
  mui_ui_element *element;
  mui_init_ui_node(parent, UI_ELEMENT_PANEL, &element);

  // Text Block
  mui_panel *panel = (mui_panel *)malloc(sizeof(mui_panel));
  panel->element = element;
  element->data = panel;

  panel->background_color = COLOR_CORNFLOWER_BLUE;

  // Set to out pointer
  *p_panel = panel;
}

void mui_render_panel(image_render_queue *render_queue, mc_node *visual_node)
{
  printf("mrp-0\n");
  mui_ui_element *element = (mui_ui_element *)visual_node->data;
  mui_panel *panel = (mui_panel *)element->data;

  element_render_command *render_cmd;
  obtain_element_render_command(render_queue, &render_cmd);
  printf("mrp-2\n");
  render_cmd->type = RENDER_COMMAND_COLORED_QUAD;
  printf("mrp-3\n");
  render_cmd->x = element->bounds.x;
  render_cmd->y = element->bounds.y;
  printf("mrp-4\n");
  render_cmd->data.colored_rect_info.width = element->bounds.width;
  render_cmd->data.colored_rect_info.height = element->bounds.height;
  printf("mrp-5\n");
  printf("mrp-5b %.2f\n", render_cmd->data.colored_rect_info.color.r);
  printf("mrp-5a %.2f\n", panel->background_color.r);
  render_cmd->data.colored_rect_info.color = panel->background_color;
  printf("reached end here\n");
}