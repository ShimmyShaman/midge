#include "core/core_definitions.h"
#include "env/state_definitions.h"
#include "render/render_common.h"
#include "ui/ui_definitions.h"

void mca_init_visual_project_container(mc_node *project_node)
{
  visual_project_data *project = (visual_project_data *)project_node->data;

  if (project->children->count > 0) {
    MCerror(1158, "Invalid State: must initialize container with zero children");
  }

  // Panel
  mui_panel *panel;
  mui_init_panel(project_node, &panel);
  panel->background_color = COLOR_LIGHT_SKY_BLUE;
  panel->element->bounds = {400, 200, (float)project->screen.width, (float)project->screen.height};

  // Set container to seperate field and clear children
  project->editor_container = project->children->items[0];
  project->children->count = 0;
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

  project->present_image_resource_uid = 0;
  mui_initialize_ui_state(&project->ui_state);

  // Resources
  pthread_mutex_lock(&global_data->render_thread->resource_queue.mutex);

  mcr_create_texture_resource(&global_data->render_thread->resource_queue, (unsigned int)project->screen.width,
                              (unsigned int)project->screen.height, true, &project->present_image_resource_uid);

  pthread_mutex_unlock(&global_data->render_thread->resource_queue.mutex);

  // Initialize the project visual container (what will be replaced by the OS window)
  mca_init_visual_project_container(project_node);

  // Ensure initial update is triggered
  mca_set_node_requires_update(project_node);

  printf("visual app created\n");
}

void mca_update_visual_project(mc_node *project_node)
{
  visual_project_data *project = (visual_project_data *)project_node->data;

  {
    // TEMPORARY -- FIX When UI UPDATE is fixed
    mui_ui_element *element = (mui_ui_element *)project->editor_container->data;
    if (element->requires_update) {
      element->requires_update = false;

      // Trigger rerender
      mca_set_node_requires_rerender(element->visual_node);
    }
  }

  mca_update_node_list(project->children);

  // Projects always require updating
  project->requires_update = true;
}

void mca_render_project_headless(mc_node *project_node)
{
  visual_project_data *project = (visual_project_data *)project_node->data;

  mca_render_node_list_headless(project->children);
}

void mca_render_project_present(image_render_queue *render_queue, mc_node *project_node)
{
  visual_project_data *project = (visual_project_data *)project_node->data;

  mui_render_panel(render_queue, project->editor_container);

  mca_render_node_list_present(render_queue, project->children);
}