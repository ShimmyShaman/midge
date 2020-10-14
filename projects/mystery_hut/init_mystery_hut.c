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
  } cube;
} mystery_hut;

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

  mcr_render_model(irq, mh_data->cube.model);

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

  // printf("ct_data=%p\n", ct_data);
  // printf("&ct_data->cube.model=%p\n", &ct_data->cube.model);
  // printf("&ct_data->cube.model=%p\n", &(ct_data->cube.model));

  // mcr_load_wavefront_obj_model("res/cube/cube.obj", "res/cube/cube_diffuse.png", &ct_data->cube.model);
  mcr_load_wavefront_obj_model("res/models/viking_room.obj", "res/models/viking_room.png", &ct_data->cube.model);
}

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

  mca_set_node_requires_layout_update(node);

  myh_load_resources(node);
}

void set_mystery_hut_project_state(mc_node *app_root)
{
  function_info *func_info;
  // find_function_info("init_mystery_hut", &func_info);
  // mce_activate_source_editor_for_definition(func_info->source);
  find_function_info("_myh_handle_input", &func_info);
  mce_activate_source_editor_for_definition(func_info->source);
}