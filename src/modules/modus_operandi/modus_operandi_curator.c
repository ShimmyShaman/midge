
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void init_modus_operandi_curator()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Panel
  mui_panel *panel;
  mui_init_panel(global_data->global_node, &panel);

  panel->element->bounds = (mc_rectf){500, 100, 600, 480};
  panel->background_color = COLOR_DARK_GREEN;

  //   // Text Block
  //   mui_text_block *text_block;
  //   mui_init_text_block(panel->element->visual_node, &text_block);

  //   set_c_str(text_block->str, "Hello You!");
  //   mui_set_element_update(text_block->element);
  //   text_block->font_color = COLOR_GHOST_WHITE;
}