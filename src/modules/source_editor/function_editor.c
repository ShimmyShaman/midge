#include "control/mc_controller.h"

#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "modules/app_modules.h"
#include "modules/source_editor/source_editor.h"
#include "render/render_common.h"

void _mce_update_function_editor_line_start_tokens(mce_function_editor *function_editor)
{
  // printf("_mce_update_function_editor_line_start_tokens\n");
  function_editor->lines.utilized = 0;

  printf("function_editor->lines.count:%u \n", function_editor->lines.count);
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
    source_line->initial_token = function_editor->code.line_initial_tokens[code_line_index];
    mca_set_node_requires_rerender(source_line->node);
  }
}

int _mce_update_line_positions(mce_function_editor *fedit, mc_rectf *available_area)
{
  int y_index = 0;

  bool line_count_changed = false;
  while (y_index * fedit->lines.vertical_stride < available_area->height) {

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
    line->node->layout->padding = {fedit->lines.padding.left,
                                   fedit->lines.padding.top + fedit->lines.vertical_stride * y_index, 0.f, 0.f};
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
    _mce_update_function_editor_line_start_tokens(fedit);
  }

  return 0;
}

void _mce_determine_function_editor_extents(mc_node *node, layout_extent_restraints restraints)
{
  // const float MAX_EXTENT_VALUE = 100000.f;

  // mce_function_editor *function_editor = (mce_function_editor *)node->data;

  // Width
  if (node->layout->preferred_width) {
    node->layout->determined_extents.width = node->layout->preferred_width;
  }
  else {
    // MCerror(7295, "NotYetSupported");
  }

  // Height
  if (node->layout->preferred_height) {
    node->layout->determined_extents.height = node->layout->preferred_height;
  }
  else {
    MCerror(7301, "NotYetSupported");
  }
}

void _mce_update_function_editor_layout(mc_node *node, mc_rectf *available_area)
{
  mce_function_editor *function_editor = (mce_function_editor *)node->data;

  mca_update_typical_node_layout(node, available_area);
  // printf("function_editor-available %.3f %.3f %.3f*%.3f\n", available_area->x, available_area->y,
  // available_area->width,
  //        available_area->height);
  // printf("function_editor-padding %.3f %.3f %.3f*%.3f\n", node->layout->padding.left, node->layout->padding.top,
  //        node->layout->padding.right, node->layout->padding.bottom);
  // printf("function_editor-bounds %.3f %.3f %.3f*%.3f\n", node->layout->__bounds.x, node->layout->__bounds.y,
  //        node->layout->__bounds.width, node->layout->__bounds.height);

  // Align text lines to fit to the container
  _mce_update_line_positions(function_editor, &node->layout->__bounds);

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->update_layout) {
      // TODO fptr casting
      void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
      update_layout(child, &node->layout->__bounds);
    }
  }

  node->layout->__requires_layout_update = false;

  // Set rerender anyway because lazy TODO--maybe
  mca_set_node_requires_rerender(node);
}

void _mce_render_function_editor_headless(mc_node *node)
{
  mce_function_editor *function_editor = (mce_function_editor *)node->data;

  struct timespec debug_start_time, debug_end_time;
  clock_gettime(CLOCK_REALTIME, &debug_start_time);
  int debug_count = 0;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
      ++debug_count;
    }
  }

  clock_gettime(CLOCK_REALTIME, &debug_end_time);
  printf("FunctionEditorHeadless: rendered %i children took %.2fms\n", debug_count,
         1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
             1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
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

  if (fedit->cursor.visible) {
    render_color cursor_color = COLOR_GHOST_WHITE;
    mcr_issue_render_command_text(
        image_render_queue,
        (unsigned int)(node->layout->__bounds.x + fedit->lines.padding.left +
                       fedit->font_horizontal_stride * ((float)fedit->cursor.col - 0.5f)),
        (unsigned int)(node->layout->__bounds.y + fedit->lines.padding.top +
                       fedit->lines.vertical_stride * (fedit->cursor.line - fedit->lines.display_index_offset)),
        "|", 0U, cursor_color);
  }

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

    int line_len = fedit->code.line_lengths[fedit->cursor.line];
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

    int line_len = fedit->code.line_lengths[fedit->cursor.line];
    if (fedit->cursor.zen_col <= line_len) {
      fedit->cursor.col = fedit->cursor.zen_col;
    }
    else {
      fedit->cursor.col = line_len;
    }
  }
  else {
    fedit->cursor.col = fedit->code.line_lengths[fedit->code.count - 1];
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
      fedit->cursor.col = fedit->code.line_lengths[fedit->cursor.line];
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
  if (fedit->cursor.col >= fedit->code.line_lengths[fedit->cursor.line]) {
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
    fedit->cursor.col = fedit->code.line_lengths[fedit->cursor.line];
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
      if (document_col > fedit->code.line_lengths[document_line]) {
        fedit->cursor.col = fedit->code.line_lengths[document_line];
      }
      else {
        fedit->cursor.col = document_col;
      }
    }
  }

  printf("Cursor placed at {%i,%i}\n", fedit->cursor.line, fedit->cursor.col);
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
int _mce_obtain_function_editor_selection_bounds(mce_function_editor *fedit, int *start_line, int start_col,
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
  fedit->selection.exists = false;

  // Obtain the selection bounds
  // -- cursor can define either the start or the end of the selection
  int start_line, start_col, end_line, end_col;
  if (_mce_obtain_function_editor_selection_bounds(fedit, &start_line, &start_col, &end_line, &end_col))
    return;

  // Find the first contained source token
  // -- (split to if need be)
  mce_source_token *first_token = fedit->code.initial_line_tokens[start_line], delete_token;
  int accumulate_line = start_line, accumulate_line_len = 0;
  while (1) {
    if (accumulate_line_len + first_token->str->len < start_col) {
      if (first_token->type == MCE_SRC_EDITOR_NEW_LINE) {
        MCerror(8216, "TODO DEBUG");
      }

      accumulate_line_len += first_token->str->len;
      first_token = first_token->next;
      continue;
    }

    if (accumulate_line_len + first_token->str->len > start_col) {
      int offset_in_next_str = start_col - accumulate_line_len;

      // Register previous len
      accumulate_line_len += first_token->str->len;

      // Need to restrict the token down
      first_token->type = MCE_SRC_EDITOR_UNPROCESSED_TEXT;
      restrict_c_str(first_token->str, offset_in_next_str);
    }

    delete_token = first_token->next;
    break;
  }

  // Begin Deleting tokens until end_line/col is reached
  while (1) {
    if (delete_token->type == MCE_SRC_EDITOR_NEW_LINE) {
      MCerror(8216, "TODO DEBUG");
    }
    else {
      if (accumulate_line == end_line && start_col + delete_token->str->len > end_col) {
        // Restrict text in token
        delete_token->type = MCE_SRC_EDITOR_UNPROCESSED_TEXT;
        char *remaining = strdup(delete_token->str->text + end_col - start_col);
        set_c_str(delete_token->str, remaining);
        free(remaining);
        break;
      }
      else {
        // Delete whole token
        if (accumulate_line == end_line && start_col + delete_token->str->len > end_col)
          first_token =

              // Just reappend to the pool collection
              append_to_collection((void ***)&source_editor_pool->source_tokens.items,
                                   &source_editor_pool->source_tokens.capacity,
                                   &source_editor_pool->source_tokens.count, delete_token);
      }
    }
  }
}

void mce_insert_string_at_cursor(mce_function_editor *fedit, const char *str)
{
  // Sort Argument
  int str_len = strlen(str);
  if (!str_len) {
    // Nothing to insert
    return;
  }

  bool contiguous_str_type = true;
  for (int a = 1; a < str_len; ++a) {
    if (str[a - 1] != str[a] && ((str[a - 1] == ' ' || str[a - 1] == '\n') || (str[a] == ' ' || str[a] == '\n'))) {
      contiguous_str_type = false;
      break;
    }
  }

  // Token
  int cursor_start_line = fedit->cursor.line;
  mce_source_token *token = fedit->code.line_initial_tokens[cursor_start_line];

  // DEBUG
  bool DEBUG_SPLIT = false;
  mce_source_token *debug_initial_token = token;
  {
    printf("was:");
    mce_source_token *debug_token = debug_initial_token;
    while (debug_token && debug_token->type != MCE_SRC_EDITOR_NEW_LINE) {

      printf("#%s", debug_token->str->text);

      debug_token = debug_token->next;
    }
    printf("#\n");
  }
  // DEBUG

  int accumulate_line_len = 0;
  while (token) {

    if (accumulate_line_len + token->str->len < fedit->cursor.col) {
      if (token->type == MCE_SRC_EDITOR_NEW_LINE) {
        MCerror(8216, "TODO DEBUG");
      }

      accumulate_line_len += token->str->len;
      token = token->next;
      continue;
    }

    if (accumulate_line_len + token->str->len > fedit->cursor.col) {
      // Need to seperate the token into two
      int offset_in_next_str = fedit->cursor.col - accumulate_line_len;

      mce_source_token *second;
      mce_obtain_source_token_from_pool(fedit->source_editor_pool, &second);
      second->type = MCE_SRC_EDITOR_UNPROCESSED_TEXT;
      set_c_str(second->str, token->str->text + offset_in_next_str);
      second->next = token->next;

      token->type = MCE_SRC_EDITOR_UNPROCESSED_TEXT;
      restrict_c_str(token->str, offset_in_next_str);
      token->next = second;

      printf("spl:");
      mce_source_token *debug_token = debug_initial_token;
      while (debug_token && debug_token->type != MCE_SRC_EDITOR_NEW_LINE) {

        printf("#%s", debug_token->str->text);

        debug_token = debug_token->next;
      }
      printf("#\n");
    }

    if (accumulate_line_len + token->str->len != fedit->cursor.col) {
      MCerror(9242, "DEBUG CHECK");
    }

    // Update render for this line
    int render_line_index = cursor_start_line - fedit->lines.display_index_offset;
    if (render_line_index >= 0 && render_line_index < fedit->lines.count) {
      mca_set_node_requires_rerender(fedit->lines.items[render_line_index]->node);
    }

    // Append to the token what can be appended
    // TODO -- seperate into empty text and non-empty text
    int new_line_count = 0;
    while (*str != '\0') {
      switch (*str) {
      case '\n': {
        // Construct a new-line token
        mce_source_token *new_line;
        mce_obtain_source_token_from_pool(fedit->source_editor_pool, &new_line);
        new_line->type = MCE_SRC_EDITOR_NEW_LINE;
        set_c_str(new_line->str, "\n");

        // Insert
        new_line->next = token->next;
        token->next = new_line;
        ++new_line_count;

        insert_in_collection((void ***)&fedit->code.line_initial_tokens, &fedit->code.capacity, &fedit->code.count,
                             cursor_start_line + new_line_count, new_line->next);

        ++fedit->cursor.line;
        fedit->cursor.col = 0;

        render_line_index = cursor_start_line - fedit->lines.display_index_offset + new_line_count;
        if (render_line_index >= 0 && render_line_index < fedit->lines.count) {
          mca_set_node_requires_rerender(fedit->lines.items[render_line_index]->node);
        }
      } break;
      case ' ':
      default: {
        append_char_to_c_str(token->str, *str);
        ++fedit->cursor.col;
      } break;
      }

      ++str;
    }

    // Recalculate line lengths for affected lines
    if (new_line_count) {
      if (fedit->code.line_lengths_size < fedit->code.count) {
        reallocate_array((void **)&fedit->code.line_lengths, &fedit->code.line_lengths_size, fedit->code.capacity,
                         sizeof(unsigned int));
      }

      for (int n = fedit->code.count + new_line_count - 1; n > cursor_start_line + new_line_count; --n) {
        fedit->code.line_lengths[n] = fedit->code.line_lengths[n - 1];
      }

      _mce_update_function_editor_line_start_tokens(fedit);
    }
    for (int n = cursor_start_line; n <= cursor_start_line + new_line_count; ++n) {
      token = fedit->code.line_initial_tokens[n];

      // Add
      unsigned int line_length = 0;
      while (token && token->type != MCE_SRC_EDITOR_NEW_LINE) {
        line_length += token->str->len;
        token = token->next;
      }

      // Set
      fedit->code.line_lengths[n] = line_length;
      // printf("set line %i length:%u\n", n, line_length);
    }

    // Break from loop
    break;
  }

  {
    printf("now:");
    mce_source_token *debug_token = debug_initial_token;
    while (debug_token && debug_token->type != MCE_SRC_EDITOR_NEW_LINE) {

      printf("#%s", debug_token->str->text);

      debug_token = debug_token->next;
    }
    printf("#\n");
  }

  // mce_set_function_editor_cursor_position(fedit, fedit->cursor.line, fedit->cursor.col + str_len);
  // if (line_list->count) {
  //   fedit->lines.items[fedit->cursor.line]->node->layout->visible = true;
  // }
}

// void mce_insert_string_at_cursor(mce_function_editor *fedit, const char *str)
// {
//   int s = 0;
//   for (int a = 0;; ++a) {
//     if (str[a] == '\0')
//       break;
//     else if (str[a] == '\n') {
//       if (a > s) {
//         char *part = strndup(str + s, a - s);
//         _mce_insert_string_at_cursor(fedit, part);
//         free(part);
//       }

//       // Insert New Line
//       mce_insert_new_line_at_cursor(mce_function_editor * fedit);

//       s = a + 1;
//     }
//   }

//   if (*(str + s) != '\0')
//     _mce_insert_string_at_cursor(fedit, str + s);
// }

void _mce_function_editor_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mce_function_editor_handle_input %p %p\n", node, input_event);
  mce_function_editor *fedit = (mce_function_editor *)node->data;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    // printf("obb\n");
    if (input_event->button_code == MOUSE_BUTTON_LEFT) {

      // printf("left_click:offset=%i %.3f  line_index:%i\n", input_event->input_state->mouse.y,
      // node->layout->__bounds.y,
      //        (int)((input_event->input_state->mouse.y - node->layout->__bounds.y - fedit->lines.padding.top) /
      //              fedit->lines.vertical_stride));
      int line_index = -1;
      int click_relative_y =
          input_event->input_state->mouse.y - (int)(node->layout->__bounds.y - fedit->lines.padding.top);
      if (click_relative_y >= 0) {
        line_index = (int)((float)click_relative_y / fedit->lines.vertical_stride);
      }
      if (line_index >= 0) {
        // Find the column index
        int click_relative_x =
            input_event->input_state->mouse.x - (int)(node->layout->__bounds.x - fedit->lines.padding.left + 0.4f);
        if (click_relative_x < 0 && click_relative_x > -3)
          click_relative_x = 0;
        if (click_relative_x >= 0) {

          mce_set_function_editor_cursor_position(fedit, line_index,
                                                  (int)((float)click_relative_x / fedit->font_horizontal_stride));
        }
      }
    }

    input_event->handled = true;
  }
  else if (input_event->type == INPUT_EVENT_KEY_RELEASE) {
    // printf("obc %i\n", input_event->input_state->ctrl_function);
    // printf("function-editor: Key_Release: %i\n", input_event->button_code);

    input_event->handled = true;

    if (input_event->input_state->ctrl_function & BUTTON_STATE_DOWN) {
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
      default:
        break;
      }
    }
    else {
      switch (input_event->button_code) {
      case KEY_CODE_ENTER: {
        char c[2];
        c[1] = '\0';
        c[0] = '\n';
        mce_insert_string_at_cursor(fedit, c);
      } break;
      case KEY_CODE_BACKSPACE: {
        if (fedit->selection.exists) {
          MCerror(7651, "TODO");
        }

        if (fedit->cursor.line == 0 && fedit->cursor.col == 0) {
          // Already at start of document
          break;
        }

        // Delete the character behind the cursor
        fedit->selection.exists = true;
        if (fedit->cursor.col == 0) {
          fedit->selection.line = fedit->cursor.line - 1;
          fedit->selection.col = fedit->code.line_lengths[fedit->selection.line];
        }
        else {
          fedit->selection.line = fedit->cursor.line;
          fedit->selection.col = fedit->cursor.col - 1;
        }

        mce_delete_selection(fedit);
      } break;
      case KEY_CODE_DELETE: {
        if (fedit->selection.exists) {
          MCerror(7651, "TODO");
        }

        // TODO -- bounds checking ?? code.count etc
        if (fedit->cursor.line == fedit->code.count - 1 &&
            fedit->cursor.col == fedit->code.line_lengths[fedit->cursor.line]) {
          // Already at end of document
          break;
        }

        // Delete the character next from the cursor
        fedit->selection.exists = true;
        if (fedit->cursor.col == fedit->code.line_lengths[fedit->cursor.line]) {
          fedit->selection.line = fedit->cursor.line + 1;
          fedit->selection.col = 0;
        }
        else {
          fedit->selection.line = fedit->cursor.line;
          fedit->selection.col = fedit->cursor.col + 1;
        }

        mce_delete_selection(fedit);
      } break;
      default: {
        char c[2];
        c[1] = '\0';
        int res = get_key_input_code_char((input_event->input_state->shift_function & BUTTON_STATE_DOWN),
                                          (mc_key_code)input_event->button_code, &c[0]);

        if (!res) {
          printf("print string '%s'\n", c);
          mce_insert_string_at_cursor(fedit, c);
        }
      }
      }
      // printf("obd %i\n", input_event->type);
    }
  }
}

void mce_init_function_editor(mc_node *parent_node, mce_source_editor_pool *source_editor_pool,
                              mce_function_editor **p_function_editor)
{
  mce_function_editor *function_editor = (mce_function_editor *)malloc(sizeof(mce_function_editor));
  mca_init_mc_node(parent_node, NODE_TYPE_FUNCTION_EDITOR, &function_editor->node);
  function_editor->node->data = function_editor;

  function_editor->source_editor_pool = source_editor_pool;
  function_editor->lines.vertical_stride = 22.f;
  function_editor->lines.padding.left = 2.f;
  function_editor->lines.padding.top = 4.f;
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
  function_editor->code.line_lengths_size = 0;

  // init_c_str(&function_editor->code.rtf);
  // function_editor->code.syntax = NULL;

  function_editor->lines.count = 0;
  function_editor->lines.capacity = 0;
  function_editor->lines.display_index_offset = 0;

  *p_function_editor = function_editor;
}

void _mce_set_function_editor_code_with_plain_text(mce_function_editor *fedit, const char *code)
{
  // printf("_mce_set_function_editor_code_with_plain_text\n");

  // Use previous token list as much as possible
  mce_source_editor_pool *source_editor_pool = fedit->source_editor_pool;
  mce_source_token *token;
  if (fedit->code.count > 0) {
    token = fedit->code.line_initial_tokens[0];
  }
  else {
    mce_obtain_source_token_from_pool(source_editor_pool, &token);
    token->next = NULL;
  }

  // Reset code storage
  fedit->code.count = 0;

  // Iterate through the given text
  int line_index = 0;
  int i = 0;
  while (code[i] != '\0') {
    append_to_collection((void ***)&fedit->code.line_initial_tokens, &fedit->code.capacity, &fedit->code.count, token);
    if (fedit->code.line_lengths_size < fedit->code.count) {
      reallocate_array((void **)&fedit->code.line_lengths, &fedit->code.line_lengths_size, fedit->code.capacity,
                       sizeof(unsigned int));

      // fedit->code.line

      //     for (int k = 0; k < fedit->code.count - 1; ++k) {
      //       printf("-%u", fedit->code.line_lengths[k]);
      //     }
      //     printf("\n");
    }
    //   // if (fedit->code.count > 1) {
    //   //   printf("line_len:%u = %u\n", fedit->code.count - 2, fedit->code.line_lengths[fedit->code.count - 2]);
    //   // }

    unsigned int *line_len = &fedit->code.line_lengths[fedit->code.count - 1];
    *line_len = 0;
    while (code[i] != '\n' && code[i] != '\0') {
      int s = i;
      switch (code[i]) {
      case '\n':
      case '\0':
        break;
      default: {
        while (code[i] != '\n' && code[i] != '\0') {
          ++i;
        }
        token->type = MCE_SRC_EDITOR_UNPROCESSED_TEXT;
        set_c_strn(token->str, code + s, i - s);
        *line_len = (unsigned int)(*line_len + i - s);

        // Set Next Token
        if (!token->next) {
          mce_obtain_source_token_from_pool(source_editor_pool, &token->next);
          token->next->next = NULL;
        }
        token = token->next;
      } break;
      }
    }

    if (code[i] == '\n') {
      // New-line
      token->type = MCE_SRC_EDITOR_NEW_LINE;
      set_c_str(token->str, "\n");
      ++i;

      // -- Set Next token
      if (!token->next) {
        mce_obtain_source_token_from_pool(source_editor_pool, &token->next);
        token->next->next = NULL;
      }
      token = token->next;
      continue;
    }
    else if (code[i] == '\0') {
      token->type = MCE_SRC_EDITOR_END_OF_FILE;
      set_c_str(token->str, "");

      // Remove any remaining source tokens and return them to the pool
      mce_source_token *return_token = token->next;
      token->next = NULL;
      while (return_token) {
        printf("dHDOULHND TBE HERe\n");
        // Just reappend to the pool collection
        append_to_collection((void ***)&source_editor_pool->source_tokens.items,
                             &source_editor_pool->source_tokens.capacity, &source_editor_pool->source_tokens.count,
                             return_token);

        return_token = return_token->next;
      }
      break;
    }

    //   MCerror(8834, "Flow error");
  }

  printf("fedit->code.count:%i\n", fedit->code.count);
  token = fedit->code.line_initial_tokens[0];
  while (token) {
    printf("token:'%s'\n", token->str->text);
    token = token->next;
  }

  _mce_update_function_editor_line_start_tokens(fedit);
}

// Do not call directly, prefer calling through mce_activate_source_editor_for_definition
int _mce_set_definition_to_function_editor(mce_function_editor *function_editor, function_info *function)
{
  // Set
  function_editor->function = function;

  _mce_set_function_editor_code_with_plain_text(function_editor, function->source->code);

  // TODO -- queue up an asynchronous semantic highlighting and information keepsake parsing of the text

  // // Parse
  // int result = parse_definition_to_syntax_tree(function->source->code, &function_editor->code.syntax);
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