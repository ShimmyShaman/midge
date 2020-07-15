
void insert_text_into_editor(code_editor_state *state, char *text, int line_index, int col)
{
  // TODO errors
  if (line < 0 || line >= state->text->line_count || col < 0 || col >= strlen(state->text->lines[line])) {
    return;
  }

  int insert_len = strlen(text);
  if (insert_len < 1) {
    return;
  }

  int i = 0;
  unsigned int line_alloc = col + 6;
  char *line = (char *)malloc(sizeof(char) * line_alloc);
  line[0] = '\0';

  int current_line_len = strlen(state->text->lines[line_index]);
  if (col && current_line_len - col > 0) {
    // Copy the 'before' section of the current line
    append_to_cstrn(&line_alloc, &line, state->text->lines[line_index], col);

    bool eof = false;
    for (;; ++i) {
      if (text[i] == '\0') {
        eof = true;
        break;
      }
      else if (text[i] == '\n') {
        ++i;
        break;
      }
    }
    append_to_cstrn(&line_alloc, &line, text, i);

    if (eof) {
      // Append the 'after' section of the current line
      append_to_cstrn(&line_alloc, &line, state->text->lines[line_index] + col);
    }

    // Set to the current line
    free(state->text->lines[line]);
    state->text->lines[line] = line;

    if ((int)line - state->line_display_offset > 0 &&
        line - state->line_display_offset < CODE_EDITOR_RENDERED_CODE_LINES) {
      state->render_lines[line - state->line_display_offset]->requires_render_update = true;
      state->cursor_requires_render_update = true; // TODO this field should be a
                                                   // code-editor_lines_requires_render_update one ie.
    }

    if (eof) {
      return;
    }

    ++line;
  }

  while (1) {
    // Find the end of the next line
    int s = i;
    bool eof = false;
    for (;; ++i) {
      if (text[i] == '\0') {
        eof = true;
        break;
      }
      else if (text[i] == '\n') {
        ++i;
        break;
      }
    }

    append_to_cstrn(&line_alloc, &line, text + s, i - s);

    // Insert into editor

    int current_line_len = strlen(state->text->lines[line]);

    unsigned int line_alloc = current_line_len + 6;
    char *line = (char *)malloc(sizeof(char) * line_alloc);

    if (state->cursorCol) {
      strncpy(new_line, state->text->lines[line], state->cursorCol);
    }
    new_line[state->cursorCol] = c;
    new_line[state->cursorCol + 1] = '\0';
    if (current_line_len - state->cursorCol) {
      strcat(new_line, state->text->lines[line] + state->cursorCol);
    }
    new_line[current_line_len + 1] = '\0';

    // printf("new_line:'%s'\n", new_line);

    free(state->text->lines[line]);
    state->text->lines[line] = new_line;

    ++state->cursorCol;
  }