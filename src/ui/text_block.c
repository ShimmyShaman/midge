
#include "ui/ui_definitions.h"

void mui_init_text_block(node *parent, mui_text_block **p_text_block)
{
  mui_text_block *text_block = (mui_text_block *)malloc(sizeof(mui_text_block));

  text_block->visual_node = (node *)malloc(sizeof(node));
  text_block->visual_node->type = NODE_TYPE_VISUAL;

  attach_node_to_heirarchy(parent, text_block->visual_node);

  *p_text_block = text_block;
}