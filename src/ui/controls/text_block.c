#include "render/render_thread.h"
#include "ui/ui_definitions.h"
#include <string.h>

void mui_init_text_block(mc_node *parent, mui_text_block **p_text_block)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // UI Element
  mui_ui_element *element;
  mui_init_ui_element(parent, UI_ELEMENT_TEXT_BLOCK, &element);

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
  // printf("mui_render_text_block\n");
  mui_ui_element *element = (mui_ui_element *)visual_node->data;
  mui_text_block *text_block = (mui_text_block *)element->data;

  mcr_issue_render_command_text(render_queue, (unsigned int)element->bounds.x, (unsigned int)element->bounds.y,
                                text_block->str->text, text_block->font_resource_uid, text_block->font_color);
}