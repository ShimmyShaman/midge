
#include "core/core_definitions.h"
#include "env/environment_definitions.h"

void mca_global_root_create_new_module(mc_node *context_node, const char *selected_text)
{
  mca_create_new_visual_project("Capitalistic Robotic Colony");
}

void mca_global_root_create_add_button(mc_node *context_node, const char *selected_text)
{
  printf("mca_global_root_create_add_button\n");
}

void mca_init_global_node_context_menu_options()
{
  void *arg = (void *)&mca_global_root_create_new_module;
  mca_global_context_menu_add_option_to_node_context(NODE_TYPE_GLOBAL_ROOT, "new module...", arg);

  arg = (void *)&mca_global_root_create_add_button;
  mca_global_context_menu_add_option_to_node_context(NODE_TYPE_GLOBAL_ROOT, "add button", arg);
}