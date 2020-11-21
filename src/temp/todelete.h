#include "midge_error_handling.h"

/* render_common.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stb/stb_truetype.h"

#include "core/core_definitions.h"
#include "render/render_common.h"
// #include "render/resources/tiny_obj_loader_c.h"

#include "core/midge_app.h"

int mcr_obtain_image_render_request(render_thread_info *render_thread, image_render_details **p_request) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_obtain_image_render_request", __FILE__, __LINE__, &midge_error_stack_index);

  image_render_details *render_request;

  pthread_mutex_lock(&render_thread->render_request_object_pool->mutex);
  if (render_thread->render_request_object_pool->count) {
    render_request = render_thread->render_request_object_pool->items[render_thread->render_request_object_pool->count - 1];
    --render_thread->render_request_object_pool->count;
  }
  else {
    render_request = (image_render_details *)malloc(sizeof(image_render_details));
    render_request->commands_allocated = 0;
  }

  pthread_mutex_unlock(&render_thread->render_request_object_pool->mutex);

  render_request->command_count = 0;

  *p_request = render_request;


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


int mcr_submit_image_render_request(render_thread_info *render_thread, image_render_details *request) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_submit_image_render_request", __FILE__, __LINE__, &midge_error_stack_index);

  pthread_mutex_lock(&render_thread->image_queue->mutex);

  // printf("sirr-0\n");
  int res = append_to_collection((void ***)&render_thread->image_queue->items, &render_thread->image_queue->alloc, &render_thread->image_queue->count, request);


  // printf("sirr-1\n");
  // printf("sirr %u\n", render_thread->image_queue->count);
  pthread_mutex_unlock(&render_thread->image_queue->mutex);


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return res;
  }

}


int mcr_obtain_element_render_command(image_render_details *image_queue, element_render_command **p_command) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_obtain_element_render_command", __FILE__, __LINE__, &midge_error_stack_index);

  if (image_queue->commands_allocated<image_queue->command_count + 1) {
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


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


int mcr_obtain_resource_command(resource_queue *queue, resource_command **p_command) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_obtain_resource_command", __FILE__, __LINE__, &midge_error_stack_index);

  if (queue->allocated<queue->count + 1) {
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


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


// Ensure this function is accessed within a thread mutex lock of the @resource_queue
int mcr_create_texture_resource(unsigned int width, unsigned int height, mvk_image_sampler_usage image_usage, mcr_texture_image **p_resource) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_create_texture_resource", __FILE__, __LINE__, &midge_error_stack_index);

  *p_resource = NULL;

  midge_app_info *global_data;

  mc_obtain_midge_app_info(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;

  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_resource = (void *)p_resource;
  command->create_texture.width = width;
  command->create_texture.height = height;
  command->create_texture.image_usage = image_usage;

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


// Ensure this function is accessed within a thread mutex lock of the @resource_queue
int mcr_load_texture_resource(const char *path, mcr_texture_image **p_resource) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_load_texture_resource", __FILE__, __LINE__, &midge_error_stack_index);

  *p_resource = NULL;

  midge_app_info *global_data;

  mc_obtain_midge_app_info(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;

  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_TEXTURE;
  command->p_resource = (void *)p_resource;
  command->load_texture.path = path;  // strdup(path);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


// Ensure this function is accessed within a thread mutex lock of the @resource_queue
int mcr_obtain_font_resource(resource_queue *resource_queue, const char *font_path, float font_height, mcr_font_resource **p_resource) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_obtain_font_resource", __FILE__, __LINE__, &midge_error_stack_index);

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


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


int mcr_create_render_program(mcr_render_program_create_info *create_info, mcr_render_program **p_resource) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_create_render_program", __FILE__, __LINE__, &midge_error_stack_index);

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
  memcpy(rpci->buffer_bindings, create_info->buffer_bindings, sizeof(mcr_layout_binding) * create_info->buffer_binding_count);

  rpci->input_binding_count = create_info->input_binding_count;
  memcpy(rpci->input_bindings, create_info->input_bindings, sizeof(mcr_input_binding) * create_info->input_binding_count);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


int mcr_determine_text_display_dimensions(mcr_font_resource *font, const char *text, float *text_width, float *text_height) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_determine_text_display_dimensions", __FILE__, __LINE__, &midge_error_stack_index);

  if (text==NULL||text[0]=='\0') {
    *text_width = 0;
    *text_height = 0;

    {
      // Return
      register_midge_stack_return(midge_error_stack_index);
      return 0;
    }

  }


  midge_app_info *global_data;

  mc_obtain_midge_app_info(&global_data);

  if (!font) {
    font = global_data->ui_state->default_font_resource;
  }


  *text_width = 0;
  *text_height = font->draw_vertical_offset;

  int text_length = strlen(text);

  for (int  c = 0  ; c<text_length  ; ++c  ) {
    char letter = text[c];

    if (letter<32||letter>127) {
      MCerror(7874, "TODO character '%i' not supported.\n", letter);
    }


    // Source texture bounds
    stbtt_aligned_quad q;


    // printf("garbagein: %i %i %f %f %i\n", (int)font_image->width, (int)font_image->height, align_x, align_y, letter -
    // 32);

    stbtt_GetBakedQuad(font->char_data, 256, 256, letter - 32, text_width, text_height, &q, 1);
  }



  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
int mcr_issue_render_command_text(image_render_details *image_render_queue, unsigned int x, unsigned int y, const char *text, mcr_font_resource *font, render_color font_color) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_issue_render_command_text", __FILE__, __LINE__, &midge_error_stack_index);

  element_render_command *render_cmd;

  mcr_obtain_element_render_command(image_render_queue, &render_cmd);
  render_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  render_cmd->x = x;
  render_cmd->y = y;
  // printf("mcu_rtb-3 %p %u %u\n", render_cmd, render_cmd->x, render_cmd->y);

  // TODO -- make the render cmd a mc_str??
  render_cmd->print_text.text = strdup(text);

  if (font) {
    render_cmd->print_text.font = font;
  }
  else {
    midge_app_info *app_info;

    mc_obtain_midge_app_info(&app_info);

    render_cmd->print_text.font = app_info->ui_state->default_font_resource;
  }

  render_cmd->print_text.color = font_color;


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
int mcr_issue_render_command_colored_quad(image_render_details *image_render_queue, unsigned int x, unsigned int y, unsigned int width, unsigned int height, render_color color) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_issue_render_command_colored_quad", __FILE__, __LINE__, &midge_error_stack_index);

  if (!color.a) 
  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }


  element_render_command *render_cmd;

  mcr_obtain_element_render_command(image_render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_COLORED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->colored_rect_info.width = width;
  render_cmd->colored_rect_info.height = height;
  render_cmd->colored_rect_info.color = color;


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}


// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
int mcr_issue_render_command_textured_quad(image_render_details *image_render_queue, unsigned int x, unsigned int y, unsigned int width, unsigned int height, mcr_texture_image *texture_resource) {
  int midge_error_stack_index;
  register_midge_stack_function_entry("mcr_issue_render_command_textured_quad", __FILE__, __LINE__, &midge_error_stack_index);

  element_render_command *render_cmd;

  mcr_obtain_element_render_command(image_render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_TEXTURED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->textured_rect_info.width = width;
  render_cmd->textured_rect_info.height = height;
  render_cmd->textured_rect_info.texture = texture_resource;


  {
    // Return
    register_midge_stack_return(midge_error_stack_index);
    return 0;
  }

}