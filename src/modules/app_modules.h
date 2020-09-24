#ifndef APP_MODULES_H
#define APP_MODULES_H

#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "ui/ui_definitions.h"

typedef enum mc_hv_hierarchical_item_type {
  MC_HV_HIERARCHICAL_NULL = 0,
  MC_HV_HIERARCHICAL_FOLDER,
  MC_HV_HIERARCHICAL_C_SOURCE,
  MC_HV_HIERARCHICAL_C_HEADER,
  MC_HV_HIERARCHICAL_STRUCT_DEFINITION,
  MC_HV_HIERARCHICAL_FUNCTION_DEFINITION,
  MC_HV_HIERARCHICAL_ENUM_DEFINITION,
} mc_hv_hierarchical_item_type;

// TODO better way of this -- seperate includes and includes brought in etc
// hierarchy_viewer.c
typedef struct mc_hv_source_path_state {
  mc_hv_hierarchical_item_type item_type;
  char *item_name;
  void *data;
  bool collapsed;

  struct {
    unsigned int alloc, count;
    mc_hv_source_path_state **items;
  } children;
} mc_hv_source_path_state;

typedef struct mc_hv_hierarchy_view_state {
  mc_node *root_node;

  struct {
    unsigned int size, visible_count;
    mc_node **items;
  } text_lines;

  struct {
    unsigned int alloc, count;
    mc_hv_source_path_state **items;
  } path_states;
} mc_hv_hierarchy_view_state;

typedef struct mcm_source_line {
  mc_node *node;

  c_str *rtf;

  unsigned int font_resource_uid;
  struct {
    unsigned int width, height;
    unsigned int resource_uid;
  } render_target;
} mcm_source_line;

struct mcm_source_editor_pool;
typedef struct mcm_function_editor {
  mc_node *node;
  mcm_source_editor_pool *source_editor_pool;

  function_info *function;
  render_color background_color;

  struct {
    c_str *rtf;
    mc_syntax_node *syntax;
  } code;

  struct {
    unsigned int count, alloc;
    mcm_source_line **items;
  } lines;

  int line_display_offset;

} mcm_function_editor;

typedef struct mcm_source_editor_pool {
  unsigned int max_instance_count;
  struct {
    unsigned int size;
    mcm_function_editor **instances;
  } function_editor;
} mcm_source_editor_pool;

extern "C" {
// modus_operandi_curator.c
void init_modus_operandi_curator();

// hierarchy_viewer.c
void init_hierarchy_viewer();

// source_editor/source_editor.c
void mca_init_source_editor_pool();
void mca_activate_source_editor_for_definition(source_definition *definition);

// source_editor/source_line.c
void mcm_init_source_line(mc_node *parent_node, mcm_source_line **source_line);

// source_editor/function_editor.c
void mcm_init_function_editor(mc_node *parent_node, mcm_source_editor_pool *source_editor_pool,
                              mcm_function_editor **p_function_editor);

// three_d/three_d.c
void init_three_d_portal();
}

#endif // APP_MODULES_H
