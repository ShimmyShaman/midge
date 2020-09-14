#include "core/core_definitions.h"
#include "env/state_definitions.h"
#include "render/render_common.h"
#include "ui/ui_definitions.h"

void mca_init_visual_project_container(mc_node *project_node)
{
  visual_project_data *project = (visual_project_data *)project_node->data;

  // Panel
  mui_panel *panel;
  mui_init_panel(project_node, &panel);
  panel->background_color = COLOR_LIGHT_SKY_BLUE;
  panel->element->bounds = {400, 200, (float)project->screen.width, (float)project->screen.height};

  mui_set_element_update(panel->element);
}

void mca_create_new_visual_project(const char *project_name)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mc_node *project_node;
  mca_init_mc_node(global_data->global_node, NODE_TYPE_VISUAL_PROJECT, &project_node);

  visual_project_data *project = (visual_project_data *)malloc(sizeof(visual_project_data));
  project_node->data = project;

  project->name = strdup(project_name);

  project->screen.width = 600;
  project->screen.height = 480;

  project->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  project->children->alloc = 0;
  project->children->count = 0;

  mca_set_node_requires_update(project_node);
  mca_set_node_requires_rerender(project_node);
  project->present_image_resource_uid = 0;
  mui_initialize_ui_state(&project->ui_state);

  // Resources
  pthread_mutex_lock(&global_data->render_thread->resource_queue.mutex);

  mcr_create_texture_resource(&global_data->render_thread->resource_queue, (unsigned int)project->screen.width,
                              (unsigned int)project->screen.height, true, &project->present_image_resource_uid);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue.mutex);

  // Initialize the UI
  mca_init_visual_project_container(project_node);

  printf("visual app created\n");
}

void mca_update_visual_project(mc_node *project_node)
{
  visual_project_data *project = (visual_project_data *)project_node->data;

  mca_update_node_list(project->children);

  // Projects always require updating
  project->requires_update = true;
}