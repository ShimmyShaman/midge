#ifndef APP_MODULES_H
#define APP_MODULES_H

#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "ui/ui_definitions.h"

// TODO better way of this -- seperate includes and includes brought in etc
// hierarchy_viewer.c
typedef struct mc_hv_source_path_state {
  char *section_name;
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

extern "C" {
// modus_operandi_curator.c
void init_modus_operandi_curator();

// hierarchy_viewer.c
void init_hierarchy_viewer();
}

#endif // APP_MODULES_H
