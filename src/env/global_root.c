
#include "core/core_definitions.h"
#include "env/environment_definitions.h"

void mca_global_root_create_new_module(mc_node *context_node, mc_point context_location, const char *selected_text)
{
  mca_create_new_visual_project("Capitalistic Robotic Colony");
}

void mca_init_global_node_context_menu_options()
{
  void *arg = (void *)&mca_global_root_create_new_module;
  mca_global_context_menu_add_option_to_node_context(NODE_TYPE_GLOBAL_ROOT, "new module...", arg);
}