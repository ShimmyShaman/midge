#ifndef MC_UTIL_H
#define MC_UTIL_H

#include "core/core_definitions.h"

typedef struct mui_ui_state {
  bool requires_update;
  mc_node_list *cache_layered_hit_list;
  unsigned int default_font_resource;

  mc_node *global_context_menu;
} mui_ui_state;

typedef struct visual_project_data {
  const char *name;
  struct {
    unsigned int width, height;
  } screen;

  // App Stuff
  struct mui_ui_state *ui_state;
  bool requires_update, requires_rerender;
  unsigned int present_image_resource_uid;

  mc_node *editor_container;
  mc_node_list *children;

} visual_project_data;

// hierarchy.c
void exit_app(mc_node *hierarchical_call_scope, int result);
void mca_attach_node_to_hierarchy(mc_node *hierarchy_node, mc_node *node_to_attach);
void mca_init_mc_node(mc_node *hierarchy_node, node_type type, mc_node **node);
void mca_update_node_list(mc_node_list *node_list);
void mca_render_node_list_headless(mc_node_list *node_list);
void mca_render_node_list_present(image_render_queue *render_queue, mc_node_list *node_list);
void mca_set_node_requires_update(mc_node *node);
void mca_set_node_requires_rerender(mc_node *node);

// util.c
// @desired_allocation may be zero indicating the reallocate amount will be expanded by a 'reasonable' amount.
// @optional_item_allocation_size may be zero indicating no memory shall be assigned to the later allocation sizes.
int reallocate_collection(void ***collection, unsigned int *current_allocation, unsigned int desired_allocation,
                          size_t optional_item_allocation_size);

// project_management.c
void mca_create_new_visual_project(const char *project_name);
void mca_update_visual_project(mc_node *project_node);
void mca_render_project_headless(mc_node *visual_project);
void mca_render_project_present(image_render_queue *render_queue, mc_node *visual_project);

// global_context_menu.c
void mca_init_global_context_menu();
void mca_render_global_context_menu(image_render_queue *render_queue, mc_node *node);
void mca_activate_global_context_menu(mc_node *node, int screen_x, int screen_y);
void mca_hide_global_context_menu();
#endif // MC_UTIL_H