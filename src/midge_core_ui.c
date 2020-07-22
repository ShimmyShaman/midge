#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

#define RENDERED_CORE_ENTRIES 34
typedef struct core_entry {
  struct {
    uint alloc, count;
    core_entry **items;
  } children;

  int type;
  void *data;

  bool collapsed;
} core_entry;

typedef struct core_display_state {
  uint font_resource_uid;
  struct {
    unsigned int x, y, width, height;
  } function_button_bounds;
  struct {
    uint utilized_count;
    uint alloc, count;
    node **items;
  } entry_visual_nodes;
  struct {
    uint alloc, count;
    core_entry **items;
    bool requires_render_update;
  } entries;
} core_display_state;

int core_display_entry_handle_input_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  //   // printf("code_editor_handle_input_v1-a\n");
  // frame_time const *elapsed = *(frame_time const **)argv[0];
  //   mc_node_v1 *fedit = *(mc_node_v1 **)argv[1];
  //   mc_input_event_v1 *event = *(mc_input_event_v1 **)argv[2];

  //   if (fedit->data.visual.hidden)
  //     return 0;

  //   if (event->type != INPUT_EVENT_KEY_PRESS)
  //     return 0;

  //   function_edit_info *state = (function_edit_info *)fedit->extra;

  //   if (event->code == INPUT_EVENT_CODE_ENTER || event->code == INPUT_EVENT_CODE_RETURN) {
  //     if (event->ctrlDown) {

  //       // Read the code from the editor
  //       function_info *func_info;
  //       MCcall(read_and_declare_function_from_editor(state, &func_info));

  //       // Compile the function definition
  //       uint transcription_alloc = 4;
  //       char *transcription = (char *)malloc(sizeof(char) * transcription_alloc);
  //       transcription[0] = '\0';
  //       int code_index = 0;
  //       MCcall(transcribe_c_block_to_mc(func_info, func_info->mc_code, &code_index, &transcription_alloc,
  //       &transcription));

  //       printf("final transcription:\n%s\n", transcription);

  //       // Define the new function
  //       {
  //         void *mc_vargs[3];
  //         mc_vargs[0] = (void *)&func_info->name;
  //         mc_vargs[1] = (void *)&transcription;
  //         MCcall(instantiate_function(2, mc_vargs));
  //       }

  //       return 0;
  //     }
  //     else {
  //       // TODO
  //     }
  //   }

  //   char c = '\0';
  //   int res = get_key_input_code_char(event->shiftDown, event->code, &c);
  //   if (res)
  //     return 0; // TODO

  //   // Update the text
  //   {
  //     int current_line_len = strlen(state->text.lines[state->cursorLine]);
  //     char *new_line = (char *)malloc(sizeof(char) * (current_line_len + 1 + 1));
  //     if (state->cursorCol) {
  //       strncpy(new_line, state->text.lines[state->cursorLine], state->cursorCol);
  //     }
  //     new_line[state->cursorCol] = c;
  //     if (current_line_len - state->cursorCol) {
  //       strcat(new_line + state->cursorCol + 1, state->text.lines[state->cursorLine]);
  //     }
  //     new_line[current_line_len + 1] = '\0';

  //     free(state->text.lines[state->cursorLine]);
  //     state->text.lines[state->cursorLine] = new_line;
  //   }

  //   // Update the rendered line for the text
  //   if (state->cursorLine > state->line_display_offset &&
  //       state->cursorLine - state->line_display_offset < +CODE_EDITOR_RENDERED_CODE_LINES) {

  //     if (state->render_lines[state->cursorLine - state->line_display_offset].text) {
  //       free(state->render_lines[state->cursorLine - state->line_display_offset].text);
  //     }
  //     allocate_and_copy_cstr(state->render_lines[state->cursorLine - state->line_display_offset].text,
  //                            state->text.lines[state->cursorLine]);
  //     state->render_lines[state->cursorLine - state->line_display_offset].requires_render_update = true;
  //     fedit->data.visual.requires_render_update = true;
  //   }

  return 0;
}

int build_core_entry(node *core_display, int index)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  printf("bce-0\n");
  core_display_state *cdstate = (core_display_state *)core_display->extra;

  printf("bce-1\n");
  mc_node_v1 *core_entry = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  MCcall(append_to_collection((void ***)&cdstate->entry_visual_nodes.items, &cdstate->entry_visual_nodes.alloc,
                              &cdstate->entry_visual_nodes.count, core_entry));
  printf("bce-2\n");
  core_entry->parent = core_display;
  cprintf(core_entry->name, "core_entry%i", index);
  core_entry->type = NODE_TYPE_VISUAL;

  printf("bce-3\n");
  core_entry->data.visual.bounds.x = 0;
  core_entry->data.visual.bounds.y = 0;
  core_entry->data.visual.bounds.x = core_display->data.visual.bounds.x + 2;
  core_entry->data.visual.bounds.y = core_display->data.visual.bounds.y + 2 + index * 26;
  core_entry->data.visual.bounds.width = 296;
  core_entry->data.visual.bounds.height = 26;
  core_entry->data.visual.image_resource_uid = 0;
  core_entry->data.visual.requires_render_update = true;
  core_entry->data.visual.render_delegate = NULL;
  core_entry->data.visual.hidden = false;
  core_entry->data.visual.input_handler = &core_display_entry_handle_input;

  MCcall(append_to_collection((void ***)&core_display->children, &core_display->children_alloc,
                              &core_display->child_count, core_entry));

  printf("bce-5\n");
  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Function Editor Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &core_entry->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = core_entry->data.visual.bounds.width;
  command->data.create_texture.height = core_entry->data.visual.bounds.height;
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  printf("bce-8\n");
  return 0;
}

int update_core_entries(node *core_display)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  core_display_state *cdstate = (core_display_state *)core_display->extra;

  if (cdstate->entries.count > 0) {
    MCerror(176, "TODO -- maintain collapsed states etc between updates");
  }

  for (int sfi = 0; sfi < command_hub->source_files.count; ++sfi) {
    mc_source_file_info_v1 *source_file = command_hub->source_files.items[sfi];

    if (source_file->definitions.count < 1) {
      continue;
    }

    if (source_file->definitions.count == 1) {
      // Add only the definition
      core_entry *entry = (core_entry *)malloc(sizeof(core_entry));
      entry->type = source_file->definitions.items[0]->type;
      entry->data = source_file->definitions.items[0]->data;
      entry->children.alloc = 0;
      entry->children.count = 0;
      entry->collapsed = false;

      MCcall(append_to_collection((void ***)&cdstate->entries.items, &cdstate->entries.alloc, &cdstate->entries.count,
                                  entry));
    }
    else {
      // Add an entry for the source file
      core_entry *entry = (core_entry *)malloc(sizeof(core_entry));
      entry->type = (source_definition_type)SOURCE_FILE_MC_DEFINITIONS;
      entry->data = (void *)source_file;
      entry->children.alloc = 0;
      entry->children.count = 0;
      entry->collapsed = false;

      MCcall(append_to_collection((void ***)&cdstate->entries.items, &cdstate->entries.alloc, &cdstate->entries.count,
                                  entry));

      // Add an entry for each of its definitions
      for (int a = 0; a < source_file->definitions.count; ++a) {
        core_entry *child = (core_entry *)malloc(sizeof(core_entry));
        child->type = source_file->definitions.items[a]->type;
        child->data = source_file->definitions.items[a]->data;
        child->children.alloc = 0;
        child->children.count = 0;
        child->collapsed = false;

        MCcall(append_to_collection((void ***)&entry->children.items, &entry->children.alloc, &entry->children.count,
                                    child));
      }
    }
  }
  cdstate->entries.requires_render_update = true;

  return 0;
}

int mcu_render_core_entry(core_display_state *cdstate, core_entry *entry, int indent)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  image_render_queue *sequence;
  element_render_command *element_cmd;

  // printf("mrce-0\n");

  node *child = (node *)cdstate->entry_visual_nodes.items[cdstate->entry_visual_nodes.utilized_count++];
  child->data.visual.hidden = false;

  MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  sequence->image_width = child->data.visual.bounds.width;
  sequence->image_height = child->data.visual.bounds.height;
  sequence->clear_color = (render_color){0.16f, 0.16f, 0.16f, 0.2f};
  sequence->data.target_image.image_uid = child->data.visual.image_resource_uid;

  MCcall(obtain_element_render_command(sequence, &element_cmd));
  element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  element_cmd->x = 6;
  element_cmd->y = 18;
  element_cmd->data.print_text.font_resource_uid = cdstate->font_resource_uid;

  int indent_len = indent * 2 + indent ? 1 : 0;
  char indent_str[indent_len + 1];
  for (int a = 0; a < indent_len; ++a) {
    indent_str[a] = ' ';
  }
  indent_str[indent_len] = '\0';

  // printf("mrce-2\n");
  switch (entry->type) {
  case SOURCE_DEFINITION_FUNCTION: {
    function_info *func_info = (function_info *)entry->data;
    cprintf(element_cmd->data.print_text.text, "%s%s", indent_str, func_info->name);
    element_cmd->data.print_text.color = COLOR_FUNCTION_GREEN;
  } break;
  case SOURCE_DEFINITION_STRUCT: {
    struct_info *str_info = (struct_info *)entry->data;
    cprintf(element_cmd->data.print_text.text, "%s%s", indent_str, str_info->name);
    element_cmd->data.print_text.color = COLOR_LIGHT_YELLOW;
  } break;
  case SOURCE_FILE_MC_DEFINITIONS: {
    mc_source_file_info_v1 *source_file = (mc_source_file_info_v1 *)entry->data;
    if (entry->collapsed) {
      cprintf(element_cmd->data.print_text.text, "%s+%s", indent_str, source_file->filepath);
    }
    else {
      cprintf(element_cmd->data.print_text.text, "%s-%s", indent_str, source_file->filepath);
    }
    element_cmd->data.print_text.color = COLOR_LIGHT_SKY_BLUE;
  } break;
  default: {
    MCerror(278, "Unsupported type:%i", entry->type);
  }
  }

  // printf("mrce-6\n");
  if (!entry->collapsed) {
    for (int b = 0;
         b < entry->children.count && cdstate->entry_visual_nodes.utilized_count < cdstate->entry_visual_nodes.count;
         ++b) {
      core_entry *subentry = entry->children.items[b];
      MCcall(mcu_render_core_entry(cdstate, subentry, 1));
    }
  }

  // printf("mrce-8\n");
  return 0;
}

int core_display_render_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // printf("core_display_render_v1()\n");
  frame_time const *elapsed = *(frame_time const **)argv[0];
  mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

  if (visual_node->data.visual.hidden)
    return 0;
  core_display_state *cdd = (core_display_state *)visual_node->extra;

  image_render_queue *sequence;
  element_render_command *element_cmd;

  if (cdd->entries.requires_render_update) {
    cdd->entry_visual_nodes.utilized_count = 0;
    for (int a = 0; a < cdd->entries.count && cdd->entry_visual_nodes.utilized_count < cdd->entry_visual_nodes.count;
         ++a) {

      core_entry *core_entry = cdd->entries.items[a];

      MCcall(mcu_render_core_entry(cdd, core_entry, 0));
    }

    // Hide the rest
    for (int a = cdd->entry_visual_nodes.utilized_count; a < cdd->entry_visual_nodes.count; ++a) {
      cdd->entry_visual_nodes.items[a]->data.visual.hidden = true;
    }

    cdd->entries.requires_render_update = false;
  }

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

  for (int i = 0; i < visual_node->child_count; ++i) {
    node *child = (node *)visual_node->children[i];

    if (child->data.visual.hidden) {
      continue;
    }

    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
    element_cmd->x = child->data.visual.bounds.x;
    element_cmd->y = child->data.visual.bounds.y;
    element_cmd->data.textured_rect_info.width = child->data.visual.bounds.width;
    element_cmd->data.textured_rect_info.height = child->data.visual.bounds.height;
    element_cmd->data.textured_rect_info.texture_uid = child->data.visual.image_resource_uid;
  }

  // Function Button
  {
    // MCcall(obtain_element_render_command(sequence, &element_cmd));
    // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    // element_cmd->x = cdd->function_button_bounds.x;
    // element_cmd->y = cdd->function_button_bounds.y;
    // element_cmd->data.colored_rect_info.width = cdd->function_button_bounds.width;
    // element_cmd->data.colored_rect_info.height = cdd->function_button_bounds.height;
    // element_cmd->data.colored_rect_info.color = COLOR_PURPLE;

    // MCcall(obtain_element_render_command(sequence, &element_cmd));
    // element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    // element_cmd->x = cdd->function_button_bounds.x + 2;
    // element_cmd->y = cdd->function_button_bounds.y + 2;
    // element_cmd->data.colored_rect_info.width = cdd->function_button_bounds.width - 4;
    // element_cmd->data.colored_rect_info.height = cdd->function_button_bounds.height - 4;
    // element_cmd->data.colored_rect_info.color = (render_color){0.03f, 0.33f, 0.03f, 1.f};

    // MCcall(obtain_element_render_command(sequence, &element_cmd));
    // element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
    // element_cmd->x = cdd->function_button_bounds.x + 2;
    // element_cmd->y = cdd->function_button_bounds.y + 18;
    // element_cmd->data.print_text.font_resource_uid = cdd->font_resource_uid;
    // const char *function_text = "function";
    // element_cmd->data.print_text.text = &function_text;
    // element_cmd->data.print_text.color = COLOR_GHOST_WHITE;
  }

  return 0;
}

int core_display_handle_input_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  frame_time const *elapsed = *(frame_time const **)argv[0];
  mc_node_v1 *core_display = *(mc_node_v1 **)argv[1];
  mc_input_event_v1 *event = *(mc_input_event_v1 **)argv[2];

  if (core_display->data.visual.hidden)
    return 0;

  core_display_state *cdd = (core_display_state *)core_display->extra;

  // Function Button
  // if (event->detail.mouse.x >= cdd->function_button_bounds.x &&
  //     event->detail.mouse.y >= cdd->function_button_bounds.y &&
  //     event->detail.mouse.x < cdd->function_button_bounds.x + cdd->function_button_bounds.width &&
  //     event->detail.mouse.y < cdd->function_button_bounds.y + cdd->function_button_bounds.height) {
  //   MCcall(build_core_entry(core_display, "unnamed"));
  //   MCcall(update_core_entries(core_display));

  //   core_display->data.visual.requires_render_update = true;
  //   return 0;
  // }

  // Find the core entry being clicked on

  for (int i = 0; !event->handled && i < cdd->entry_visual_nodes.count; ++i) {
    node *entry_node = cdd->entry_visual_nodes.items[i];

    if (entry_node->data.visual.hidden) {
      continue;
    }

    // Check is visual and has input handler and mouse event is within bounds
    // if (!*child->data.visual.input_handler)
    //   continue;
    if (event->detail.mouse.x < entry_node->data.visual.bounds.x ||
        event->detail.mouse.y < entry_node->data.visual.bounds.y ||
        event->detail.mouse.x >= entry_node->data.visual.bounds.x + entry_node->data.visual.bounds.width ||
        event->detail.mouse.y >= entry_node->data.visual.bounds.y + entry_node->data.visual.bounds.height)
      continue;
    printf("x:%u y:%u button:%u\n", event->detail.mouse.x, event->detail.mouse.y, event->detail.mouse.button);

    switch (event->type) {
    case INPUT_EVENT_MOUSE_PRESS: {
      // Find the entry this node represents
      int ei = 0;
      core_entry *entry = NULL;
      for (int a = 0; a < cdd->entries.count && entry == NULL; ++a) {
        core_entry *iter = cdd->entries.items[a];
        if (ei == i) {
          entry = iter;
          break;
        }
        ++ei;

        if (!iter->collapsed && iter->children.count) {
          for (int b = 0; b < iter->children.count && entry == NULL; ++b) {
            if (ei == i) {
              entry = iter->children.items[b];
              break;
            }
            ++ei;
            if (iter->children.items[b]->children.count) {
              MCerror(482, "Not doing nested yet");
            }
          }
        }
      }

      if (!entry) {
        continue;
      }
      event->handled = true;

      switch (entry->type) {
      case SOURCE_DEFINITION_FUNCTION: {
        void *vargs[1];
        vargs[0] = (void **)&entry->data;
        MCcall(load_existing_function_into_code_editor(1, vargs));
      } break;
      case SOURCE_DEFINITION_STRUCT: {
        void *mc_vargs[2];
        mc_vargs[0] = (void *)&command_hub->global_node->children[0];
        mc_vargs[1] = (void *)&entry->data;
        MCcall(load_existing_struct_into_code_editor(2, mc_vargs));
      } break;
      case SOURCE_FILE_MC_DEFINITIONS: {
        entry->collapsed = !entry->collapsed;
        cdd->entries.requires_render_update = true;
        core_display->data.visual.requires_render_update = true;
      } break;
      default: {
        MCerror(513, "Unsupported type:%i", entry->type);
      }
      }
    }
    }
  }

  return 0;
}

int build_core_display_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // printf("build_core_display_v1\n");
  // Build the function editor window
  // Instantiate: node global;
  mc_node_v1 *core_objects_display = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  core_objects_display->name = "core_objects_display";
  core_objects_display->parent = command_hub->global_node;
  core_objects_display->type = NODE_TYPE_VISUAL;

  core_objects_display->child_count = 0;
  core_objects_display->children_alloc = 4;
  core_objects_display->children = (node **)malloc(sizeof(void *) * core_objects_display->children_alloc);

  core_objects_display->data.visual.bounds.x = 0;
  core_objects_display->data.visual.bounds.y = 0;
  core_objects_display->data.visual.bounds.width = 300;
  core_objects_display->data.visual.bounds.height = 900;
  core_objects_display->data.visual.image_resource_uid = 0;
  core_objects_display->data.visual.requires_render_update = true;
  core_objects_display->data.visual.render_delegate = &core_display_render;
  core_objects_display->data.visual.hidden = false;
  core_objects_display->data.visual.input_handler = &core_display_handle_input;

  core_display_state *cdd = (core_display_state *)malloc(sizeof(core_display_state));
  core_objects_display->extra = cdd;
  cdd->entries.alloc = 0;
  cdd->entries.count = 0;
  cdd->entries.requires_render_update = true;
  cdd->entry_visual_nodes.alloc = 0;
  cdd->entry_visual_nodes.count = 0;
  cdd->font_resource_uid = 0;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, core_objects_display));

  printf("bcd-0\n");
  for (int a = 0; a < RENDERED_CORE_ENTRIES; ++a) {
    MCcall(build_core_entry(core_objects_display, a));
  }
  printf("bcd-1\n");
  // MCcall(build_core_entry(core_objects_display, "find_struct_info"));
  // MCcall(build_core_entry(core_objects_display, "special_data"));
  // MCcall(build_core_entry(core_objects_display, "special_modification"));
  // MCcall(build_core_entry(core_objects_display, "special_update"));
  // MCcall(build_core_entry(core_objects_display, "move_cursor_up"));
  // MCcall(build_core_entry(core_objects_display, "save_function_to_file"));
  // MCcall(build_core_entry(core_objects_display, "save_struct_to_file"));
  // MCcall(build_core_entry(core_objects_display, "insert_text_into_editor"));
  // MCcall(build_core_entry(core_objects_display, "delete_selection"));
  // MCcall(build_core_entry(core_objects_display, "read_selected_editor_text"));
  // MCcall(build_core_entry(core_objects_display, "load_existing_struct_into_code_editor"));
  // MCcall(build_core_entry(core_objects_display, "code_editor_handle_keyboard_input"));
  // MCcall(build_core_entry(core_objects_display, "code_editor_handle_input"));
  // MCcall(build_core_entry(core_objects_display, "code_editor_state"));

  MCcall(update_core_entries(core_objects_display));
  printf("bcd-2\n");

  // cdd->function_button_bounds.x = 2;
  // cdd->function_button_bounds.y = core_objects_display->data.visual.bounds.height - 26;
  // cdd->function_button_bounds.width = 80;
  // cdd->function_button_bounds.height = 24;

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Function Editor Image
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = &core_objects_display->data.visual.image_resource_uid;
  command->data.create_texture.use_as_render_target = true;
  command->data.create_texture.width = core_objects_display->data.visual.bounds.width;
  command->data.create_texture.height = core_objects_display->data.visual.bounds.height;

  // Font
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &cdd->font_resource_uid;
  command->data.font.height = 18;
  command->data.font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  return 0;
}