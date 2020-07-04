#include "midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

int special_update_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  frame_time *elapsed = (frame_time *)argv[0];

  return 0;
}

int core_display_render_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

    printf("core_display_render_v1-a\n");
    frame_time const *const elapsed = (frame_time const *const)argv[0];
    mc_node_v1 *visual_node = *(mc_node_v1 **)argv[1];

    printf("visual_node is %s\n", visual_node->name);

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

  //   // Lines
  //   function_edit_info *state = (function_edit_info *)visual_node->extra;
  //   code_line *lines = state->render_lines;

  //   for (int i = 0; i < FUNCTION_EDITOR_RENDERED_CODE_LINES; ++i) {
  //     if (lines[i].requires_render_update) {
  //       lines[i].requires_render_update = false;

  //       MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  //       sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  //       sequence->clear_color = (render_color){0.13f, 0.13f, 0.13f, 1.f};
  //       sequence->image_width = lines[i].width;
  //       sequence->image_height = lines[i].height;
  //       sequence->data.target_image.image_uid = lines[i].image_resource_uid;

  //       if (lines[i].text && strlen(lines[i].text)) {
  //         MCcall(obtain_element_render_command(sequence, &element_cmd));
  //         element_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  //         element_cmd->x = 4;
  //         element_cmd->y = 2 + 18;
  //         element_cmd->data.print_text.text = (const char **)&lines[i].text;
  //         element_cmd->data.print_text.font_resource_uid = command_hub->interactive_console->font_resource_uid;
  //         element_cmd->data.print_text.color = (render_color){0.61f, 0.86f, 0.99f, 1.f};
  //       }
  //     }
  //   }

  //   // Render
  //   // printf("OIRS: w:%u h:%u uid:%u\n", visual_node->data.visual.bounds.width, visual_node->data.visual.bounds.height,
  //   //        visual_node->data.visual.image_resource_uid);
  //   MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  //   sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  //   sequence->image_width = visual_node->data.visual.bounds.width;
  //   sequence->image_height = visual_node->data.visual.bounds.height;
  //   sequence->clear_color = COLOR_GHOST_WHITE;
  //   sequence->data.target_image.image_uid = visual_node->data.visual.image_resource_uid;


  //   for (int i = 0; i < FUNCTION_EDITOR_RENDERED_CODE_LINES; ++i) {
  //     MCcall(obtain_element_render_command(sequence, &element_cmd));
  //     element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
  //     element_cmd->x = 2;
  //     element_cmd->y = 8 + i * 24;
  //     element_cmd->data.textured_rect_info.width = lines[i].width;
  //     element_cmd->data.textured_rect_info.height = lines[i].height;
  //     element_cmd->data.textured_rect_info.texture_uid = lines[i].image_resource_uid;
  //   }

  return 0;
}

int core_display_handle_input_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  //   // printf("function_editor_handle_input_v1-a\n");
  //   frame_time const *const elapsed = (frame_time const *const)argv[0];
  //   mc_node_v1 *fedit = *(mc_node_v1 **)argv[1];
  //   mc_input_event_v1 *event = (mc_input_event_v1 *)argv[2];

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

int build_core_display_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  // Build the function editor window
  // Instantiate: node global;
  mc_node_v1 *core_objects_display = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  core_objects_display->name = "core_objects_display";
  core_objects_display->parent = command_hub->global_node;
  core_objects_display->type = NODE_TYPE_VISUAL;

  core_objects_display->data.visual.bounds.x = 0;
  core_objects_display->data.visual.bounds.y = 0;
  core_objects_display->data.visual.bounds.width = 200;
  core_objects_display->data.visual.bounds.height = 900;
  core_objects_display->data.visual.image_resource_uid = 0;
  core_objects_display->data.visual.requires_render_update = true;
  core_objects_display->data.visual.render_delegate = &core_display_render_v1;
  core_objects_display->data.visual.hidden = false;
  core_objects_display->data.visual.input_handler = &core_display_handle_input;

  MCcall(append_to_collection((void ***)&command_hub->global_node->children, &command_hub->global_node->children_alloc,
                              &command_hub->global_node->child_count, core_objects_display));

  // Obtain visual resources
  pthread_mutex_lock(&command_hub->renderer.resource_queue->mutex);
  resource_command *command;

  // Code Lines
  //   function_edit_info *state = (function_edit_info *)malloc(sizeof(function_edit_info));
  //   // printf("state:'%p'\n", state);
  //   state->cursorLine = 0;
  //   state->cursorCol = 0;
  //   state->line_display_offset = 0;
  //   state->text.lines_allocated = 0;
  //   state->text.lines_count = 0;
  //   code_line *code_lines = state->render_lines;
  //   for (int i = 0; i < FUNCTION_EDITOR_RENDERED_CODE_LINES; ++i) {

  //     code_lines[i].index = i;
  //     code_lines[i].requires_render_update = true;
  //     code_lines[i].text = NULL;
  //     //  "!this is twenty nine letters! "
  //     //  "!this is twenty nine letters! "
  //     //  "!this is twenty nine letters! ";

  //     MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
  //     command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  //     command->p_uid = &code_lines[i].image_resource_uid;
  //     command->data.create_texture.use_as_render_target = true;
  //     command->data.create_texture.width = code_lines[i].width = fedit->data.visual.bounds.width - 4;
  //     command->data.create_texture.height = code_lines[i].height = 28;
  //   }
  //   fedit->extra = (void *)state;

    // Function Editor Image
    MCcall(obtain_resource_command(command_hub->renderer.resource_queue, &command));
    command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
    command->p_uid = &core_objects_display->data.visual.image_resource_uid;
    command->data.create_texture.use_as_render_target = true;
    command->data.create_texture.width = core_objects_display->data.visual.bounds.width;
    command->data.create_texture.height = core_objects_display->data.visual.bounds.height;
  pthread_mutex_unlock(&command_hub->renderer.resource_queue->mutex);
  return 0;
}