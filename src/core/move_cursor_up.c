/* move_cursor_up.c */

#include "core/midge_core.h"

// [_mc_iteration=2]
void move_cursor_up(unsigned int *line, unsigned int *col, mc_text_line_list_v1 *text)
{
  if (*line == 0) {
    // Do Nothing
    return;
  }

  // // Increment
  // --state->cursorLine;
  // int line_len = strlen(state->text.lines[state->cursorLine]);
  // if (state->cursorCol > line_len) {
  //   state->cursorCol = line_len;
  // }

  // // Update the cursor visual
  // state->cursor_requires_render_update = true;
  // fedit->data.visual.requires_render_update = true;

  // // Adjust display offset
  // if (state->cursorLine < state->line_display_offset) {
  //   // Move display offset up
  //   state->line_display_offset = state->cursorLine;
  // }
}