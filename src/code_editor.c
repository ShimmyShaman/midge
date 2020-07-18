#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

// typedef struct editor_panel_button_info {
//   struct {
//     uint x, y, actual_width, actual_height, max_width, max_height;
//   } bounds;
//   uint image_resource_uid;
//   char *button_text;
// } editor_panel_button_info;

int build_code_editor_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/
  // printf("bfe-a\n");

  // Build the function editor window
  // Instantiate: node global;
  mc_node_v1 *fedit = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  fedit->name = "code_editor";
  fedit->parent = command_hub->global_node;
  fedit->type = NODE_TYPE_VISUAL;

  fedit->data.visual.bounds.x = 298;
  fedit->data.visual.bounds.y = 40;
  fedit->data.visual.bounds.width = APPLICATION_SET_WIDTH - 298;
  fedit->data.visual.bounds.height = APPLICATION_SET_HEIGHT - fedit->data.visual.bounds.y;
  fedit->data.visual.image_resource_uid = 0;
  fedit->data.visual.requires_render_update = true;
  fedit->data.visual.render_delegate = &code_editor_render;
  fedit->data.visual.hidden = true;
  fedit->data.visual.input_handler = &code_editor_handle_input;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, fedit));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Code Lines
  mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)malloc(sizeof(mc_code_editor_state_v1));
  // printf("state:'%p'\n", state);
  state->visual_node = fedit;
  state->in_view_function_live_debugger = false;
  state->source_data_type = CODE_EDITOR_SOURCE_DATA_NONE;
  state->source_data = NULL;
  state->font_resource_uid = 0;
  state->cursorLine = 0;
  state->cursorCol = 0;
  state->line_display_offset = 0;
  state->text = (mc_cstring_list_v1 *)malloc(sizeof(mc_cstring_list_v1));
  state->text->lines_alloc = 8;
  state->text->lines = (char **)calloc(sizeof(char *), state->text->lines_alloc);
  state->text->lines_count = 0;
  state->render_lines = (rendered_code_line **)malloc(sizeof(rendered_code_line *) * CODE_EDITOR_RENDERED_CODE_LINES);
  // state->rendered_line_count = CODE_EDITOR_RENDERED_CODE_LINES;

  {
    // FunctionLiveDebug
    state->fld_view = (fld_view_state *)malloc(sizeof(fld_view_state));
    state->fld_view->declare_incremental_uid = 0;
    state->fld_view->arguments.alloc = 0;
    state->fld_view->arguments.count = 0;
    state->fld_view->visual_code.alloc = 0;
    state->fld_view->visual_code.count = 0;
  }

  for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
    state->render_lines[i] = (rendered_code_line *)malloc(sizeof(rendered_code_line));

    state->render_lines[i]->index = i;
    state->render_lines[i]->requires_render_update = true;
    state->render_lines[i]->text = NULL;
    //  "!this is twenty nine letters! "
    //  "!this is twenty nine letters! "
    //  "!this is twenty nine letters! ";

    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &state->render_lines[i]->image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = state->render_lines[i]->width = fedit->data.visual.bounds.width - 4;
    command->data.create_texture.height = state->render_lines[i]->height = 28;
  }
  fedit->extra = (void *)state;

  {
    // Status Bar
    state->status_bar.bounds.x = 2;
    state->status_bar.bounds.y = fedit->data.visual.bounds.height - 64;
    state->status_bar.bounds.width = fedit->data.visual.bounds.width - 4;
    state->status_bar.bounds.height = 62;
    state->status_bar.image_resource_uid = 0;
    state->status_bar.message = NULL;
    state->status_bar.requires_render_update = true;

    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &state->status_bar.image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = state->status_bar.bounds.width;
    command->data.create_texture.height = state->status_bar.bounds.height;
  }

  // state->editor_button_panel.alloc = 0;
  // state->editor_button_panel.count = 0;
  // for (int i = 0; i < 2; ++i) {
  //   editor_panel_button_info *button = (editor_panel_button_info *)malloc(sizeof(editor_panel_button_info));

  //   button->bounds.max_width = 160;
  //   button->bounds.max_height = 24;

  //   MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  //   command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  //   command->p_uid = &button->image_resource_uid;
  //   command->data.create_texture.use_as_render_target = true;
  //   command->data.create_texture.width = fedit->data.visual.bounds.max_width;
  //   command->data.create_texture.height = fedit->data.visual.bounds.max_height;
  // }

  // Function Editor Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &fedit->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = fedit->data.visual.bounds.width;
  command->data.create_texture.height = fedit->data.visual.bounds.height;

  // MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  // command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  // command->p_uid = &console->input_line.image_resource_uid;
  // command->data.create_texture.use_as_render_target = true;
  // command->data.create_texture.width = console->input_line.width;
  // command->data.create_texture.height = console->input_line.height;

  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &state->font_resource_uid;
  command->data.font.height = 20;
  command->data.font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  return 0;
}

int code_editor_render_fld_view_code(frame_time const *elapsed, mc_node_v1 *visual_node)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // Entity States
  mc_code_editor_state_v1 *cestate = (mc_code_editor_state_v1 *)visual_node->extra;
  fld_view_state *fld_view = cestate->fld_view;

  image_render_queue *sequence;
  element_render_command *element_cmd;

  {
    const int EDITOR_LINE_STRIDE = 22;
    const float EDITOR_FONT_HORIZONTAL_STRIDE = 10.31f;
    int line_index = 0;
    int col_index = 0;

    // printf("fldr-0 fld_view->visual_code.count:%i\n", fld_view->visual_code.count);
    int previous_line_index = -1;
    int ce_offset_line_index;
    bool line_is_visible;
    for (int i = 0; i < fld_view->visual_code.count; ++i) {

      if (line_index != previous_line_index) {
        ce_offset_line_index = line_index - cestate->line_display_offset;
        line_is_visible = (ce_offset_line_index >= 0 && ce_offset_line_index < CODE_EDITOR_RENDERED_CODE_LINES);
        previous_line_index = line_index;

        // Render Line
        if (line_is_visible) {
          MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
          sequence->render_target = NODE_RENDER_TARGET_IMAGE;
          sequence->clear_color = COLOR_TRANSPARENT;
          sequence->image_width = cestate->render_lines[ce_offset_line_index]->width;
          sequence->image_height = cestate->render_lines[ce_offset_line_index]->height;
          sequence->data.target_image.image_uid = cestate->render_lines[ce_offset_line_index]->image_resource_uid;

          if (cestate->render_lines[ce_offset_line_index]->text) {
            free(cestate->render_lines[ce_offset_line_index]->text);
            cestate->render_lines[ce_offset_line_index]->text = NULL;
          }
        }
      }

      switch (fld_view->visual_code.items[i]->type) {
      case FLD_CODE_CSTRING: {
        char *text = (char *)fld_view->visual_code.items[i]->data;
        int c = 0;
        while (1) {
          int s = c;
          bool eos = false;
          for (;; ++c) {
            if (text[c] == '\0') {
              eos = true;
              break;
            }
            else if (text[c] == '\n') {
              break;
            }
          }

          if (line_index != previous_line_index) {
            ce_offset_line_index = line_index - cestate->line_display_offset;
            line_is_visible = (ce_offset_line_index >= 0 && ce_offset_line_index < CODE_EDITOR_RENDERED_CODE_LINES);
            previous_line_index = line_index;

            // Render Line
            if (line_is_visible) {
              MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
              sequence->render_target = NODE_RENDER_TARGET_IMAGE;
              sequence->clear_color = COLOR_TRANSPARENT;
              sequence->image_width = cestate->render_lines[ce_offset_line_index]->width;
              sequence->image_height = cestate->render_lines[ce_offset_line_index]->height;
              sequence->data.target_image.image_uid = cestate->render_lines[ce_offset_line_index]->image_resource_uid;

              if (cestate->render_lines[ce_offset_line_index]->text) {
                free(cestate->render_lines[ce_offset_line_index]->text);
                cestate->render_lines[ce_offset_line_index]->text = NULL;
              }
            }
          }

          // Print sequence
          if (line_is_visible && c - s > 0) {
            MCcall(obtain_element_render_command(sequence, &element_cmd));
            element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
            element_cmd->x = 4 + col_index * EDITOR_FONT_HORIZONTAL_STRIDE;
            element_cmd->y = 2 + 12;
            element_cmd->data.print_text.font_resource_uid = cestate->font_resource_uid;
            allocate_and_copy_cstrn(element_cmd->data.print_text.text, text + s, c - s);
            element_cmd->data.print_text.color = (render_color){0.61f, 0.86f, 0.99f, 1.f};

            // printf("printing to :%i:'%s'\n", ce_offset_line_index, element_cmd->data.print_text.text);

            col_index += c - s;
          }

          if (eos) {
            break;
          }

          ++line_index;
          col_index = 0;
          ++c;
        }
      } break;
      case FLD_CODE_SNAPSHOT: {
        if (!line_is_visible) {
          break;
        }

        fld_variable_snapshot *snapshot = (fld_variable_snapshot *)fld_view->visual_code.items[i]->data;

        MCcall(obtain_element_render_command(sequence, &element_cmd));
        element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
        element_cmd->x = 4 + col_index * EDITOR_FONT_HORIZONTAL_STRIDE;
        element_cmd->y = 2 + 12;
        element_cmd->data.print_text.font_resource_uid = cestate->font_resource_uid;
        allocate_and_copy_cstr(element_cmd->data.print_text.text, snapshot->value_text);
        element_cmd->data.print_text.color = COLOR_YELLOW;

        col_index += strlen(snapshot->value_text);
      } break;
      default: {
        MCerror(456, "TODO:%i", fld_view->visual_code.items[i]->type);
      }
      }
    }
  }
  return 0;
}

int code_editor_render_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // printf("code_editor_render_v1-a\n");
  frame_time const *elapsed = *(frame_time const **)argv[0];
  mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

  // printf("command_hub->interactive_console->visual.image_resource_uid=%u\n",
  //        command_hub->interactive_console->visual.image_resource_uid);
  image_render_queue *sequence;
  element_render_command *element_cmd;
  // Lines
  mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)visual_node->extra;

  // printf("fer-b\n");
  if (state->in_view_function_live_debugger) {
    MCcall(code_editor_render_fld_view_code(elapsed, visual_node));
  }
  else {
    for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
      if (state->render_lines[i]->requires_render_update) {
        state->render_lines[i]->requires_render_update = false;

        // printf("fer-c\n");
        MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
        sequence->render_target = NODE_RENDER_TARGET_IMAGE;
        sequence->clear_color = COLOR_TRANSPARENT;
        sequence->image_width = state->render_lines[i]->width;
        sequence->image_height = state->render_lines[i]->height;
        sequence->data.target_image.image_uid = state->render_lines[i]->image_resource_uid;

        // printf("fer-d\n");
        if (state->render_lines[i]->text && strlen(state->render_lines[i]->text)) {
          // printf("fer-e\n");
          MCcall(obtain_element_render_command(sequence, &element_cmd));
          element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
          element_cmd->x = 4;
          element_cmd->y = 2 + 12;
          allocate_and_copy_cstr(element_cmd->data.print_text.text, state->render_lines[i]->text);
          element_cmd->data.print_text.font_resource_uid = state->font_resource_uid;
          element_cmd->data.print_text.color = (render_color){0.61f, 0.86f, 0.99f, 1.f};
        }
        // printf("fer-f\n");
      }
    }
  }

  {
    // Status Bar
    MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
    sequence->render_target = NODE_RENDER_TARGET_IMAGE;
    sequence->image_width = state->status_bar.bounds.width;
    sequence->image_height = state->status_bar.bounds.height;
    sequence->clear_color = COLOR_NEARLY_BLACK;
    sequence->data.target_image.image_uid = state->status_bar.image_resource_uid;

    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    element_cmd->x = 0;
    element_cmd->y = 0;
    element_cmd->data.colored_rect_info.width = state->status_bar.bounds.width;
    element_cmd->data.colored_rect_info.height = 2;
    element_cmd->data.colored_rect_info.color = COLOR_GHOST_WHITE;

    // MCcall(obtain_element_render_command(sequence, &element_cmd));
    // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    // element_cmd->x = 0;
    // element_cmd->y = state->status_bar.bounds.height - 2;
    // element_cmd->data.colored_rect_info.width = state->status_bar.bounds.width;
    // element_cmd->data.colored_rect_info.height = 2;
    // element_cmd->data.colored_rect_info.color = COLOR_GHOST_WHITE;
  }

  // Render Main Image
  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  sequence->image_width = visual_node->data.visual.bounds.width;
  sequence->image_height = visual_node->data.visual.bounds.height;
  sequence->clear_color = COLOR_GHOST_WHITE;
  sequence->data.target_image.image_uid = visual_node->data.visual.image_resource_uid;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  element_cmd->x = 2;
  element_cmd->y = 2;
  element_cmd->data.colored_rect_info.width = visual_node->data.visual.bounds.width - 4;
  element_cmd->data.colored_rect_info.height = visual_node->data.visual.bounds.height - 4;
  element_cmd->data.colored_rect_info.color = COLOR_NEARLY_BLACK;

  const int EDITOR_LINE_STRIDE = 22;
  const float EDITOR_FONT_HORIZONTAL_STRIDE = 10.31f;
  if (state->selection_exists) {
    // Obtain selection bounds
    int selection_start_line, selection_start_col, selection_end_line, selection_end_col;
    if (state->selection_begin_line > state->cursorLine ||
        (state->selection_begin_line == state->cursorLine && state->selection_begin_col > state->cursorCol)) {
      // The selection begin comes after the cursor
      selection_start_line = (int)state->cursorLine - state->line_display_offset;
      selection_start_col = state->cursorCol;
      selection_end_line = (int)state->selection_begin_line - state->line_display_offset;
      selection_end_col = state->selection_begin_col;
    }
    else {
      // The selection begin comes before the cursor
      selection_start_line = (int)state->selection_begin_line - state->line_display_offset;
      selection_start_col = state->selection_begin_col;
      selection_end_line = (int)state->cursorLine - state->line_display_offset;
      selection_end_col = state->cursorCol;
    }

    // First line (partial)
    if (selection_start_line - state->line_display_offset >= 0 &&
        selection_start_line - state->line_display_offset < CODE_EDITOR_RENDERED_CODE_LINES) {
      int selected_columns;
      if (state->cursorLine == state->selection_begin_line) {
        selected_columns = selection_end_col - selection_start_col;
      }
      else if (state->render_lines[selection_start_line]->text) {
        selected_columns = strlen(state->render_lines[selection_start_line]->text) - selection_start_col + 1;
      }
      else
        selected_columns = 1;

      if (selected_columns > 0) {
        MCcall(obtain_element_render_command(sequence, &element_cmd));
        element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
        element_cmd->x = 6 + (uint)(selection_start_col * EDITOR_FONT_HORIZONTAL_STRIDE);
        element_cmd->y = 7 + (selection_start_line - (int)state->line_display_offset) * EDITOR_LINE_STRIDE;
        element_cmd->data.colored_rect_info.width = selected_columns * EDITOR_FONT_HORIZONTAL_STRIDE;
        element_cmd->data.colored_rect_info.height = EDITOR_LINE_STRIDE;
        element_cmd->data.colored_rect_info.color = (render_color){173.f / 255.f, 109.f / 255.f, 42.f / 255.f, 0.7f};
      }
    }

    // Lines between (if they exist)
    int between_offset_start = max(0, selection_start_line + 1 - state->line_display_offset);
    int between_offset_exclusive_end =
        min(selection_end_line - state->line_display_offset, CODE_EDITOR_RENDERED_CODE_LINES);
    for (int i = between_offset_start; i < between_offset_exclusive_end; ++i) {

      if (state->render_lines[i + state->line_display_offset]->text) {
        int selected_columns = strlen(state->render_lines[i + state->line_display_offset]->text) + 1;

        if (selected_columns > 0) {
          MCcall(obtain_element_render_command(sequence, &element_cmd));
          element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
          element_cmd->x = 6;
          element_cmd->y = 7 + i * EDITOR_LINE_STRIDE;
          element_cmd->data.colored_rect_info.width = selected_columns * EDITOR_FONT_HORIZONTAL_STRIDE;
          element_cmd->data.colored_rect_info.height = EDITOR_LINE_STRIDE;
          element_cmd->data.colored_rect_info.color = (render_color){173.f / 255.f, 109.f / 255.f, 42.f / 255.f, 0.7f};
        }
      }
    }

    // Last line (if it exists)
    if (selection_end_line > selection_start_line && selection_end_line - state->line_display_offset >= 0 &&
        selection_end_line - state->line_display_offset < CODE_EDITOR_RENDERED_CODE_LINES) {

      if (state->render_lines[selection_end_line - state->line_display_offset]->text) {
        int selected_columns =
            min(selection_end_col, strlen(state->render_lines[selection_end_line - state->line_display_offset]->text));

        if (selected_columns > 0) {
          MCcall(obtain_element_render_command(sequence, &element_cmd));
          element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
          element_cmd->x = 6;
          element_cmd->y = 7 + (selection_end_line - state->line_display_offset) * EDITOR_LINE_STRIDE;
          element_cmd->data.colored_rect_info.width = selected_columns * EDITOR_FONT_HORIZONTAL_STRIDE;
          element_cmd->data.colored_rect_info.height = EDITOR_LINE_STRIDE;
          element_cmd->data.colored_rect_info.color = (render_color){173.f / 255.f, 109.f / 255.f, 42.f / 255.f, 0.7f};
        }
      }
    }
  }

  // printf("fer-n\n");
  for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
    element_cmd->x = 2;
    element_cmd->y = 8 + i * EDITOR_LINE_STRIDE;
    element_cmd->data.textured_rect_info.width = state->render_lines[i]->width;
    element_cmd->data.textured_rect_info.height = state->render_lines[i]->height;
    element_cmd->data.textured_rect_info.texture_uid = state->render_lines[i]->image_resource_uid;
  }

  // Status bar
  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
  element_cmd->x = state->status_bar.bounds.x;
  element_cmd->y = state->status_bar.bounds.y;
  element_cmd->data.textured_rect_info.width = state->status_bar.bounds.width;
  element_cmd->data.textured_rect_info.height = state->status_bar.bounds.height;
  element_cmd->data.textured_rect_info.texture_uid = state->status_bar.image_resource_uid;

  // printf("fer-q\n");
  // Cursor
  state->cursor_requires_render_update = false;
  if (!state->in_view_function_live_debugger) {
    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    element_cmd->x = 6 + (uint)(state->cursorCol * EDITOR_FONT_HORIZONTAL_STRIDE);
    element_cmd->y = 7 + (state->cursorLine - state->line_display_offset) * EDITOR_LINE_STRIDE;
    element_cmd->data.colored_rect_info.width = 2;
    element_cmd->data.colored_rect_info.height = EDITOR_LINE_STRIDE;
    element_cmd->data.colored_rect_info.color = (render_color){0.83f, 0.83f, 0.83f, 1.f};
  }

  return 0;
}

int free_fld_visual_code_element(fld_visual_code_element **item)
{
  // Free the data
  switch ((*item)->type) {
  case FLD_CODE_CSTRING: {
    char *cstr = (char *)(*item)->data;
    free(cstr);
  } break;
  case FLD_CODE_SNAPSHOT: {
    fld_variable_snapshot *field = (fld_variable_snapshot *)(*item)->data;

    if (field->type) {
      free(field->type);
      field->type = NULL;
    }
    if (field->mc_declared_type) {
      free(field->mc_declared_type);
      field->mc_declared_type = NULL;
    }
    if (field->name) {
      free(field->name);
      field->name = NULL;
    }
    if (field->value_text) {
      free(field->value_text);
      field->value_text = NULL;
    }

    field->line_index = 0;
    field->type_deref_count = 0U;
  } break;
  default: {
    MCerror(54, "TODO:%i", (*item)->type);
  }
  }

  free(*item);
  (*item) = NULL;

  return 0;
}

int fld_report_variable_snapshot(mc_code_editor_state_v1 *cestate, fld_variable_snapshot *field, void *p_value)
{
  if (!cestate->in_view_function_live_debugger) {
    return 0;
  }

  if (field->value_text) {
    free(field->value_text);
  }

  if (!field->mc_declared_type && !strcmp(field->type, "frame_time") && field->type_deref_count == 1) {
    frame_time *ft = (frame_time *)(*(void **)p_value);
    cprintf(field->value_text, "%s:[%lds %ldus]", field->name, ft->app_secs, ft->frame_nsecs / 1000);
  }
  else {
    cprintf(field->value_text, "%s:%p", field->name, p_value);
  }

  // Notify of render update
  cestate->visual_node->data.visual.requires_render_update = true;

  return 0;
}

int fld_construct_variable_snapshot(const char *type_name, const char *mc_declared_type_name, uint type_deref_count,
                                    const char *variable_name, uint line_index, fld_variable_snapshot **field_snapshot)
{
  fld_variable_snapshot *field = (fld_variable_snapshot *)malloc(sizeof(fld_variable_snapshot));
  allocate_and_copy_cstr(field->type, type_name);
  allocate_and_copy_cstr(field->mc_declared_type, mc_declared_type_name);
  field->type_deref_count = type_deref_count;
  allocate_and_copy_cstr(field->name, variable_name);
  char buf[64];
  sprintf(buf, "%s:[--]", variable_name);
  allocate_and_copy_cstr(field->value_text, buf);
  field->line_index = line_index;

  *field_snapshot = field;

  return 0;
}

int fld_append_visual_code(fld_view_state *fld_view, const char *display_code)
{
  // Attempt to append to previous
  // if (fld_view->visual_code.count > 0 &&
  //     fld_view->visual_code.items[fld_view->visual_code.count - 1]->type == FLD_CODE_CSTRING) {

  //   char *new_string = (char *)malloc(
  //       sizeof(char) * (strlen((char *)fld_view->visual_code.items[fld_view->visual_code.count - 1]->data) +
  //                       strlen(display_code) + 1));
  //   strcpy(new_string, (char *)fld_view->visual_code.items[fld_view->visual_code.count - 1]->data);
  //   strcat(new_string, display_code);

  //   free(fld_view->visual_code.items[fld_view->visual_code.count - 1]->data);
  //   fld_view->visual_code.items[fld_view->visual_code.count - 1]->data = new_string;
  //   return 0;
  // }

  fld_visual_code_element *element = (fld_visual_code_element *)malloc(sizeof(fld_visual_code_element));
  element->type = FLD_CODE_CSTRING;
  allocate_and_copy_cstr(element->data, display_code);

  MCcall(append_to_collection((void ***)&fld_view->visual_code.items, &fld_view->visual_code.alloc,
                              &fld_view->visual_code.count, element));

  return 0;
}

int fld_append_variable_snapshot(fld_view_state *fld_view, fld_variable_snapshot *field_snapshot)
{
  fld_visual_code_element *element = (fld_visual_code_element *)malloc(sizeof(fld_visual_code_element));
  element->type = FLD_CODE_SNAPSHOT;
  element->data = field_snapshot;

  MCcall(append_to_collection((void ***)&fld_view->visual_code.items, &fld_view->visual_code.alloc,
                              &fld_view->visual_code.count, element));

  return 0;
}

int code_editor_begin_function_live_debug(mc_code_editor_state_v1 *cestate)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  if (cestate->source_data_type != CODE_EDITOR_SOURCE_DATA_FUNCTION) {
    return 0;
  }

  function_info *function = (function_info *)cestate->source_data;

  // Begin
  printf("op01\n");
  printf("function '%s' code:\n%s\n", function->name, function->mc_code);

  // Replace the function with the debug version && form the displayed code for the debug view
  c_str *debug_declaration;
  MCcall(init_c_str(&debug_declaration));
  printf("op03\n");

  // -- Clear previous values
  for (int a = 0; a < cestate->fld_view->visual_code.count; ++a) {
    MCcall(free_fld_visual_code_element(&cestate->fld_view->visual_code.items[a]));
  }
  cestate->fld_view->visual_code.count = 0;

  // Debug function name
  char debug_function_name[128];
  sprintf(debug_function_name, "fld_%u_%s_v%u", cestate->fld_view->declare_incremental_uid++, function->name,
          function->latest_iteration);

  //
  printf("op04\n");
  MCvacall(append_to_c_str(debug_declaration, "int %s(int argc, void **argv) {\n", debug_function_name));

  printf("op05\n");
  // printf("cstr: alloc=%u len=%u str=||%s||\n", debug_declaration->alloc, debug_declaration->len,
  //        debug_declaration->text);
  {
    char return_type_str[256];
    sprintf(return_type_str, "%s ", function->return_type.name);
    for (int i = 0; i < function->return_type.deref_count; ++i) {
      strcat(return_type_str, "*");
    }
    MCcall(fld_append_visual_code(cestate->fld_view, return_type_str));
    MCcall(fld_append_visual_code(cestate->fld_view, function->name));
    MCcall(fld_append_visual_code(cestate->fld_view, "("));
  }

  // Arguments
  {
    MCcall(append_to_c_str(debug_declaration, "  // Arguments\n"));
    for (int a = 0; a < function->parameter_count; ++a) {
      const char **p_utilized_type_name;
      if (function->parameters[a]->mc_declared_type) {
        p_utilized_type_name = &function->parameters[a]->mc_declared_type;
      }
      else {
        p_utilized_type_name = &function->parameters[a]->type_name;
      }
      char deref_buf[16];
      for (int d = 0; d < function->parameters[a]->type_deref_count; ++d) {
        deref_buf[d] = '*';
      }
      deref_buf[function->parameters[a]->type_deref_count] = '\0';

      MCvacall(append_to_c_str(debug_declaration, "  %s %s%s = *(%s %s*)argv[%i];\n", *p_utilized_type_name, deref_buf,
                               function->parameters[a]->name, *p_utilized_type_name, deref_buf, a));
      printf("op03\n");

      // Parameter Type
      if (a > 0) {
        MCcall(fld_append_visual_code(cestate->fld_view, ", "));
      }
      MCcall(fld_append_visual_code(cestate->fld_view, function->parameters[a]->type_name));
      MCcall(fld_append_visual_code(cestate->fld_view, " "));
      MCcall(fld_append_visual_code(cestate->fld_view, deref_buf));

      // Live Argument
      fld_variable_snapshot *argument_snapshot;
      MCcall(fld_construct_variable_snapshot(
          function->parameters[a]->type_name, function->parameters[a]->mc_declared_type,
          function->parameters[a]->type_deref_count, function->parameters[a]->name, 0, &argument_snapshot));
      MCcall(fld_append_variable_snapshot(cestate->fld_view, argument_snapshot));

      printf("op06\n");
      // Report the value at call
      MCvacall(append_to_c_str(debug_declaration,
                               "  MCcall(fld_report_variable_snapshot((mc_code_editor_state_v1 *)%p, "
                               "(fld_variable_snapshot *)%p, &%s));\n ",
                               cestate, argument_snapshot, function->parameters[a]->name));

      MCcall(append_to_collection((void ***)&cestate->fld_view->arguments.items, &cestate->fld_view->arguments.alloc,
                                  &cestate->fld_view->arguments.count, argument_snapshot));
      printf("op07\n");
    }

    MCcall(fld_append_visual_code(cestate->fld_view, ")"));
    MCcall(append_to_c_str(debug_declaration, "\n"));
  }

  // TODO
  // c_syntax_tree *code_block_ast;
  // MCcall(parse_syntax_from_mc_code(function->mc_code));

  MCvacall(append_to_c_str(debug_declaration, "  printf(\"this instead!\\n\");\n"
                                              "\n"
                                              "  return 0;\n"
                                              "}"));
  printf("lfild-func-decl:\n%s\n##########\n", debug_declaration->text);
  MCcall(clint_declare(debug_declaration->text));
  free_c_str(debug_declaration);

  char decl_buf[256];
  sprintf(decl_buf,
          "*((int (***)(int, void **))%p) = &%s;\n"
          "*((int (**)(int, void **))%p) = %s;\n"
          "%s = &%s;",
          &cestate->fld_view->ptr_function_ptr, function->name, &cestate->fld_view->previous_function_address,
          function->name, function->name, debug_function_name);
  MCcall(clint_process(decl_buf));

  // Re-render
  cestate->visual_node->data.visual.requires_render_update = true;

  return 0;
}

int code_editor_toggle_view_v1(mc_code_editor_state_v1 *state)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  if (state->source_data_type != CODE_EDITOR_SOURCE_DATA_FUNCTION) {
    return 0;
  }

  if (state->in_view_function_live_debugger) {
    state->in_view_function_live_debugger = false;

    *state->fld_view->ptr_function_ptr = state->fld_view->previous_function_address;
    state->fld_view->ptr_function_ptr = NULL;
    state->fld_view->previous_function_address = NULL;

    // MCcall(code_editor_clear_function_live_debug_view(state->fld_view));
    state->visual_node->data.visual.requires_render_update = true;
  }
  else {
    state->in_view_function_live_debugger = true;

    MCcall(code_editor_begin_function_live_debug(state));
    state->visual_node->data.visual.requires_render_update = true;
  }

  return 0;
}

int code_editor_set_function_code_to_text(mc_code_editor_state_v1 *cestate)
{
  if (cestate->source_data_type != CODE_EDITOR_SOURCE_DATA_FUNCTION) {
    MCerror(704, "TODO?");
  }
  function_info *function = (function_info *)cestate->source_data;

  for (int j = 0; j < cestate->text->lines_count; ++j) {
    free(cestate->text->lines[j]);
    allocate_and_copy_cstr(cestate->text->lines[j], "");
  }
  cestate->text->lines_count = 0;

  // Line Alloc
  uint line_alloc = 32;
  char *line = (char *)malloc(sizeof(char) * line_alloc);
  line[0] = '\0';

  // Write the current signature
  append_to_cstr(&line_alloc, &line, function->return_type.name);
  append_to_cstr(&line_alloc, &line, " ");
  for (int i = 0; i < function->return_type.deref_count; ++i) {
    append_to_cstr(&line_alloc, &line, "*");
  }

  append_to_cstr(&line_alloc, &line, function->name);
  append_to_cstr(&line_alloc, &line, "(");

  for (int i = 0; i < function->parameter_count; ++i) {
    if (i > 0) {
      append_to_cstr(&line_alloc, &line, ", ");
    }
    append_to_cstr(&line_alloc, &line, function->parameters[i]->type_name);
    append_to_cstr(&line_alloc, &line, " ");
    for (int j = 0; j < function->parameters[i]->type_deref_count; ++j)
      append_to_cstr(&line_alloc, &line, "*");
    append_to_cstr(&line_alloc, &line, function->parameters[i]->name);
  }
  append_to_cstr(&line_alloc, &line, ") {");
  cestate->text->lines[cestate->text->lines_count++] = line;

  // Code Block
  {
    line_alloc = 1;
    line = (char *)malloc(sizeof(char) * line_alloc);
    line[0] = '\0';

    // Write the code in
    // Search for each line
    int i = 0;
    bool loop = true;
    bool wrapped_block = false;
    int s = i;
    for (; loop || !wrapped_block; ++i) {

      bool copy_line = false;
      switch (function->mc_code[i]) {
      case '\0': {
        copy_line = true;
        loop = false;
      } break;
      case '\n': {
        copy_line = true;
      } break;
      default:
        break;
      }

      // printf("i-s=%i\n", i - s);
      if (copy_line || !loop) {
        // Transfer text to the buffer line
        if (i - s > 0) {
          append_to_cstrn(&line_alloc, &line, function->mc_code + s, i - s);
        }
        else if (!loop && !strlen(line)) {
          wrapped_block = true;
          append_to_cstr(&line_alloc, &line, "}");
        }

        // Add to the collection
        if (cestate->text->lines_count + 1 >= cestate->text->lines_alloc) {
          uint new_alloc = cestate->text->lines_alloc + 4 + cestate->text->lines_alloc / 4;
          char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
          if (cestate->text->lines_alloc) {
            memcpy(new_ary, cestate->text->lines, cestate->text->lines_alloc * sizeof(char *));
            free(cestate->text->lines);
          }
          for (int i = cestate->text->lines_alloc; i < new_alloc; ++i) {
            new_ary[i] = NULL;
          }

          cestate->text->lines_alloc = new_alloc;
          cestate->text->lines = new_ary;
        }

        cestate->text->lines[cestate->text->lines_count++] = line;
        // printf("Line:(%i:%i)>'%s'\n", cestate->text->lines_count - 1, i - s,
        // cestate->text->lines[cestate->text->lines_count - 1]); printf("strlen:%zu\n",
        // strlen(cestate->text->lines[cestate->text->lines_count - 1])); if (cestate->text->lines_count > 1)
        //   printf("dawn:%c\n", cestate->text->lines[1][4]);

        // Reset
        s = i + 1;
        line_alloc = 1;
        line = (char *)malloc(sizeof(char) * line_alloc);
        line[0] = '\0';
      }
    }
  }
  // printf("life-6\n");

  // Set for render update
  for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
    if (cestate->line_display_offset + i < cestate->text->lines_count) {
      // printf("life-6a\n");
      if (cestate->render_lines[i]->text) {
        // printf("life-6b\n");
        cestate->render_lines[i]->requires_render_update =
            cestate->render_lines[i]->requires_render_update ||
            strcmp(cestate->render_lines[i]->text, cestate->text->lines[cestate->line_display_offset + i]);
        // printf("life-6c\n");
        free(cestate->render_lines[i]->text);
        // printf("life-6d\n");
      }
      else {
        // printf("life-6e\n");
        // printf("dawn:%i %i\n", cestate->line_display_offset + i, cestate->text->lines_count);
        // printf("dawn:%i\n", cestate->text->lines_alloc);
        // printf("dawn:%c\n", cestate->text->lines[1][4]);
        // printf("dawn:%zu\n", strlen(cestate->text->lines[cestate->line_display_offset + i]));
        cestate->render_lines[i]->requires_render_update =
            cestate->render_lines[i]->requires_render_update ||
            !cestate->text->lines[cestate->line_display_offset + i] ||
            strlen(cestate->text->lines[cestate->line_display_offset + i]);
      }

      // printf("life-6f\n");
      // Assign
      allocate_and_copy_cstr(cestate->render_lines[i]->text, cestate->text->lines[cestate->line_display_offset + i]);
      // printf("life-6g\n");
    }
    else {
      // printf("life-6h\n");
      if (cestate->render_lines[i]->text) {
        // printf("life-6i\n");
        cestate->render_lines[i]->requires_render_update = true;
        free(cestate->render_lines[i]->text);
        // printf("life-6j\n");
        cestate->render_lines[i]->text = NULL;
      }
    }
    // printf("life-6k\n");
  }

  return 0;
}

int load_existing_function_into_code_editor_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  function_info *function = *(function_info **)argv[0];

  printf("life-begin\n");

  // Begin Writing into the Function Editor textbox
  node *code_editor = (mc_node_v1 *)command_hub->global_node->children[0]; // TODO -- better way?
  mc_code_editor_state_v1 *feState = (mc_code_editor_state_v1 *)code_editor->extra;
  feState->source_data_type = CODE_EDITOR_SOURCE_DATA_FUNCTION;
  feState->source_data = function;

  feState->line_display_offset = 0;
  MCcall(code_editor_set_function_code_to_text(feState));

  // printf("life-7\n");
  feState->cursorLine = min(feState->text->lines_count - 1, feState->cursorLine);
  feState->cursorCol = min(feState->cursorCol, strlen(feState->text->lines[feState->cursorLine]));

  feState->selection_exists = false;

  // printf("life-7a\n");
  code_editor->data.visual.hidden = false;
  code_editor->data.visual.requires_render_update = true;

  printf("ohfohe\n");

  return 0;
}