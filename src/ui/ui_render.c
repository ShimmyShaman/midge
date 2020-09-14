
#include "core/core_definitions.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

// Renders all children with headless sub-images. Then renders this node if it is such an element.
// void mui_render_children_headless_elements(mc_node *node)
// {
//   switch (node->type) {
//   default:
//     MCerror(11, "mui_render_children_headless_elements::>unsupported node type:%i", node->type);
//   }
// }

void mui_update_headless_image_node(mc_node *element_node)
{
  mui_ui_element *ui_element = (mui_ui_element *)element_node->data;

  switch (ui_element->type) {
  case UI_ELEMENT_TEXT_BLOCK:
    // Elements which do no headless image rendering, nor hold any children
    break;
  case UI_ELEMENT_PANEL: {
    // TODO
  } break;
  default:
    MCerror(21, "mui_update_headless_image_node:>Unsupported element type:%i", ui_element->type);
  }

  // if (ui_element->headless_image_resource_uid) {
  //   MCerror(22, "TODO");
  // }

  // // Render the present image
  // image_render_queue *sequence;
  // element_render_command *element_cmd;

  // MCcall(obtain_image_render_queue(command_hub->renderer.render_queue, &sequence));
  // sequence->render_target = NODE_RENDER_TARGET_IMAGE;
  // sequence->clear_color = COLOR_CORNFLOWER_BLUE;
  // sequence->image_width = global_data->screen.width;
  // sequence->image_height = global_data->screen.height;
  // sequence->data.target_image.image_uid = cestate->render_lines[ce_offset_line_index]->image_resource_uid;
}

void mui_render_ui_node(image_render_queue *render_queue, mc_node *element_node)
{
  mui_ui_element *element = (mui_ui_element *)element_node->data;

  switch (element->type) {
  case UI_ELEMENT_TEXT_BLOCK: {
    mui_render_text_block(render_queue, element_node);
  } break;
  case UI_ELEMENT_PANEL: {
    mui_render_panel(render_queue, element_node);
  } break;
  default:
    MCerror(44, "mui_render_ui_node:>Unsupported element type:%i", element->type);
  }
}