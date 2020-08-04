/* code_editor_handle_keyboard_input.c */

#include "core/midge_core.h"

void move_cursor_up(mc_code_editor_state_v1 *state)
{
  printf("state->cursor.rtf_index:%i\n", state->cursor.rtf_index);
  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int i = state->cursor.rtf_index;

  if (!state->cursor.zen_col) {
    state->cursor.zen_col = state->cursor.col;
  }

  if (state->cursor.line < 1) {
    return;
  }
  print_parse_error(code, i, "mcu-initial", "");

  // Find the new line
  --i;
  for (;; --i) {
    if (code[i] == '\n') {
      break;
    }
  }
  --state->cursor.line;
  print_parse_error(code, i, "mcu-lineup_end", "");

  // Go to the start of the line
  if (state->cursor.line > 0) {
    --i;
    for (;; --i) {
      if (code[i] == '\n') {
        ++i;
        break;
      }
    }
  }
  else {
    i = 0;
  }
  print_parse_error(code, i, "mcu-line_start", "");

  // Move along the line (as close to the zen col as possible)
  int traversed = 0;
  // printf("state->cursor.zen_col:%i\n", state->cursor.zen_col);
  for (; traversed < state->cursor.zen_col; ++i) {
    // printf("trac:'%c'\n", code[i]);
    // print_parse_error(code, i, "trac", "");
    if (code[i] == '\0') {
      --i;
      break;
    }
    else if (code[i] == '\n') {
      break;
    }
    else if (code[i] == '[') {
      if (code[i] == '[') {
        // Escaped
        ++i;
      }
      else {
        for (;; ++i) {
          if (code[i] == '\0') {
            // MCerror(42, "RTF Format Error");
            printf("42 format error\n");
          }
          else if (code[i] == ']') {
            --traversed;
            break;
          }
        }
      }
    }
    ++traversed;
    // printf("++traversed:%i\n", traversed);
  }
  // printf("traverse-OVER\n");

  state->cursor.col = traversed;
  state->cursor.rtf_index = i;

  // Update the cursor visual
  state->cursor.requires_render_update = true;
  state->visual_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
    // Move display offset down
    state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  }
}

void move_cursor_down(mc_code_editor_state_v1 *state)
{
  // printf("state->cursor.rtf_index:%i\n", state->cursor.rtf_index);
  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int i = state->cursor.rtf_index;

  if (!state->cursor.zen_col) {
    state->cursor.zen_col = state->cursor.col;
  }

  // Find the new line
  for (;; ++i) {
    if (code[i] == '\0') {
      // Do nothing
      return;
    }
    if (code[i] == '\n') {
      ++i;
      break;
    }
  }
  ++state->cursor.line;

  // Move along the line (as close to the zen col as possible)
  int traversed = 0;
  // printf("state->cursor.zen_col:%i\n", state->cursor.zen_col);
  for (; traversed < state->cursor.zen_col; ++i) {
    // printf("trac:'%c'\n", code[i]);
    print_parse_error(code, i, "trac", "");
    if (code[i] == '\0') {
      --i;
      break;
    }
    else if (code[i] == '\n') {
      break;
    }
    else if (code[i] == '[') {
      if (code[i] == '[') {
        // Escaped
        ++i;
      }
      else {
        for (;; ++i) {
          if (code[i] == '\0') {
            // MCerror(42, "RTF Format Error");
            printf("42 format error\n");
          }
          else if (code[i] == ']') {
            --traversed;
            break;
          }
        }
      }
    }
    ++traversed;
    // printf("++traversed:%i\n", traversed);
  }
  // printf("traverse-OVER\n");

  state->cursor.col = traversed;
  state->cursor.rtf_index = i;

  // Update the cursor visual
  state->cursor.requires_render_update = true;
  state->visual_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
    // Move display offset down
    state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  }
}

// [_mc_iteration=3]
void code_editor_handle_keyboard_input(frame_time *elapsed, mc_node_v1 *fedit, mc_input_event_v1 *event)
{
  mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)fedit->extra;
  // printf("keyboard key = (%i%i%i)+%i\n", event->altDown, event->ctrlDown, event->shiftDown,
  // event->detail.keyboard.key);

  switch (event->detail.keyboard.key) {
  // case KEY_CODE_DELETE: {
  //   event->handled = true;

  //   if (state->selection_exists) {
  //     // Delete the selection
  //     delete_selection(state);
  //     break;
  //   }

  //   printf("delete-0\n");
  //   int line_len = strlen(state->text->lines[state->cursor.line]);
  //   if (state->cursor.col == line_len) {
  //     if (state->cursor.line == state->text->lines_count) {
  //       // Do nothing
  //       break;
  //     }

  //     // Append the next line onto this one
  //     int current_line_len = strlen(state->text->lines[state->cursor.line]);
  //     int next_line_len = strlen(state->text->lines[state->cursor.line + 1]);
  //     char *new_line = (char *)malloc(sizeof(char) * (current_line_len + next_line_len + 1));
  //     strcpy(new_line, state->text->lines[state->cursor.line]);
  //     strcat(new_line, state->text->lines[state->cursor.line + 1]);

  //     free(state->text->lines[state->cursor.line]);
  //     state->text->lines[state->cursor.line] = new_line;

  //     // Move all lines after the next line up one
  //     for (int i = state->cursor.line + 2; i < state->text->lines_count; ++i) {
  //       state->text->lines[i - 1] = state->text->lines[i];
  //     }
  //     state->text->lines[state->text->lines_count - 1] = NULL;
  //     --state->text->lines_count;
  //     break;
  //   }

  //   // Move all characters back one
  //   for (int i = state->cursor.col + 1;; ++i) {
  //     char c = state->text->lines[state->cursor.line][i];
  //     state->text->lines[state->cursor.line][i - 1] = c;
  //     if (c == '\0') {
  //       break;
  //     }
  //   }

  // } break;
  // case KEY_CODE_BACKSPACE: {
  //   event->handled = true;
  //   if (!state->cursor.col) {
  //     if (state->cursor.line) {
  //       // Combine previous line & second line into one
  //       int previous_line_len = strlen(state->text->lines[state->cursor.line - 1]);
  //       char *combined =
  //           (char *)malloc(sizeof(char) * (previous_line_len + strlen(state->text->lines[state->cursor.line]) +
  //           1));
  //       strcpy(combined, state->text->lines[state->cursor.line - 1]);
  //       strcat(combined, state->text->lines[state->cursor.line]);

  //       free(state->text->lines[state->cursor.line - 1]);
  //       free(state->text->lines[state->cursor.line]);
  //       state->text->lines[state->cursor.line - 1] = combined;

  //       // Bring all lines up one position
  //       for (int i = state->cursor.line + 1; i < state->text->lines_count; ++i) {
  //         state->text->lines[i - 1] = state->text->lines[i];
  //       }
  //       state->text->lines[state->text->lines_count - 1] = NULL;
  //       --state->text->lines_count;

  //       --state->cursor.line;
  //       state->cursor.col = previous_line_len;
  //     }
  //     break;
  //   }

  //   // Bring all forward characters back one
  //   int line_len = strlen(state->text->lines[state->cursor.line]);
  //   for (int i = state->cursor.col - 1; i < line_len; ++i) {
  //     state->text->lines[state->cursor.line][i] = state->text->lines[state->cursor.line][i + 1];
  //   }

  //   --state->cursor.col;
  // } break;
  // case KEY_CODE_ENTER:
  // case KEY_CODE_RETURN: {
  //   event->handled = true;
  //   if (event->ctrlDown) {

  //     switch (state->source_data->type) {
  //     case SOURCE_DEFINITION_FUNCTION: {
  //       // Read the code from the editor
  //       free(state->source_data->code);
  //       read_editor_text_into_cstr(state, &state->source_data->code);

  //       mc_function_info_v1 *func_info;
  //       parse_and_process_function_definition(state->source_data, &func_info, false);

  //       printf("ERROR TODO\n");
  //       event->handled = true;
  //       return;

  //       // // Compile the function definition
  //       // uint transcription_alloc = 4;
  //       // char *transcription = (char *)malloc(sizeof(char) * transcription_alloc);
  //       // transcription[0] = '\0';
  //       // int code_index = 0;
  //       // transcribe_c_block_to_mc(func_info, func_info->, &code_index, &transcription_alloc, &transcription);

  //       // // Define the new function
  //       // instantiate_function(func_info->name, transcription);

  //       // printf("redefined function '%s' to iteration %i\n", func_info->name, func_info->latest_iteration);
  //       // event->handled = true;
  //       // break;
  //     }
  //     case SOURCE_DEFINITION_STRUCT: {
  //       // Read the code from the editor
  //       define_struct_from_code_editor(state);
  //       event->handled = true;
  //       break;
  //     }
  //     default: {
  //       // Do Nothing? TODO
  //       printf("instantiating data_source:%i is not supported\n", state->source_data->type);
  //     } break;
  //     }
  //   }
  //   else {
  //     printf("fehi-0\n");
  //     // Newline -- carrying over any extra
  //     char *cursor.line = state->text->lines[state->cursor.line];

  //     // Automatic indent
  //     int automaticIndent = 0;
  //     int cursor.lineLen = strlen(cursor.line);
  //     for (; automaticIndent < state->cursor.col && automaticIndent < cursor.lineLen; ++automaticIndent) {
  //       if (cursor.line[automaticIndent] != ' ') {
  //         break;
  //       }
  //     }

  //     // Increment lines after by one place
  //     if (state->text->lines_count + 1 >= state->text->lines_alloc) {
  //       uint new_alloc = state->text->lines_alloc + 4 + state->text->lines_alloc / 4;
  //       char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
  //       if (state->text->lines_alloc) {
  //         memcpy(new_ary, state->text->lines, state->text->lines_alloc * sizeof(char *));
  //         free(state->text->lines);
  //       }
  //       for (int i = state->text->lines_alloc; i < new_alloc; ++i) {
  //         new_ary[i] = NULL;
  //       }

  //       state->text->lines_alloc = new_alloc;
  //       state->text->lines = new_ary;
  //     }
  //     // printf("state->text->lines_alloc:%u state->text->lines_count:%u state->cursor.line:%u\n",
  //     //   state->text->lines_alloc,
  //     //  state->text->lines_count, state->cursor.line);
  //     for (int i = state->text->lines_count; i > state->cursor.line + 1; --i) {

  //       state->text->lines[i] = state->text->lines[i - 1];
  //     }
  //     ++state->text->lines_count;

  //     // printf("fehi-1\n");
  //     if (state->cursor.col >= cursor.lineLen) {
  //       // printf("fehi-1A\n");
  //       // Just create new line
  //       char *newLine = (char *)malloc(sizeof(char) * (automaticIndent + 1));
  //       for (int i = 0; i < automaticIndent; ++i) {
  //         newLine[i] = ' ';
  //       }
  //       newLine[automaticIndent] = '\0';
  //       state->text->lines[state->cursor.line + 1] = newLine;
  //     }
  //     else if (state->cursor.col) {
  //       // printf("fehi-1B\n");
  //       // Split current line at cursor column position
  //       char *firstSplit = (char *)malloc(sizeof(char) * (state->cursor.col + 1));
  //       memcpy(firstSplit, cursor.line, sizeof(char) * state->cursor.col);
  //       firstSplit[state->cursor.col] = '\0';

  //       char *secondSplit = (char *)malloc(sizeof(char) * (automaticIndent + cursor.lineLen - state->cursor.col +
  //       1)); for (int i = 0; i < automaticIndent; ++i) {
  //         secondSplit[i] = ' ';
  //       }
  //       secondSplit[automaticIndent] = '\0';
  //       strcat(secondSplit, cursor.line + state->cursor.col);

  //       free(cursor.line);
  //       state->text->lines[state->cursor.line] = firstSplit;
  //       state->text->lines[state->cursor.line + 1] = secondSplit;
  //     }
  //     else {
  //       // printf("fehi-1C\n");
  //       // Move the rest of the current line forwards
  //       state->text->lines[state->cursor.line + 1] = cursor.line;
  //       state->text->lines[state->cursor.line] = (char *)malloc(sizeof(char) * (automaticIndent + 1));
  //       for (int i = 0; i < automaticIndent; ++i) {
  //         state->text->lines[state->cursor.line][i] = ' ';
  //       }
  //       state->text->lines[state->cursor.line][automaticIndent] = '\0';
  //     }

  //     printf("fehi-2\n");
  //     // Cursor Position Update
  //     ++state->cursor.line;
  //     state->cursor.col = automaticIndent;
  //   }
  // } break;
  case KEY_CODE_ARROW_UP: {
    event->handled = true;

    move_cursor_up(state);
  } break;
  case KEY_CODE_ARROW_DOWN: {
    event->handled = true;

    move_cursor_down(state);
  } break;
  // case KEY_CODE_ARROW_LEFT: {
  //   // Increment
  //   if (state->cursor.col == 0) {
  //     if (state->cursor.line == 0) {
  //       // Nothing can be done
  //       break;
  //     }

  //     --state->cursor.line;
  //     state->cursor.col = strlen(state->text->lines[state->cursor.line]);

  //     // Adjust display offset
  //     if (state->cursor.line < state->line_display_offset) {
  //       // Move display offset up
  //       state->line_display_offset = state->cursor.line;
  //     }
  //   }
  //   else {
  //     --state->cursor.col;
  //   }

  //   // Update the cursor visual
  //   state->cursor.requires_render_update = true;
  //   fedit->data.visual.requires_render_update = true;
  // } break;
  // case KEY_CODE_ARROW_RIGHT: {
  //   int line_len = strlen(state->text->lines[state->cursor.line]);

  //   if (state->cursor.col == line_len) {
  //     if (state->cursor.line + 1 >= state->text->lines_count) {
  //       // Do Nothing
  //       break;
  //     }

  //     ++state->cursor.line;
  //     state->cursor.col = 0;

  //     // Adjust display offset
  //     if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
  //       // Move display offset down
  //       state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  //     }
  //   }
  //   else {
  //     ++state->cursor.col;
  //   }

  //   // Update the cursor visual
  //   state->cursor.requires_render_update = true;
  //   fedit->data.visual.requires_render_update = true;
  // } break;
  // case KEY_CODE_F4: {
  //   code_editor_toggle_view(state);

  // } break;
  // case KEY_CODE_HOME: {
  //   printf("past\n");
  //   state->cursor.col = 0;

  //   // Update the cursor visual
  //   state->cursor.requires_render_update = true;
  //   fedit->data.visual.requires_render_update = true;
  // } break;
  // case KEY_CODE_END: {
  //   state->cursor.col = strlen(state->text->lines[state->cursor.line]);

  //   // Update the cursor visual
  //   state->cursor.requires_render_update = true;
  //   fedit->data.visual.requires_render_update = true;
  // } break;
  default: {
    if (event->altDown) {
      //     switch (event->detail.keyboard.key) {
      //     case KEY_CODE_K: {
      //       for (int i = 0; i < 6; ++i) { // FROM KEY_CODE_ARROW_DOWN above (TODO refactor into function)
      //         if (state->cursor.line + 1 >= state->text->lines_count) {
      //           // Do Nothing
      //           break;
      //         }

      //         // Increment
      //         ++state->cursor.line;
      //         int line_len = strlen(state->text->lines[state->cursor.line]);
      //         if (state->cursor.col > line_len) {
      //           state->cursor.col = line_len;
      //         }

      //         // Update the cursor visual
      //         state->cursor.requires_render_update = true;
      //         fedit->data.visual.requires_render_update = true;

      //         // Adjust display offset
      //         if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
      //           // Move display offset down
      //           state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
      //         }
      //       }
      //     } break;
      //     case KEY_CODE_I: {
      //       for (int i = 0; i < 6; ++i) { // FROM KEY_CODE_ARROW_UP above (TODO refactor into function)
      //         move_cursor_up(fedit, state);
      //       }
      //     } break;
      //     default: {
      //       break;
      //     }
      //     }
      //   }
      //   else if (event->ctrlDown) {
      //     switch (event->detail.keyboard.key) {
      //     case KEY_CODE_C: {
      //       char *text;
      //       if (state->selection_exists) {
      //         // Copy selection into buffer
      //         text = read_selected_editor_text(state);
      //       }
      //       else {
      //         // Copy the current line into the buffer
      //         if (state->text->lines[state->cursor.line] && strlen(state->text->lines[state->cursor.line])) {
      //           cprintf(text, "%s\n", state->text->lines[state->cursor.line]);
      //         }
      //         else {
      //           // Copy empty text
      //           allocate_and_copy_cstr(text, "\n");
      //         }
      //       }

      //       if (command_hub->clipboard_text) {
      //         free(command_hub->clipboard_text);
      //       }

      //       allocate_and_copy_cstr(command_hub->clipboard_text, text);
      //       free(text);
      //     } break;
      //     case KEY_CODE_V: {
      //       if (state->selection_exists) {
      //         // Delete selection
      //         delete_selection(state);
      //       }

      //       insert_text_into_editor_at_cursor(state, command_hub->clipboard_text, state->cursor.line,
      //       state->cursor.col);
      //     } break;
      //     case KEY_CODE_J: { // FROM KEY_CODE_ARROW_LEFT above (TODO refactor into function)
      //       // Increment
      //       if (state->cursor.col == 0) {
      //         if (state->cursor.line == 0) {
      //           // Nothing can be done
      //           break;
      //         }

      //         --state->cursor.line;
      //         state->cursor.col = strlen(state->text->lines[state->cursor.line]);

      //         // Adjust display offset
      //         if (state->cursor.line < state->line_display_offset) {
      //           // Move display offset up
      //           state->line_display_offset = state->cursor.line;
      //         }
      //       }
      //       else {
      //         --state->cursor.col;
      //       }

      //       // Update the cursor visual
      //       state->cursor.requires_render_update = true;
      //       fedit->data.visual.requires_render_update = true;
      //     } break;
      //     case KEY_CODE_L: { // FROM KEY_CODE_ARROW_RIGHT above (TODO refactor into function) (this one has
      //     selection)

      //       if (event->shiftDown) {
      //         if (!state->selection_exists) {
      //           state->selection_exists = true;
      //           state->selection_begin_line = state->cursor.line;
      //           state->selection_begin_col = state->cursor.col;
      //         }
      //       }
      //       else {
      //         if (state->selection_exists) {
      //           state->selection_exists = false;
      //         }
      //       }

      //       int line_len = strlen(state->text->lines[state->cursor.line]);
      //       if (state->cursor.col == line_len) {
      //         if (state->cursor.line + 1 < state->text->lines_count) {

      //           ++state->cursor.line;
      //           state->cursor.col = 0;

      //           // Adjust display offset
      //           if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
      //             // Move display offset down
      //             state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
      //           }
      //         }
      //       }
      //       else {
      //         ++state->cursor.col;
      //       }

      //       // Update the cursor visual
      //       state->cursor.requires_render_update = true;
      //       fedit->data.visual.requires_render_update = true;
      //     } break;
      //     case KEY_CODE_K: { // FROM KEY_CODE_ARROW_DOWN above (TODO refactor into function)
      //       if (state->cursor.line + 1 >= state->text->lines_count) {
      //         // Do Nothing
      //         break;
      //       }

      //       // Increment
      //       ++state->cursor.line;
      //       int line_len = strlen(state->text->lines[state->cursor.line]);
      //       if (state->cursor.col > line_len) {
      //         state->cursor.col = line_len;
      //       }

      //       // Update the cursor visual
      //       state->cursor.requires_render_update = true;
      //       fedit->data.visual.requires_render_update = true;

      //       // Adjust display offset
      //       if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
      //         // Move display offset down
      //         state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
      //       }
      //     } break;
      //     case KEY_CODE_I: { // FROM KEY_CODE_ARROW_UP above (TODO refactor into function)
      //       move_cursor_up(fedit, state);
      //     } break;
      //     case KEY_CODE_SEMI_COLON: {
      //       state->cursor.col = strlen(state->text->lines[state->cursor.line]);

      //       // Update the cursor visual
      //       state->cursor.requires_render_update = true;
      //       fedit->data.visual.requires_render_update = true;
      //     } break;
      //     case KEY_CODE_S: {
      //       // Save the file
      //       if (!state->source_data || !state->source_data->source_file) {
      //         printf("code has no source file\n");
      //         break;
      //       }

      //       char *filepath;
      //       switch (state->source_data->type) {
      //       case SOURCE_DEFINITION_FUNCTION: {
      //         mc_function_info_v1 *function = (mc_function_info_v1 *)state->source_data;

      //         // Read the code from the editor
      //         char *function_definition;
      //         read_editor_text_into_cstr(state, &function_definition);

      //         save_function_to_file(function, function_definition);

      //         free(function_definition);
      //       } break;
      //       case SOURCE_DEFINITION_STRUCT: {
      //         mc_struct_info_v1 *structure = (mc_struct_info_v1 *)state->source_data;

      //         // // Read the code from the editor
      //         char *structure_definition;
      //         read_editor_text_into_cstr(state, &structure_definition);

      //         // printf("structure_definition:\n%s||\n", structure_definition);

      //         save_struct_to_file(structure, structure_definition);
      //         free(structure_definition);
      //       } break;
      //       default: {
      //         printf("saving source_data_type=%i is not supported\n", state->source_data->type);
      //       } break;
      //       }
      //     } break;
      //     default: {
      //       break;
      //     }
      //     }
    }
    else {
      // printf("fehi-3\n");
      char c[2];
      int res = get_key_input_code_char(event->shiftDown, event->detail.keyboard.key, &c[0]);
      if (res) {
        break;
        // TODO
      }
      event->handled = true;

      // Update the text
      {
        // if (state->selection_exists) {
        //   // Delete selection
        //   delete_selection(state);
        // }

        c[1] = '\0';
        insert_text_into_editor_at_cursor(state, &c[0]);
      }
    }
  } break;
  }
}