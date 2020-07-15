/* code_editor_handle_keyboard_input.c */

#include "core/midge_core.h"

// [_mc_iteration=3]
void code_editor_handle_keyboard_input(frame_time *elapsed, mc_node_v1 *fedit, mc_input_event_v1 *event)
{
  mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)fedit->extra;
  // printf("keyboard key = (%i%i%i)+%i\n", event->altDown, event->ctrlDown, event->shiftDown,
  // event->detail.keyboard.key);

  switch (event->detail.keyboard.key) {
  case KEY_CODE_DELETE: {
    event->handled = true;

    printf("delete-0\n");
    int line_len = strlen(state->text->lines[state->cursorLine]);
    if (state->cursorCol == line_len) {
      if (state->cursorLine == state->text->lines_count) {
        // Do nothing
        break;
      }

      // Append the next line onto this one
      int current_line_len = strlen(state->text->lines[state->cursorLine]);
      int next_line_len = strlen(state->text->lines[state->cursorLine + 1]);
      char *new_line = (char *)malloc(sizeof(char) * (current_line_len + next_line_len + 1));
      strcpy(new_line, state->text->lines[state->cursorLine]);
      strcat(new_line, state->text->lines[state->cursorLine + 1]);

      free(state->text->lines[state->cursorLine]);
      state->text->lines[state->cursorLine] = new_line;

      // Move all lines after the next line up one
      for (int i = state->cursorLine + 2; i < state->text->lines_count; ++i) {
        state->text->lines[i - 1] = state->text->lines[i];
      }
      state->text->lines[state->text->lines_count - 1] = NULL;
      --state->text->lines_count;
      break;
    }

    // Move all characters back one
    for (int i = state->cursorCol + 1;; ++i) {
      char c = state->text->lines[state->cursorLine][i];
      state->text->lines[state->cursorLine][i - 1] = c;
      if (c == '\0') {
        break;
      }
    }

  } break;
  case KEY_CODE_BACKSPACE: {
    event->handled = true;
    if (!state->cursorCol) {
      if (state->cursorLine) {
        // Combine previous line & second line into one
        int previous_line_len = strlen(state->text->lines[state->cursorLine - 1]);
        char *combined =
            (char *)malloc(sizeof(char) * (previous_line_len + strlen(state->text->lines[state->cursorLine]) + 1));
        strcpy(combined, state->text->lines[state->cursorLine - 1]);
        strcat(combined, state->text->lines[state->cursorLine]);

        free(state->text->lines[state->cursorLine - 1]);
        free(state->text->lines[state->cursorLine]);
        state->text->lines[state->cursorLine - 1] = combined;

        // Bring all lines up one position
        for (int i = state->cursorLine + 1; i < state->text->lines_count; ++i) {
          state->text->lines[i - 1] = state->text->lines[i];
        }
        state->text->lines[state->text->lines_count - 1] = NULL;
        --state->text->lines_count;

        --state->cursorLine;
        state->cursorCol = previous_line_len;
      }
      break;
    }

    // Bring all forward characters back one
    int line_len = strlen(state->text->lines[state->cursorLine]);
    for (int i = state->cursorCol - 1; i < line_len; ++i) {
      state->text->lines[state->cursorLine][i] = state->text->lines[state->cursorLine][i + 1];
    }

    --state->cursorCol;
  } break;
  case KEY_CODE_ENTER:
  case KEY_CODE_RETURN: {
    event->handled = true;
    if (event->ctrlDown) {

      switch (state->source_data_type) {
      case CODE_EDITOR_SOURCE_DATA_FUNCTION: {
        // Read the code from the editor
        char *function_definition;
        read_editor_text_into_cstr(state, &function_definition);

        mc_function_info_v1 *func_info;
        parse_and_process_function_definition(function_definition, &func_info, false);
        free(function_definition);

        // Compile the function definition
        uint transcription_alloc = 4;
        char *transcription = (char *)malloc(sizeof(char) * transcription_alloc);
        transcription[0] = '\0';
        int code_index = 0;
        transcribe_c_block_to_mc(func_info, func_info->mc_code, &code_index, &transcription_alloc, &transcription);

        // Define the new function
        instantiate_function(func_info->name, transcription);

        printf("redefined function '%s' to iteration %i\n", func_info->name, func_info->latest_iteration);
        event->handled = true;
        break;
      }
      case CODE_EDITOR_SOURCE_DATA_STRUCT: {
        // Read the code from the editor
        define_struct_from_code_editor(state);
        event->handled = true;
        break;
      }
      default: {
        // Do Nothing? TODO
        printf("instantiating data_source:%i is not supported\n", state->source_data_type);
      } break;
      }
    }
    else {
      printf("fehi-0\n");
      // Newline -- carrying over any extra
      char *cursorLine = state->text->lines[state->cursorLine];

      // Automatic indent
      int automaticIndent = 0;
      int cursorLineLen = strlen(cursorLine);
      for (; automaticIndent < state->cursorCol && automaticIndent < cursorLineLen; ++automaticIndent) {
        if (cursorLine[automaticIndent] != ' ') {
          break;
        }
      }

      // Increment lines after by one place
      if (state->text->lines_count + 1 >= state->text->lines_allocated) {
        uint new_alloc = state->text->lines_allocated + 4 + state->text->lines_allocated / 4;
        char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
        if (state->text->lines_allocated) {
          memcpy(new_ary, state->text->lines, state->text->lines_allocated * sizeof(char *));
          free(state->text->lines);
        }
        for (int i = state->text->lines_allocated; i < new_alloc; ++i) {
          new_ary[i] = NULL;
        }

        state->text->lines_allocated = new_alloc;
        state->text->lines = new_ary;
      }
      // printf("state->text->lines_allocated:%u state->text->lines_count:%u state->cursorLine:%u\n",
      //   state->text->lines_allocated,
      //  state->text->lines_count, state->cursorLine);
      for (int i = state->text->lines_count; i > state->cursorLine + 1; --i) {

        state->text->lines[i] = state->text->lines[i - 1];
      }
      state->text->lines[state->cursorLine + 1] = NULL;
      ++state->text->lines_count;

      // printf("fehi-1\n");
      if (state->cursorCol >= cursorLineLen) {
        // printf("fehi-1A\n");
        // Just create new line
        char *newLine = (char *)malloc(sizeof(char) * (automaticIndent + 1));
        for (int i = 0; i < automaticIndent; ++i) {
          newLine[i] = ' ';
        }
        newLine[automaticIndent] = '\0';
        state->text->lines[state->cursorLine + 1] = newLine;
      }
      else if (state->cursorCol) {
        // printf("fehi-1B\n");
        // Split current line at cursor column position
        char *firstSplit = (char *)malloc(sizeof(char) * (state->cursorCol + 1));
        memcpy(firstSplit, cursorLine, sizeof(char) * state->cursorCol);
        firstSplit[state->cursorCol] = '\0';

        char *secondSplit = (char *)malloc(sizeof(char) * (automaticIndent + cursorLineLen - state->cursorCol + 1));
        for (int i = 0; i < automaticIndent; ++i) {
          secondSplit[i] = ' ';
        }
        secondSplit[automaticIndent] = '\0';
        strcat(secondSplit, cursorLine + state->cursorCol);

        free(cursorLine);
        state->text->lines[state->cursorLine] = firstSplit;
        state->text->lines[state->cursorLine + 1] = secondSplit;
      }
      else {
        // printf("fehi-1C\n");
        // Move the rest of the current line forwards
        state->text->lines[state->cursorLine + 1] = cursorLine;
        state->text->lines[state->cursorLine] = (char *)malloc(sizeof(char) * (automaticIndent + 1));
        for (int i = 0; i < automaticIndent; ++i) {
          state->text->lines[state->cursorLine][i] = ' ';
        }
        state->text->lines[state->cursorLine][automaticIndent] = '\0';
      }

      printf("fehi-2\n");
      // Cursor Position Update
      ++state->cursorLine;
      state->cursorCol = automaticIndent;
    }
  } break;
  case KEY_CODE_ARROW_UP: {
    move_cursor_up(fedit, state);
  } break;
  case KEY_CODE_ARROW_DOWN: {
    if (state->cursorLine + 1 >= state->text->lines_count) {
      // Do Nothing
      break;
    }

    // Increment
    ++state->cursorLine;
    int line_len = strlen(state->text->lines[state->cursorLine]);
    if (state->cursorCol > line_len) {
      state->cursorCol = line_len;
    }

    // Update the cursor visual
    state->cursor_requires_render_update = true;
    fedit->data.visual.requires_render_update = true;

    // Adjust display offset
    if (state->cursorLine >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
      // Move display offset down
      state->line_display_offset = state->cursorLine - CODE_EDITOR_RENDERED_CODE_LINES + 1;
    }
  } break;
  case KEY_CODE_ARROW_LEFT: {
    // Increment
    if (state->cursorCol == 0) {
      if (state->cursorLine == 0) {
        // Nothing can be done
        break;
      }

      --state->cursorLine;
      state->cursorCol = strlen(state->text->lines[state->cursorLine]);

      // Adjust display offset
      if (state->cursorLine < state->line_display_offset) {
        // Move display offset up
        state->line_display_offset = state->cursorLine;
      }
    }
    else {
      --state->cursorCol;
    }

    // Update the cursor visual
    state->cursor_requires_render_update = true;
    fedit->data.visual.requires_render_update = true;
  } break;
  case KEY_CODE_ARROW_RIGHT: {
    int line_len = strlen(state->text->lines[state->cursorLine]);

    if (state->cursorCol == line_len) {
      if (state->cursorLine + 1 >= state->text->lines_count) {
        // Do Nothing
        break;
      }

      ++state->cursorLine;
      state->cursorCol = 0;

      // Adjust display offset
      if (state->cursorLine >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
        // Move display offset down
        state->line_display_offset = state->cursorLine - CODE_EDITOR_RENDERED_CODE_LINES + 1;
      }
    }
    else {
      ++state->cursorCol;
    }

    // Update the cursor visual
    state->cursor_requires_render_update = true;
    fedit->data.visual.requires_render_update = true;
  } break;
  case KEY_CODE_HOME: {
    printf("past\n");
    state->cursorCol = 0;

    // Update the cursor visual
    state->cursor_requires_render_update = true;
    fedit->data.visual.requires_render_update = true;
  } break;
  case KEY_CODE_END: {
    state->cursorCol = strlen(state->text->lines[state->cursorLine]);

    // Update the cursor visual
    state->cursor_requires_render_update = true;
    fedit->data.visual.requires_render_update = true;
  } break;
  default: {
    if (event->altDown) {
      switch (event->detail.keyboard.key) {
      case KEY_CODE_K: {
        for (int i = 0; i < 6; ++i) { // FROM KEY_CODE_ARROW_DOWN above (TODO refactor into function)
          if (state->cursorLine + 1 >= state->text->lines_count) {
            // Do Nothing
            break;
          }

          // Increment
          ++state->cursorLine;
          int line_len = strlen(state->text->lines[state->cursorLine]);
          if (state->cursorCol > line_len) {
            state->cursorCol = line_len;
          }

          // Update the cursor visual
          state->cursor_requires_render_update = true;
          fedit->data.visual.requires_render_update = true;

          // Adjust display offset
          if (state->cursorLine >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
            // Move display offset down
            state->line_display_offset = state->cursorLine - CODE_EDITOR_RENDERED_CODE_LINES + 1;
          }
        }
      } break;
      case KEY_CODE_I: {
        for (int i = 0; i < 6; ++i) { // FROM KEY_CODE_ARROW_UP above (TODO refactor into function)
          move_cursor_up(fedit, state);
        }
      } break;
      default: {
        break;
      }
      }
    }
    else if (event->ctrlDown) {
      switch (event->detail.keyboard.key) {
      case KEY_CODE_C: {
        char *text;
        if (state->selection_exists) {
          // Copy selection into buffer
          text = read_selected_editor_text(state);
        }
        else {
          // Copy the current line into the buffer
          if (state->text->lines[state->cursorLine] && strlen(state->text->lines[state->cursorLine])) {
            cprintf(text, "%s\n", state->text->lines[state->cursorLine]);
          }
          else {
            // Copy empty text
            allocate_and_copy_cstr(text, "\n");
          }
        }

        if (command_hub->clipboard_text) {
          free(command_hub->clipboard_text);
        }

        allocate_and_copy_cstr(command_hub->clipboard_text, text);
        free(text);
      } break;
      case KEY_CODE_V: {
        printf("paste args:\n");
        printf("-- state:%p\n", state);
        printf("-- command_hub->clipboard_text:'%s'\n", command_hub->clipboard_text);
        printf("-- state->cursorLine:%i\n", state->cursorLine);
        printf("-- state->cursorCol:%i\n", state->cursorCol);
        insert_text_into_editor(state, command_hub->clipboard_text, state->cursorLine, state->cursorCol);
      } break;
      case KEY_CODE_J: { // FROM KEY_CODE_ARROW_LEFT above (TODO refactor into function)
        // Increment
        if (state->cursorCol == 0) {
          if (state->cursorLine == 0) {
            // Nothing can be done
            break;
          }

          --state->cursorLine;
          state->cursorCol = strlen(state->text->lines[state->cursorLine]);

          // Adjust display offset
          if (state->cursorLine < state->line_display_offset) {
            // Move display offset up
            state->line_display_offset = state->cursorLine;
          }
        }
        else {
          --state->cursorCol;
        }

        // Update the cursor visual
        state->cursor_requires_render_update = true;
        fedit->data.visual.requires_render_update = true;
      } break;
      case KEY_CODE_L: { // FROM KEY_CODE_ARROW_RIGHT above (TODO refactor into function) (this one has selection)

        if (event->shiftDown) {
          if (!state->selection_exists) {
            state->selection_exists = true;
            state->selection_begin_line = state->cursorLine;
            state->selection_begin_col = state->cursorCol;
          }
        }
        else {
          if (state->selection_exists) {
            state->selection_exists = false;
          }
        }

        int line_len = strlen(state->text->lines[state->cursorLine]);
        if (state->cursorCol == line_len) {
          if (state->cursorLine + 1 < state->text->lines_count) {

            ++state->cursorLine;
            state->cursorCol = 0;

            // Adjust display offset
            if (state->cursorLine >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
              // Move display offset down
              state->line_display_offset = state->cursorLine - CODE_EDITOR_RENDERED_CODE_LINES + 1;
            }
          }
        }
        else {
          ++state->cursorCol;
        }

        // Update the cursor visual
        state->cursor_requires_render_update = true;
        fedit->data.visual.requires_render_update = true;
      } break;
      case KEY_CODE_K: { // FROM KEY_CODE_ARROW_DOWN above (TODO refactor into function)
        if (state->cursorLine + 1 >= state->text->lines_count) {
          // Do Nothing
          break;
        }

        // Increment
        ++state->cursorLine;
        int line_len = strlen(state->text->lines[state->cursorLine]);
        if (state->cursorCol > line_len) {
          state->cursorCol = line_len;
        }

        // Update the cursor visual
        state->cursor_requires_render_update = true;
        fedit->data.visual.requires_render_update = true;

        // Adjust display offset
        if (state->cursorLine >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
          // Move display offset down
          state->line_display_offset = state->cursorLine - CODE_EDITOR_RENDERED_CODE_LINES + 1;
        }
      } break;
      case KEY_CODE_I: { // FROM KEY_CODE_ARROW_UP above (TODO refactor into function)
        move_cursor_up(fedit, state);
      } break;
      case KEY_CODE_SEMI_COLON: {
        state->cursorCol = strlen(state->text->lines[state->cursorLine]);

        // Update the cursor visual
        state->cursor_requires_render_update = true;
        fedit->data.visual.requires_render_update = true;
      } break;
      case KEY_CODE_S: {
        // Save the file
        char *filepath;
        switch (state->source_data_type) {
        case CODE_EDITOR_SOURCE_DATA_FUNCTION: {
          mc_function_info_v1 *function = (mc_function_info_v1 *)state->source_data;
          if (!function->source_filepath) {
            printf("function has no source filepath\n");
            break;
          }

          // Read the code from the editor
          char *function_definition;
          read_editor_text_into_cstr(state, &function_definition);

          save_function_to_file(function, function_definition);

          free(function_definition);
        } break;
        case CODE_EDITOR_SOURCE_DATA_STRUCT: {
          mc_struct_info_v1 *structure = (mc_struct_info_v1 *)state->source_data;
          if (!structure->source_filepath) {
            printf("structure has no source filepath\n");
            break;
          }

          // // Read the code from the editor
          char *structure_definition;
          read_editor_text_into_cstr(state, &structure_definition);

          // printf("structure_definition:\n%s||\n", structure_definition);

          save_struct_to_file(structure, structure_definition);
          free(structure_definition);
        } break;
        default: {
          printf("saving source_data_type=%i is not supported\n", state->source_data_type);
        } break;
        }
      } break;
      default: {
        break;
      }
      }
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
      printf("-- state:%p\n", state);
      printf("-- command_hub->clipboard_text:'%s'\n", c);
      printf("-- state->cursorLine:%i\n", state->cursorLine);
      printf("-- state->cursorCol:%i\n", state->cursorCol);
      c[1] = '\0';
      insert_text_into_editor(state, &c[0], state->cursorLine, state->cursorCol);
    }
  } break;
  }
}