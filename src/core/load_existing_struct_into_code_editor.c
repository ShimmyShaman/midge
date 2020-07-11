/* special_update.c */

#include "core/midge_core.h"

// [_mc_iteration=1]
void load_existing_struct_into_code_editor(mc_node_v1 *code_editor, mc_struct_info_v1 *p_struct_info)
{
  printf("load_existing_struct_into_code_editor()\n");

  // Begin Writing into the Function Editor textbox
  mc_code_editor_state_v1 *feState = (mc_code_editor_state_v1 *)code_editor->extra;
  // feState-> = function;
  for (int j = 0; j < feState->text->lines_count; ++j) {
    free(feState->text->lines[j]);
    feState->text->lines[j] = NULL;
  }
  feState->text->lines_count = 0;

  //   // Line Alloc
  //   uint line_alloc = 32;
  //   char *line = (char *)malloc(sizeof(char) * line_alloc);
  //   line[0] = '\0';
}