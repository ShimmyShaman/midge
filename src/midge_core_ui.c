#include "midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

int special_update_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  frame_time const *elapsed = *(frame_time const **)argv[0];

  return 0;
}

typedef struct core_display_data {
  uint font_resource_uid;
} core_display_data;

int core_display_entry_handle_input_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  //   // printf("function_editor_handle_input_v1-a\n");
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
  //       MCcall(transcribe_c_block_to_mc(func_info, func_info->mc_code, &code_index, &transcription_alloc, &transcription));

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
  //       state->cursorLine - state->line_display_offset < +FUNCTION_EDITOR_RENDERED_CODE_LINES) {

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

int build_core_entry(node *core_display, const char *name)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  mc_node_v1 *core_entry = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  allocate_and_copy_cstr(core_entry->name, name);
  core_entry->parent = core_display;
  core_entry->type = NODE_TYPE_VISUAL;

  core_entry->data.visual.bounds.x = 0;
  core_entry->data.visual.bounds.y = 0;
  core_entry->data.visual.bounds.width = 296;
  core_entry->data.visual.bounds.height = 26;
  core_entry->data.visual.image_resource_uid = 0;
  core_entry->data.visual.requires_render_update = true;
  core_entry->data.visual.render_delegate = NULL;
  core_entry->data.visual.hidden = false;
  core_entry->data.visual.input_handler = &core_display_entry_handle_input;

  MCcall(append_to_collection((void ***)&core_display->children, &core_display->children_alloc, &core_display->child_count,
                              core_entry));

  return 0;
}

int update_core_entries(node *core_display)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  for (int i = 0; i < core_display->child_count; ++i) {
    node *child = (node *)core_display->children[i];

    child->data.visual.bounds.x = core_display->data.visual.bounds.x + 2;
    child->data.visual.bounds.y = core_display->data.visual.bounds.y + 2 + i * 26;
    child->data.visual.bounds.width = 296;
    child->data.visual.bounds.height = 26;
  }

  return 0;
}

int core_display_render_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
                                  /*mcfuncreplace*/

  frame_time const *elapsed = *(frame_time const **)argv[0];
  mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

  if (visual_node->data.visual.hidden)
    return 0;
  core_display_data *cdd = (core_display_data *)visual_node->extra;

  image_render_queue *sequence;
  element_render_command *element_cmd;

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

  for (int i = 0; i < visual_node->child_count; ++i) {
    node *child = (node *)visual_node->children[i];

    printf("core_child.bounds x=%u y=%u width=%u height=%u\n", child->data.visual.bounds.x, child->data.visual.bounds.y,
           child->data.visual.bounds.width, child->data.visual.bounds.height);

    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_COLORED_RECTANGLE;
    element_cmd->x = child->data.visual.bounds.x;
    element_cmd->y = child->data.visual.bounds.y;
    element_cmd->data.colored_rect_info.width = child->data.visual.bounds.width;
    element_cmd->data.colored_rect_info.height = child->data.visual.bounds.height;
    element_cmd->data.colored_rect_info.color = (render_color){0.13f, 0.19f, 0.28f, 1.f};

    MCcall(obtain_element_render_command(sequence, &element_cmd));
    element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
    element_cmd->x = child->data.visual.bounds.x + 6;
    element_cmd->y = child->data.visual.bounds.y + 18;
    element_cmd->data.print_text.font_resource_uid = cdd->font_resource_uid;
    element_cmd->data.print_text.text = &child->name;
    element_cmd->data.print_text.color = COLOR_GHOST_WHITE;
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

  for (int i = 0; !event->handled && i < core_display->child_count; ++i) {
    node *child = (node *)core_display->children[i];

    // Check is visual and has input handler and mouse event is within bounds
    if (child->type != NODE_TYPE_VISUAL)
      continue;
    // if (!*child->data.visual.input_handler)
    //   continue;
    if (event->detail.mouse.x < child->data.visual.bounds.x || event->detail.mouse.y < child->data.visual.bounds.y ||
        event->detail.mouse.x >= child->data.visual.bounds.x + child->data.visual.bounds.width ||
        event->detail.mouse.y >= child->data.visual.bounds.y + child->data.visual.bounds.height)
      continue;
    printf("x:%u y:%u button:%u\n", event->detail.mouse.x, event->detail.mouse.y, event->detail.mouse.button);

    switch (event->type) {
    case INPUT_EVENT_MOUSE_PRESS: {
      event->handled = true;

      // Find the core object the name represents
      {
        function_info *function;
        void *vargs[3];
        vargs[0] = (void **)&function;
        vargs[1] = (void **)&command_hub->global_node;
        vargs[2] = (void **)&child->name;
        find_function_info(3, vargs);

        if (function) {
          // Exists as function
          // Make visible the function editor and set
          // mc_node_v1 *function_editor = (mc_node_v1 *)command_hub->global_node->children[0]; // TODO -- better way...
          // function_editor->data.visual.hidden = false;
          // function_editor->data.visual.requires_render_update = true;

          MCcall(load_existing_function_into_function_editor(function));
          break;
        }
      }
    } break;

    default:
      break;
    }
  }

  return 0;
}

int build_core_display_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

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
  core_objects_display->data.visual.render_delegate = &core_display_render_v1;
  core_objects_display->data.visual.hidden = false;
  core_objects_display->data.visual.input_handler = &core_display_handle_input;

  core_display_data *cdd = (core_display_data *)malloc(sizeof(core_display_data));
  core_objects_display->extra = cdd;
  cdd->font_resource_uid = 0;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, core_objects_display));

  MCcall(build_core_entry(core_objects_display, "special_update"));
  MCcall(build_core_entry(core_objects_display, "function_editor_handle_input"));

  MCcall(update_core_entries(core_objects_display));

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
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);

  // Font
  MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = &cdd->font_resource_uid;
  command->data.font.height = 18;
  command->data.font.path = "res/font/DroidSansMono.ttf";
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  return 0;
}