#include "render/render_common.h"
#include "core/core_definitions.h"
#include "render/mc_vulkan.h"
// #include "render/resources/tiny_obj_loader_c.h"

#include "stb_truetype.h"

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
  int res = append_to_collection((void ***)&render_thread->image_queue->items, &render_thread->image_queue->alloc,
                                 &render_thread->image_queue->count, request);

  // printf("sirr-1\n");
  // printf("sirr %u\n", render_thread->image_queue->count);
  pthread_mutex_unlock(&render_thread->image_queue->mutex);

  return res;
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
void mcr_create_texture_resource(unsigned int width, unsigned int height, mvk_image_sampler_usage image_usage,
                                 unsigned int *p_resource_uid)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;
  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_CREATE_TEXTURE;
  command->p_uid = p_resource_uid;
  command->create_texture.width = width;
  command->create_texture.height = height;
  command->create_texture.image_usage = image_usage;

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);
}

// Ensure this function is accessed within a thread mutex lock of the @resource_queue
void mcr_load_texture_resource(const char *path, unsigned int *p_resource_uid)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

  resource_command *command;
  mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_TEXTURE;
  command->p_uid = p_resource_uid;
  command->load_texture.path = path; // strdup(path);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);
}

// Ensure this function is accessed within a thread mutex lock of the @resource_queue
void mcr_obtain_font_resource(resource_queue *resource_queue, const char *font_path, float font_height,
                              font_resource **p_resource)
{
  pthread_mutex_lock(&resource_queue->mutex);
  // printf("mcr_obtain_font_resource-font_height:%f\n", font_height);
  resource_command *command;
  mcr_obtain_resource_command(resource_queue, &command);
  command->type = RESOURCE_COMMAND_LOAD_FONT;
  command->p_uid = (unsigned int *)p_resource;
  command->font.height = font_height;
  command->font.path = font_path;
  // printf("hrc-resource_cmd->font.height:%f\n", command->font.height);
  pthread_mutex_unlock(&resource_queue->mutex);
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
  font_resource *font = NULL;
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

// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
void mcr_issue_render_command_text(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                   const char *text, unsigned int font_resource_uid, render_color font_color)
{
  element_render_command *render_cmd;
  mcr_obtain_element_render_command(image_render_queue, &render_cmd);
  render_cmd->type = RENDER_COMMAND_PRINT_TEXT;
  render_cmd->x = x;
  render_cmd->y = y;
  // printf("mui_rtb-3 %u %u\n", render_cmd->x, render_cmd->y);

  // TODO -- make the render cmd a c_str??
  render_cmd->print_text.text = strdup(text);

  if (font_resource_uid) {
    render_cmd->print_text.font_resource_uid = font_resource_uid;
  }
  else {
    global_root_data *global_data;
    obtain_midge_global_root(&global_data);

    render_cmd->print_text.font_resource_uid = global_data->ui_state->default_font_resource;
    // printf("set defaultfont %u\n", render_cmd->print_text.font_resource_uid);
  }
  render_cmd->print_text.color = font_color;
}

// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
void mcr_issue_render_command_colored_quad(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                           unsigned int width, unsigned int height, render_color color)
{
  if (!color.a)
    return;

  element_render_command *render_cmd;
  mcr_obtain_element_render_command(image_render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_COLORED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->colored_rect_info.width = width;
  render_cmd->colored_rect_info.height = height;
  render_cmd->colored_rect_info.color = color;
}

// Ensure this function is accessed within a thread mutex lock of the @image_render_queue
void mcr_issue_render_command_textured_quad(image_render_details *image_render_queue, unsigned int x, unsigned int y,
                                            unsigned int width, unsigned int height, unsigned int texture_resource)
{
  element_render_command *render_cmd;
  mcr_obtain_element_render_command(image_render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_TEXTURED_QUAD;
  render_cmd->x = x;
  render_cmd->y = y;
  render_cmd->textured_rect_info.width = width;
  render_cmd->textured_rect_info.height = height;
  render_cmd->textured_rect_info.texture_uid = texture_resource;
}

// void mcr_load_wavefront_obj_model(const char *obj_path, mcr_model **loaded_model)
// {
//   global_root_data *global_data;
//   obtain_midge_global_root(&global_data);

//   tinyobj_obj *parsed_obj;

//   {
//     int ret = tinyobj_parse_obj(obj_path, &parsed_obj);
//     if (ret) {
//       MCerror(9319, "Failed to load obj model");
//     }
//   }

//   mcr_model *model = (mcr_model *)malloc(sizeof(mcr_model));
//   model->parsed_obj = parsed_obj;
//   model->mesh_resource_uid = 0;

//   pthread_mutex_lock(&global_data->render_thread->resource_queue->mutex);

//   resource_command *command;
//   mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
//   command->type = RESOURCE_COMMAND_LOAD_MESH;
//   command->p_uid = &model->mesh_resource_uid;
//   command->load_mesh.p_data = parsed_obj->attrib.vertices;
//   command->load_mesh.data_count = parsed_obj->attrib.num_vertices * 3;

//   // mcr_obtain_resource_command(global_data->render_thread->resource_queue, &command);
//   // command->type = RESOURCE_COMMAND_LOAD_INDEX_BUFFER;
//   // command->p_uid = &model->mesh_resource_uid;
//   // command->load_mesh.p_data = parsed_obj->attrib.faces;
//   // command->load_mesh.data_count = parsed_obj->attrib.num_vertices * 3;

//   pthread_mutex_unlock(&global_data->render_thread->resource_queue->mutex);

//   *loaded_model = model;
//   printf("load_mesh:%p (%u)\n", command->load_mesh.p_data, command->load_mesh.data_count);

//   // for (int i = 0; i < parsed_obj->attrib.num_faces)

//   //   mcr_obtain_resource_command(resource_queue, &command);
//   // command->type = RESOURCE_COMMAND_MESH;
//   // command->p_uid = &model->mesh_resource_uid;
//   // command->data.load_mesh.p_vertex_data = parsed_obj->attrib.vertices;
//   // command->data.load_mesh.vertex_count = parsed_obj->attrib.vertex_count;
//   // parsed_obj->attrib.vertices =

//   // tinyobj_attrib_t attrib;
//   // tinyobj_shape_t *shapes = NULL;
//   // size_t num_shapes;
//   // tinyobj_material_t *materials = NULL;
//   // size_t num_materials;

//   // {
//   //   unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
//   //   int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
//   //                               &num_materials, filename, get_file_data, flags);
//   //   if (ret != TINYOBJ_SUCCESS) {
//   //     return 0;
//   //   }
// }