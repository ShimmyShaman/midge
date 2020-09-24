#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void __mcm_determine_source_line_extents(mc_node *node, layout_extent_restraints restraints)
{
  mcm_source_line *source_line = (mcm_source_line *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;

  float rtf_width, rtf_height;
  if (!node->layout->preferred_width || !node->layout->preferred_height)
    mcr_determine_text_display_dimensions(source_line->font_resource_uid, source_line->rtf->text, &rtf_width,
                                          &rtf_height);

  // Width
  if (node->layout->preferred_width)
    new_bounds.width = node->layout->preferred_width;
  else
    new_bounds.width = rtf_width;

  // Height
  if (node->layout->preferred_height)
    new_bounds.height = node->layout->preferred_height;
  else
    new_bounds.height = rtf_height;

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;
    mca_set_node_requires_layout_update(node);
  }
}

void __mcm_update_source_line_layout(mc_node *node, mc_rectf *available_area)
{
  mcm_source_line *source_line = (mcm_source_line *)node->data;

  mc_rectf new_bounds = node->layout->__bounds;
  new_bounds.x = available_area->x + node->layout->padding.left;
  new_bounds.y = available_area->y + node->layout->padding.top;

  // Determine if the new bounds is worth setting
  if (new_bounds.x != node->layout->__bounds.x || new_bounds.y != node->layout->__bounds.y ||
      new_bounds.width != node->layout->__bounds.width || new_bounds.height != node->layout->__bounds.height) {
    node->layout->__bounds = new_bounds;

    if (node->layout->__bounds.width > 0 && node->layout->__bounds.height > 0) {
      mca_set_node_requires_rerender(node);

      // Ensure render target dimensions are sufficient
      if (source_line->render_target.resource_uid) {
        bool recreate_texture_resource = false;
        if (!source_line->render_target.width || source_line->render_target.width < new_bounds.width) {
          recreate_texture_resource = true;
          source_line->render_target.width = (unsigned int)new_bounds.width;
        }
        if (!source_line->render_target.height || source_line->render_target.height < new_bounds.height) {
          recreate_texture_resource = true;
          source_line->render_target.height = (unsigned int)new_bounds.height;
        }
        if (recreate_texture_resource) {
          // Dispose of the current one
          MCerror(8264, "TODO -- texture resource disposal");

          // Create another
          source_line->render_target.resource_uid = 0;
          mcr_create_texture_resource(source_line->render_target.width, source_line->render_target.height, true,
                                      &source_line->render_target.resource_uid);
        }
      }
      else {
        source_line->render_target.width = (unsigned int)node->layout->__bounds.width;
        source_line->render_target.height = (unsigned int)node->layout->__bounds.height;
        printf("creating texture %.3f*%.3f\n", node->layout->__bounds.width, node->layout->__bounds.height);
        printf("creating texture %u*%u\n", source_line->render_target.width, source_line->render_target.height);
        mcr_create_texture_resource(source_line->render_target.width, source_line->render_target.height, true,
                                    &source_line->render_target.resource_uid);
      }
    }
  }

  node->layout->__requires_layout_update = false;

  // Set rerender anyway because lazy TODO--maybe
  mca_set_node_requires_rerender(node);
  /***************************************8*/
  double brackets does something , try not print them
}

void __mcm_render_source_line_headless(mc_node *node)
{
  mcm_source_line *source_line = (mcm_source_line *)node->data;

  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  image_render_queue *rq;
  obtain_image_render_queue(&global_data->render_thread->render_queue, &rq);
  rq->render_target = NODE_RENDER_TARGET_IMAGE;
  rq->clear_color = COLOR_NEARLY_BLACK;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  rq->image_width = source_line->render_target.width;   // TODO
  rq->image_height = source_line->render_target.height; // TODO
  rq->data.target_image.image_uid = source_line->render_target.resource_uid;
  rq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  rq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;

  render_color font_color = COLOR_GHOST_WHITE;
  render_color quad_color = COLOR_POWDER_BLUE;

  mcr_issue_render_command_text(rq, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
                                source_line->rtf->text, source_line->font_resource_uid, font_color);
}

void __mcm_render_source_line_present(image_render_queue *render_queue, mc_node *node)
{
  mcm_source_line *source_line = (mcm_source_line *)node->data;

  // Text
  // printf("rendersource_line- %u %u %s %u\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, source_line->rtf->text, source_line->font_resource_uid);

  mcr_issue_render_command_textured_quad(render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, source_line->render_target.width,
                                         source_line->render_target.height, source_line->render_target.resource_uid);
}

void mcm_init_source_line(mc_node *parent, mcm_source_line **p_source_line)
{
  // Node
  mc_node *node;
  mca_init_mc_node(parent, NODE_TYPE_MCM_SOURCE_LINE, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->determine_layout_extents = (void *)&__mcm_determine_source_line_extents;
  node->layout->update_layout = (void *)&__mcm_update_source_line_layout;
  node->layout->render_headless = (void *)&__mcm_render_source_line_headless;
  node->layout->render_present = (void *)&__mcm_render_source_line_present;

  // Control
  mcm_source_line *source_line = (mcm_source_line *)malloc(sizeof(mcm_source_line));
  source_line->node = node;
  node->data = source_line;

  init_c_str(&source_line->rtf);
  source_line->font_resource_uid = 0;
  source_line->render_target.resource_uid = 0;
  source_line->render_target.width = 0;
  source_line->render_target.height = 0;

  source_line->render_target.resource_uid = 0;

  // Set to out pointer
  *p_source_line = source_line;
}