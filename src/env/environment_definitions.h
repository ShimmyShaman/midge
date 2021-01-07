#ifndef MC_UTIL_H
#define MC_UTIL_H

#include "core/core_definitions.h"
#include "render/render_common.h"

typedef enum layout_extent_restraints {
  LAYOUT_RESTRAINT_NONE = 0,
  LAYOUT_RESTRAINT_HORIZONTAL = 1 << 0,
  LAYOUT_RESTRAINT_VERTICAL = 1 << 1,
} layout_extent_restraints;

typedef enum horizontal_alignment_type {
  HORIZONTAL_ALIGNMENT_NULL = 0,
  HORIZONTAL_ALIGNMENT_LEFT,
  HORIZONTAL_ALIGNMENT_CENTRED,
  HORIZONTAL_ALIGNMENT_RIGHT,
} horizontal_alignment_type;

typedef enum vertical_alignment_type {
  VERTICAL_ALIGNMENT_NULL = 0,
  VERTICAL_ALIGNMENT_TOP,
  VERTICAL_ALIGNMENT_CENTRED,
  VERTICAL_ALIGNMENT_BOTTOM,
} vertical_alignment_type;

typedef struct mc_paddingf {
  float left, top, right, bottom;
} mc_paddingf;

typedef struct mca_node_layout {
  bool visible;

  mc_node *focused_child;

  // Layout Properties
  horizontal_alignment_type horizontal_alignment;
  vertical_alignment_type vertical_alignment;
  float preferred_width, preferred_height;
  float min_width, min_height;
  float max_width, max_height;
  mc_paddingf padding;

  struct {
    float width, height;
  } determined_extents;

  unsigned int z_layer_index;

  // Input Handlers
  // void (*handle_input_event)(mc_node *, mci_input_event *)
  void *handle_input_event;

  // Functional Delegates
  // void (*determine_node_layout_extents)(mc_node *, layout_extent_restraints)
  void *determine_layout_extents;
  // void (*update_node_layout)(mc_node *, mc_rectf const *)
  void *update_layout;
  // void (*render_node_headless)(mc_node *)
  void *render_headless;
  // void (*render_node_presentation)(image_render_details *, mc_node *)
  void *render_present;

  // Application Set Fields
  // -- set these fields with the mca_ functions set_node_...
  bool __requires_layout_update, __requires_rerender;

  mc_rectf __bounds;

} mca_node_layout;

typedef struct mca_node_functionality {

} mca_node_functionality;

typedef struct mca_global_context_node_option {
  char *option_text;
  void *event_handler;
} mca_global_context_node_option;

typedef struct mca_global_context_node_option_list {
  node_type node_type;

  unsigned int alloc, count;
  mca_global_context_node_option **items;
} mca_global_context_node_option_list;

typedef struct mcu_ui_state {
  bool requires_update;
  mc_node_list *cache_layered_hit_list;
  mcr_font_resource *default_font_resource;

  // Modules
  struct {
    mc_node *node;
    mc_node *context_node;
    mc_point context_location;
    struct {
      unsigned int alloc, count;
      mca_global_context_node_option_list **items;
    } context_options;

  } global_context_menu;

  void *hierarchy_viewer_state;
  void *source_editor_pool;
} mcu_ui_state;

// typedef struct visual_project_data {
//   const char *name;
//   struct {
//     unsigned int offset_x, offset_y;
//     unsigned int width, height;
//   } screen;

//   // App Stuff
//   struct mcu_ui_state *ui_state;
//   bool requires_update, requires_rerender;
//   unsigned int present_image;

//   mc_node *editor_container;
//   mca_node_layout *layout;
//   mc_node_list *children;

// } visual_project_data;

// hierarchy.c
void exit_app(mc_node *hierarchical_call_scope, int result);

int mca_get_sub_hierarchy_node_list(mc_node *hierarchy_node, mc_node_list **sub_node_list);
int mca_attach_node_to_hierarchy(mc_node *hierarchy_node, mc_node *node_to_attach);
int mca_modify_z_layer_index(mc_node *hierarchy_node, unsigned int new_z_layer_index);
int mca_init_node_layout(mca_node_layout **layout);
int mca_init_mc_node(node_type type, const char *name, mc_node **node);

int mca_determine_typical_node_extents(mc_node *node, layout_extent_restraints restraints);
int mca_update_typical_node_layout(mc_node *node, mc_rectf const *available_area);
int mca_determine_typical_node_extents_partially(mc_node *node, layout_extent_restraints restraints,
                                                 bool determine_children);
int mca_update_typical_node_layout_partially(mc_node *node, mc_rectf const *available_area, bool update_x,
                                             bool update_y, bool update_width, bool update_height,
                                             bool update_children);

int mca_render_node_list_headless(render_thread_info *render_thread, mc_node_list *node_list);
int mca_render_node_list_present(image_render_details *image_render_queue, mc_node_list *node_list);

int mca_attach_to_ancestor_root(mc_node *ancestor, mc_node *node_to_attach);
int mca_detach_from_parent(mc_node *node);

int mca_set_node_requires_layout_update(mc_node *node);
/* A recursive function initiating layout update on all members of the node list and THEIR descendents as well */
int mca_set_descendents_require_layout_update(mc_node_list *node_descendents_list);
int mca_set_node_requires_rerender(mc_node *node);

int mca_render_typical_nodes_children_headless(render_thread_info *render_thread, mc_node_list *children);
int mca_render_typical_nodes_children_present(image_render_details *image_render_queue, mc_node_list *children);

int mca_focus_node(mc_node *node);
int mca_obtain_focused_node(mc_node **node);

int mca_register_event_handler(mc_app_event_type event_type, void *handler_delegate, void *handler_state);
int mca_fire_event(mc_app_event_type event_type, void *event_arg);
int mca_fire_event_and_release_data(mc_app_event_type event_type, void *event_arg, int release_count, ...);
int mca_register_loaded_project(mc_project_info *project);

// /* Implemented in midge_app.c */
// int mca_register_update_timer(unsigned int usecs_period, bool reset_timer_on_update, void *callback_state,
//                               int (*update_callback)(frame_time *, void *));

// util.c
/* Adds to an array which doubles in size every time it reaches a count equal to a power of two */
int mc_grow_array(void **ary, unsigned int *ptr_count, size_t item_allocation_size, void **ptr_data);
int reallocate_array(void **array, unsigned int *current_allocation, unsigned int desired_allocation,
                     size_t item_allocation_size, bool newmem_set_zero);
// @desired_allocation may be zero indicating the reallocate amount will be expanded by a 'reasonable' amount.
// @optional_item_allocation_size may be zero indicating no memory shall be assigned to the later allocation sizes.
int reallocate_collection(void ***collection, unsigned int *current_allocation, unsigned int desired_allocation,
                          size_t optional_item_allocation_size);

// project_management.c
void mca_init_visual_project_management();
void mca_create_new_visual_project(const char *project_name);
void mca_update_visual_project(mc_node *project_node);
void mca_render_project_headless(render_thread_info *render_thread, mc_node *visual_project);
void mca_render_project_present(image_render_details *image_render_queue, mc_node *visual_project);

// global_context_menu.c
void mca_init_global_context_menu();
void mca_render_global_context_menu(image_render_details *image_render_queue, mc_node *node);
void mca_global_context_menu_create_context_list(node_type node_type,
                                                 mca_global_context_node_option_list **context_list);
void mca_global_context_menu_add_option_to_node_context(
    node_type node_type, const char *option_text,
    /*void (*event_handler)(mc_node *, const char *)*/ void *event_handler);
void mca_activate_global_context_menu(int screen_x, int screen_y);
void mca_hide_global_context_menu();

// global_root.c
void mca_init_global_node_context_menu_options();
#endif // MC_UTIL_H