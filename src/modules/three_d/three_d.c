#include "cglm/cglm.h"
#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"

typedef struct mctd_portal {
  mc_node *node;

  struct {
    unsigned int width, height;
    unsigned int resource_uid;
  } render_target;
} mctd_portal;

void __mctd_determine_td_portal_extents(mc_node *node, layout_extent_restraints restraints)
{
  mctd_portal *td_portal = (mctd_portal *)node->data;

  // Determine
  if (node->layout->preferred_width) {
    node->layout->determined_extents.width = node->layout->preferred_width;
  }
  else {
    MCerror(7525, "NotYetSupported");
  }
  if (node->layout->preferred_height) {
    node->layout->determined_extents.height = node->layout->preferred_height;
  }
  else {
    MCerror(8932, "NotYetSupported");
  }
}

void __mctd_update_td_portal_layout(mc_node *node, mc_rectf *available_area)
{
  mctd_portal *td_portal = (mctd_portal *)node->data;

  mca_update_typical_node_layout(node, available_area);
}

void __mctd_render_td_portal_headless(mc_node *node)
{
  mctd_portal *td_portal = (mctd_portal *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }

  // Render the render target
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  image_render_request *irq;
  obtain_image_render_request(&global_data->render_thread->render_queue, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = COLOR_CORNFLOWER_BLUE;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = td_portal->render_target.width;   // TODO
  irq->image_height = td_portal->render_target.height; // TODO
  irq->data.target_image.image_uid = td_portal->render_target.resource_uid;
  irq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  irq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_present)(image_render_request *, mc_node *) =
          (void (*)(image_render_request *, mc_node *))child->layout->render_present;
      render_node_present(irq, child);
    }
  }
}

void __mctd_render_td_portal_present(image_render_request *render_queue, mc_node *node)
{
  mctd_portal *td_portal = (mctd_portal *)node->data;

  mcr_issue_render_command_textured_quad(render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, td_portal->render_target.width,
                                         td_portal->render_target.height, td_portal->render_target.resource_uid);

  mca_set_node_requires_rerender(node);
}

typedef struct cube_child {
  mat4 model;
  unsigned int mesh_resource_uid;
} cube_child;

void mctd_render_cube_present(image_render_request *render_queue, mc_node *node)
{
  element_render_command *render_cmd;
  obtain_element_render_command(render_queue, &render_cmd);

  render_cmd->type = RENDER_COMMAND_CUBE;
  // render_cmd->x = 800;
  // render_cmd->y = 600;
  // // render_cmd->colored_rect_info.width = width;
  // // render_cmd->colored_rect_info.height = height;
  // // render_cmd->colored_rect_info.color = color;
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  cube_child *cube = (cube_child *)node->data;

  mat4 vpc;
  // mat4 model;
  glm_mat4_identity((vec4 *)&cube->model);
  vec3 axis = {0.f, -1.f, 0.f};
  // float rotate = global_data->elapsed->app_secsf - (((int)global_data->elapsed->app_secsf / 90) * 90);
  // glm_rotate((vec4 *)&cube->model, rotate, axis);
  // glm_translate((vec4 *)cube->model)
  {
    // Construct the Vulkan View/Projection/Clip for the render target image
    mat4 view;
    mat4 proj;
    mat4 clip = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};

    glm_lookat((vec3){0, 0, -10}, (vec3){0, 0, 0}, (vec3){0, -1, 0}, (vec4 *)view);
    float fovy = 72.f / 180.f * 3.1459f;
    glm_perspective(fovy, (float)render_queue->image_width / render_queue->image_height, 0.01f, 1000.f, (vec4 *)&proj);
    // glm_ortho_default((float)sequence->image_width / sequence->image_height, (vec4 *)&proj);
    // glm_mat4_mul((vec4 *)&proj, (vec4 *)vpc, (vec4 *)vpc);
    glm_mat4_mul((vec4 *)&proj, (vec4 *)vpc, (vec4 *)vpc);
    glm_mat4_mul((vec4 *)&clip, (vec4 *)vpc, (vec4 *)vpc);
  }

  render_cmd->mesh.world_matrix = (float *)cube->model;

  // mcr_issue_render_command_textured_mesh(cube->mesh_resouce_uid, cube->texture_resource_uid, vpc)

  // render_cmd
}

void mctd_append_cube_child(mc_node *portal_node)
{
  // Node
  mc_node *node;
  mca_init_mc_node(portal_node, NODE_TYPE_3D_PORTAL, &node);

  mca_init_node_layout(&node->layout);

  // vec3 mesh_data[] = {{0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 1.f},
  //                     {1.f, 0.f, 0.f}, {1.f, 0.f, 1.f}, {1.f, 1.f, 0.f}, {1.f, 1.f, 1.f}};
  // unsigned int indices[] = {
  //     0, 1, 2, 2, 3, 1, 4, 5, 6, 6, 7, 5, 0, 1, 4, 4, 5, 1, 2, 3, 6, 6, 7, 3, 0, 2, 4, 4, 6, 2, 1, 3, 5, 5, 7, 3,
  // };

  cube_child *cube = (cube_child *)malloc(sizeof(cube_child));
  node->data = cube;
  // cube->render_program_uid = 0; // TODO -- share render program resource somehow
  // cube->mesh_resource_uid = 0;
  // cube->texture_resource_uid = 0;
  // cube->model_matrix = (float *)Identity;
  // mcr_create_render_program((float *)mesh_data, 3 * 8, indices, 6 * 2 * 3, &cube->mesh_resource_uid);
  // mcr_load_texture_resource((float *)mesh_data, ... , &cube->texture_resource_uid);
  // mcr_create_mesh_resource((float *)mesh_data, 3 * 8, indices, 6 * 2 * 3, &cube->mesh_resource_uid);

  node->layout->render_present = &mctd_render_cube_present;
}

void init_three_d_portal()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Node
  mc_node *node;
  mca_init_mc_node(global_data->global_node, NODE_TYPE_3D_PORTAL, &node);

  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  mca_init_node_layout(&node->layout);
  node->layout->preferred_width = 900;
  node->layout->preferred_height = 600;

  node->layout->padding.left = 400;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&__mctd_determine_td_portal_extents;
  node->layout->update_layout = (void *)&__mctd_update_td_portal_layout;
  node->layout->render_headless = (void *)&__mctd_render_td_portal_headless;
  node->layout->render_present = (void *)&__mctd_render_td_portal_present;

  mca_set_node_requires_layout_update(node);

  // Portal
  mctd_portal *portal = (mctd_portal *)malloc(sizeof(mctd_portal));
  node->data = portal;
  portal->node = node;

  portal->render_target.resource_uid = 0;
  portal->render_target.width = node->layout->preferred_width;
  portal->render_target.height = node->layout->preferred_height;
  mcr_create_texture_resource(portal->render_target.width, portal->render_target.height, true,
                              &portal->render_target.resource_uid);

  // Content
  mctd_append_cube_child(node);
}