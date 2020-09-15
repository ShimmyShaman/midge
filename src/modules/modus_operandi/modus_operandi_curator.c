
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void init_modus_operandi_curator()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // // Panel
  // mui_panel *panel;
  // mui_init_panel(global_data->global_node, &panel);

  // panel->element->bounds = (mc_rectf){500, 100, 600, 480};
  // panel->background_color = COLOR_GREEN;

  // mui_set_element_update(panel->element);

  // Text Block
  mui_text_block *text_block;
  mui_init_text_block(global_data->global_node, &text_block);

  text_block->element->bounds = {150, 300, 0, 0};

  set_c_str(text_block->str, "Hello You!");
  text_block->font_color = COLOR_LIGHT_YELLOW;

  mca_set_node_requires_update(text_block->element->visual_node);
}