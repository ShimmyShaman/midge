#include "env/environment_definitions.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

// void _mcm_determine_source_line_extents(mc_node *node, layout_extent_restraints restraints)
// {
// // printf("_mcm_determine_source_line_extents\n");
// const float MAX_EXTENT_VALUE = 100000.f;

// mcm_source_line *source_line = (mcm_source_line *)node->data;

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

void _mcm_update_source_line_layout(mc_node *node, mc_rectf *available_area)
{
  mcm_source_line *source_line = (mcm_source_line *)node->data;

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

void _mcm_render_source_line_headless(mc_node *node)
{
  mcm_source_line *source_line = (mcm_source_line *)node->data;

  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

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

  // render_color font_color = COLOR_GHOST_WHITE;
  render_color quad_color = COLOR_POWDER_BLUE;

  // printf("rendering_text:%s\n", source_line->rtf->text);
  // Render the text
  render_color font_color = COLOR_BURLY_WOOD;
  int i = 0, s, rm = 0;
  while (i < source_line->rtf->len) {
    s = i;
    if (source_line->rtf->text[i] == '[') {
      if (source_line->rtf->text[i + 1] == '[') {
        // Escaped. Rearrange start
        rm += 1;
        s = i + 1;
        i += 2;
      }
      else if (!strncmp(source_line->rtf->text + i, "[color=", 7)) {
        int j = i + 7;
        int r, g, b;
        mce_parse_past_integer(source_line->rtf->text, &j, &r);
        parse_past(source_line->rtf->text, &j, ",");
        mce_parse_past_integer(source_line->rtf->text, &j, &g);
        parse_past(source_line->rtf->text, &j, ",");
        mce_parse_past_integer(source_line->rtf->text, &j, &b);
        parse_past(source_line->rtf->text, &j, "]");

        font_color.r = (float)r / 255.f;
        font_color.g = (float)g / 255.f;
        font_color.b = (float)b / 255.f;

        rm += j - s;
        s = i = j;
        continue;
      }
      else {
        MCerror(359, "Unsupported:'%s'", source_line->rtf->text + i);
      }
    }

    bool eof = false;
    for (;; ++i) {
      if (source_line->rtf->text[i] == '\0') {
        eof = true;
        break;
      }
      else if (source_line->rtf->text[i] == '[') {
        break;
      }
    }

    while (i - s > 0) {
      char buf[256];
      int write_len = min(i - s, 255);
      strncpy(buf, source_line->rtf->text + s, write_len);
      buf[write_len] = '\0';
      printf("rendering_text:%s %u\n", buf,
             0); //(unsigned int)(node->layout->__bounds.x + (s - rm) * source_line->font_horizontal_stride));
      mcr_issue_render_command_text(
          rq, (unsigned int)(node->layout->__bounds.x + (s - rm) * source_line->font_horizontal_stride),
          (unsigned int)(node->layout->__bounds.y), buf, source_line->font_resource_uid, font_color);
      s += write_len;
    }
  }

  // printf("rq->render_target:%i\n", rq->render_target);
  mcr_submit_image_render_request(global_data->render_thread, rq);
}

void _mcm_render_source_line_present(image_render_details *image_render_queue, mc_node *node)
{
  mcm_source_line *source_line = (mcm_source_line *)node->data;

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

void mcm_init_source_line(mc_node *parent, mcm_source_line **p_source_line)
{
  // Node
  mc_node *node;
  mca_init_mc_node(parent, NODE_TYPE_MCM_SOURCE_LINE, &node);

  // Layout
  mca_init_node_layout(&node->layout);
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  node->layout->determine_layout_extents =
      (void *)&mca_determine_typical_node_extents; //_mcm_determine_source_line_extents;
  node->layout->update_layout = (void *)&_mcm_update_source_line_layout;
  node->layout->render_headless = (void *)&_mcm_render_source_line_headless;
  node->layout->render_present = (void *)&_mcm_render_source_line_present;

  // Control
  mcm_source_line *source_line = (mcm_source_line *)malloc(sizeof(mcm_source_line));
  source_line->node = node;
  node->data = source_line;

  init_c_str(&source_line->rtf);
  source_line->font_resource_uid = 0;
  source_line->render_target.resource_uid = 0;
  source_line->render_target.width = 0;
  source_line->render_target.height = 0;

  source_line->font_horizontal_stride = 9.2794f;

  // Set to out pointer
  *p_source_line = source_line;
}