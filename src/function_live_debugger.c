#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

#include <stdarg.h>
#include <stdio.h>

typedef struct c_str {
  uint alloc;
  uint len;
  char *text;
} c_str;
int init_c_str(c_str **ptr);
int append_to_c_str(c_str *cstr, const char *format, ...);

int init_c_str(c_str **ptr)
{
  (*ptr) = (c_str *)malloc(sizeof(c_str));
  (*ptr)->alloc = 2;
  (*ptr)->len = 0;
  (*ptr)->text = (char *)malloc(sizeof(char) * (*ptr)->alloc);

  return 0;
}

int free_c_str(c_str *ptr)
{
  if (ptr->alloc > 0 && ptr->text) {
    free(ptr->text);
  }

  free(ptr);

  return 0;
}

int append_to_c_str(c_str *cstr, const char *format, ...)
{
  // printf("atcs-0\n");
  int chunk_size = 4;
  int i = 0;

  va_list valist;
  va_start(valist, format);

  // printf("atcs-1\n");
  while (1) {
    if (cstr->len + chunk_size + 1 >= cstr->alloc) {
      unsigned int new_allocated_size = chunk_size + cstr->alloc + 16 + (chunk_size + cstr->alloc) / 10;
      // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
      char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
      // printf("atc-4\n");
      memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
      // printf("atc-5\n");
      free(cstr->text);
      // printf("atc-6\n");
      cstr->text = newptr;
      // printf("atc-7\n");
      cstr->alloc = new_allocated_size;
      // printf("atc-8\n");
    }
    // printf("atcs-2\n");
    // sleep(1);

    // printf("'%c' chunk_size=%i cstr->len=%u\n", format[i], chunk_size, cstr->len);
    for (int a = 0; a < chunk_size; ++a) {
      cstr->text[cstr->len++] = format[i];
      cstr->text[cstr->len] = '\0';
      // printf("cstr:'%s'\n", cstr->text);

      if (format[i] == '\0') {
        // printf("atcs-3\n");
        --cstr->len;
        va_end(valist);
        return 0;
      }

      if (format[i] == '%') {
        if (format[i + 1] == '%') {
          // printf("atcs-4\n");
          // Use as an escape character
          ++i;
        }
        else {
          // printf("atcs-5 i:%i i:'%c'\n", i, format[i]);
          --cstr->len;
          // Search to replace the format
          ++i;
          switch (format[i]) {
          case 'i': {
            int value = va_arg(valist, int);

            char buf[18];
            sprintf(buf, "%i", value);
            append_to_c_str(cstr, buf);
          } break;
          case 'p': {
            void *value = va_arg(valist, void *);

            char buf[18];
            sprintf(buf, "%p", value);
            append_to_c_str(cstr, buf);
          } break;
          case 's': {
            char *value = va_arg(valist, char *);
            append_to_c_str(cstr, value);
          } break;
          case 'u': {
            unsigned int value = va_arg(valist, unsigned int);

            char buf[18];
            sprintf(buf, "%u", value);
            append_to_c_str(cstr, buf);
          } break;
          default: {
            MCerror(99, "TODO:%c", format[i]);
          }
          }
        }
      }

      ++i;
      // printf("atcs-8 i:%i i:'%c'\n", i, format[i]);
    }

    // printf("atcs-6\n");
    chunk_size = (chunk_size * 5) / 3;
  }
}

typedef struct fld_variable_snapshot {
  int line_index;
  char *type;
  char *mc_declared_type;
  unsigned int type_deref_count;
  char *name;
  char *value_text;
} fld_variable_snapshot;

typedef enum fld_code_type {
  FLD_CODE_NONE,
  FLD_CODE_CSTRING, // TODO -- type/function-name/field-name/literal/number/newline/etc
  FLD_CODE_SNAPSHOT,
} fld_code_type;

typedef struct fld_visual_code_element {
  fld_code_type type;
  void *data;
} fld_visual_code_element;

typedef struct fld_module_state {
  node *visual_node;
  unsigned int declare_uid;
  unsigned int font_resource_uid;

  function_info *function;
  struct {
    uint alloc;
    uint count;
    fld_variable_snapshot **list;
  } arguments;

  struct {
    uint alloc;
    uint count;
    fld_visual_code_element **items;
  } visual_code;

} fld_module_state;

int free_fld_visual_code_element(fld_visual_code_element **item)
{
  // Free the data
  switch ((*item)->type) {
  case FLD_CODE_CSTRING: {
    char *cstr = (char *)(*item)->data;
    free(cstr);
  } break;
  // case FLD_CODE_SNAPSHOT: {

  // } break;
  default: {
    MCerror(54, "TODO:%i", (*item)->type);
  }
  }

  free(*item);
  (*item) = NULL;

  return 0;
}

int fld_report_variable_snapshot(fld_module_state *fld_state, fld_variable_snapshot *field, void *p_value)
{
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
  fld_state->visual_node->data.visual.requires_render_update = true;

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
  allocate_and_copy_cstr(field->value_text, "(unset)");
  field->line_index = line_index;

  *field_snapshot = field;

  return 0;
}

int fld_append_visual_code(fld_module_state *fld_state, const char *display_code)
{
  // Attempt to append to previous
  // if (fld_state->visual_code.count > 0 &&
  //     fld_state->visual_code.items[fld_state->visual_code.count - 1]->type == FLD_CODE_CSTRING) {

  //   char *new_string = (char *)malloc(
  //       sizeof(char) * (strlen((char *)fld_state->visual_code.items[fld_state->visual_code.count - 1]->data) +
  //                       strlen(display_code) + 1));
  //   strcpy(new_string, (char *)fld_state->visual_code.items[fld_state->visual_code.count - 1]->data);
  //   strcat(new_string, display_code);

  //   free(fld_state->visual_code.items[fld_state->visual_code.count - 1]->data);
  //   fld_state->visual_code.items[fld_state->visual_code.count - 1]->data = new_string;
  //   return 0;
  // }

  fld_visual_code_element *element = (fld_visual_code_element *)malloc(sizeof(fld_visual_code_element));
  element->type = FLD_CODE_CSTRING;
  allocate_and_copy_cstr(element->data, display_code);

  MCcall(append_to_collection((void ***)&fld_state->visual_code.items, &fld_state->visual_code.alloc,
                              &fld_state->visual_code.count, element));

  return 0;
}

int fld_append_variable_snapshot(fld_module_state *fld_state, fld_variable_snapshot *field_snapshot)
{
  fld_visual_code_element *element = (fld_visual_code_element *)malloc(sizeof(fld_visual_code_element));
  element->type = FLD_CODE_SNAPSHOT;
  element->data = field_snapshot;

  MCcall(append_to_collection((void ***)&fld_state->visual_code.items, &fld_state->visual_code.alloc,
                              &fld_state->visual_code.count, element));

  return 0;
}

int load_function_into_live_debugger(fld_module_state *fld_state, const char *function_name)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  {
    void *mc_vargs[3];
    mc_vargs[0] = &fld_state->function;
    mc_vargs[1] = &command_hub->nodespace;
    mc_vargs[2] = &function_name;
    MCcall(find_function_info(3, mc_vargs));
  }

  if (!fld_state->function) {
    MCerror(25, "Can't find function with name=%s", function_name);
  }

  // Begin
  printf("op01\n");
  printf("function '%s' code:\n%s\n", fld_state->function->name, fld_state->function->mc_code);

  // Replace the function with the debug version && form the displayed code for the debug view
  c_str *debug_declaration;
  MCcall(init_c_str(&debug_declaration));
  printf("op03\n");

  // -- Clear previous values
  for (int a = 0; a < fld_state->visual_code.count; ++a) {
    MCcall(free_fld_visual_code_element(&fld_state->visual_code.items[a]));
  }
  fld_state->visual_code.count = 0;

  // Debug function name
  char debug_function_name[128];
  sprintf(debug_function_name, "fld_%u_%s_v%u", fld_state->declare_uid++, fld_state->function->name,
          fld_state->function->latest_iteration);

  //
  printf("op04\n");
  MCvacall(append_to_c_str(debug_declaration, "int %s(int argc, void **argv) {\n", debug_function_name));

  printf("op05\n");
  // printf("cstr: alloc=%u len=%u str=||%s||\n", debug_declaration->alloc, debug_declaration->len,
  //        debug_declaration->text);
  {
    char return_type_str[256];
    sprintf(return_type_str, "%s ", fld_state->function->return_type.name);
    for (int i = 0; i < fld_state->function->return_type.deref_count; ++i) {
      strcat(return_type_str, "*");
    }
    MCcall(fld_append_visual_code(fld_state, return_type_str));
    MCcall(fld_append_visual_code(fld_state, fld_state->function->name));
    MCcall(fld_append_visual_code(fld_state, "("));
  }

  // Arguments
  {
    MCcall(append_to_c_str(debug_declaration, "  // Arguments\n"));
    for (int a = 0; a < fld_state->function->parameter_count; ++a) {
      const char **p_utilized_type_name;
      if (fld_state->function->parameters[a]->mc_declared_type) {
        p_utilized_type_name = &fld_state->function->parameters[a]->mc_declared_type;
      }
      else {
        p_utilized_type_name = &fld_state->function->parameters[a]->type_name;
      }
      char deref_buf[16];
      for (int d = 0; d < fld_state->function->parameters[a]->type_deref_count; ++d) {
        deref_buf[d] = '*';
      }
      deref_buf[fld_state->function->parameters[a]->type_deref_count] = '\0';

      MCvacall(append_to_c_str(debug_declaration, "  %s %s%s = *(%s %s*)argv[%i];\n", *p_utilized_type_name, deref_buf,
                               fld_state->function->parameters[a]->name, *p_utilized_type_name, deref_buf, a));
      printf("op03\n");

      // Parameter Type
      if (a > 0) {
        MCcall(fld_append_visual_code(fld_state, ", "));
      }
      MCcall(fld_append_visual_code(fld_state, fld_state->function->parameters[a]->type_name));
      MCcall(fld_append_visual_code(fld_state, " "));
      MCcall(fld_append_visual_code(fld_state, deref_buf));

      // Live Argument
      fld_variable_snapshot *argument_snapshot;
      MCcall(fld_construct_variable_snapshot(fld_state->function->parameters[a]->type_name,
                                             fld_state->function->parameters[a]->mc_declared_type,
                                             fld_state->function->parameters[a]->type_deref_count,
                                             fld_state->function->parameters[a]->name, 0, &argument_snapshot));
      MCcall(fld_append_variable_snapshot(fld_state, argument_snapshot));

      printf("op06\n");
      // Report the value at call
      MCvacall(append_to_c_str(
          debug_declaration,
          "  MCcall(fld_report_variable_snapshot((fld_module_state *)%p, (fld_variable_snapshot *)%p, &%s));\n ",
          fld_state, argument_snapshot, fld_state->function->parameters[a]->name));

      MCcall(append_to_collection((void ***)&fld_state->arguments.list, &fld_state->arguments.alloc,
                                  &fld_state->arguments.count, argument_snapshot));
      printf("op07\n");
    }

    MCcall(fld_append_visual_code(fld_state, ")"));
    MCcall(append_to_c_str(debug_declaration, "\n"));
  }

  // TODO
  // c_syntax_tree *code_block_ast;
  // MCcall(parse_syntax_from_mc_code(fld_state->function->mc_code));

  MCvacall(append_to_c_str(debug_declaration, "  printf(\"this instead!\\n\");\n"
                                              "\n"
                                              "  return 0;\n"
                                              "}"));
  printf("lfild-func-decl:\n%s\n##########\n", debug_declaration->text);
  MCcall(clint_declare(debug_declaration->text));
  free_c_str(debug_declaration);

  char decl_buf[256];
  sprintf(decl_buf, "%s = &%s;", fld_state->function->name, debug_function_name);
  MCcall(clint_process(decl_buf));

  return 0;
}

int function_live_debugger_handle_input_v1(int argc, void **argv) { return 0; }

int function_live_debugger_render_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  printf("function_live_debugger_render_v1()\n");

  frame_time const *elapsed = *(frame_time const **)argv[0];
  mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

  if (visual_node->data.visual.hidden)
    return 0;
  fld_module_state *state = (fld_module_state *)visual_node->extra;

  image_render_queue *sequence;
  element_render_command *element_cmd;

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

  {
    const int EDITOR_LINE_STRIDE = 22;
    const float EDITOR_FONT_HORIZONTAL_STRIDE = 10.31f;
    int line_index = 0;
    int col_index = 0;

    printf("fldr-0 state->visual_code.count:%i\n", state->visual_code.count);
    for (int i = 0; i < state->visual_code.count; ++i) {
      switch (state->visual_code.items[i]->type) {
      case FLD_CODE_CSTRING: {
        char *text = (char *)state->visual_code.items[i]->data;
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

          // Print sequence
          if (c - s) {
            MCcall(obtain_element_render_command(sequence, &element_cmd));
            element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
            element_cmd->x = 62 + 4 + col_index * EDITOR_FONT_HORIZONTAL_STRIDE;
            element_cmd->y = 62 + 4 + 14 + line_index * EDITOR_LINE_STRIDE;
            element_cmd->data.print_text.font_resource_uid = state->font_resource_uid;
            allocate_and_copy_cstrn(element_cmd->data.print_text.text, text + s, c - s);
            element_cmd->data.print_text.color = COLOR_GHOST_WHITE;

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
        fld_variable_snapshot *snapshot = (fld_variable_snapshot *)state->visual_code.items[i]->data;

        MCcall(obtain_element_render_command(sequence, &element_cmd));
        element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
        element_cmd->x = 62 + 4 + col_index * EDITOR_FONT_HORIZONTAL_STRIDE;
        element_cmd->y = 62 + 4 + 14 + line_index * EDITOR_LINE_STRIDE;
        element_cmd->data.print_text.font_resource_uid = state->font_resource_uid;
        allocate_and_copy_cstr(element_cmd->data.print_text.text, snapshot->value_text);
        element_cmd->data.print_text.color = COLOR_YELLOW;

        col_index += strlen(snapshot->value_text);
      } break;
      default: {
        MCerror(456, "TODO:%i", state->visual_code.items[i]->type);
      }
      }
    }
  }

  // if (state->arguments.count > 0) {

  //   // printf("rendered the text:'%s'\n", state->arguments.list[0]->value_text);
  // }

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

// int update_function_live_debugger_v1(int argc, void **argv)
// {
//   // printf("update_function_live_debugger_v1\n");
//   frame_time const *elapsed = *(frame_time const **)argv[0];
//   fld_module_state *state = (fld_module_state *)argv[1];

//   // Set the text
//   if (state->arguments.list[0]->value_text) {
//     free(state->arguments.list[0]->value_text);
//   }
//   if (!state->arguments.list[0]->ptr_value) {
//     allocate_and_copy_cstr(state->arguments.list[0]->value_text, "(NULL)");
//   }
//   else {
//     frame_time *arg_value = (frame_time *)(state->arguments.list[0]->ptr_value);
//     cprintf(state->arguments.list[0]->value_text, "(frame_time app_secs=%ld)", arg_value->app_secs);
//   }

//   state->visual_node->data.visual.requires_render_update = true;

//   return 0;
// }

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
  fld->data.visual.hidden = true;
  fld->data.visual.input_handler = &function_live_debugger_handle_input;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, fld));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  fld_module_state *state = (fld_module_state *)malloc(sizeof(fld_module_state));
  fld->extra = (void *)state;
  state->visual_node = fld;
  state->declare_uid = 10;
  state->function = NULL;
  state->arguments.alloc = 0;
  state->arguments.count = 0;
  state->visual_code.alloc = 0;
  state->visual_code.count = 0;
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