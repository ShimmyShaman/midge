#include "modules/obj_loader/obj_loader.h"

typedef struct mcr_model {
  unsigned int vertex_buffer_uid;
  float *vertex_data;
  unsigned int index_buffer_uid;
  unsigned int *index_data;
} mcr_model;

void mcr_load_wavefront_obj_model(const char *obj_path, mcr_model **loaded_model)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  tinyobj_obj *parsed_obj;

  {
    int ret = tinyobj_parse_obj(obj_path, &parsed_obj);
    if (ret) {
      MCerror(9319, "Failed to load obj model");
    }
  }

  mcr_model *model = (mcr_model *)malloc(sizeof(mcr_model));
  model->vertex_buffer_uid = 0;

  //   float *vertex_data =

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

  *loaded_model = model;
  //   printf("loaded .obj:%p (%u)\n", command->load_mesh.p_data, command->load_mesh.data_count);

  // for (int i = 0; i < parsed_obj->attrib.num_faces)

  //   mcr_obtain_resource_command(resource_queue, &command);
  // command->type = RESOURCE_COMMAND_MESH;
  // command->p_uid = &model->mesh_resource_uid;
  // command->data.load_mesh.p_vertex_data = parsed_obj->attrib.vertices;
  // command->data.load_mesh.vertex_count = parsed_obj->attrib.vertex_count;
  // parsed_obj->attrib.vertices =

  // tinyobj_attrib_t attrib;
  // tinyobj_shape_t *shapes = NULL;
  // size_t num_shapes;
  // tinyobj_material_t *materials = NULL;
  // size_t num_materials;

  // {
  //   unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
  //   int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
  //                               &num_materials, filename, get_file_data, flags);
  //   if (ret != TINYOBJ_SUCCESS) {
  //     return 0;
  //   }
}