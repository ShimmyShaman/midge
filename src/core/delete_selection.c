void delete_selection(code_editor_state *state)
{
  return;
  // state->selection_begin_line++;

  // printf("delete_selection-0\n");
  // if (!state->selection_exists) {
  //   return;
  // }

  // // Obtain selection bounds
  // int selection_start_line, selection_start_col, selection_end_line, selection_end_col;
  // if (state->selection_begin_line > state->cursorLine ||
  //     (state->selection_begin_line == state->cursorLine && state->selection_begin_col > state->cursorCol)) {
  //   // The selection begin comes after the cursor
  //   selection_start_line = (int)state->cursorLine;
  //   selection_start_col = state->cursorCol;
  //   selection_end_line = (int)state->selection_begin_line;
  //   selection_end_col = state->selection_begin_col;
  // }
  // else {
  //   // The selection begin comes before the cursor
  //   selection_start_line = (int)state->selection_begin_line;
  //   selection_start_col = state->selection_begin_col;
  //   selection_end_line = (int)state->cursorLine;
  //   selection_end_col = state->cursorCol;
  // }

  // // Clear the selection
  // state->selection_exists = false;

  // // Adjust cursor
  // state->cursorLine = selection_start_line;
  // state->cursorCol = selection_start_col;

  // // Single line selection
  // char *new_line;
  // if (selection_start_line == selection_end_line) {
  //   if (selection_end_col - selection_start_col < 1) {
  //     return;
  //   }

  //   new_line = (char *)malloc(sizeof(char) * (strlen(state->text->lines[selection_start_line]) -
  //                                             (selection_end_col - selection_start_col) + 1));
  //   if (selection_start_col) {
  //     strncpy(new_line, state->text->lines[selection_start_line], selection_start_col);
  //   }
  //   new_line[selection_start_col] = '\0';
  //   strcat(new_line, state->text->lines[selection_start_line] + selection_end_col);

  //   // Replace
  //   free(state->text->lines[selection_start_line]);
  //   state->text->lines[selection_start_line] = new_line;

  //   return;
  // }

  // // Obtain the initial part of the first line & delete it
  // uint new_line_alloc = selection_start_col + 1 + 1;
  // {
  //   new_line = (char *)malloc(sizeof(char) * new_line_alloc);
  //   if (selection_start_col) {
  //     strncpy(new_line, state->text->lines[selection_start_line], selection_start_col);
  //   }
  //   new_line[selection_start_col] = '\0';

  //   // Replace
  //   free(state->text->lines[selection_start_line]);
  //   state->text->lines[selection_start_line] = NULL;
  // }

  // // Delete all lines in-between
  // for (int a = selection_start_line + 1; a < selection_end_line; ++a) {
  //   free(state->text->lines[a]);
  //   state->text->lines[a] = NULL;
  // }

  // // Obtain the remainder part of the last line and delete it
  // {
  //   int last_line_len = strlen(state->text->lines[selection_end_line]);
  //   if (last_line_len - selection_end_col > 0) {
  //     append_to_cstr(&new_line_alloc, &new_line, state->text->lines[selection_end_line] + selection_end_col);
  //   }

  //   free(state->text->lines[selection_end_line]);
  //   state->text->lines[selection_end_line] = NULL;
  // }

  // // Set
  // state->text->lines[selection_start_line] = new_line;

  // // Bring all lines that were after the last line up to after the start line
  // for (int a = selection_end_line + 1; a < state->text->lines_count; ++a) {
  //   state->text->lines[selection_start_line + a - selection_end_line] = state->text->lines[a];
  //   state->text->lines[a] = NULL;
  // }
  // state->text->lines_count -= selection_end_line - selection_start_line;
}