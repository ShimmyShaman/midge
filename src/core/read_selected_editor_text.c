#include "core/midge_core.h"

void read_selected_editor_text(code_editor_state *editor_state, char **result)
{
  // if (!editor_state->selection_exists) {
    // Copy empty string
    (*result) = (char *)malloc(sizeof(char) * 1);
    (*result)[0] = '\0';
    return;
  // }

  // // printf("rset-0\n");
  // // Obtain selection bounds
  // int selection_start_line, selection_start_col, selection_end_line, selection_end_col;
  // if (editor_state->selection_begin_line > editor_state->cursorLine ||
  //     (editor_state->selection_begin_line == editor_state->cursorLine &&
  //      editor_state->selection_begin_col > editor_state->cursorCol)) {
  //   // The selection begin comes after the cursor
  //   selection_start_line = (int)editor_state->cursorLine - editor_state->line_display_offset;
  //   selection_start_col = editor_state->cursorCol;
  //   selection_end_line = (int)editor_state->selection_begin_line - editor_state->line_display_offset;
  //   selection_end_col = editor_state->selection_begin_col;
  // }
  // else {
  //   // The selection begin comes before the cursor
  //   selection_start_line = (int)editor_state->selection_begin_line - editor_state->line_display_offset;
  //   selection_start_col = editor_state->selection_begin_col;
  //   selection_end_line = (int)editor_state->cursorLine - editor_state->line_display_offset;
  //   selection_end_col = editor_state->cursorCol;
  // }

  // // printf("rset-1\n");
  // // Obtain the cstr length
  // unsigned int result_alloc = 4;
  // (*result) = (char *)malloc(sizeof(char) * 4);
  // (*result)[0] = '\0';
  // if (selection_start_line == selection_end_line) {

  //   // printf("rset-2\n");
  //   if (selection_end_col - selection_start_col > 0) {
  //     append_to_cstrn(&result_alloc, result, editor_state->text->lines[selection_start_line] + selection_start_col,
  //                     selection_end_col - selection_start_col);
  //   }
  // }
  // else {
  //   // printf("rset-3\n");
  //   // Start line
  //   append_to_cstr(&result_alloc, result, editor_state->text->lines[selection_start_line] + selection_start_col);
  //   append_to_cstr(&result_alloc, result, "\n");

  //   // printf("rset-4\n");
  //   // Between lines
  //   for (int i = selection_start_line + 1; i < selection_end_line; ++i) {
  //     append_to_cstr(&result_alloc, result, editor_state->text->lines[i]);
  //     append_to_cstr(&result_alloc, result, "\n");
  //   }

  //   // printf("rset-5\n");
  //   if (selection_end_col) {
  //     // Last line
  //     append_to_cstrn(&result_alloc, result, editor_state->text->lines[selection_end_line], selection_end_col);
  //   }
  // }

  // printf("rset-6\n");
}