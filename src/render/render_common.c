#include "render/render_common.h"

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

void mcr_obtain_font_resource(resource_queue *resource_queue, const char *font_path, float font_height,
                              unsigned int *p_resource_uid)
{
  resource_command *command;
  obtain_resource_command(resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = p_resource_uid;
  command->data.font.height = font_height;
  command->data.font.path = font_path;
}