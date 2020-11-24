#ifndef APP_MODULES_H
#define APP_MODULES_H

// #include "core/core_definitions.h"
// #include "env/environment_definitions.h"
// #include "ui/ui_definitions.h"

// typedef enum mc_hv_hierarchical_item_type {
//   MC_HV_HIERARCHICAL_NULL = 0,
//   MC_HV_HIERARCHICAL_FOLDER,
//   MC_HV_HIERARCHICAL_C_SOURCE,
//   MC_HV_HIERARCHICAL_C_HEADER,
//   MC_HV_HIERARCHICAL_STRUCT_DEFINITION,
//   MC_HV_HIERARCHICAL_FUNCTION_DEFINITION,
//   MC_HV_HIERARCHICAL_ENUM_DEFINITION,
// } mc_hv_hierarchical_item_type;

// // TODO better way of this -- seperate includes and includes brought in etc
// // hierarchy_viewer.c
// typedef struct mc_hv_source_path_state {
//   mc_hv_hierarchical_item_type item_type;
//   char *item_name;
//   void *data;
//   bool collapsed;

//   struct {
//     unsigned int alloc, count;
//     mc_hv_source_path_state **items;
//   } children;
// } mc_hv_source_path_state;

// typedef struct mc_hv_hierarchy_view_state {
//   mc_node *root_node;

//   struct {
//     unsigned int size, visible_count;
//     mc_node **items;
//   } text_lines;

//   struct {
//     unsigned int alloc, count;
//     mc_hv_source_path_state **items;
//   } path_states;
// } mc_hv_hierarchy_view_state;

int mca_load_modules();

#endif // APP_MODULES_H
