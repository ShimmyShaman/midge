#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

// void _mce_determine_source_line_extents(mc_node *node, layout_extent_restraints restraints)
// {
// // printf("_mce_determine_source_line_extents\n");
// const float MAX_EXTENT_VALUE = 100000.f;

// mce_source_line *source_line = (mce_source_line *)node->data;

// mc_rectf new_bounds = node->layout->__bounds;

// float rtf_width, rtf_height;
// if (!node->layout->preferred_width || !node->layout->preferred_height)
//   mcr_determine_text_display_dimensions(source_line->font_resource_uid, source_line->rtf->text, &rtf_width,
//                                         &rtf_height);

// // Width
// if (node->layout->preferred_width)
//   node->layout->determined_extents.width = node->layout->preferred_width;
// else {
//   if (restraints & LAYOUT_RESTRAINT_HORIZONTAL) {
//     node->layout->determined_extents.width = rtf_width;
//   }
//   else
//     node->layout->determined_extents.width = MAX_EXTENT_VALUE;
// }

// // Height
// if (node->layout->preferred_height)
//   node->layout->determined_extents.height = node->layout->preferred_height;
// else {
//   if (restraints & LAYOUT_RESTRAINT_VERTICAL) {
//     node->layout->determined_extents.height = rtf_height;
//   }
//   else
//     node->layout->determined_extents.height = MAX_EXTENT_VALUE;
// }
// }

void _mce_update_source_line_layout(mc_node *node, mc_rectf *available_area)
{
  mce_source_line *source_line = (mce_source_line *)node->data;

  mca_update_typical_node_layout(node, available_area);

  int midge_error_r;

  // Determine if the new bounds is worth setting
  if (node->layout->__requires_rerender) {

    // printf("source-line-bounds %.3f*%.3f\n", node->layout->__bounds.width, node->layout->__bounds.height);
    unsigned int layout_width = (unsigned int)node->layout->__bounds.width;
    unsigned int layout_height = (unsigned int)node->layout->__bounds.height;
    if (layout_width && layout_height) {
      // Ensure render target dimensions are sufficient
      if (source_line->render_target.resource_uid) {
        bool recreate_texture_resource = false;
        if (!source_line->render_target.width || source_line->render_target.width < layout_width) {
          recreate_texture_resource = true;
          source_line->render_target.width = layout_width;
        }
        if (!source_line->render_target.height || source_line->render_target.height < layout_height) {
          recreate_texture_resource = true;
          source_line->render_target.height = layout_height;
        }
        if (recreate_texture_resource) {
          // Dispose of the current one
          MCerror(8264, "TODO -- texture resource disposal");

          // Create another
          source_line->render_target.resource_uid = 0;
          // printf("recreating texture %u*%u\n", source_line->render_target.width, source_line->render_target.height);
          mcr_create_texture_resource(source_line->render_target.width, source_line->render_target.height,
                                      MVK_IMAGE_USAGE_RENDER_TARGET_2D, &source_line->render_target.resource_uid);
        }
      }
      else {
        source_line->render_target.width = layout_width;
        source_line->render_target.height = layout_height;
        // printf("creating texture %.3f*%.3f\n", node->layout->__bounds.width, node->layout->__bounds.height);
        mcr_create_texture_resource(source_line->render_target.width, source_line->render_target.height,
                                    MVK_IMAGE_USAGE_RENDER_TARGET_2D, &source_line->render_target.resource_uid);
      }
    }
  }
  node->layout->__requires_layout_update = false;

  // Set rerender anyway because lazy TODO--maybe
  mca_set_node_requires_rerender(node);
}

void _mce_render_source_line_headless(mc_node *node)
{
  // Toggle
  node->layout->__requires_rerender = false;

  // Render New Image
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);
  mce_source_line *source_line = (mce_source_line *)node->data;

  image_render_details *rq;
  mcr_obtain_image_render_request(global_data->render_thread, &rq);
  rq->render_target = NODE_RENDER_TARGET_IMAGE;
  rq->clear_color = COLOR_NEARLY_BLACK;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  rq->image_width = source_line->render_target.width;   // TODO
  rq->image_height = source_line->render_target.height; // TODO
  rq->data.target_image.image_uid = source_line->render_target.resource_uid;
  rq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  rq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;
  // printf("%u %u \n", rq->data.target_image.screen_offset_coordinates.x,
  // rq->data.target_image.screen_offset_coordinates.y);

  // Render the source token linked list
  mce_source_token *token = source_line->initial_token;
  int horizontal_offset = 0;
  int debug_token_count = 0;
  while (token) {
    render_color font_color;
    bool end_of_line = false;
    switch (token->type) {
    case MCE_SRC_EDITOR_NEW_LINE:
    case MCE_SRC_EDITOR_END_OF_FILE:
      end_of_line = true;
      break;
    case MCE_SRC_EDITOR_UNPROCESSED_TEXT:
      font_color = COLOR_POWDER_BLUE;
      break;
    default:
      MCerror(9288, "Unsupported mce_source_token_type=%i", token->type);
    }

    if (end_of_line)
      break;

    printf("source-line text:(%i)'%s'\n", horizontal_offset, token->str->text);
    mcr_issue_render_command_text(rq, (unsigned int)(node->layout->__bounds.x + horizontal_offset),
                                  (unsigned int)(node->layout->__bounds.y), token->str->text,
                                  source_line->font_resource_uid, font_color);
    ++debug_token_count;

    horizontal_offset += token->str->len * source_line->font_horizontal_stride;
    token = token->next;
  }

  printf("source-line-text rendered through %i tokens\n", debug_token_count);
  mcr_submit_image_render_request(global_data->render_thread, rq);
}

void _mce_render_source_line_present(image_render_details *image_render_queue, mc_node *node)
{
  mce_source_line *source_line = (mce_source_line *)node->data;

  // Text
  // render_color color = COLOR_PURPLE;
  // mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                       (unsigned int)node->layout->__bounds.y, source_line->render_target.width,
  //                                       source_line->render_target.height, color);

  // printf("rendersource_line- {%u %u %u %u} %s %u\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, (unsigned int)node->layout->__bounds.width, (unsigned
  //        int)node->layout->__bounds.height, source_line->rtf->text, source_line->font_resource_uid);

  mcr_issue_render_command_textured_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
      source_line->render_target.resource_uid);
}

void mce_init_source_line(mc_node *parent, mce_source_line **p_source_line)
{
  // Node
  mc_node *node;
  mca_init_mc_node(parent, NODE_TYPE_MCM_SOURCE_LINE, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  node->layout->determine_layout_extents =
      (void *)&mca_determine_typical_node_extents; //_mce_determine_source_line_extents;
  node->layout->update_layout = (void *)&_mce_update_source_line_layout;
  node->layout->render_headless = (void *)&_mce_render_source_line_headless;
  node->layout->render_present = (void *)&_mce_render_source_line_present;

  // Control
  mce_source_line *source_line = (mce_source_line *)malloc(sizeof(mce_source_line));
  source_line->node = node;
  node->data = source_line;

  // init_c_str(&source_line->rtf);
  source_line->initial_token = NULL;
  source_line->font_resource_uid = 0;
  source_line->render_target.resource_uid = 0;
  source_line->render_target.width = 0;
  source_line->render_target.height = 0;

  source_line->font_horizontal_stride = 9.2794f;

  // Set to out pointer
  *p_source_line = source_line;
}