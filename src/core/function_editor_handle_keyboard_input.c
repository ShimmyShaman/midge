/* function_editor_handle_keyboard_input.c */

#include "core/midge_core.h"

void function_editor_handle_keyboard_input_v1(frame_time *elapsed, mc_node_v1 *fedit, mc_input_event_v1 *event)
{
  function_editor_state *state = (function_editor_state *)fedit->extra;

  switch (event->detail.keyboard.key) {
  case KEY_CODE_BACKSPACE: {
    event->handled = true;
    if (!state->cursorCol) {
      if (state->cursorLine) {
        // Combine previous line & second line into one
        int previous_line_len = strlen(state->text.lines[state->cursorLine - 1]);
        char *combined = (char *)malloc(sizeof(char) * (previous_line_len + strlen(state->text.lines[state->cursorLine]) + 1));
        strcpy(combined, state->text.lines[state->cursorLine - 1]);
        strcat(combined, state->text.lines[state->cursorLine]);

        free(state->text.lines[state->cursorLine - 1]);
        free(state->text.lines[state->cursorLine]);
        state->text.lines[state->cursorLine - 1] = combined;

        // Bring all lines up one position
        for (int i = state->cursorLine + 1; i < state->text.lines_count; ++i) {
          state->text.lines[i - 1] = state->text.lines[i];
        }
        state->text.lines[state->text.lines_count - 1] = NULL;
        --state->text.lines_count;

        --state->cursorLine;
        state->cursorCol = previous_line_len;
      }
      break;
    }

    // Bring all forward characters back one
    int line_len = strlen(state->text.lines[state->cursorLine]);
    for (int i = state->cursorCol - 1; i < line_len; ++i) {
      state->text.lines[state->cursorLine][i] = state->text.lines[state->cursorLine][i + 1];
    }

    --state->cursorCol;
  } break;
  case KEY_CODE_ENTER:
  case KEY_CODE_RETURN: {
    event->handled = true;
    if (event->ctrlDown) {

      // Read the code from the editor
      char *function_declaration;
      read_function_from_editor(state, &function_declaration);

      mc_function_info_v1 *func_info;
      parse_and_process_function_definition(function_declaration, &func_info, false);
      free(function_declaration);

      // Compile the function definition
      uint transcription_alloc = 4;
      char *transcription = (char *)malloc(sizeof(char) * transcription_alloc);
      transcription[0] = '\0';
      int code_index = 0;
      transcribe_c_block_to_mc(func_info, func_info->mc_code, &code_index, &transcription_alloc, &transcription);

      printf("final transcription:\n%s\n", transcription);

      // Define the new function
      instantiate_function(func_info->name, transcription);
      //   {
      //     void *mc_vargs[3];
      //     mc_vargs[0] = (void *)&func_info->name;
      //     mc_vargs[1] = (void *)&transcription;
      //     MCcall(instantiate_function(2, mc_vargs));
      //   }

      event->handled = true;
      return;
    }
    else {
      printf("fehi-0\n");
      // Newline -- carrying over any extra
      char *cursorLine = state->text.lines[state->cursorLine];

      // Automatic indent
      int automaticIndent = 0;
      int cursorLineLen = strlen(cursorLine);
      for (; automaticIndent < state->cursorCol && automaticIndent < cursorLineLen; ++automaticIndent) {
        if (cursorLine[automaticIndent] != ' ') {
          break;
        }
      }

      // Increment lines after by one place
      if (state->text.lines_count + 1 >= state->text.lines_allocated) {
        uint new_alloc = state->text.lines_allocated + 4 + state->text.lines_allocated / 4;
        char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
        if (state->text.lines_allocated) {
          memcpy(new_ary, state->text.lines, state->text.lines_allocated * sizeof(char *));
          free(state->text.lines);
        }
        for (int i = state->text.lines_allocated; i < new_alloc; ++i) {
          new_ary[i] = NULL;
        }

        state->text.lines_allocated = new_alloc;
        state->text.lines = new_ary;
      }
      // printf("state->text.lines_allocated:%u state->text.lines_count:%u state->cursorLine:%u\n",
      //   state->text.lines_allocated,
      //  state->text.lines_count, state->cursorLine);
      for (int i = state->text.lines_count; i > state->cursorLine + 1; --i) {

        state->text.lines[i] = state->text.lines[i - 1];
      }
      state->text.lines[state->cursorLine + 1] = NULL;
      ++state->text.lines_count;

      // printf("fehi-1\n");
      if (state->cursorCol >= cursorLineLen) {
        // printf("fehi-1A\n");
        // Just create new line
        char *newLine = (char *)malloc(sizeof(char) * (automaticIndent + 1));
        for (int i = 0; i < automaticIndent; ++i) {
          newLine[i] = ' ';
        }
        newLine[automaticIndent] = '\0';
        state->text.lines[state->cursorLine + 1] = newLine;
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
        memcpy(secondSplit + automaticIndent, cursorLine + state->cursorCol,
               sizeof(char) * (cursorLineLen - state->cursorCol + 1));
        secondSplit[cursorLineLen - state->cursorCol] = '\0';

        free(cursorLine);
        state->text.lines[state->cursorLine] = firstSplit;
        state->text.lines[state->cursorLine + 1] = secondSplit;
      }
      else {
        // printf("fehi-1C\n");
        // Move the rest of the current line forwards
        state->text.lines[state->cursorLine + 1] = cursorLine;
        state->text.lines[state->cursorLine] = (char *)malloc(sizeof(char) * (automaticIndent + 1));
        for (int i = 0; i < automaticIndent; ++i) {
          state->text.lines[state->cursorLine][i] = ' ';
        }
      }

      printf("fehi-2\n");
      // Cursor Position Update
      ++state->cursorLine;
      state->cursorCol = automaticIndent;
    }
  } break;
  default: {
    // printf("fehi-3\n");
    char c = '\0';
    int res = get_key_input_code_char(event->shiftDown, event->detail.keyboard.key, &c);
    if (res) {
      return; // TODO
    }
    event->handled = true;

    // Update the text
    {
      int current_line_len = strlen(state->text.lines[state->cursorLine]);
      char *new_line = (char *)malloc(sizeof(char) * (current_line_len + 1 + 1));
      if (state->cursorCol) {
        strncpy(new_line, state->text.lines[state->cursorLine], state->cursorCol);
      }
      new_line[state->cursorCol] = c;
      if (current_line_len - state->cursorCol) {
        strcat(new_line + state->cursorCol + 1, state->text.lines[state->cursorLine]);
      }
      new_line[current_line_len + 1] = '\0';

      free(state->text.lines[state->cursorLine]);
      state->text.lines[state->cursorLine] = new_line;

      ++state->cursorCol;
    }
  } break;
  }
}