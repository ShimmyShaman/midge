#include "render/render_thread.h"
#include "ui/ui_definitions.h"
#include <string.h>

void mui_init_text_block(mc_node *parent, mui_text_block **p_text_block)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Node
  mc_node *node = (mc_node *)malloc(sizeof(mc_node));

  node->type = NODE_TYPE_UI;
  attach_node_to_hierarchy(parent, node);
  // // pthread_mutex_lock(&global_data->uid_counter.mutex);
  // // node->uid = global_data->uid_counter.uid_index++;
  // // pthread_mutex_unlock(&global_data->uid_counter.mutex);

  // UI Element
  mui_ui_element *element = (mui_ui_element *)malloc(sizeof(mui_ui_element));
  node->data = element;

  element->bounds.x = 0;
  element->bounds.y = 0;
  element->visual_node = node;
  element->type = UI_ELEMENT_TEXT_BLOCK;
  element->requires_update = true;
  element->requires_rerender = true;

  // Text Block
  mui_text_block *text_block = (mui_text_block *)malloc(sizeof(mui_text_block));
  text_block->element = element;
  element->data = text_block;

  init_c_str(&text_block->str);
  text_block->font_resource_uid = 0;
  text_block->font_color = COLOR_GHOST_WHITE;

  // Set to out pointer
  *p_text_block = text_block;
}

void mui_render_text_block(image_render_queue *render_queue, mc_node *visual_node)
{
  mui_ui_element *element = (mui_ui_element *)visual_node->data;
  mui_text_block *text_block = (mui_text_block *)element->data;

  element_render_command *render_cmd;
  obtain_element_render_command(render_queue, &render_cmd);
  render_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  render_cmd->x = element->bounds.x;
  render_cmd->y = element->bounds.y;
  // TODO -- make the render cmd a c_str??
  render_cmd->data.print_text.text = strdup(text_block->str->text);

  if (text_block->font_resource_uid) {
    render_cmd->data.print_text.font_resource_uid = text_block->font_resource_uid;
    printf("set textblockfont %u\n", render_cmd->data.print_text.font_resource_uid);
  }
  else {
    global_root_data *global_data;
    obtain_midge_global_root(&global_data);

    render_cmd->data.print_text.font_resource_uid = global_data->default_font_resource;
    printf("set defaultfont %u\n", render_cmd->data.print_text.font_resource_uid);
  }
  render_cmd->data.print_text.color = text_block->font_color;
}