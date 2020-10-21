#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "modules/source_editor/source_editor.h"
#include "render/render_common.h"

typedef struct mystery_hut {
  mc_node *node;

  struct {
    unsigned int width, height;
    unsigned int resource_uid;
  } render_target;

  struct {
    mat4 world;
    mcr_model *model;

    unsigned int render_program_resource_uid;
  } cube;
} mystery_hut;

// void create_wvp_matrix(mat4 *)
// {
//   // Matrix
//   mat4 *vpc = (mat4 *)malloc(sizeof(mat4));

//   // Construct the Vulkan View/Projection/Clip for the render target image
//   mat4 view;
//   mat4 proj;
//   mat4 clip = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};

//   global_root_data *global_data;
//   obtain_midge_global_root(&global_data);

//   glm_lookat((vec3){0, -4, -4}, (vec3){0, 0, 0}, (vec3){0, -1, 0}, (vec4 *)vpc);
//   float fovy = 72.f / 180.f * 3.1459f;
//   glm_perspective(fovy, (float)image_render->image_width / image_render->image_height, 0.01f, 1000.f, (vec4 *)&proj);
//   // glm_ortho_default((float)image_render->image_width / image_render->image_height, (vec4 *)&proj);

//   // if (((int)global_data->elapsed->app_secsf) % 2 == 1) {
//   mat4 world;
//   glm_mat4_identity((vec4 *)&world);
//   vec3 axis = {0.f, 1.f, 0.f};
//   glm_rotate((vec4 *)&world, 180.f, axis);
//   axis[1] = 0.f;
//   axis[2] = 1.f;
//   glm_rotate((vec4 *)&world, 180.f, axis);
//   axis[0] = 1.f;
//   axis[2] = 0.f;
//   glm_rotate((vec4 *)&world, -90.f, axis);
//   glm_mat4_mul((vec4 *)vpc, (vec4 *)&world, (vec4 *)vpc);
//   // }
//   // else {
//   // glm_mat4_mul((vec4 *)cmd->mesh.world_matrix, (vec4 *)vpc, (vec4 *)vpc);
//   // }
//   // if (((int)global_data->elapsed->app_secsf / 2) % 2 == 1) {
//   //   glm_mat4_mul((vec4 *)&clip, (vec4 *)proj, (vec4 *)proj);
//   // }
//   // else {
//   glm_mat4_mul((vec4 *)proj, (vec4 *)&clip, (vec4 *)proj);
//   // }
//   // if (((int)global_data->elapsed->app_secsf / 2) % 2 == 1) {
//   glm_mat4_mul((vec4 *)&proj, (vec4 *)vpc, (vec4 *)vpc);
//   // }
//   // else {
//   // glm_mat4_mul((vec4 *)vpc, (vec4 *)&proj, (vec4 *)vpc);
//   // }

//   // glm_mat4_mul((vec4 *)vpc, (vec4 *)cmd->mesh.world_matrix, (vec4 *)vpc);
//   // glm_mat4_mul((vec4 *)&proj, (vec4 *)vpc, (vec4 *)vpc);
//   // glm_mat4_mul((vec4 *)&clip, (vec4 *)vpc, (vec4 *)vpc);

//   // printf("(&copy_buffer->vpc_desc_buffer_info)[0].offset=%lu\n", (&copy_buffer.vpc_desc_buffer_info)[0].offset);
// }

void _myh_render_mh_data_headless(mc_node *node)
{
  mystery_hut *mh_data = (mystery_hut *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }

  // Render the render target
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  image_render_details *irq;
  mcr_obtain_image_render_request(global_data->render_thread, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = COLOR_CORNFLOWER_BLUE;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = mh_data->render_target.width;   // TODO
  irq->image_height = mh_data->render_target.height; // TODO
  irq->data.target_image.image_uid = mh_data->render_target.resource_uid;
  irq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  irq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_present)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_present(irq, child);
    }
  }

  // mcr_render_model(irq, mh_data->cube.model);
  element_render_command *render_cmd;
  mcr_obtain_element_render_command(irq, &render_cmd);

  render_cmd->type = RENDER_COMMAND_PROGRAM;
  render_cmd->render_program.program_uid = mh_data->cube.render_program_resource_uid;

  // render_cmd->render_program
  //     .

  render_cmd->render_program.vertex_buffer = mh_data->cube.model->vertex_buffer;
  render_cmd->render_program.index_buffer = mh_data->cube.model->index_buffer;
  render_cmd->render_program.texture_uid = mh_data->cube.model->texture;

  // mat4 world;
  // mat4 view;
  // mat4 projection;
  // vertex_buffer (pos/uv/normals*)
  // index_buffer
  // texture
  // mcr_render_with_program(irq, )

  mcr_submit_image_render_request(global_data->render_thread, irq);
}

void _myh_render_mh_data_present(image_render_details *image_render_queue, mc_node *node)
{
  mystery_hut *mh_data = (mystery_hut *)node->data;

  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, mh_data->render_target.width,
                                         mh_data->render_target.height, mh_data->render_target.resource_uid);

  // mca_set_node_requires_rerender(node);
}

void _myh_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_myh_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }

  // TODO -- asdf
}

void myh_load_resources(mc_node *module_node)
{
  // cube_template
  mystery_hut *ct_data = (mystery_hut *)malloc(sizeof(mystery_hut));
  module_node->data = ct_data;
  ct_data->node = module_node;

  ct_data->render_target.resource_uid = 0;
  ct_data->render_target.width = module_node->layout->preferred_width;
  ct_data->render_target.height = module_node->layout->preferred_height;
  mcr_create_texture_resource(ct_data->render_target.width, ct_data->render_target.height,
                              MVK_IMAGE_USAGE_RENDER_TARGET_3D, &ct_data->render_target.resource_uid);

  // Render Program
  ct_data->cube.render_program_resource_uid = 0U;
  mcr_render_program_create_info create_info = {};
  create_info.vertex_shader_filepath = (char *)"projects/mystery_hut/model.vert";
  create_info.fragment_shader_filepath = (char *)"projects/mystery_hut/model.frag";

  mcr_create_render_program(&create_info, &ct_data->cube.render_program_resource_uid);

  // Cube Model
  // mcr_load_wavefront_obj_model("res/cube/cube.obj", "res/cube/cube_diffuse.png", &ct_data->cube.model);
  mcr_load_wavefront_obj_model("res/models/viking_room.obj", "res/models/debug_texture.png", &ct_data->cube.model);
}

void _myh_update(frame_time *elapsed, mci_input_state *input_state, void *state) {}

void init_mystery_hut(mc_node *app_root)
{
  //   printf("instantiate file:'%s'\n", str->text);
  //   instantiate_all_definitions_from_file(global_data->global_node, str->text, NULL);

  //   module_node->
  mc_node *node;
  mca_init_mc_node(app_root, NODE_TYPE_ABSTRACT, &node);
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 900;
  node->layout->preferred_height = 600;

  node->layout->padding.left = 400;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_myh_render_mh_data_headless;
  node->layout->render_present = (void *)&_myh_render_mh_data_present;
  node->layout->handle_input_event = (void *)&_myh_handle_input;

  myh_load_resources(node);

  void *update_delegate = (void *)&_myh_update;
  // mca_register_loop_update(app_root, update_delegate, node->data);

  mca_set_node_requires_layout_update(node);
}

void set_mystery_hut_project_state(mc_node *app_root)
{
  function_info *func_info;
  // find_function_info("init_mystery_hut", &func_info);
  // mce_activate_source_editor_for_definition(func_info->source);
  find_function_info("_myh_handle_input", &func_info);
  mce_activate_source_editor_for_definition(func_info->source);

  find_function_info("_myh_handle_input", &func_info);
  // find_function_info("mce_delete_selection", &func_info);
  mce_activate_function_debugging(func_info);
}