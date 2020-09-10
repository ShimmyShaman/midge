/* ui_definitions.h */

#ifndef UI_DEFINITIONS_H
#define UI_DEFINITIONS_H

#include "core/midge_core.h"
#include "render/render_common.h"

typedef enum horizontal_alignment {
  HORIZONTAL_ALIGNMENT_NULL = 0,
  HORIZONTAL_ALIGNMENT_LEFT,
  HORIZONTAL_ALIGNMENT_CENTRED,
  HORIZONTAL_ALIGNMENT_RIGHT,
} horizontal_alignment;

typedef enum vertical_alignment {
  VERTICAL_ALIGNMENT_NULL = 0,
  VERTICAL_ALIGNMENT_LEFT,
  VERTICAL_ALIGNMENT_CENTRED,
  VERTICAL_ALIGNMENT_RIGHT,
} vertical_alignment;

typedef struct mui_text_block {
  node *visual_node;

  c_str *text;
} mui_text_block;

void mui_update_ui() { printf("28, TODO\n"); }

void mui_init_text_block(node *visual_node, mui_text_block **p_text_block);

#endif // UI_DEFINITIONS_H