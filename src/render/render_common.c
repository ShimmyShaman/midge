#include "render/render_common.h"
#include "core/core_definitions.h"
#include "render/mc_vulkan.h"

#include "stb_truetype.h"

// Ensure this function is accessed within a thread mutex lock of the @resource_queue
void mcr_create_texture_resource(unsigned int width, unsigned int height, bool use_as_render_target,
                                 unsigned int *p_resource_uid)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue.mutex);

  resource_command *command;
  obtain_resource_command(&global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = p_resource_uid;
  command->data.create_texture.width = width;
  command->data.create_texture.height = height;
  command->data.create_texture.use_as_render_target = use_as_render_target;

  pthread_mutex_unlock(&global_data->render_thread->resource_queue.mutex);
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

void mcr_determine_text_display_dimensions(unsigned int font_resource, const char *text, float *text_width,
                                           float *text_height)
{
  if (text == NULL || text[0] == '\0') {
    *text_width = 0;
    *text_height = 0;
    return;
  }

  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  if (font_resource == 0) {
    // Use the global default font resource
    font_resource = global_data->ui_state->default_font_resource;
  }

  // Obtain the font
  loaded_font_info *font = NULL;
  for (int f = 0; f < global_data->render_thread->loaded_fonts->count; ++f) {
    if (global_data->render_thread->loaded_fonts->fonts[f].resource_uid == font_resource) {
      font = &global_data->render_thread->loaded_fonts->fonts[f];
      break;
    }
  }

  if (!font) {
    MCerror(7857, "Could not find requested font uid=%u\n", font_resource);
  }

  *text_width = 0;
  *text_height = font->draw_vertical_offset;

  int text_length = strlen(text);
  for (int c = 0; c < text_length; ++c) {

    char letter = text[c];
    if (letter < 32 || letter > 127) {
      MCerror(7874, "TODO character '%i' not supported.\n", letter);
    }

    // Source texture bounds
    stbtt_aligned_quad q;

    // printf("garbagein: %i %i %f %f %i\n", (int)font_image->width, (int)font_image->height, align_x, align_y, letter -
    // 32);

    stbtt_GetBakedQuad(font->char_data, 256, 256, letter - 32, text_width, text_height, &q, 1);
    // printf("char:'%c' w:%.2f h:%.2f\n", letter, *text_width, *text_height);
  }
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
void mcr_issue_render_command_colored_quad(image_render_queue *render_queue, unsigned int x, unsigned int y,
                                           unsigned int width, unsigned int height, render_color color)
{
  if (!color.a)
    return;

  element_render_command *render_cmd;
  obtain_element_render_command(render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_COLORED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->data.colored_rect_info.width = width;
  render_cmd->data.colored_rect_info.height = height;
  render_cmd->data.colored_rect_info.color = color;
}

// Ensure this function is accessed within a thread mutex lock of the @render_queue
void mcr_issue_render_command_textured_quad(image_render_queue *render_queue, unsigned int x, unsigned int y,
                                            unsigned int width, unsigned int height, unsigned int texture_resource)
{
  element_render_command *render_cmd;
  obtain_element_render_command(render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_TEXTURED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->data.textured_rect_info.width = width;
  render_cmd->data.textured_rect_info.height = height;
  render_cmd->data.textured_rect_info.texture_uid = texture_resource;
}