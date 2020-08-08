#include "core/midge_core.h"

void build_code_editor()
{
  printf("build_code_editor()\n");

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
  fedit->data.visual.visible = false;
  fedit->data.visual.input_handler = &code_editor_handle_input;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, fedit));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;
  // printf("bce-d\n");

  // Code Lines
  mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)malloc(sizeof(mc_code_editor_state_v1));
  // printf("state:'%p'\n", state);
  state->visual_node = fedit;
  state->in_view_function_live_debugger = false;
  state->font_resource_uid = 0;
  state->cursor.line = 0;
  state->cursor.col = 0;
  state->cursor.zen_col = 0;
  state->cursor.rtf_index = 0;
  state->cursor.requires_render_update = false;
  state->line_display_offset = 0;
  // state->text = (mc_cstring_list_v1 *)malloc(sizeof(mc_cstring_list_v1));
  // state->text->lines_alloc = 8;
  // state->text->lines = (char **)calloc(sizeof(char *), state->text->lines_alloc);
  // state->text->lines_count = 0;
  state->render_lines = (rendered_code_line **)malloc(sizeof(rendered_code_line *) * CODE_EDITOR_RENDERED_CODE_LINES);
  // state->rendered_line_count = CODE_EDITOR_RENDERED_CODE_LINES;

  {
    // Source
    state->source_data = NULL;
    state->code.syntax = NULL;
    init_c_str(&state->code.rtf);
    state->code.syntax_updated = false;
  }

  {
    // FunctionLiveDebug
    state->fld_view = (fld_view_state *)malloc(sizeof(fld_view_state));
    state->fld_view->declare_incremental_uid = 0;
    state->fld_view->arguments.alloc = 0;
    state->fld_view->arguments.count = 0;
    state->fld_view->visual_code.alloc = 0;
    state->fld_view->visual_code.count = 0;
  }

  // printf("bce-s\n");
  for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
    state->render_lines[i] = (rendered_code_line *)malloc(sizeof(rendered_code_line));

    state->render_lines[i]->index = i;
    state->render_lines[i]->requires_render_update = true;
    MCcall(init_c_str(&state->render_lines[i]->rtf));
    // MCcall(set_c_str(state->render_lines[i]->rtf, ""));
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
  // printf("bce-w\n");

  {
    // Status Bar
    state->status_bar.bounds.x = 2;
    state->status_bar.bounds.y = fedit->data.visual.bounds.height - 50;
    state->status_bar.bounds.width = fedit->data.visual.bounds.width - 4;
    state->status_bar.bounds.height = 48;
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

  {
    // Suggestion Box
    state->suggestion_box.visible = false;
    state->suggestion_box.requires_render_update = false;

    state->suggestion_box.selected_index = 0;
    state->suggestion_box.entries.count = 0;
    state->suggestion_box.entries.max_count = 8;
    state->suggestion_box.entries.items = (char **)malloc(sizeof(char *) * state->suggestion_box.entries.max_count);
    for (int a = 0; a < state->suggestion_box.entries.max_count; ++a) {
      state->suggestion_box.entries.items[a] = NULL;
    }
    state->suggestion_box.bounds.x = 2;
    state->suggestion_box.bounds.y = 2;
    state->suggestion_box.bounds.width = 280;
    state->suggestion_box.bounds.height = 12 + 8 * 24;
    state->suggestion_box.image_resource_uid = 0;

    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &state->suggestion_box.image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = state->suggestion_box.bounds.width;
    command->data.create_texture.height = state->suggestion_box.bounds.height;
  }

  mc_process_action_arg_info_v1 **context_args =
      (mc_process_action_arg_info_v1 **)malloc(sizeof(mc_process_action_arg_info_v1 *) * 1);
  context_args[0] = (mc_process_action_arg_info_v1 *)malloc(sizeof(mc_process_action_arg_info_v1));
  context_args[0]->name = "context_syntax_node";
  context_args[0]->type_name = "int";

  mc_process_action_arg_info_v1 **result_args =
      (mc_process_action_arg_info_v1 **)malloc(sizeof(mc_process_action_arg_info_v1 *) * 1);
  result_args[0] = (mc_process_action_arg_info_v1 *)malloc(sizeof(mc_process_action_arg_info_v1));
  result_args[0]->name = "output";
  result_args[0]->type_name = "char *";

  construct_process_action_database(2424, &state->entry_pad, "Code-Editor:User-Key-Type", 1, context_args, 1,
                                    result_args);

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
  // printf("bce-z\n");
  // printf("41cestate->code.syntax=%p\n", state->code.syntax);
}

int code_editor_render(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // printf("code_editor_render_v1-a\n");
  frame_time *elapsed = *(frame_time **)argv[0];
  mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

  const int EDITOR_LINE_STRIDE = 22;
  const float EDITOR_FONT_HORIZONTAL_STRIDE = 10.31f;

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
    render_color font_color = COLOR_GHOST_WHITE;

    for (int a = 0; a < CODE_EDITOR_RENDERED_CODE_LINES; ++a) {
      rendered_code_line *rendered_line = state->render_lines[a];
      if (rendered_line->requires_render_update) {
        rendered_line->requires_render_update = false;

        // printf("fer-c\n");
        MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
        sequence->render_target = NODE_RENDER_TARGET_IMAGE;
        sequence->clear_color = COLOR_TRANSPARENT;
        sequence->image_width = rendered_line->width;
        sequence->image_height = rendered_line->height;
        sequence->data.target_image.image_uid = rendered_line->image_resource_uid;

        // Move through the rtf
        int i = 0;
        int s = i;
        int t = 0;
        // printf("rltext:'%s'\n", rendered_line->rtf->text);
        while (1) {
          if (rendered_line->rtf->text[i] == ' ') {
            while (rendered_line->rtf->text[i] == ' ') {
              ++i;
            }
            t += i - s;
            s = i;
            continue;
          }
          if (rendered_line->rtf->text[i] == '[') {
            if (rendered_line->rtf->text[i + 1] == '[') {
              // Escaped. Rearrange start
              s = i + 1;
              i += 2;
            }
            else if (!strncmp(rendered_line->rtf->text + i, "[color=", 7)) {
              int j = i + 7;
              int r, g, b;
              MCcall(mce_parse_past_integer(rendered_line->rtf->text, &j, &r));
              MCcall(parse_past(rendered_line->rtf->text, &j, ","));
              MCcall(mce_parse_past_integer(rendered_line->rtf->text, &j, &g));
              MCcall(parse_past(rendered_line->rtf->text, &j, ","));
              MCcall(mce_parse_past_integer(rendered_line->rtf->text, &j, &b));
              MCcall(parse_past(rendered_line->rtf->text, &j, "]"));

              font_color.r = (float)r / 255.f;
              font_color.g = (float)g / 255.f;
              font_color.b = (float)b / 255.f;

              s = i = j;
              continue;
            }
            else {
              MCerror(359, "Unsupported:'%s'", rendered_line->rtf->text + i);
            }
          }

          bool eof = false;
          for (;; ++i) {
            if (rendered_line->rtf->text[i] == '\0') {
              eof = true;
              break;
            }
            else if (rendered_line->rtf->text[i] == '[') {
              break;
            }
          }

          if (i - s > 0) {
            MCcall(obtain_element_render_command(sequence, &element_cmd));
            element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
            element_cmd->x = 4 + t * EDITOR_FONT_HORIZONTAL_STRIDE;
            element_cmd->y = 2 + 12;
            allocate_and_copy_cstrn(element_cmd->data.print_text.text, rendered_line->rtf->text + s, i - s);
            // printf("line:%i text:'%s' @ %i\n", a, element_cmd->data.print_text.text, t);
            element_cmd->data.print_text.font_resource_uid = state->font_resource_uid;
            element_cmd->data.print_text.color = font_color;
            // (render_color){0.61f, 0.86f, 0.99f, 1.f};;
            t += i - s;
          }
          if (eof) {
            break;
          }
        }
      }
    }
  }

  // Status Bar
  if (state->status_bar.requires_render_update) {
    state->status_bar.requires_render_update = false;

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

    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
    element_cmd->x = 4;
    element_cmd->y = 2 + 12 + 4;
    allocate_and_copy_cstr(element_cmd->data.print_text.text, state->status_bar.message);
    element_cmd->data.print_text.font_resource_uid = state->font_resource_uid;
    element_cmd->data.print_text.color = (render_color){0.95f, 0.72f, 0.49f, 1.f};
  }

  // Suggestion Box
  if (state->suggestion_box.visible && state->suggestion_box.requires_render_update) {
    state->suggestion_box.requires_render_update = false;

    MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
    sequence->render_target = NODE_RENDER_TARGET_IMAGE;
    sequence->image_width = state->suggestion_box.bounds.width;
    sequence->image_height = state->suggestion_box.bounds.height;
    sequence->clear_color = COLOR_GHOST_WHITE;
    sequence->data.target_image.image_uid = state->suggestion_box.image_resource_uid;

    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    element_cmd->x = 1;
    element_cmd->y = 1;
    element_cmd->data.colored_rect_info.width = state->suggestion_box.bounds.width - 2;
    element_cmd->data.colored_rect_info.height = state->suggestion_box.bounds.height - 2;
    element_cmd->data.colored_rect_info.color = COLOR_NEARLY_BLACK;

    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    element_cmd->x = 1;
    element_cmd->y = 2 + state->suggestion_box.selected_index * EDITOR_LINE_STRIDE;
    element_cmd->data.colored_rect_info.width = state->suggestion_box.bounds.width - 2;
    element_cmd->data.colored_rect_info.height = EDITOR_LINE_STRIDE;
    element_cmd->data.colored_rect_info.color = COLOR_DIM_GRAY;

    for (int a = 0; a < state->suggestion_box.entries.count; ++a) {
      MCcall(obtain_element_render_command(sequence, &element_cmd));
      element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
      element_cmd->x = 4;
      element_cmd->y = 2 + a * EDITOR_LINE_STRIDE + 12 + 4;
      allocate_and_copy_cstr(element_cmd->data.print_text.text, state->suggestion_box.entries.items[a]);
      element_cmd->data.print_text.font_resource_uid = state->font_resource_uid;
      element_cmd->data.print_text.color = COLOR_LIGHT_YELLOW;
    }
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

  if (state->selection_exists) {
    // Obtain selection bounds
    int selection_start_line, selection_start_col, selection_end_line, selection_end_col;
    if (state->selection_begin_line > state->cursor.line ||
        (state->selection_begin_line == state->cursor.line && state->selection_begin_col > state->cursor.col)) {
      // The selection begin comes after the cursor
      selection_start_line = (int)state->cursor.line - state->line_display_offset;
      selection_start_col = state->cursor.col;
      selection_end_line = (int)state->selection_begin_line - state->line_display_offset;
      selection_end_col = state->selection_begin_col;
    }
    else {
      // The selection begin comes before the cursor
      selection_start_line = (int)state->selection_begin_line - state->line_display_offset;
      selection_start_col = state->selection_begin_col;
      selection_end_line = (int)state->cursor.line - state->line_display_offset;
      selection_end_col = state->cursor.col;
    }

    // First line (partial)
    if (selection_start_line - state->line_display_offset >= 0 &&
        selection_start_line - state->line_display_offset < CODE_EDITOR_RENDERED_CODE_LINES) {
      int selected_columns;
      if (state->cursor.line == state->selection_begin_line) {
        selected_columns = selection_end_col - selection_start_col;
      }
      else if (state->render_lines[selection_start_line]->rtf->len) {
        selected_columns = state->render_lines[selection_start_line]->rtf->len - selection_start_col + 1;
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

      if (state->render_lines[i + state->line_display_offset]->rtf->len) {
        int selected_columns = state->render_lines[i + state->line_display_offset]->rtf->len + 1;

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

      if (state->render_lines[selection_end_line - state->line_display_offset]->rtf->len) {
        int selected_columns =
            min(selection_end_col, state->render_lines[selection_end_line - state->line_display_offset]->rtf->len);

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

  // Suggestion box
  if (state->suggestion_box.visible) {
    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
    // element_cmd->x = state->suggestion_box.bounds.x;
    // element_cmd->y = state->suggestion_box.bounds.y;
    element_cmd->x = 6 + (uint)((state->cursor.col + 1) * EDITOR_FONT_HORIZONTAL_STRIDE);
    element_cmd->y = 7 + (state->cursor.line - state->line_display_offset + 1) * EDITOR_LINE_STRIDE;
    element_cmd->data.textured_rect_info.width = state->suggestion_box.bounds.width;
    element_cmd->data.textured_rect_info.height = state->suggestion_box.bounds.height;
    element_cmd->data.textured_rect_info.texture_uid = state->suggestion_box.image_resource_uid;
  }

  // printf("fer-q\n");
  // Cursor
  state->cursor.requires_render_update = false;
  if (!state->in_view_function_live_debugger) {
    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    element_cmd->x = 6 + (uint)(state->cursor.col * EDITOR_FONT_HORIZONTAL_STRIDE);
    element_cmd->y = 7 + (state->cursor.line - state->line_display_offset) * EDITOR_LINE_STRIDE;
    element_cmd->data.colored_rect_info.width = 2;
    element_cmd->data.colored_rect_info.height = EDITOR_LINE_STRIDE;
    element_cmd->data.colored_rect_info.color = (render_color){0.83f, 0.83f, 0.83f, 1.f};
  }

  return 0;
}
