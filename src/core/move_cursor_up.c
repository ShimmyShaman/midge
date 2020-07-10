/* move_cursor_up.c */

#include "core/midge_core.h"

// [_mc_iteration=1]
void move_cursor_up(mc_node_v1 *code_editor_node, mc_code_editor_state_v1 *state)
{
  if (state->cursorLine == 0) {
    // Do Nothing
    return;
  }

  // Increment
  --state->cursorLine;
  int line_len;
  if (state->text->lines[state->cursorLine]) {
    line_len = strlen(state->text->lines[state->cursorLine]);
  }
  else {
    line_len = 0;
  }
  if (state->cursorCol > line_len) {
    state->cursorCol = line_len;
  }

  // Update the cursor visual
  state->cursor_requires_render_update = true;
  code_editor_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursorLine < state->line_display_offset) {
    // Move display offset up
    state->line_display_offset = state->cursorLine;
  }
}