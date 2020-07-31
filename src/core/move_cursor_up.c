/* move_cursor_up.c */

#include "core/midge_core.h"

// [_mc_iteration=2]
void move_cursor_up(node *code_editor_node, code_editor_state *state)
{
    return;
  if (state->cursorLine == 0) {
    // Do Nothing
    return;
  }

  // ERR(ERROR_CHECK, "This is the way things go around");

  // Increment
  --state->cursorLine;
  int line_len;
  // if (state->text->lines[state->cursorLine]) {
  //   line_len = strlen(state->text->lines[state->cursorLine]);
  // }
  // else {
  //   line_len = 0;
  // }
  if (state->cursorCol > line_len) {
    state->cursorCol = line_len;

    // if(state->cursor_zen_col == 0) {
    //   state->cursor_zen_col = state->cursorCol;
    // }
  }
  // if (state->cursorZenCol > line_len) {
  //   state->cursorCol = line_len;
  // } else {
  //   state->cursorCol = state->cursorZenCol;
  // }

  // Update the cursor visual
  state->cursor_requires_render_update = true;
  code_editor_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursorLine < state->line_display_offset) {
    // Move display offset up
    state->line_display_offset = state->cursorLine;
  }
}