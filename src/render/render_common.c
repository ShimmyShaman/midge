/* render_common.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/core_definitions.h"
#include "render/render_common.h"
// #include "render/resources/tiny_obj_loader_c.h"

#include "core/midge_app.h"

int mcr_obtain_image_render_request(render_thread_info *render_thread, image_render_details **p_request)
{
  image_render_details *render_request;
  pthread_mutex_lock(&render_thread->render_request_object_pool->mutex);
  if (render_thread->render_request_object_pool->count) {
    render_request =
        render_thread->render_request_object_pool->items[render_thread->render_request_object_pool->count - 1];
    --render_thread->render_request_object_pool->count;
    // printf("render_requestu=%p %u\n", render_request, render_thread->render_request_object_pool->count);
  }
  else {
    // Construct another
    render_request = (image_render_details *)malloc(sizeof(image_render_details));
    render_request->commands_allocated = 0;
    // printf("render_requestn=%p\n", render_request);
  }
  pthread_mutex_unlock(&render_thread->render_request_object_pool->mutex);

  render_request->command_count = 0;

  *p_request = render_request;

  return 0;
}

int mcr_submit_image_render_request(render_thread_info *render_thread, image_render_details *request)
{
  // printf("mcr_submit_image_render_request\n");
  pthread_mutex_lock(&render_thread->image_queue->mutex);

  // printf("sirr-0\n");
  MCcall(append_to_collection((void ***)&render_thread->image_queue->items, &render_thread->image_queue->alloc,
                              &render_thread->image_queue->count, request));

  // printf("sirr-1\n");
  // printf("sirr %u\n", render_thread->image_queue->count);
  pthread_mutex_unlock(&render_thread->image_queue->mutex);

  return 0;
}

int mcr_obtain_element_render_command(image_render_details *image_queue, element_render_command **p_command)
{
  // MCcall(obtain_item_from_collection((void **)image_queue->commands, &image_queue->commands_allocated,
  //                                    &image_queue->command_count, sizeof(element_render_command), (void
  //                                    **)p_command));
  if (image_queue->commands_allocated < image_queue->command_count + 1) {
    int new_alloc = image_queue->commands_allocated + 4 + image_queue->commands_allocated / 4;
    element_render_command *new_ary = (element_render_command *)malloc(sizeof(element_render_command) * new_alloc);

    if (image_queue->commands_allocated) {
      memcpy(new_ary, image_queue->commands, sizeof(element_render_command) * image_queue->command_count);
      free(image_queue->commands);
    }
    // printf("resized element render command list from %u to %u\n", image_queue->commands_allocated, new_alloc);
    image_queue->commands = new_ary;
    image_queue->commands_allocated = new_alloc;
  }

  *p_command = &image_queue->commands[image_queue->command_count++];

  return 0;
}

int mcr_obtain_resource_command(resource_queue *queue, resource_command **p_command)
{
  // MCcall(obtain_item_from_collection((void **)resource_queue->commands, &resource_queue->allocated,
  // &resource_queue->count,
  //                                    sizeof(resource_command), (void **)p_command));
  // printf("orc-0\n %p", queue);
  if (queue->allocated < queue->count + 1) {
    // printf("orc-1\n");
    int new_allocated = (queue->count + 1) + 4 + (queue->count + 1) / 4;
    // printf("orc-2 \n");
    resource_command *new_ary = (resource_command *)malloc(sizeof(resource_command) * new_allocated);
    // printf("orc-3\n");

    if (queue->allocated) {
      memcpy(new_ary, queue->commands, sizeof(resource_command) * queue->count);
      free(queue->commands);
    }
    // printf("orc-4\n");
    queue->commands = new_ary;
    queue->allocated = new_allocated;
  }
  // printf("orc-5\n");

  *p_command = &queue->commands[queue->count++];

  return 0;
}

// Ensure this function is accessed within a thread mutex lock of the @resource_queue
int mcr_create_texture_resource(unsigned int width, unsigned int height, mvk_image_sampler_usage image_usage,
                                mcr_texture_image **p_resource)
{
  *p_resource = NULL;

  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  MCcall(pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex));

  resource_command *command;
  MCcall(mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command));
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_resource = (void *)p_resource;
  command->create_texture.width = width;
  command->create_texture.height = height;
  command->create_texture.image_usage = image_usage;

  MCcall(pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex));

  return 0;
}

// Ensure this function is accessed within a thread mutex lock of the @resource_queue
int mcr_load_texture_resource(const char *path, mcr_texture_image **p_resource)
{
  *p_resource = NULL;

  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;
  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_TEXTURE;
  command->p_resource = (void *)p_resource;
  command->load_texture.path = path; // strdup(path);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);

  return 0;
}

// Ensure this function is accessed within a thread mutex lock of the @resource_queue
int mcr_obtain_font_resource(resource_queue *resource_queue, const char *font_path, float font_height,
                             mcr_font_resource **p_resource)
{
  *p_resource = NULL;

  pthread_mutex_lock(&resource_queue->mutex);
  // printf("mcr_obtain_font_resource-font_height:%f\n", font_height);
  resource_command *command;
  mcr_obtain_resource_command(resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_resource = (void *)p_resource;
  command->font.height = font_height;
  command->font.path = font_path;
  // printf("hrc-resource_cmd->font.height:%f\n", command->font.height);
  pthread_mutex_unlock(&resource_queue->mutex);

  return 0;
}

int mcr_create_render_program(mcr_render_program_create_info *create_info, mcr_render_program **p_resource)
{
  *p_resource = NULL;

  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;
  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_CREATE_RENDER_PROGRAM;
  command->p_resource = (void *)p_resource;

  // Straight duplicate the create info so it can be safely accessed and properly released on another thread
  mcr_render_program_create_info *rpci = &command->create_render_program.create_info;

  if (create_info->vertex_shader_filepath) {
    rpci->vertex_shader_filepath = strdup(create_info->vertex_shader_filepath);
  }
  else {
    MCerror(8173, "NotYetSupported");
  }
  if (create_info->fragment_shader_filepath) {
    rpci->fragment_shader_filepath = strdup(create_info->fragment_shader_filepath);
  }
  else {
    MCerror(8174, "NotYetSupported");
  }

  // for (int i = 0; i < create_info->buffer_binding_count; ++i) {
  //   printf("crbuffer-bindings[%i]={%i,%i,%lu}\n", i, create_info->buffer_bindings[i].type,
  //          create_info->buffer_bindings[i].stage_bit, create_info->buffer_bindings[i].size_in_bytes);
  // }
  // for (int i = 0; i < create_info->input_binding_count; ++i) {
  //   printf("crinput-bindings[%i]={%i,%lu}\n", i, create_info->input_bindings[i].format,
  //          create_info->input_bindings[i].size_in_bytes);
  // }

  // printf("sizeof(mcr_layout_binding):%lu\n", sizeof(mcr_layout_binding));
  rpci->buffer_binding_count = create_info->buffer_binding_count;
  memcpy(rpci->buffer_bindings, create_info->buffer_bindings,
         sizeof(mcr_layout_binding) * create_info->buffer_binding_count);

  rpci->input_binding_count = create_info->input_binding_count;
  memcpy(rpci->input_bindings, create_info->input_bindings,
         sizeof(mcr_input_binding) * create_info->input_binding_count);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);

  return 0;
}

int mcr_load_index_buffer(unsigned int *indices, unsigned int index_count, bool release_original_data_on_creation,
                          mcr_index_buffer **p_index_buffer)
{
  *p_index_buffer = NULL;

  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;
  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_INDEX_BUFFER;
  command->p_resource = (void *)p_index_buffer;
  command->load_indices.p_data = indices;
  command->load_indices.data_count = index_count;
  command->load_indices.release_original_data_on_copy = release_original_data_on_creation; // TODO -- toggle to true?

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);

  return 0;
}

int mcr_load_vertex_buffer(float *vertices, unsigned int vertex_count, bool release_original_data_on_creation,
                           mcr_vertex_buffer **p_vertex_buffer)
{
  *p_vertex_buffer = NULL;

  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;
  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_VERTEX_BUFFER;
  command->p_resource = (void *)p_vertex_buffer;
  command->load_mesh.p_data = vertices;
  command->load_mesh.data_count = vertex_count;
  command->load_mesh.release_original_data_on_copy = release_original_data_on_creation; // TODO -- toggle to true?

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);

  return 0;
}

int mcr_remap_buffer_memory(VkDescriptorBufferInfo *buffer, VkDeviceMemory memory, void *new_data,
                            size_t data_size_in_bytes)
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;
  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_MAP_MEMORY;
  command->p_resource = NULL;
  command->map_mem.mem = memory;
  command->map_mem.buf_info = buffer;
  command->map_mem.data = new_data;
  command->map_mem.data_size_in_bytes = data_size_in_bytes;

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);

  return 0;
}

// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
int mcr_issue_render_command_text(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                  mc_rect *clip, const char *text, mcr_font_resource *font, render_color font_color)
{
  element_render_command *render_cmd;
  mcr_obtain_element_render_command(image_render_queue, &render_cmd);
  render_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  render_cmd->x = x;
  render_cmd->y = y;
  if (clip)
    render_cmd->print_text.clip = *clip;
  else
    render_cmd->print_text.clip.extents.width = render_cmd->print_text.clip.extents.height = 0U;

  // printf("mcu_rtb-3 %p '%s' %u %u\n", render_cmd, text, render_cmd->x, render_cmd->y);

  // TODO -- make the render cmd a mc_str??
  render_cmd->print_text.text = strdup(text);

  if (font) {
    render_cmd->print_text.font = font;
  }
  else {
    midge_app_info *app_info;
    mc_obtain_midge_app_info(&app_info);

    render_cmd->print_text.font = app_info->ui_state->default_font_resource;
    // printf("set defaultfont %u\n", render_cmd->print_text.font->resource_uid);
  }
  render_cmd->print_text.color = font_color;

  return 0;
}

// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
int mcr_issue_render_command_colored_quad(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                          unsigned int width, unsigned int height, render_color color)
{
  if (!color.a)
    return 0;

  element_render_command *render_cmd;
  mcr_obtain_element_render_command(image_render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_COLORED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->colored_rect_info.width = width;
  render_cmd->colored_rect_info.height = height;
  render_cmd->colored_rect_info.color = color;

  return 0;
}

// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
int mcr_issue_render_command_textured_quad(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                           unsigned int width, unsigned int height, mcr_texture_image *texture_resource)
{
  element_render_command *render_cmd;
  mcr_obtain_element_render_command(image_render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_TEXTURED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->textured_rect_info.width = width;
  render_cmd->textured_rect_info.height = height;
  render_cmd->textured_rect_info.texture = texture_resource;

  return 0;
}
// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
int mcr_issue_render_command_render_program(image_render_details *image_render_queue, mcr_render_program *program,
                                            mcr_render_program_data *program_data)
{
  element_render_command *render_cmd;
  mcr_obtain_element_render_command(image_render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_PROGRAM;
  render_cmd->render_program.program = program;
  render_cmd->render_program.data = program_data;

  return 0;
}