
void insert_text_into_editor(code_editor_state *state, char *text, int line_index, int col)
{
  // printf("insert_text_into_editor()\n");

  // TODO errors
  if (line_index < 0 || line_index >= state->text->lines_count) {
    // printf("itie-r0 line_index:%i\n", line_index);
    return;
  }

  int current_line_len = strlen(state->text->lines[line_index]);
  if (col < 0 || col > current_line_len) {
    // printf("itie-r1 col:%i\n", col);
    return;
  }

  int insert_len = strlen(text);
  if (insert_len < 1) {
    // printf("itie-r2\n");
    return;
  }

  // printf("itie-0\n");

  // The line
  int i = 0;
  unsigned int line_alloc = col + 6;
  char *line = (char *)malloc(sizeof(char) * line_alloc);
  line[0] = '\0';

  // Copy the 'before' section of the current line
  if (col == 0 || current_line_len == 0) {
    // Do nothing
  }
  else {
    append_to_cstrn(&line_alloc, &line, state->text->lines[line_index], col);
  }

  // Move through the text to identify the end of any line (or the text)
  bool eof = false;
  while (1) {
    if (text[i] == '\0') {
      eof = true;
      break;
    }
    else if (text[i] == '\n') {
      break;
    }
    ++i;
  }
  ++i;

  if (i > 0) {
    append_to_cstrn(&line_alloc, &line, text, i);
  }

  // 'After' Section
  char *after_section;
  // printf("itie-2\n");
  if (eof) {
    // Append the 'after' section of the current line
    append_to_cstr(&line_alloc, &line, state->text->lines[line_index] + col);
  }
  else {
    // Set the 'after' section for later use
    if (col == current_line_len) {
      allocate_and_copy_cstr(after_section, "");
    }
    else {
      allocate_and_copy_cstr(after_section, state->text->lines[line_index] + col);
    }
  }

  // Set to the current line
  // printf("itie-3\n");
  free(state->text->lines[line_index]);
  state->text->lines[line_index] = line;
  // printf("itie- setting line:'%s'\n", line);

  // if ((int)line_index - state->line_display_offset > 0 &&
  //     line_index - state->line_display_offset < CODE_EDITOR_RENDERED_CODE_LINES) {
  //   state->render_lines[line_index - state->line_display_offset]->requires_render_update = true;
  //   state->cursor_requires_render_update = true; // TODO this field should be a
  //                                                // code-editor_lines_requires_render_update one ie.
  // }

  // printf("itie-4\n");
  if (eof) {
    // In-line cursor adjustment
    state->cursorCol += insert_len;
    return;
  }
  ++line_index;

  // Batch the remaining line updates
  // printf("itie-5\n");
  while (1) {
    // New line
    line_alloc = 1;
    line = (char *)malloc(sizeof(char) * line_alloc);
    line[0] = '\0';

    // Find the end of the next line
    int s = i;
    bool eof = false;
    while (1) {
      if (text[i] == '\0') {
        eof = true;
        break;
      }
      else if (text[i] == '\n') {
        break;
      }
      ++i;
    }

    // printf("itie-6\n");
    if (i - s > 0) {
      append_to_cstrn(&line_alloc, &line, text + s, i - s);
    }
    else {
      // Nothing
    }

    // printf("itie-7\n");
    if (eof) {
      state->cursorLine = line_index;
      state->cursorCol = strlen(line);

      append_to_cstr(&line_alloc, &line, after_section);
      free(after_section);
    }

    // printf("itie-8\n");
    insert_in_collection((void ***)&state->text->lines, &state->text->lines_alloc, &state->text->lines_count,
                         line_index, line);
    ++line_index;

    if (eof) {
      break;
    }

    // Increment
    ++i;

    // printf("itie-9\n");
  }

  // Insert into editor

  // int current_line_len = strlen(state->text->lines[line]);

  //   // unsigned int line_alloc = current_line_len + 6;
  //   // char *line = (char *)malloc(sizeof(char) * line_alloc);

  //   // if (state->cursorCol) {
  //   //   strncpy(new_line, state->text->lines[line], state->cursorCol);
  //   // }
  //   // new_line[state->cursorCol] = c;
  //   // new_line[state->cursorCol + 1] = '\0';
  //   // if (current_line_len - state->cursorCol) {
  //   //   strcat(new_line, state->text->lines[line] + state->cursorCol);
  //   // }
  //   // new_line[current_line_len + 1] = '\0';

  //   // // printf("new_line:'%s'\n", new_line);

  //   // free(state->text->lines[line]);
  //   // state->text->lines[line] = new_line;

  //   // ++state->cursorCol;
  // }
}