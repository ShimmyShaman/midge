/* render_util.c */

#include <stdio.h>

#include "env/environment_definitions.h"

#include "modules/render_utilities/render_util.h"

int mcr_render_border(image_render_details *irq, mc_node *node, unsigned int thickness, render_color color)
{
  // Top
  MCcall(mcr_issue_render_command_colored_quad(irq, (unsigned int)node->layout->__bounds.x,
                                               (unsigned int)node->layout->__bounds.y,
                                               (unsigned int)node->layout->__bounds.width, thickness, color));

  // Left
  MCcall(mcr_issue_render_command_colored_quad(irq, (unsigned int)node->layout->__bounds.x,
                                               (unsigned int)node->layout->__bounds.y + thickness, thickness,
                                               (unsigned int)node->layout->__bounds.height - thickness * 2, color));

  // Right
  MCcall(mcr_issue_render_command_colored_quad(
      irq, (unsigned int)node->layout->__bounds.x + node->layout->__bounds.width - thickness,
      (unsigned int)node->layout->__bounds.y + thickness, thickness,
      (unsigned int)node->layout->__bounds.height - thickness * 2, color));

  // Bottom
  MCcall(mcr_issue_render_command_colored_quad(
      irq, (unsigned int)node->layout->__bounds.x,
      (unsigned int)(node->layout->__bounds.y + node->layout->__bounds.height) - thickness,
      (unsigned int)node->layout->__bounds.width, thickness, color));

  return 0;
}