#include "render/render_common.h"

// Ensure this function is accessed within a thread mutex lock of the @resource_queue
void mcr_create_texture_resource(resource_queue *resource_queue, unsigned int width, unsigned int height,
                                 bool use_as_render_target, unsigned int *p_resource_uid)
{
  resource_command *command;
  obtain_resource_command(resource_queue, &command);
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = p_resource_uid;
  command->data.create_texture.width = width;
  command->data.create_texture.height = height;
  command->data.create_texture.use_as_render_target = use_as_render_target;
}

// Ensure this function is accessed within a thread mutex lock of the @resource_queue
void mcr_obtain_font_resource(resource_queue *resource_queue, const char *font_path, float font_height,
                              unsigned int *p_resource_uid)
{
  // pthread_mutex_lock(&resource_queue->mutex);
  // printf("mcr_obtain_font_resource-font_height:%f\n", font_height);
  resource_command *command;
  obtain_resource_command(resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = p_resource_uid;
  command->data.font.height = font_height;
  command->data.font.path = font_path;
  // printf("hrc-resource_cmd->data.font.height:%f\n", command->data.font.height);
  // pthread_mutex_unlock(&resource_queue->mutex);
}

// Ensure this function is accessed within a thread mutex lock of the @render_queue
void mcr_issue_render_command_text(image_render_queue *render_queue, unsigned int x, unsigned int y, const char *text,
                                   unsigned int font_resource_uid, render_color font_color)
{
  element_render_command *render_cmd;
  obtain_element_render_command(render_queue, &render_cmd);
  render_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  render_cmd->x = x;
  render_cmd->y = y;
  // printf("mui_rtb-3 %u %u\n", render_cmd->x, render_cmd->y);

  // TODO -- make the render cmd a c_str??
  render_cmd->data.print_text.text = strdup(text);

  if (font_resource_uid) {
    render_cmd->data.print_text.font_resource_uid = font_resource_uid;
  }
  else {
    global_root_data *global_data;
    obtain_midge_global_root(&global_data);

    render_cmd->data.print_text.font_resource_uid = global_data->ui_state->default_font_resource;
    // printf("set defaultfont %u\n", render_cmd->data.print_text.font_resource_uid);
  }
  render_cmd->data.print_text.color = font_color;
}

// Ensure this function is accessed within a thread mutex lock of the @render_queue
void mcr_issue_render_command_colored_rect(image_render_queue *render_queue, unsigned int x, unsigned int y,
                                           unsigned int width, unsigned int height, render_color color)
{
  element_render_command *render_cmd;
  obtain_element_render_command(render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_COLORED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->data.colored_rect_info.width = width;
  render_cmd->data.colored_rect_info.height = height;
  render_cmd->data.colored_rect_info.color = color;
}