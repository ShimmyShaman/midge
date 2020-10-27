#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "modules/source_editor/source_editor.h"
#include "render/render_common.h"

typedef struct mystery_hut {
  mc_node *node;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

  bool rerender_toggle;
  struct {
    float rotX, rotY, rotZ;
    mcr_render_program *render_program;
    mcr_render_program_data render_data;
  } cube;
} mystery_hut;

void create_wvp_matrix(mystery_hut *mh_data, mat4 **out_wvp)
{
  // Matrix
  mat4 *vpc = (mat4 *)malloc(sizeof(mat4));
  *out_wvp = vpc;

  // Construct the Vulkan View/Projection/Clip for the render target image
  mat4 view;
  mat4 proj;
  mat4 clip = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};

  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  glm_lookat((vec3){0, -4, -4}, (vec3){0, 0, 0}, (vec3){0, -1, 0}, (vec4 *)vpc);
  float fovy = 72.f / 180.f * 3.1459f;
  glm_perspective(fovy, (float)APPLICATION_SET_WIDTH / APPLICATION_SET_HEIGHT, 0.01f, 1000.f, (vec4 *)&proj);
  // glm_ortho_default((float)image_render->image_width / image_render->image_height, (vec4 *)&proj);

  // if (((int)global_data->elapsed->app_secsf) % 2 == 1) {
  mat4 world;
  glm_mat4_identity((vec4 *)&world);
  vec3 axis = {0.f, 1.f, 0.f};
  glm_rotate((vec4 *)&world, mh_data->cube.rotY / 180.f * 3.1459f, axis);
  axis[1] = 0.f;
  axis[2] = 1.f;
  glm_rotate((vec4 *)&world, mh_data->cube.rotZ / 180.f * 3.1459f, axis);
  axis[0] = 1.f;
  axis[2] = 0.f;
  glm_rotate((vec4 *)&world, mh_data->cube.rotX / 180.f * 3.1459f, axis);
  glm_mat4_mul((vec4 *)vpc, (vec4 *)&world, (vec4 *)vpc);
  // }
  // else {
  // glm_mat4_mul((vec4 *)cmd->mesh.world_matrix, (vec4 *)vpc, (vec4 *)vpc);
  // }
  // if (((int)global_data->elapsed->app_secsf / 2) % 2 == 1) {
  //   glm_mat4_mul((vec4 *)&clip, (vec4 *)proj, (vec4 *)proj);
  // }
  // else {
  glm_mat4_mul((vec4 *)proj, (vec4 *)&clip, (vec4 *)proj);
  // }
  // if (((int)global_data->elapsed->app_secsf / 2) % 2 == 1) {
  glm_mat4_mul((vec4 *)&proj, (vec4 *)vpc, (vec4 *)vpc);
  // }
  // else {
  // glm_mat4_mul((vec4 *)vpc, (vec4 *)&proj, (vec4 *)vpc);
  // }

  // glm_mat4_mul((vec4 *)vpc, (vec4 *)cmd->mesh.world_matrix, (vec4 *)vpc);
  // glm_mat4_mul((vec4 *)&proj, (vec4 *)vpc, (vec4 *)vpc);
  // glm_mat4_mul((vec4 *)&clip, (vec4 *)vpc, (vec4 *)vpc);

  // printf("(&copy_buffer->vpc_desc_buffer_info)[0].offset=%lu\n", (&copy_buffer.vpc_desc_buffer_info)[0].offset);
}

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
  irq->data.target_image.image = mh_data->render_target.image;
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
  render_cmd->render_program.program = mh_data->cube.render_program;
  render_cmd->render_program.data = &mh_data->cube.render_data;

  // render_cmd->render_program
  //     .

  // render_cmd->render_program.vertex_buffer = mh_data->cube.model->vertex_buffer;
  // render_cmd->render_program.index_buffer = mh_data->cube.model->index_buffer;
  // render_cmd->render_program.texture_uid = mh_data->cube.model->texture;

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
                                         mh_data->render_target.height, mh_data->render_target.image);

  if (mh_data->rerender_toggle)
    mca_set_node_requires_rerender(node);
}

void _myh_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_myh_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    mca_focus_node(node);
  }

  // TODO -- asdf
  if (input_event->type == INPUT_EVENT_KEY_PRESS) {
    mystery_hut *mh_data = (mystery_hut *)node->data;
    bool recalc = true;
    switch (input_event->button_code) {
    case KEY_CODE_D: {
      mh_data->rerender_toggle = mh_data->rerender_toggle ? false : true;
      mh_data->cube.rotX += 5.f;
      if (mh_data->cube.rotX > 180.f) {
        mh_data->cube.rotX -= 360.f;
      }
      printf("D:%.3f\n", mh_data->cube.rotX);
    } break;
    case KEY_CODE_A: {
      mh_data->cube.rotX -= 5.f;
      if (mh_data->cube.rotX < -180.f) {
        mh_data->cube.rotX += 360.f;
      }
      printf("A:%.3f\n", mh_data->cube.rotX);
    } break;
    case KEY_CODE_W: {
      mh_data->cube.rotY += 5.f;
      if (mh_data->cube.rotY > 180.f)
        mh_data->cube.rotY -= 360.f;
      printf("W:%.3f\n", mh_data->cube.rotY);
    } break;
    case KEY_CODE_S: {
      mh_data->cube.rotY -= 5.f;
      if (mh_data->cube.rotY < -180.f)
        mh_data->cube.rotY += 360.f;
      printf("S:%.3f\n", mh_data->cube.rotY);
    } break;
    case KEY_CODE_E: {
      mh_data->cube.rotZ += 5.f;
      if (mh_data->cube.rotZ > 180.f)
        mh_data->cube.rotZ -= 360.f;
      printf("E:%.3f\n", mh_data->cube.rotZ);
    } break;
    case KEY_CODE_Q: {
      mh_data->cube.rotZ -= 5.f;
      if (mh_data->cube.rotZ < -180.f)
        mh_data->cube.rotZ += 360.f;
      printf("Q:%.3f\n", mh_data->cube.rotZ);
    } break;

    default:
      recalc = false;
      break;
    }

    if (recalc) {
      create_wvp_matrix(mh_data, (mat4 **)&mh_data->cube.render_data.input_buffers[0]);
      mca_set_node_requires_rerender(node);
    }
  }
}

void myh_load_resources(mc_node *module_node)
{
  // cube_template
  mystery_hut *mh_data = (mystery_hut *)malloc(sizeof(mystery_hut));
  module_node->data = mh_data;
  mh_data->node = module_node;

  mh_data->render_target.width = module_node->layout->preferred_width;
  mh_data->render_target.height = module_node->layout->preferred_height;
  mcr_create_texture_resource(mh_data->render_target.width, mh_data->render_target.height,
                              MVK_IMAGE_USAGE_RENDER_TARGET_3D, &mh_data->render_target.image);

  // Render Program
  mcr_render_program_create_info create_info = {};
  create_info.vertex_shader_filepath = (char *)"projects/mystery_hut/model.vert";
  create_info.fragment_shader_filepath = (char *)"projects/mystery_hut/model.frag";
  create_info.buffer_binding_count = 2;
  create_info.buffer_bindings[0] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, sizeof(mat4)};
  create_info.buffer_bindings[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0U};
  create_info.input_binding_count = 2;
  create_info.input_bindings[0] = {VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3};
  create_info.input_bindings[1] = {VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2};
  mcr_create_render_program(&create_info, &mh_data->cube.render_program);

  // Render Data
  mh_data->cube.render_data.input_buffers = (void **)malloc(sizeof(void *) * 2);

  // world-view-projection
  mh_data->rerender_toggle = false;
  mh_data->cube.rotX = mh_data->cube.rotY = mh_data->cube.rotZ = 0;
  create_wvp_matrix(mh_data, (mat4 **)&mh_data->cube.render_data.input_buffers[0]);

  // Cube Model
  // mcr_load_wavefront_obj_model("res/cube/cube.obj", "res/cube/cube_diffuse.png", &mh_data->cube.model);
  // mcr_load_wavefront_obj("res/models/viking_room.obj", );

  // mcr_load_wavefront_obj("res/cube/cube.obj", &mh_data->cube.render_data.vertices,
  // &mh_data->cube.render_data.indices);
  mcr_load_wavefront_obj("res/models/viking_room.obj", &mh_data->cube.render_data.vertices,
                         &mh_data->cube.render_data.indices);
  mcr_load_texture_resource("res/models/viking_room.png",
                            (mcr_texture_image *)&mh_data->cube.render_data.input_buffers[1]);
}

void _myh_update(frame_time *elapsed, mci_input_state *input_state, void *state) {}

void init_mystery_hut(mc_node *app_root)
{
  //   printf("instantiate file:'%s'\n", str->text);
  //   instantiate_all_definitions_from_file(global_data->global_node, str->text, NULL);

  //   module_node->
  mc_node *node;
  mca_init_mc_node(app_root, NODE_TYPE_ABSTRACT, &node);
  node->name = "mystery_hut";

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