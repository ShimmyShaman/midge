/* code_editor_handle_keyboard_input.c */

#include "core/midge_core.h"
void insert_text_into_editor_at_cursor(code_editor_state *state, char *text)
{
  register_midge_error_tag("insert_text_into_editor_at_cursor()");
  // printf("insert_text_into_editor_at_cursor()\n");

  // // TODO errors
  // if (line_index < 0 || line_index >= state->text->lines_count) {
  //   // printf("itie-r0 line_index:%i\n", line_index);
  //   return;
  // }

  // int current_line_len = strlen(state->text->lines[line_index]);
  // if (col < 0 || col > current_line_len) {
  //   // printf("itie-r1 col:%i\n", col);
  //   return;
  // }

  int insert_len = strlen(text);
  if (insert_len < 1) {
    printf("itie-r2\n");
    return;
  }
  // printf("itie-p:%p\n", state);
  // printf("itie-0:\n%s||\n", state->code.rtf->text);
  // printf("itie-1:\n%s||\n", text);
  // printf("itie-2:\n%i||\n", state->cursor.rtf_index);

  insert_into_c_str(state->code.rtf, text, state->cursor.rtf_index);
  // printf("itie-3\n");
  state->cursor.rtf_index += strlen(text);
  update_code_editor_cursor_line_and_column(state);
  // printf("itie-3\n");
  mce_update_rendered_text(state);
  // printf("itie-4\n");
  // update_code_editor_suggestion(state);
}

void move_cursor_up(mc_code_editor_state_v1 *state)
{
  if (state->suggestion_box.visible) {
    // Takes priority
    --state->suggestion_box.selected_index;
    while (state->suggestion_box.selected_index < 0) {
      state->suggestion_box.selected_index += state->suggestion_box.entries.count;
    }

    state->suggestion_box.requires_render_update = true;
    state->visual_node->data.visual.requires_render_update = true;
    return;
  }

  // printf("state->cursor.rtf_index:%i\n", state->cursor.rtf_index);
  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int i = state->cursor.rtf_index;

  if (!state->cursor.zen_col) {
    state->cursor.zen_col = state->cursor.col;
  }

  if (state->cursor.line < 1) {
    return;
  }
  // print_parse_error(code, i, "mcu-initial", "");

  // Find the new line
  --i;
  for (;; --i) {
    if (code[i] == '\n') {
      break;
    }
  }
  --state->cursor.line;
  // print_parse_error(code, i, "mcu-lineup_end", "");

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
  // print_parse_error(code, i, "mcu-line_start", "");

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
      ++i;
      if (code[i] == '[') {
        // Escaped
      }
      else {
        for (;; ++i) {
          // printf("42:'%c'\n", code[i]);
          if (code[i] == '\0') {
            // MCerror(42, "RTF Format Error");
            printf("42 format error\n");
            return;
          }
          else if (code[i] == ']') {
            break;
          }
        }
        continue;
      }
    }
    ++traversed;
    // printf("++traversed:%i\n", traversed);
  }
  // print_parse_error(code, i, "mcu-zen_col", "");
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
  if (state->suggestion_box.visible) {
    // Takes priority
    state->suggestion_box.selected_index =
        (state->suggestion_box.selected_index + 1) % state->suggestion_box.entries.count;

    state->suggestion_box.requires_render_update = true;
    state->visual_node->data.visual.requires_render_update = true;
    return;
  }
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
    // print_parse_error(code, i, "trac", "");
    if (code[i] == '\0') {
      --i;
      break;
    }
    else if (code[i] == '\n') {
      break;
    }
    else if (code[i] == '[') {
      ++i;
      if (code[i] == '[') {
        // Escaped
      }
      else {
        for (;; ++i) {
          if (code[i] == '\0') {
            // MCerror(42, "RTF Format Error");
            printf("42 format error\n");
          }
          else if (code[i] == ']') {
            break;
          }
        }
        continue;
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

void move_cursor_left(mc_code_editor_state_v1 *state)
{
  if (state->cursor.rtf_index == 0) {
    return;
  }

  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int i = state->cursor.rtf_index;

  state->cursor.zen_col = 0;

  // print_parse_error(code, i, "mcu-initial", "");
  // Move
  --i;
  while (code[i] == ']') {
    // Identify if it is a rtf element
    bool is_rtf_element = false;
    int j = i - 1;
    for (;; --j) {
      if (code[j] == '[') {
        is_rtf_element = (code[j - 1] != '[');
        break;
      }
    }
    if (!is_rtf_element) {
      break;
    }

    i = j - 1;
  }
  // print_parse_error(code, i, "mcu-end", "");

  state->cursor.rtf_index = i;
  update_code_editor_cursor_line_and_column(state);

  // Update the cursor visual
  state->cursor.requires_render_update = true;
  state->visual_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
    // Move display offset down
    state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  }
}

void move_cursor_right(mc_code_editor_state_v1 *state)
{
  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int i = state->cursor.rtf_index;

  if (code[i] == '\0') {
    return;
  }

  state->cursor.zen_col = 0;

  print_parse_error(code, i, "mcu-initial", "");
  // Move
  while (code[i] == '[') {
    if (code[i + 1] == '[') {
      ++i;
      break;
    }

    while (code[i] != ']') {
      ++i;
    }
    ++i;
  }
  if (code[i] == '\0') {
    --i;
  }
  ++i;
  // Move
  while (code[i] == '[') {
    if (code[i + 1] == '[') {
      ++i;
      break;
    }

    while (code[i] != ']') {
      ++i;
    }
    ++i;
  }
  if (code[i] == '\0') {
    --i;
  }
  print_parse_error(code, i, "mcu-end", "");

  state->cursor.rtf_index = i;
  update_code_editor_cursor_line_and_column(state);

  // Update the cursor visual
  state->cursor.requires_render_update = true;
  state->visual_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
    // Move display offset down
    state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  }
}

void move_cursor_home(mc_code_editor_state_v1 *state)
{
  if (state->cursor.rtf_index == 0) {
    return;
  }

  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int i = state->cursor.rtf_index;

  state->cursor.zen_col = 0;

  // print_parse_error(code, i, "mcu-initial", "");
  // Move
  --i;
  for (; i > 0; --i) {
    if (code[i] == '\n') {
      ++i;
      break;
    }
    else if (code[i] == ']') {
      // Identify if it is a rtf element
      bool is_rtf_element = false;
      int j = i - 1;
      for (;; --j) {
        if (code[j] == '[') {
          is_rtf_element = (code[j - 1] != '[');
          break;
        }
      }
      if (!is_rtf_element) {
        break;
      }

      i = j;
      continue;
    }
    else if (code[i] == ' ') {
      bool only_white_space = true;
      int j = i - 1;
      for (; j > 0; --j) {
        if (code[j] == '\n') {
          break;
        }
        else if (code[j] == ']') {
          // Identify if it is a rtf element
          bool is_rtf_element = false;
          int k = j - 1;
          for (;; --k) {
            if (code[k] == '[') {
              is_rtf_element = (code[k - 1] != '[');
              break;
            }
          }
          if (!is_rtf_element) {
            only_white_space = false;
            break;
          }
        }
        else if (code[j] != ' ') {
          only_white_space = false;
          break;
        }
      }
      if (only_white_space) {
        ++i;
        break;
      }
    }
  }
  // print_parse_error(code, i, "mcu-end", "");

  state->cursor.rtf_index = i;
  update_code_editor_cursor_line_and_column(state);

  // Update the cursor visual
  state->cursor.requires_render_update = true;
  state->visual_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
    // Move display offset down
    state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  }
}

void move_cursor_end(mc_code_editor_state_v1 *state)
{
  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int i = state->cursor.rtf_index;

  state->cursor.zen_col = 0;

  // Find the new line
  for (;; ++i) {
    if (code[i] == '\0') {
      --i;
      break;
    }
    if (code[i] == '\n') {
      break;
    }
  }
  // print_parse_error(code, i, "mcu-end", "");

  state->cursor.rtf_index = i;
  update_code_editor_cursor_line_and_column(state);

  // Update the cursor visual
  state->cursor.requires_render_update = true;
  state->visual_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
    // Move display offset down
    state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  }
}

void backspace_from_cursor(mc_code_editor_state_v1 *state)
{
  if (state->cursor.rtf_index == 0) {
    return;
  }

  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int si = state->cursor.rtf_index - 1;

  while (code[si] == ']') {
    // Identify if it is a rtf element
    bool is_rtf_element = false;
    int j = si - 1;
    for (;; --j) {
      if (code[j] == '[') {
        is_rtf_element = (code[j - 1] != '[');
        break;
      }
    }
    if (!is_rtf_element) {
      break;
    }

    si = j - 1;
  }

  // Copy over it
  int gap = state->cursor.rtf_index - si;
  for (int a = si;; ++a) {
    code[a] = code[a + gap];
    if (code[a] == '\0') {
      break;
    }
  }

  // printf("code:\n%s||\n", code);

  state->cursor.rtf_index = si;
  update_code_editor_cursor_line_and_column(state);
  mce_update_rendered_text(state);

  // Update the cursor visual
  state->cursor.requires_render_update = true;
  state->visual_node->data.visual.requires_render_update = true;
  state->suggestion_box.visible = false;

  // Adjust display offset
  if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
    // Move display offset down
    state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  }

  return;
}

void auto_fill_code_editor_suggestion(mc_code_editor_state_v1 *state)
{

  if (state->suggestion_box.selected_index >= state->suggestion_box.entries.count) {
    return;
  }

  // Get the complete word surrounding the cursor
  char *code = state->code.rtf->text;
  int s = state->cursor.rtf_index;
  while (s > 0) {
    --s;
    bool brk = false;
    switch (code[s]) {
    case ' ':
    case '\n':
    case ';':
    case '[':
    case ')':
    case ',': {
      brk = true;
      ++s;
    } break;
    case ']': {
      // Discover whether it is a rtf attribute or code access operator close
      bool rtf_attribute = false;
      // for(int r = s - 1; r >= 0; --r) {
      //   if(code[r] == '[') {
      //     rtf_attribute = code[r - 1] != '[';
      //   }
      // }
      // if(rtf_attribute) {

      // }
      brk = true;
      ++s;
    } break;
    default:
      break;
    }
    if (brk) {
      break;
    }
  }

  // Copy over it
  int gap = state->cursor.rtf_index - s;
  for (int a = s;; ++a) {
    code[a] = code[a + gap];
    if (code[a] == '\0') {
      break;
    }
  }
  state->cursor.rtf_index = s;

  insert_text_into_editor_at_cursor(state, state->suggestion_box.entries.items[state->suggestion_box.selected_index]);

  state->suggestion_box.visible = false;
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
  case KEY_CODE_ESCAPE: {
    if (state->suggestion_box.visible) {
      event->handled = true;

      state->suggestion_box.visible = false;
      state->visual_node->data.visual.requires_render_update = true;
    }
  } break;
  case KEY_CODE_BACKSPACE: {
    event->handled = true;

    backspace_from_cursor(state);
  } break;
  case KEY_CODE_ENTER:
  case KEY_CODE_RETURN: {

    char c[2];
    c[0] = '\n';
    c[1] = '\0';
    insert_text_into_editor_at_cursor(state, &c[0]);
  } break;
  case KEY_CODE_TAB: {
    if (state->suggestion_box.visible) {

      auto_fill_code_editor_suggestion(state);
    }
    else {

      char c[3];
      c[0] = ' ';
      c[1] = ' ';
      c[2] = '\0';
      insert_text_into_editor_at_cursor(state, &c[0]);
    }
  } break;
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
  case KEY_CODE_ARROW_LEFT: {
    event->handled = true;

    move_cursor_left(state);
  } break;
  case KEY_CODE_ARROW_RIGHT: {
    event->handled = true;

    move_cursor_right(state);
  } break;
  case KEY_CODE_HOME: {
    event->handled = true;

    move_cursor_home(state);
  } break;
  case KEY_CODE_END: {
    event->handled = true;

    move_cursor_end(state);
  } break;
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
  default: {
    if (event->altDown) {
      switch (event->detail.keyboard.key) {
      case KEY_CODE_A: {
        move_cursor_home(state);
      } break;
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
      case KEY_CODE_I: {
        for (int i = 0; i < 6; ++i) {
          move_cursor_up(state);
        }
      } break;
      default: {
        break;
      }
      }
    }
    else if (event->ctrlDown) {
      switch (event->detail.keyboard.key) {
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
      case KEY_CODE_L: {
        move_cursor_right(state);
      } break;
      case KEY_CODE_K: {
        move_cursor_down(state);
      } break;
      case KEY_CODE_J: {
        move_cursor_left(state);
      } break;
      case KEY_CODE_I: {
        move_cursor_up(state);
      } break;
      case KEY_CODE_SEMI_COLON: {
        move_cursor_end(state);
      } break;
      case KEY_CODE_S: {
        // Save the file
        if (!state->source_data || !state->source_data->source_file) {
          printf("code has no source file\n");
          break;
        }

        char *filepath;
        switch (state->source_data->type) {
        case SOURCE_DEFINITION_FUNCTION: {
          // mc_function_info_v1 *function = (mc_source_definition_v1 *)state->source_data;

          // Read the code from the editor into the function source
          char *function_definition;
          read_editor_text_into_cstr(state, &function_definition);

          if (state->source_data->code) {
            free(state->source_data->code);
          }
          state->source_data->code = function_definition;

          save_function_to_file(state->source_data->func_info);

          free(function_definition);
        } break;
        // case SOURCE_DEFINITION_STRUCT: {
        //   mc_struct_info_v1 *structure = (mc_struct_info_v1 *)state->source_data;

        //   // // Read the code from the editor
        //   char *structure_definition;
        //   read_editor_text_into_cstr(state, &structure_definition);

        //   // printf("structure_definition:\n%s||\n", structure_definition);

        //   save_struct_to_file(structure, structure_definition);
        //   free(structure_definition);
        // } break;
        default: {
          printf("saving source_data_type=%i is not supported\n", state->source_data->type);
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

      uint action_uid;
      {
        // Register User Entry
        void **context = (void **)malloc(sizeof(void *) * 2);
        int *source_type = (int *)malloc(sizeof(int));
        *source_type = (int)state->source_data->type;
        context[0] = (void *)source_type;
        if (state->source_data->type == SOURCE_DEFINITION_FUNCTION) {
          int *context_syntax_node_type = (int *)malloc(sizeof(int));
          *context_syntax_node_type = state->source_data->type;
          context[1] = (void *)context_syntax_node_type;
        }
        else {
          context[1] = NULL;
        }

        register_user_action(state->entry_pad, (uint)event->detail.keyboard.key, context, &action_uid);
      }

      // Update the text
      {
        // if (state->selection_exists) {
        //   // Delete selection
        //   delete_selection(state);
        // }

        c[1] = '\0';
        insert_text_into_editor_at_cursor(state, &c);
        {
          void **result = (void **)malloc(sizeof(void *) * 1);
          allocate_and_copy_cstr(result[0], c);
          report_user_action_effect(state->entry_pad, action_uid, result);
        }
      }
    }
  } break;
  }
}