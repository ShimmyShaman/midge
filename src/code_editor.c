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
  fedit->data.visual.bounds.width = APPLICATION_SET_WIDTH - 300;
  fedit->data.visual.bounds.height = APPLICATION_SET_HEIGHT - 70;
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

int code_editor_render_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
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

  // Render
  // printf("fer-g\n");
  // printf("OIRS: w:%u h:%u uid:%u\n", visual_node->data.visual.bounds.width, visual_node->data.visual.bounds.height,
  //        visual_node->data.visual.image_resource_uid);
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
  element_cmd->data.colored_rect_info.color = (render_color){0.13f, 0.13f, 0.13f, 1.f};

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

  // printf("fer-q\n");
  // Cursor
  state->cursor_requires_render_update = false;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  element_cmd->x = 6 + (uint)(state->cursorCol * EDITOR_FONT_HORIZONTAL_STRIDE);
  element_cmd->y = 7 + (state->cursorLine - state->line_display_offset) * EDITOR_LINE_STRIDE;
  element_cmd->data.colored_rect_info.width = 2;
  element_cmd->data.colored_rect_info.height = EDITOR_LINE_STRIDE;
  element_cmd->data.colored_rect_info.color = (render_color){0.83f, 0.83f, 0.83f, 1.f};

  return 0;
}

int code_editor_toggle_view_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // Arguments
  mc_code_editor_state_v1 *state = *(mc_code_editor_state_v1 **)argv[0];
  if (state->source_data_type != CODE_EDITOR_SOURCE_DATA_FUNCTION) {
    return 0;
  }

  printf("code_editor_toggle_view!\n");

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
  for (int j = 0; j < feState->text->lines_count; ++j) {
    free(feState->text->lines[j]);
    allocate_and_copy_cstr(feState->text->lines[j], "");
  }
  feState->text->lines_count = 0;

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
  feState->text->lines[feState->text->lines_count++] = line;

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
        if (feState->text->lines_count + 1 >= feState->text->lines_alloc) {
          uint new_alloc = feState->text->lines_alloc + 4 + feState->text->lines_alloc / 4;
          char **new_ary = (char **)malloc(sizeof(char *) * new_alloc);
          if (feState->text->lines_alloc) {
            memcpy(new_ary, feState->text->lines, feState->text->lines_alloc * sizeof(char *));
            free(feState->text->lines);
          }
          for (int i = feState->text->lines_alloc; i < new_alloc; ++i) {
            new_ary[i] = NULL;
          }

          feState->text->lines_alloc = new_alloc;
          feState->text->lines = new_ary;
        }

        feState->text->lines[feState->text->lines_count++] = line;
        // printf("Line:(%i:%i)>'%s'\n", feState->text->lines_count - 1, i - s,
        // feState->text->lines[feState->text->lines_count - 1]); printf("strlen:%zu\n",
        // strlen(feState->text->lines[feState->text->lines_count - 1])); if (feState->text->lines_count > 1)
        //   printf("dawn:%c\n", feState->text->lines[1][4]);

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
  feState->line_display_offset = 0;
  for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
    if (feState->line_display_offset + i < feState->text->lines_count) {
      // printf("life-6a\n");
      if (feState->render_lines[i]->text) {
        // printf("life-6b\n");
        feState->render_lines[i]->requires_render_update =
            feState->render_lines[i]->requires_render_update ||
            strcmp(feState->render_lines[i]->text, feState->text->lines[feState->line_display_offset + i]);
        // printf("life-6c\n");
        free(feState->render_lines[i]->text);
        // printf("life-6d\n");
      }
      else {
        // printf("life-6e\n");
        // printf("dawn:%i %i\n", feState->line_display_offset + i, feState->text->lines_count);
        // printf("dawn:%i\n", feState->text->lines_alloc);
        // printf("dawn:%c\n", feState->text->lines[1][4]);
        // printf("dawn:%zu\n", strlen(feState->text->lines[feState->line_display_offset + i]));
        feState->render_lines[i]->requires_render_update =
            feState->render_lines[i]->requires_render_update ||
            !feState->text->lines[feState->line_display_offset + i] ||
            strlen(feState->text->lines[feState->line_display_offset + i]);
      }

      // printf("life-6f\n");
      // Assign
      allocate_and_copy_cstr(feState->render_lines[i]->text, feState->text->lines[feState->line_display_offset + i]);
      // printf("life-6g\n");
    }
    else {
      // printf("life-6h\n");
      if (feState->render_lines[i]->text) {
        // printf("life-6i\n");
        feState->render_lines[i]->requires_render_update = true;
        free(feState->render_lines[i]->text);
        // printf("life-6j\n");
        feState->render_lines[i]->text = NULL;
      }
    }
    // printf("life-6k\n");
  }

  // printf("life-7\n");
  feState->cursorLine = 1;
  feState->cursorCol = strlen(feState->text->lines[feState->cursorLine]);

  feState->selection_exists = false;

  // printf("life-7a\n");
  code_editor->data.visual.hidden = false;
  code_editor->data.visual.requires_render_update = true;

  printf("ohfohe\n");

  return 0;
}