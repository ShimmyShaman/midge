/* move_cursor_up.c */

#include "core/midge_core.h"

// [_mc_iteration=1]
void move_cursor_up(mc_node_v1 *function_editor_node, function_editor_state *state)
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
  function_editor_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursorLine < state->line_display_offset) {
    // Move display offset up
    state->line_display_offset = state->cursorLine;
  }
}