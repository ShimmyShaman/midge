/* function_editor.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/app_modules.h"
#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"

#include "control/mc_controller.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"

#include "modules/source_editor/source_editor.h"

void _mce_update_function_editor_visible_source_lines(mce_function_editor *function_editor)
{
  // printf("_mce_update_function_editor_visible_source_lines\n");
  function_editor->lines.utilized = 0;

  // printf("function_editor->lines.count:%u \n", function_editor->lines.count);
  for (int line = 0; line < function_editor->lines.count; ++line) {
    int code_line_index = line + function_editor->lines.display_index_offset;
    mce_source_line *source_line = function_editor->lines.items[line];

    // printf("code_line_index:%u displayoffsetindex:%u \n", code_line_index,
    // function_editor->lines.display_index_offset);
    if (code_line_index >= function_editor->code.count) {
      // printf("set line %i visible false\n", line);
      source_line->node->layout->visible = false;
      continue;
    }

    // Set
    source_line->node->layout->visible = true;
    // printf("set line %i visible true\n", line);

    // TODO -- this is where you'd determine the hash and compare with what is already rendered
    source_line->line_token = function_editor->code.line_tokens[code_line_index];
    // printf("%p\n", source_line->line_token->first);
    // printf("%s\n", source_line->line_token->first->str->text);
    mca_set_node_requires_rerender(source_line->node);
  }
}

int _mce_update_line_positions(mce_function_editor *fedit,mc_rectf const *available_area
{
  int y_index = 0;

  bool line_count_changed = false;
  while (y_index * fedit->lines.vertical_stride < available_area->height - 20 /* TODO ? */) {

    // Obtain the line control
    mce_source_line *line;
    if (y_index >= fedit->lines.count) {
      // Construct a new one
      mce_init_source_line(fedit->node, &line);
      append_to_collection((void ***)&fedit->lines.items, &fedit->lines.capacity, &fedit->lines.count, line);
      line_count_changed = true;
    }
    else {
      line = fedit->lines.items[y_index];
    }

    // Set line layout
    line->node->layout->padding = (mc_paddingf){
        fedit->lines.padding.left, fedit->lines.padding.top + fedit->lines.vertical_stride * y_index, 0.f, 0.f};
    line->node->layout->preferred_height = fedit->lines.vertical_stride;

    // // Attach line source (if it exists)
    // if (y_index + fedit->lines.display_index_offset < fedit->code.lines.count) {
    //   line->source_list = fedit->code.lines.items[y_index + fedit->lines.display_index_offset];
    //   line->node->layout->visible = line->source_list->count;
    // }
    // else {
    //   line->source_list = NULL;
    //   line->node->layout->visible = false;
    // }

    // Continue
    ++y_index;
  }

  if (line_count_changed) {
    _mce_update_function_editor_visible_source_lines(fedit);
  }

  return 0;
}

void _mce_determine_function_editor_extents(mc_node *node, layout_extent_restraints restraints)
{
  mca_determine_typical_node_extents(node, restraints);
}

void _mce_update_function_editor_layout(mc_node *node,mc_rectf const *available_area
{
  // Clear
  node->layout->__requires_layout_update = false;

  // Preferred value > padding (within min/max if set)
  mc_rectf bounds;
  mca_node_layout *layout = node->layout;
  layout->__requires_layout_update = false;

  // Width
  if (layout->preferred_width) {
    // Set to preferred width
    bounds.width = layout->preferred_width;
  }
  else {
    // padding adjusted from available
    bounds.width = available_area->width - layout->padding.right - layout->padding.left;

    // Specified bounds
    if (layout->min_width && bounds.width < layout->min_width) {
      bounds.width = layout->min_width;
    }
    if (layout->max_width && bounds.width > layout->max_width) {
      bounds.width = layout->max_width;
    }

    if (bounds.width < 0) {
      bounds.width = 0;
    }
  }

  // Height
  if (layout->preferred_height) {
    // Set to preferred height
    bounds.height = layout->preferred_height;
  }
  else {
    // padding adjusted from available
    bounds.height = available_area->height - layout->padding.bottom - layout->padding.top;

    // Specified bounds
    if (layout->min_height && bounds.height < layout->min_height) {
      bounds.height = layout->min_height;
    }
    if (layout->max_height && bounds.height > layout->max_height) {
      bounds.height = layout->max_height;
    }

    if (bounds.height < 0) {
      bounds.height = 0;
    }
  }

  // X
  switch (layout->horizontal_alignment) {
  case HORIZONTAL_ALIGNMENT_LEFT: {
    // printf("left %.3f %.3f\n", available_area->x, layout->padding.left);
    bounds.x = available_area->x + layout->padding.left;
  } break;
  case HORIZONTAL_ALIGNMENT_RIGHT: {
    // printf("right %.3f %.3f %.3f %.3f\n", available_area->x, layout->padding.left, layout->padding.right,
    // bounds.width);
    bounds.x = available_area->x + available_area->width - layout->padding.right - bounds.width;
  } break;
  case HORIZONTAL_ALIGNMENT_CENTRED: {
    // printf("centred %.3f %.3f %.3f %.3f %.3f\n", available_area->x, layout->padding.left, available_area->width,
    //  layout->padding.right, bounds.width);
    bounds.x = available_area->x + layout->padding.left +
               (available_area->width - (layout->padding.left + bounds.width + layout->padding.right)) / 2.f;
  } break;
  default:
    MCVerror(7371, "NotSupported:%i", layout->horizontal_alignment);
  }

  // Y
  switch (layout->vertical_alignment) {
  case VERTICAL_ALIGNMENT_TOP: {
    bounds.y = available_area->y + layout->padding.top;
  } break;
  case VERTICAL_ALIGNMENT_BOTTOM: {
    bounds.y = available_area->y + available_area->height - layout->padding.bottom - bounds.height;
  } break;
  case VERTICAL_ALIGNMENT_CENTRED: {
    bounds.y = available_area->y + layout->padding.top +
               (available_area->height - (layout->padding.bottom + bounds.height + layout->padding.top)) / 2.f;
  } break;
  default:
    MCVerror(7387, "NotSupported:%i", layout->vertical_alignment);
  }

  // Set if different
  if (bounds.x != layout->__bounds.x || bounds.y != layout->__bounds.y || bounds.width != layout->__bounds.width ||
      bounds.height != layout->__bounds.height) {
    layout->__bounds = bounds;
    // printf("setrerender\n");
    mca_set_node_requires_rerender(node);
  }

  // printf("function_editor-available %.3f %.3f %.3f*%.3f\n", available_area->x, available_area->y,
  // available_area->width,
  //        available_area->height);
  // printf("function_editor-padding %.3f %.3f %.3f*%.3f\n", node->layout->padding.left, node->layout->padding.top,
  //        node->layout->padding.right, node->layout->padding.bottom);
  // printf("function_editor-bounds %.3f %.3f %.3f*%.3f\n", node->layout->__bounds.x, node->layout->__bounds.y,
  //        node->layout->__bounds.width, node->layout->__bounds.height);

  // Align text lines to fit to the container
  mce_function_editor *fedit = (mce_function_editor *)node->data;
  _mce_update_line_positions(fedit, &node->layout->__bounds);
  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->update_layout) {
      // TODO fptr casting
      void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
      update_layout(child, &node->layout->__bounds);
    }
  }
  // node->layout->__requires_layout_update = false;

  // // Set rerender anyway because lazy TODO--maybe
  // mca_set_node_requires_rerender(node);
}

void _mce_render_function_editor_headless(render_thread_info *render_thread, mc_node *node)
{
  // puts("_mce_render_function_editor_headless");
  mce_function_editor *function_editor = (mce_function_editor *)node->data;

  // struct timespec debug_start_time, debug_end_time;
  // clock_gettime(CLOCK_REALTIME, &debug_start_time);
  int debug_count = 0;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(render_thread_info *, mc_node *) = (void (*)(render_thread_info *, mc_node *))child->layout->render_headless;
      render_node_headless(render_thread, child);
      ++debug_count;
    }
  }

  // clock_gettime(CLOCK_REALTIME, &debug_end_time);
  // printf("FunctionEditorHeadless: rendered %i children took %.2fms\n", debug_count,
  //        1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
  //            1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
}

void _mce_render_function_editor_present(image_render_details *image_render_queue, mc_node *node)
{
  mce_function_editor *fedit = (mce_function_editor *)node->data;

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, fedit->background_color);
  //   printf("_mce_render_function_editor_present %u %u %u %u\n", (unsigned int)node->layout->__bounds.x,
  //          (unsigned int)node->layout->__bounds.y, (unsigned int)node->layout->__bounds.width,
  //          (unsigned int)node->layout->__bounds.height);

  // // DEBUG - TIME
  // struct timespec debug_start_time, debug_end_time;
  // clock_gettime(CLOCK_REALTIME, &debug_start_time);
  // // DEBUG - TIME
  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_presentation)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_presentation(image_render_queue, child);
    }
  }
  // // DEBUG - TIME
  // clock_gettime(CLOCK_REALTIME, &debug_end_time);
  // printf("\ninterest-Rerender took %.2fms\n", 1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
  //                                               1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
  // // DEBUG - TIME

  if (fedit->cursor.visible) {
    render_color cursor_color = COLOR_GHOST_WHITE;
    mcr_issue_render_command_text(
        image_render_queue,
        (unsigned int)(node->layout->__bounds.x + fedit->lines.padding.left +
                       fedit->font_horizontal_stride * ((float)fedit->cursor.col - 0.5f)),
        (unsigned int)(node->layout->__bounds.y + fedit->lines.padding.top +
                       fedit->lines.vertical_stride * (fedit->cursor.line - fedit->lines.display_index_offset)),
        "|", NULL, cursor_color);
  }

  render_color title_color = COLOR_FUNCTION_GREEN;
  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)fedit->lines.padding.top - 4, title_color);

  {
    // Border
    mcr_issue_render_command_colored_quad(
        image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
        (unsigned int)node->layout->__bounds.width, (unsigned int)fedit->border.thickness, fedit->border.color);
    mcr_issue_render_command_colored_quad(
        image_render_queue, (unsigned int)node->layout->__bounds.x,
        (unsigned int)node->layout->__bounds.y + fedit->border.thickness, (unsigned int)fedit->border.thickness,
        (unsigned int)node->layout->__bounds.height - fedit->border.thickness, fedit->border.color);
    mcr_issue_render_command_colored_quad(
        image_render_queue,
        (unsigned int)node->layout->__bounds.x + node->layout->__bounds.width - fedit->border.thickness,
        (unsigned int)node->layout->__bounds.y + fedit->border.thickness, (unsigned int)fedit->border.thickness,
        (unsigned int)node->layout->__bounds.height - fedit->border.thickness, fedit->border.color);
    mcr_issue_render_command_colored_quad(
        image_render_queue, (unsigned int)node->layout->__bounds.x + fedit->border.thickness,
        (unsigned int)node->layout->__bounds.y + node->layout->__bounds.height - fedit->border.thickness,
        (unsigned int)node->layout->__bounds.width - fedit->border.thickness * 2, (unsigned int)fedit->border.thickness,
        fedit->border.color);
  }
}

void mce_move_cursor_up(mce_function_editor *fedit)
{
  if (fedit->cursor.line > 0) {
    if (!fedit->cursor.zen_col) {
      fedit->cursor.zen_col = fedit->cursor.col;
    }

    --fedit->cursor.line;

    int line_len = fedit->code.line_tokens[fedit->cursor.line]->len;
    if (fedit->cursor.zen_col <= line_len) {
      fedit->cursor.col = fedit->cursor.zen_col;
    }
    else {
      fedit->cursor.col = line_len;
    }
  }
  else {
    fedit->cursor.col = 0;
  }

  mca_set_node_requires_rerender(fedit->node);
}

void mce_move_cursor_down(mce_function_editor *fedit)
{
  if (fedit->cursor.line + 1 < fedit->code.count) {
    if (!fedit->cursor.zen_col) {
      fedit->cursor.zen_col = fedit->cursor.col;
    }

    ++fedit->cursor.line;

    int line_len = fedit->code.line_tokens[fedit->cursor.line]->len;
    if (fedit->cursor.zen_col <= line_len) {
      fedit->cursor.col = fedit->cursor.zen_col;
    }
    else {
      fedit->cursor.col = line_len;
    }
  }
  else {
    fedit->cursor.col = fedit->code.line_tokens[fedit->code.count - 1]->len;
  }

  mca_set_node_requires_rerender(fedit->node);
}

void mce_move_cursor_left(mce_function_editor *fedit)
{
  fedit->cursor.zen_col = 0;

  // Decrement the cursor col
  if (fedit->cursor.col == 0) {
    if (fedit->cursor.line > 0) {
      --fedit->cursor.line;
      fedit->cursor.col = fedit->code.line_tokens[fedit->cursor.line]->len;
    }
    else {
      // Do nothing -- already at beginning of document
    }
  }
  else {
    --fedit->cursor.col;
  }

  mca_set_node_requires_rerender(fedit->node);
}

void mce_move_cursor_right(mce_function_editor *fedit)
{
  fedit->cursor.zen_col = 0;

  // Increment the cursor col right
  if (fedit->cursor.col >= fedit->code.line_tokens[fedit->cursor.line]->len) {
    if (fedit->cursor.line + 1 >= fedit->code.count) {
      // Do nothing - already at edge of document
    }
    else {
      fedit->cursor.col = 0;
      ++fedit->cursor.line;
      // TODO  -- determine line display offset
    }
  }
  else {
    ++fedit->cursor.col;
  }

  mca_set_node_requires_rerender(fedit->node);
}

void mce_move_cursor_end(mce_function_editor *fedit)
{
  fedit->cursor.zen_col = 0;

  // Set the cursor col to the end
  fedit->cursor.col = fedit->code.line_tokens[fedit->cursor.line]->len;

  mca_set_node_requires_rerender(fedit->node);
}

void mce_move_cursor_home(mce_function_editor *fedit)
{
  fedit->cursor.zen_col = 0;

  mce_source_token *token = fedit->code.line_tokens[fedit->cursor.line]->first;
  int end_of_empty = 0;
  while (token) {
    int off = end_of_empty;
    while (end_of_empty - off < token->str->len) {
      if (token->str->text[end_of_empty - off] != ' ' && token->str->text[end_of_empty - off] != '\t')
        break;
      ++end_of_empty;
    }
    if (end_of_empty - off >= token->str->len) {
      token = token->next;
      continue;
    }
    break;
  }

  // Set
  if (fedit->cursor.col != 0 && end_of_empty >= fedit->cursor.col) {
    fedit->cursor.col = 0;
  }
  else {
    fedit->cursor.col = end_of_empty;
  }

  mca_set_node_requires_rerender(fedit->node);
}

void mce_set_function_editor_cursor_position(mce_function_editor *fedit, int document_line, int document_col)
{
  fedit->cursor.zen_col = 0;

  // Offset --
  // preferred_line -= fedit->lines.display_index_offset;
  // if (preferred_line + fedit->lines.display_index_offset < 0)
  //   preferred_line = 0;
  // if (preferred_line +fedit->lines.display_index_offset >= fedit->code.lines.count) {
  //   preferred_line
  // }

  // Determine how the cursor will move
  if (document_line >= fedit->code.count) {
    fedit->cursor.line = fedit->code.count - 1;
    fedit->cursor.col = fedit->code.line_tokens[fedit->cursor.line]->len;
  }
  else {
    if (document_line < 0) {
      document_line = 0;
      document_col = 0;
    }

    fedit->cursor.line = document_line;
    if (document_col <= 0) {
      fedit->cursor.col = 0;
    }
    else {
      if (document_col > fedit->code.line_tokens[document_line]->len) {
        fedit->cursor.col = fedit->code.line_tokens[document_line]->len;
      }
      else {
        fedit->cursor.col = document_col;
      }
    }
  }

  // printf("Cursor placed at {%i,%i}\n", fedit->cursor.line, fedit->cursor.col);
  fedit->cursor.visible = true;
  mca_set_node_requires_rerender(fedit->node);
}

// void mce_insert_new_line_at_cursor(mce_function_editor *fedit)
// {
//   mce_source_token_list *line_list = fedit->code.lines.items[fedit->lines.display_index_offset + fedit->cursor.line];

//   int accumulate_line_len = 0;
//   bool inserted = false;
//   for (int a = 0; a < line_list->count; ++a) {
//     mce_source_token *next = line_list->items[a];

//     if (fedit->cursor.col == accumulate_line_len) {
//     }
//     else if (fedit->cursor.col < accumulate_line_len + next->str->len) {
// // Split

//       continue;
//     }
//     else {
//       accumulate_line_len += next->str->len;
//     }
//   }

//   mce_source_token_list *new_line_list;
//   mce_obtain_source_token_list_from_pool(function_editor->source_editor_pool, &new_line_list);
//   insert_in_collection((void ***)&function_editor->code.lines.items, &function_editor->code.lines.capacity,
//                        &function_editor->code.lines.count, fedit->cursor.line, new_line_list);
//   line_token_list->count = 0;
//   line_token_list->line_len = 0;
// }

// Given cursor can define beginning or end of selection, this function returns the absolute bounds of that selection
// @returns 1 if the selection length is 0, otherwise 0
int _mce_obtain_function_editor_selection_bounds(mce_function_editor *fedit, int *start_line, int *start_col,
                                                 int *end_line, int *end_col)
{
  if (fedit->selection.line < fedit->cursor.line) {
    *start_line = fedit->selection.line;
    *start_col = fedit->selection.col;
    *end_line = fedit->cursor.line;
    *end_col = fedit->cursor.col;
  }
  else {
    *start_line = fedit->cursor.line;
    *end_line = fedit->selection.line;

    if (*start_line == *end_line) {
      if (fedit->cursor.col > fedit->selection.col) {
        *start_col = fedit->selection.col;
        *end_col = fedit->cursor.col;
      }
      else if (fedit->cursor.col == fedit->selection.col) {
        // Nothing to delete
        return 1;
      }
      else {
        *start_col = fedit->cursor.col;
        *end_col = fedit->selection.col;
      }
    }
    else {
      *start_col = fedit->cursor.col;
      *end_col = fedit->selection.col;
    }
  }

  return 0;
}

void mce_delete_selection(mce_function_editor *fedit)
{
  // // MCerror(8421, "TODO");
  fedit->selection.exists = false;

  // Obtain the selection bounds
  // -- cursor can define either the start or the end of the selection
  int start_line, start_col, end_line, end_col;
  int zero_size_selection =
      _mce_obtain_function_editor_selection_bounds(fedit, &start_line, &start_col, &end_line, &end_col);
  if (zero_size_selection)
    return;
  int in_between_line_count = end_line - start_line - 1;

  printf("start_line=%i, start_col=%i, end_line=%i, end_col=%i\n", start_line, start_col, end_line, end_col);

  // DEBUG
  {
    for (int a = start_line; a < fedit->code.count; ++a) {
      mce_source_token *ttoken = fedit->code.line_tokens[a]->first;
      printf("mdsBefore[%i]:", a);
      while (ttoken) {
        printf("#");
        printf("%s(%i)", ttoken->str->text, ttoken->type);
        ttoken = ttoken->next;
      }
      printf("#\n");
    }
  }
  // DEBUG

  // Delete from the first line
  mce_source_line_token *start_line_token = fedit->code.line_tokens[start_line];
  mce_source_token *before_token, *del_token;
  int accumulate_line_len = 0;
  int delete_char_count = (end_line > start_line ? start_line_token->len : end_col) - start_col;
  if (start_col == 0) {
    before_token = NULL;
    del_token = start_line_token->first;
  }
  else {
    before_token = del_token = start_line_token->first;
    while (accumulate_line_len + del_token->str->len < start_col) {
      accumulate_line_len += del_token->str->len;
      del_token = del_token->next;
      if (!del_token) {
        break;
      }
    }

    printf("del_token->str->len:%i start_col:%i accumulate_line_len:%i delete_char_count:%i\n", del_token->str->len,
           start_col, accumulate_line_len, delete_char_count);
    if (del_token && start_col > accumulate_line_len) {
      // Delete a section of the tokens string
      int delete_amount = del_token->str->len - (start_col - accumulate_line_len);
      if (start_line == end_line && delete_char_count < delete_amount) {
        delete_amount = delete_char_count;
      }
      delete_char_count -= delete_amount;

      char *remaining = NULL;
      if (del_token->str->len > (start_col - accumulate_line_len) + delete_amount) {
        remaining = strdup(del_token->str->text + start_col - accumulate_line_len + delete_amount);
      }
      printf("stra:'%s'\n", del_token->str->text);
      mc_restrict_str(del_token->str, start_col - accumulate_line_len);
      printf("strb:'%s'\n", del_token->str->text);
      if (remaining) {
        mc_append_to_str(del_token->str, remaining);
        free(remaining);
      }
      printf("strc:'%s'\n", del_token->str->text);
      del_token = del_token->next;
    }
  }

  // Delete what remains required from the start line
  mce_source_token *ttoken;
  mce_source_line_token *tline;
  printf("mds-3a: delete_char_count:%i\n", delete_char_count);
  while (del_token && (delete_char_count > 0 || end_line > start_line)) {
    if (del_token->str->len <= delete_char_count) {
      printf("mds-3b: %i del_token->str->len:%u %s\n", del_token->type, del_token->str->len, del_token->str->text);
      delete_char_count -= del_token->str->len;
      ttoken = del_token->next;

      // Delete the token fully
      append_to_collection((void ***)&fedit->source_editor_pool->source_tokens.items,
                           &fedit->source_editor_pool->source_tokens.capacity,
                           &fedit->source_editor_pool->source_tokens.count, del_token);
      del_token = ttoken;
      continue;
    }

    // Delete the first section of the token
    del_token->type = MCE_SE_UNPROCESSED_TEXT;
    char *remaining = strdup(del_token->str->text + delete_char_count);
    mc_set_str(del_token->str, remaining);
    free(remaining);
    break;
  }

  printf("mds-4\n");
  if (end_line > start_line) {
    printf("mds-4a in_between_line_count:%i\n", in_between_line_count);

    // Delete any in-between lines
    tline = start_line_token;
    for (int n = 0; n < in_between_line_count; ++n) {
      tline = tline->next;
      del_token = tline->first;
      while (del_token) {
        // Delete the token fully
        append_to_collection((void ***)&fedit->source_editor_pool->source_tokens.items,
                             &fedit->source_editor_pool->source_tokens.capacity,
                             &fedit->source_editor_pool->source_tokens.count, del_token);
        del_token = del_token->next; // WATCH - if pool return method is changed, this needs reconfiguring
      }

      tline->prev->next = tline->next;
      if (tline->next) {
        tline->next->prev = tline->prev;
      }

      free(tline);
    }

    // Delete the last line
    del_token = tline->next->first;

    tline->next = tline->next->next;
    if (tline->next) {
      tline->next->prev = tline;
    }
    printf("mds-4d tline->len:%u delete_char_count=%i\n", tline->len, delete_char_count);
    printf("mds-4f del_token:(%i)'%s'\n", del_token->type, del_token->str->text);

    while (delete_char_count > 0) {
      if (del_token->str->len <= delete_char_count) {
        delete_char_count -= del_token->str->len;
        ttoken = del_token->next;

        // Delete the token fully
        append_to_collection((void ***)&fedit->source_editor_pool->source_tokens.items,
                             &fedit->source_editor_pool->source_tokens.capacity,
                             &fedit->source_editor_pool->source_tokens.count, del_token);
        del_token = ttoken;
        printf("loop4\n");
        continue;
      }

      // Delete the first section of the token
      del_token->type = MCE_SE_UNPROCESSED_TEXT;
      char *remaining = strdup(del_token->str->text + delete_char_count);
      printf("mds-4h remaining:'%s'\n", remaining);
      mc_set_str(del_token->str, remaining);
      free(remaining);

      break;
    }
  }
  printf("mds-7 before_token:%p del_token:%p\n", before_token, del_token);

  if (before_token) {
    printf("mds-7a before_token:'%s'\n", before_token->str->text);
    before_token->next = del_token;
  }
  else {
    start_line_token->first = del_token;
  }
  printf("mds-7b del_token:'%s'\n", del_token ? del_token->str->text : "(nil)");

  // Recount the start line
  tline = fedit->code.line_tokens[start_line];
  unsigned int *line_len = &tline->len;
  *line_len = 0U;
  ttoken = tline->first;
  while (ttoken) {
    *line_len += ttoken->str->len;
    ttoken = ttoken->next;
  }

  if (end_line > start_line) {
    // Remove line token range
    for (int n = end_line + 1; n < fedit->code.count; ++n) {
      fedit->code.line_tokens[n - end_line + start_line] = fedit->code.line_tokens[n];
    }

    fedit->code.count -= end_line - start_line;
  }

  fedit->cursor.line = start_line;
  fedit->cursor.col = start_col;

  // DEBUG
  {
    for (int a = start_line; a < fedit->code.count; ++a) {
      ttoken = fedit->code.line_tokens[a]->first;
      if (a > 0 && fedit->code.line_tokens[a]->prev != fedit->code.line_tokens[a - 1]) {
        MCVerror(6733, "TODO:%i", a);
      }
      if (a + 1 < fedit->code.count && fedit->code.line_tokens[a]->next != fedit->code.line_tokens[a + 1]) {
        MCVerror(6746, "TODO:%i", a);
      }

      printf("mdsAFter[%i]:", a);
      while (ttoken) {
        printf("#");
        printf("%s(%i)", ttoken->str->text, ttoken->type);
        ttoken = ttoken->next;
      }
      printf("#\n");
    }
  }
  // DEBUG

  _mce_update_function_editor_visible_source_lines(fedit);
}

int mce_obtain_source_token_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token **token)
{
  if (!source_editor_pool->source_tokens.count) {
    *token = (mce_source_token *)malloc(sizeof(mce_source_token));
    mc_alloc_str(&(*token)->str);
  }
  else {
    --source_editor_pool->source_tokens.count;
    *token = source_editor_pool->source_tokens.items[source_editor_pool->source_tokens.count];
  }

  return 0;
}

void mce_insert_string_at_cursor(mce_function_editor *fedit, const char *str)
{
  // Sort Argument
  if (*str == '\0') {
    return;
  }

  int cursor_start_line = fedit->cursor.line;
  mce_source_line_token *line = fedit->code.line_tokens[cursor_start_line];

  const char *c = str;
  mce_source_token *token = line->first;
  if (!token && *c != '\n') {
    mce_obtain_source_token_from_pool(fedit->source_editor_pool, &token);
    line->first = token;

    token->type = MCE_SE_UNPROCESSED_TEXT;
    mc_set_str(token->str, "");
    token->next = NULL;
  }
  int accumulate_line_len = 0;
  while (1) {
    if (accumulate_line_len + token->str->len < fedit->cursor.col) {
      accumulate_line_len += token->str->len;
      token = token->next;
      if (!token) {
        MCVerror(9422, "TODO");
      }
      continue;
    }

    break;
  }

  if (accumulate_line_len + token->str->len > fedit->cursor.col) {
    int offset_in_next_str = fedit->cursor.col - accumulate_line_len;

    mce_source_token *second;
    mce_obtain_source_token_from_pool(fedit->source_editor_pool, &second);
    second->type = MCE_SE_UNPROCESSED_TEXT;
    mc_set_str(second->str, token->str->text + offset_in_next_str);

    token->type = MCE_SE_UNPROCESSED_TEXT;
    mc_restrict_str(token->str, offset_in_next_str);

    second->next = token->next;
    token->next = second;
  }

  bool altered_line_order = false;

  while (*c != '\0') {
    if (*c == '\n') {
      // Form a new line with the remainder tokens
      mce_source_line_token *new_line = (mce_source_line_token *)calloc(sizeof(mce_source_line_token), 1);
      new_line->next = line->next;
      if (line->next) {
        line->next->prev = new_line;
      }
      new_line->prev = line;
      line->next = new_line;

      ++fedit->cursor.line;
      fedit->cursor.col = 0;
      altered_line_order = true;

      mce_obtain_source_token_from_pool(fedit->source_editor_pool, &new_line->first);
      new_line->first->type = MCE_SE_UNPROCESSED_TEXT;
      mc_set_str(new_line->first->str, "");

      if (token) {
        new_line->first->next = token->next;
        token->next = NULL;
      }

      line = new_line;
      token = line->first;
    }
    else {
      mc_append_char_to_str(token->str, *c);
      ++fedit->cursor.col;
    }

    // Increment
    ++c;
  }

  // Recount
  // TODO-- recounts everything from start line onwards -- make it recount only lines that were added-to/created
  fedit->code.count = cursor_start_line;
  line = fedit->code.line_tokens[cursor_start_line];
  while (line) {
    token = line->first;
    line->len = 0;
    while (token) {
      line->len += token->str->len;
      token = token->next;
    }

    append_to_collection((void ***)&fedit->code.line_tokens, &fedit->code.capacity, &fedit->code.count, line);
    line = line->next;
  }

  _mce_update_function_editor_visible_source_lines(fedit);
}

void mce_read_text_from_function_editor(mce_function_editor *fedit, char **code)
{
  mc_str *str;
  mc_alloc_str(&str);

  mce_source_line_token *line = fedit->code.first_line;
  mce_source_token *token;

  while (line) {
    token = line->first;
    while (token) {
      mc_append_to_str(str, token->str->text);

      token = token->next;
    }

    line = line->next;
  }

  *code = str->text;
  mc_release_str(str, false);
}

void _mce_function_editor_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mce_function_editor_handle_input %p %p\n", node, input_event);
  mce_function_editor *fedit = (mce_function_editor *)node->data;
  input_event->handled = true;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    // printf("obb\n");
    if (input_event->button_code == MOUSE_BUTTON_LEFT) {

      // printf("left_click:offset=%i %.3f  line_index:%i\n", input_event->input_state->mouse.y,
      // node->layout->__bounds.y,
      //        (int)((input_event->input_state->mouse.y - node->layout->__bounds.y - fedit->lines.padding.top) /
      //              fedit->lines.vertical_stride));
      int line_index = -1;
      int click_relative_y =
          input_event->input_state->mouse.y - (int)(node->layout->__bounds.y + fedit->lines.padding.top);
      if (click_relative_y >= 0) {
        line_index = (int)((float)click_relative_y / fedit->lines.vertical_stride);
      }
      if (line_index >= 0) {
        // Find the column index
        int click_relative_x =
            input_event->input_state->mouse.x -
            (int)(node->layout->__bounds.x + fedit->lines.padding.left - fedit->font_horizontal_stride * 0.5f);
        if (click_relative_x < 0 && click_relative_x > -3)
          click_relative_x = 0;
        if (click_relative_x >= 0) {

          mce_set_function_editor_cursor_position(fedit, line_index,
                                                  (int)((float)click_relative_x / fedit->font_horizontal_stride));
        }
      }
    }

    mca_focus_node(node);
  }
  else if (input_event->type == INPUT_EVENT_KEY_RELEASE) {
    // printf("obc %i\n", input_event->input_state->ctrl_function);
    // printf("function-editor: Key_Release: %i\n", input_event->button_code);

    input_event->handled = true;

    if (input_event->input_state->alt_function & BUTTON_STATE_DOWN) {
      switch (input_event->button_code) {
      case KEY_CODE_A:
        mce_move_cursor_home(fedit);
        break;
      default:
        break;
      }
    }
    else if (input_event->input_state->ctrl_function & BUTTON_STATE_DOWN) {
      switch (input_event->button_code) {
      case KEY_CODE_L:
        mce_move_cursor_right(fedit);
        break;
      case KEY_CODE_J:
        mce_move_cursor_left(fedit);
        break;
      case KEY_CODE_I:
        mce_move_cursor_up(fedit);
        break;
      case KEY_CODE_K:
        mce_move_cursor_down(fedit);
        break;
      case KEY_CODE_SEMI_COLON:
        mce_move_cursor_end(fedit);
        break;
      case KEY_CODE_S: {
        char *code;
        mce_read_text_from_function_editor(fedit, &code);
        printf("code:\n%s||\n", code);

        MCVerror(5859, "progress");
      } break;
      default:
        break;
      }
    }
    else {
      switch (input_event->button_code) {
      case KEY_CODE_ENTER: {
        char c[2];
        c[0] = '\n';
        c[1] = '\0';
        mce_insert_string_at_cursor(fedit, c);
      } break;
      case KEY_CODE_HOME: {
        mce_move_cursor_home(fedit);
      } break;
      case KEY_CODE_END: {
        mce_move_cursor_end(fedit);
      } break;
      case KEY_CODE_BACKSPACE: {
        // return;
        if (fedit->selection.exists) {
          MCVerror(7651, "TODO");
        }

        if (fedit->cursor.line == 0 && fedit->cursor.col == 0) {
          // Already at start of document
          break;
        }

        // Delete the character behind the cursor
        fedit->selection.exists = true;
        if (fedit->cursor.col == 0) {
          fedit->selection.line = fedit->cursor.line - 1;
          fedit->selection.col = fedit->code.line_tokens[fedit->selection.line]->len;
        }
        else {
          fedit->selection.line = fedit->cursor.line;
          fedit->selection.col = fedit->cursor.col - 1;
        }

        mce_delete_selection(fedit);
      } break;
      case KEY_CODE_DELETE: {
        // return;
        if (fedit->selection.exists) {
          MCVerror(7651, "TODO");
        }

        // TODO -- bounds checking ?? code.count etc
        if (fedit->cursor.line == fedit->code.count - 1 &&
            fedit->cursor.col == fedit->code.line_tokens[fedit->cursor.line]->len) {
          // Already at end of document
          break;
        }

        // Delete the character next from the cursor
        fedit->selection.exists = true;
        if (fedit->cursor.col == fedit->code.line_tokens[fedit->cursor.line]->len) {
          fedit->selection.line = fedit->cursor.line + 1;
          fedit->selection.col = 0;
        }
        else {
          fedit->selection.line = fedit->cursor.line;
          fedit->selection.col = fedit->cursor.col + 1;
        }

        printf("delete\n");
        mce_delete_selection(fedit);
      } break;
      default: {
        char c[2];
        c[1] = '\0';
        int res = get_key_input_code_char((input_event->input_state->shift_function & BUTTON_STATE_DOWN),
                                          (mc_key_code)input_event->button_code, &c[0]);

        if (!res) {
          // printf("print string '%s'\n", c);
          mce_insert_string_at_cursor(fedit, c);
        }
      }
      }
      // printf("obd %i\n", input_event->type);
    }
  }
}

int mce_init_function_editor(mc_node *parent_node, mce_source_editor_pool *source_editor_pool,
                             mce_function_editor **p_function_editor)
{
  mce_function_editor *function_editor = (mce_function_editor *)malloc(sizeof(mce_function_editor));
  MCcall(mca_init_mc_node(NODE_TYPE_FUNCTION_EDITOR, "function_editor", &function_editor->node));
  function_editor->node->data = function_editor;

  function_editor->source_editor_pool = source_editor_pool;
  function_editor->lines.vertical_stride = 22.f;
  function_editor->lines.padding.left = 6.f;
  function_editor->lines.padding.top = 18.f;
  // TODO
  function_editor->font_horizontal_stride = 9.2794f;

  // Layout
  mca_init_node_layout(&function_editor->node->layout);
  mca_node_layout *layout = function_editor->node->layout;
  layout->determine_layout_extents = (void *)&_mce_determine_function_editor_extents;
  layout->update_layout = (void *)&_mce_update_function_editor_layout;
  layout->render_headless = (void *)&_mce_render_function_editor_headless;
  layout->render_present = (void *)&_mce_render_function_editor_present;
  layout->handle_input_event = (void *)&_mce_function_editor_handle_input;

  // layout->preferred_width = 980;
  layout->preferred_height = 720;
  layout->padding.left = 20;
  layout->padding.top = 20;
  layout->padding.right = 520;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  function_editor->background_color = COLOR_NEARLY_BLACK;
  function_editor->border.color = COLOR_GHOST_WHITE;
  function_editor->border.thickness = 2U;

  function_editor->node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  function_editor->node->children->alloc = 0;
  function_editor->node->children->count = 0;

  function_editor->code.capacity = 0;
  function_editor->code.count = 0;
  function_editor->code.first_line = NULL;

  // mc_alloc_str(&function_editor->code.rtf);
  // function_editor->code.syntax = NULL;

  function_editor->lines.count = 0;
  function_editor->lines.capacity = 0;
  function_editor->lines.display_index_offset = 0;

  *p_function_editor = function_editor;

  MCcall(mca_attach_node_to_hierarchy(parent_node, function_editor->node));

  return 0;
}

void _mce_set_function_editor_code_with_plain_text(mce_function_editor *fedit, const char *code)
{
  // printf("_mce_set_function_editor_code_with_plain_text\n");

  mce_source_line_token *line_tk = fedit->code.first_line;
  if (!line_tk) {
    line_tk = (mce_source_line_token *)calloc(sizeof(mce_source_line_token), 1);
    fedit->code.first_line = line_tk;
    append_to_collection((void ***)&fedit->code.line_tokens, &fedit->code.capacity, &fedit->code.count, line_tk);
  }

  fedit->cursor.zen_col = 0;
  fedit->cursor.line = 0;
  fedit->cursor.col = 0;

  const char *c = code;
  while (1) {
    line_tk->len = 0;
    if (!line_tk->first) {
      mce_obtain_source_token_from_pool(fedit->source_editor_pool, &line_tk->first);
      mc_set_str(line_tk->first->str, "");
      line_tk->first->next = NULL;
    }

    mce_source_token *token = line_tk->first;
    token->type = MCE_SE_UNPROCESSED_TEXT;
    while (*c != '\n' && *c != '\0') {
      mc_append_char_to_str(token->str, *c);
      ++line_tk->len;
      ++c;

      if (token->next) {
        mc_throw_delayed_error(1271, "TODO--mempool", 2020, 10, 28);

        token->next = NULL;
      }
    }

    if (*c == '\0') {
      break;
    }
    ++c;

    // printf("%s\n", line_tk->first->str->text);

    if (!line_tk->next) {
      line_tk->next = (mce_source_line_token *)calloc(sizeof(mce_source_line_token), 1);
      line_tk->next->prev = line_tk;
      line_tk = line_tk->next;
      append_to_collection((void ***)&fedit->code.line_tokens, &fedit->code.capacity, &fedit->code.count, line_tk);
    }
  }

  _mce_update_function_editor_visible_source_lines(fedit);
}

// Do not call directly, prefer calling through mce_activate_source_editor_for_definition
int _mce_set_definition_to_function_editor(mce_function_editor *function_editor, function_info *function)
{
  // Set
  function_editor->function = function;

  // printf("'%p' function->source=%p\n", function, function->source);
  _mce_set_function_editor_code_with_plain_text(function_editor, function->source->code);

  // TODO -- queue up an asynchronous semantic highlighting and information keepsake parsing of the text

  // // Parse
  // int result = mcs_parse_definition_to_syntax_tree(function->source->code, &function_editor->code.syntax);
  // if (result) {
  //   // printf("cees-4\n");
  //   printf("PARSE_ERROR:8154\n");
  //   // mc_pprintf(&function_editor->status_bar.message, "ERR[%i]: read console output", result);
  //   return 0;
  // }
  // else {

  //   printf("function_editor: loaded %s(...)\n", function->name);
  //   // print_syntax_node(code_syntax, 0);
  //   // mc_pprintf(&function_editor->status_bar.message, "loaded %s(...)", function->name);
  //   // function_editor->status_bar.requires_render_update = true;
  // }
  // // printf("cees-7\n");

  // Reset State
  function_editor->lines.display_index_offset = 0;
  function_editor->cursor.line = 0;
  function_editor->cursor.col = 0;

  function_editor->selection.exists = false;

  // mce_convert_syntax_to_rtf(function_editor->code.rtf, function_editor->code.syntax);
  // printf("code.rtf:\n%s||\n", function_editor->code.rtf->text);

  mca_focus_node(function_editor->node);

  return 0;
}