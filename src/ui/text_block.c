#include "ui/ui_definitions.h"

void mui_init_text_block(mc_node *parent, mui_text_block **p_text_block)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Node
  mc_node *node = (mc_node *)malloc(sizeof(node));

  node->type = NODE_TYPE_UI;
  node->parent = parent;

  attach_node
  // pthread_mutex_lock(&global_data->uid_counter.mutex);
  // node->uid = global_data->uid_counter.uid_index++;
  // pthread_mutex_unlock(&global_data->uid_counter.mutex);

  // UI Element
  mui_ui_element *element = (mui_ui_element *)malloc(sizeof(mui_ui_element));
  node->data = element;

  element->visual_node = node;
  element->type = UI_ELEMENT_TEXT_BLOCK;
  element->requires_update = true;
  element->requires_rerender = true;

  // Text Block
  mui_text_block *text_block = (mui_text_block *)malloc(sizeof(mui_text_block));
  text_block->element = element;
  element->data = text_block;

  init_c_str(&text_block->text);
  text_block->font_resource_uid = 0;

  // Set to out pointer
  *p_text_block = text_block;
}