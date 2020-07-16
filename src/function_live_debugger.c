#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

typedef struct function_live_debug_field {
  char *type;
  char *mc_declared_type;
  unsigned int type_deref_count;
  char *name;
  void *ptr_value;
  char *value_text;
} function_live_debug_field;

typedef struct function_live_debug_state {
  node *visual_node;
  unsigned int declare_uid;
  unsigned int font_resource_uid;

  function_info *function;
  struct {
    uint alloc;
    uint count;
    function_live_debug_field **list;
  } arguments;

  

} function_live_debug_state;

int report_function_live_debug_value(function_live_debug_state *fld_state, const char *field_name, void *p_value)
{
  printf("reported! '%s':%p\n", field_name, p_value);

  return 0;
}

int load_function_into_live_debugger(function_live_debug_state *fld_state, const char *function_name)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  {
    void *mc_vargs[3];
    mc_vargs[0] = &fld_state->function;
    mc_vargs[1] = &command_hub->global_node;
    mc_vargs[2] = &function_name;
    MCcall(find_function_info(3, mc_vargs));
  }

  if (!fld_state->function) {
    MCerror(25, "Can't find function with name=%s", function_name);
  }

  printf("op01\n");
  printf("function '%s' code:\n%s\n", fld_state->function->name, fld_state->function->mc_code);

  // Replace the function with the debug version
  char debug_function_name[128];
  sprintf(debug_function_name, "fld_%u_%s_v%u", fld_state->declare_uid++, fld_state->function->name,
          fld_state->function->latest_iteration);

  char buf[1024];
  unsigned int arguments_alloc = 4;
  char *arguments = (char *)malloc(sizeof(char) * arguments_alloc);
  arguments[0] = '\0';
  {
    for (int a = 0; a < fld_state->function->parameter_count; ++a) {
      const char **p_utilized_type_name;
      if (fld_state->function->parameters[a]->mc_declared_type) {
        p_utilized_type_name = &fld_state->function->parameters[a]->mc_declared_type;
      }
      else {
        p_utilized_type_name = &fld_state->function->parameters[a]->type_name;
      }

      MCcall(append_to_cstr(&arguments_alloc, &arguments, "  "));
      MCcall(append_to_cstr(&arguments_alloc, &arguments, *p_utilized_type_name));
      MCcall(append_to_cstr(&arguments_alloc, &arguments, " "));
      for (int d = 0; d < fld_state->function->parameters[a]->type_deref_count; ++d) {
        MCcall(append_to_cstr(&arguments_alloc, &arguments, "*"));
      }
      MCcall(append_to_cstr(&arguments_alloc, &arguments, fld_state->function->parameters[a]->name));

      printf("op02a\n");
      MCcall(append_to_cstr(&arguments_alloc, &arguments, " = "));

      MCcall(append_to_cstr(&arguments_alloc, &arguments, "*("));
      MCcall(append_to_cstr(&arguments_alloc, &arguments, *p_utilized_type_name));
      MCcall(append_to_cstr(&arguments_alloc, &arguments, " "));
      printf("op02b\n");
      for (int d = 0; d < fld_state->function->parameters[a]->type_deref_count + 1; ++d) {
        MCcall(append_to_cstr(&arguments_alloc, &arguments, "*"));
      }
      MCcall(append_to_cstr(&arguments_alloc, &arguments, ")argv["));
      sprintf(buf, "%i", a);
      printf("op02c\n");
      MCcall(append_to_cstr(&arguments_alloc, &arguments, buf));
      MCcall(append_to_cstr(&arguments_alloc, &arguments, "];\n"));
      printf("op03\n");

      // Pointer reference
      function_live_debug_field *field = (function_live_debug_field *)malloc(sizeof(function_live_debug_field));
      allocate_and_copy_cstr(field->mc_declared_type, fld_state->function->parameters[a]->mc_declared_type);
      allocate_and_copy_cstr(field->type, fld_state->function->parameters[a]->type_name);
      field->type_deref_count = fld_state->function->parameters[a]->type_deref_count;
      allocate_and_copy_cstr(field->name, fld_state->function->parameters[a]->name);
      field->ptr_value = NULL;
      allocate_and_copy_cstr(field->value_text, "(unset)");

      // printf("op05\n");
      // sprintf(buf, "  *((void **)%p) = %s;\n", &field->ptr_value, fld_state->function->parameters[a]->name);
      sprintf(buf, "  MCcall(report_function_live_debug_value(fld_state, \"%s\", &%s));\n",
              fld_state->function->parameters[a]->name, fld_state->function->parameters[a]->name);
      MCcall(append_to_cstr(&arguments_alloc, &arguments, buf));

      printf("op06\n");
      MCcall(append_to_collection((void ***)&fld_state->arguments.list, &fld_state->arguments.alloc,
                                  &fld_state->arguments.count, field));
      printf("op07\n");
    }
  }

  sprintf(buf,
          "int %s(int argc, void **argv) {\n"
          "  // FLD-State\n"
          "  function_live_debug_state *fld_state = (function_live_debug_state *)%p;\n"
          "\n"
          "  // Arguments\n"
          "%s"
          "\n"
          "  printf(\"this instead!\\n\");\n"
          "\n"
          "  return 0;\n"
          "}",
          debug_function_name, fld_state, arguments);
  printf("lfild-func-decl:\n%s\n##########\n", buf);
  MCcall(clint_declare(buf));

  sprintf(buf, "%s = &%s;", fld_state->function->name, debug_function_name);
  MCcall(clint_process(buf));

  return 0;
}

int function_live_debugger_handle_input_v1(int argc, void **argv) { return 0; }

int function_live_debugger_render_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // printf("function_live_debugger_render_v1()\n");

  frame_time const *elapsed = *(frame_time const **)argv[0];
  mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

  if (visual_node->data.visual.hidden)
    return 0;
  function_live_debug_state *state = (function_live_debug_state *)visual_node->extra;

  image_render_queue *sequence;
  element_render_command *element_cmd;

  // for (int i = 0; i < visual_node->child_count; ++i) {
  //   node *child = (node *)visual_node->children[i];

  //   if (!child->data.visual.requires_render_update) {
  //     continue;
  //   }

  //   // printf("core_child.bounds x=%u y=%u width=%u height=%u\n", child->data.visual.bounds.x,
  //   // child->data.visual.bounds.y,
  //   //        child->data.visual.bounds.width, child->data.visual.bounds.height);

  //   MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  //   sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  //   sequence->image_width = child->data.visual.bounds.width;
  //   sequence->image_height = child->data.visual.bounds.height;
  //   sequence->clear_color = (render_color){0.12f, 0.16f, 0.22f, 1.f};
  //   sequence->data.target_image.image_uid = child->data.visual.image_resource_uid;

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  // element_cmd->x = 522;
  // element_cmd->y = 400;
  // element_cmd->data.print_text.font_resource_uid = state->font_resource_uid;
  // const char *word = "hello there!";
  // element_cmd->data.print_text.text = &word;
  // element_cmd->data.print_text.color = COLOR_GHOST_WHITE;
  // }

  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  sequence->image_width = visual_node->data.visual.bounds.width;
  sequence->image_height = visual_node->data.visual.bounds.height;
  sequence->clear_color = (render_color){0.56f, 0.37f, 0.48f, 1.f};
  sequence->data.target_image.image_uid = visual_node->data.visual.image_resource_uid;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
  element_cmd->x = 62;
  element_cmd->y = 62;
  element_cmd->data.colored_rect_info.width = visual_node->data.visual.bounds.width - 124;
  element_cmd->data.colored_rect_info.height = visual_node->data.visual.bounds.height - 124;
  element_cmd->data.colored_rect_info.color = (render_color){0.13f, 0.13f, 0.13f, 1.f};

  if (state->arguments.count > 0) {
    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
    element_cmd->x = 200;
    element_cmd->y = 200;
    element_cmd->data.print_text.font_resource_uid = state->font_resource_uid;
    element_cmd->data.print_text.text = (const char **)&state->arguments.list[0]->value_text;
    element_cmd->data.print_text.color = COLOR_GHOST_WHITE;

    // printf("rendered the text:'%s'\n", state->arguments.list[0]->value_text);
  }

  // for (int i = 0; i < visual_node->child_count; ++i) {
  //   node *child = (node *)visual_node->children[i];

  //   MCcall(obtain_element_render_command(sequence, &element_cmd));
  //   element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
  //   element_cmd->x = child->data.visual.bounds.x;
  //   element_cmd->y = child->data.visual.bounds.y;
  //   element_cmd->data.textured_rect_info.width = child->data.visual.bounds.width;
  //   element_cmd->data.textured_rect_info.height = child->data.visual.bounds.height;
  //   element_cmd->data.textured_rect_info.texture_uid = child->data.visual.image_resource_uid;
  // }
  return 0;
}

int update_function_live_debugger_v1(int argc, void **argv)
{
  // printf("update_function_live_debugger_v1\n");
  frame_time const *elapsed = *(frame_time const **)argv[0];
  function_live_debug_state *state = (function_live_debug_state *)argv[1];

  // Set the text
  if (state->arguments.list[0]->value_text) {
    free(state->arguments.list[0]->value_text);
  }
  if (!state->arguments.list[0]->ptr_value) {
    allocate_and_copy_cstr(state->arguments.list[0]->value_text, "(NULL)");
  }
  else {
    frame_time *arg_value = (frame_time *)(state->arguments.list[0]->ptr_value);
    cprintf(state->arguments.list[0]->value_text, "(frame_time app_secs=%ld)", arg_value->app_secs);
  }

  state->visual_node->data.visual.requires_render_update = true;

  return 0;
}

int build_function_live_debugger_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  mc_node_v1 *fld = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  fld->name = "function_live_debugger";
  fld->parent = command_hub->global_node;
  fld->type = NODE_TYPE_VISUAL;

  fld->data.visual.bounds.x = 190;
  fld->data.visual.bounds.y = 40;
  fld->data.visual.bounds.width = 1130;
  fld->data.visual.bounds.height = 800;
  fld->data.visual.image_resource_uid = 0;
  fld->data.visual.requires_render_update = true;
  fld->data.visual.render_delegate = &function_live_debugger_render;
  fld->data.visual.hidden = false;
  fld->data.visual.input_handler = &function_live_debugger_handle_input;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, fld));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  function_live_debug_state *state = (function_live_debug_state *)malloc(sizeof(function_live_debug_state));
  fld->extra = (void *)state;
  state->visual_node = fld;
  state->declare_uid = 10;
  state->function = NULL;
  state->arguments.alloc = 0;
  state->arguments.count = 0;
  // state->font_resource

  // MCcall(register_update_timer(&update_function_live_debugger_v1, 200 * 1000, true, (void *)state));

  // // Code Lines
  // mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)malloc(sizeof(mc_code_editor_state_v1));
  // // printf("state:'%p'\n", state);
  // state->source_data_type = CODE_EDITOR_SOURCE_DATA_NONE;
  // state->source_data = NULL;
  // state->font_resource_uid = 0;
  // state->cursorLine = 0;
  // state->cursorCol = 0;
  // state->line_display_offset = 0;
  // state->text = (mc_cstring_list_v1 *)malloc(sizeof(mc_cstring_list_v1));
  // state->text->lines_alloc = 8;
  // state->text->lines = (char **)calloc(sizeof(char *), state->text->lines_alloc);
  // state->text->lines_count = 0;
  // state->render_lines = (rendered_code_line **)malloc(sizeof(rendered_code_line *) *
  // CODE_EDITOR_RENDERED_CODE_LINES);

  // for (int i = 0; i < CODE_EDITOR_RENDERED_CODE_LINES; ++i) {
  //   state->render_lines[i] = (rendered_code_line *)malloc(sizeof(rendered_code_line));

  //   state->render_lines[i]->index = i;
  //   state->render_lines[i]->requires_render_update = true;
  //   state->render_lines[i]->text = NULL;
  //   //  "!this is twenty nine letters! "
  //   //  "!this is twenty nine letters! "
  //   //  "!this is twenty nine letters! ";

  //   MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  //   command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  //   command->p_uid = &state->render_lines[i]->image_resource_uid;
  //   command->data.create_texture.use_as_render_target = true;
  //   command->data.create_texture.width = state->render_lines[i]->width = fedit->data.visual.bounds.width - 4;
  //   command->data.create_texture.height = state->render_lines[i]->height = 28;
  // }
  // fedit->extra = (void *)state;

  // Function Editor Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &fld->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = fld->data.visual.bounds.width;
  command->data.create_texture.height = fld->data.visual.bounds.height;

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

  MCcall(load_function_into_live_debugger(state, "special_update"));

  return 0;
}