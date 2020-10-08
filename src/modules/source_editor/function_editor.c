#include "control/mc_controller.h"

#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "modules/app_modules.h"
#include "modules/source_editor/source_editor.h"
#include "render/render_common.h"

void mce_move_cursor_up(mce_function_editor *fedit)
{
  // Get the line length
  if (fedit->code.lines.count < fedit->cursor.line) {
    MCerror(9114, "TODO");
  }

  if (fedit->cursor.line + fedit->lines.display_offset_index > 0) {
    if (!fedit->cursor.zen_col) {
      fedit->cursor.zen_col = fedit->cursor.col;
    }

    --fedit->cursor.line;

    int line_len = fedit->code.lines.items[fedit->cursor.line + fedit->lines.display_offset_index]->line_len;
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
  // Get the line length
  if (fedit->code.lines.count < fedit->cursor.line) {
    MCerror(9114, "TODO");
  }

  if (fedit->cursor.line + fedit->lines.display_offset_index + 1 < fedit->code.lines.count) {
    if (!fedit->cursor.zen_col) {
      fedit->cursor.zen_col = fedit->cursor.col;
    }

    ++fedit->cursor.line;

    int line_len = fedit->code.lines.items[fedit->lines.display_offset_index + fedit->cursor.line]->line_len;
    if (fedit->cursor.zen_col <= line_len) {
      fedit->cursor.col = fedit->cursor.zen_col;
    }
    else {
      fedit->cursor.col = line_len;
    }
  }
  else {
    fedit->cursor.col = fedit->code.lines.items[fedit->code.lines.count - 1]->line_len;
  }

  mca_set_node_requires_rerender(fedit->node);
}

void mce_move_cursor_left(mce_function_editor *fedit)
{
  // Get the line length
  if (fedit->code.lines.count < fedit->cursor.line) {
    MCerror(9114, "TODO");
  }

  fedit->cursor.zen_col = 0;

  // Decrement the cursor col
  if (fedit->cursor.col == 0) {
    if (fedit->cursor.line > 0) {
      --fedit->cursor.line;
      fedit->cursor.col = fedit->code.lines.items[fedit->lines.display_offset_index + fedit->cursor.line]->line_len;
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
  // Get the line length
  if (fedit->code.lines.count < fedit->cursor.line) {
    MCerror(9114, "TODO");
  }

  fedit->cursor.zen_col = 0;

  // Determine how the cursor will move
  mce_source_token_list *line_token_list =
      fedit->code.lines.items[fedit->lines.display_offset_index + fedit->cursor.line];

  // Increment the cursor col right
  if (fedit->cursor.col >= line_token_list->line_len) {
    if (fedit->cursor.line + 1 >= fedit->code.lines.count) {
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
  if (fedit->lines.display_offset_index) {
    MCerror(9924, "TODO");
  }
  // preferred_line -= fedit->lines.display_offset_index;
  // if (preferred_line + fedit->lines.display_offset_index < 0)
  //   preferred_line = 0;
  // if (preferred_line +fedit->lines.display_offset_index >= fedit->code.lines.count) {
  //   preferred_line
  // }

  // Determine how the cursor will move
  if (document_line >= fedit->code.lines.count) {
    fedit->cursor.line = fedit->code.lines.count - 1 - fedit->lines.display_offset_index;
    fedit->cursor.col = fedit->code.lines.items[fedit->code.lines.count - 1]->line_len;
  }
  else {
    if (document_line < 0) {
      document_line = 0;
      document_col = 0;
    }

    fedit->cursor.line = document_line - fedit->lines.display_offset_index;
    if (document_col <= 0) {
      fedit->cursor.col = 0;
    }
    else {
      if (document_col > fedit->code.lines.items[document_line]->line_len) {
        fedit->cursor.col = fedit->code.lines.items[document_line]->line_len;
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

int _mce_update_line_details(mce_function_editor *fedit, mc_rectf *available_area)
{
  int y_index = 0;

  while (y_index * fedit->lines.vertical_stride < available_area->height) {

    // Obtain the line control
    mce_source_line *line;
    if (y_index >= fedit->lines.count) {
      // Construct a new one
      mce_init_source_line(fedit->node, &line);
      append_to_collection((void ***)&fedit->lines.items, &fedit->lines.capacity, &fedit->lines.count, line);
    }
    else {
      line = fedit->lines.items[y_index];
    }

    // Set line layout
    printf("set line %i visible true\n", y_index);
    line->node->layout->padding = {fedit->lines.padding.left,
                                   fedit->lines.padding.top + fedit->lines.vertical_stride * y_index, 0.f, 0.f};
    line->node->layout->preferred_height = fedit->lines.vertical_stride;

    // Attach line source (if it exists)
    if (y_index + fedit->lines.display_offset_index < fedit->code.lines.count) {
      line->source_list = fedit->code.lines.items[y_index + fedit->lines.display_offset_index];
      line->node->layout->visible = line->source_list->count;
    }
    else {
      line->source_list = NULL;
      line->node->layout->visible = false;
    }

    // Continue
    ++y_index;
  }

  for (int a = y_index; a < fedit->lines.count; ++a) {
    // Dont render the rest of the lines
    fedit->lines.items[a]->node->layout->visible = false;
    printf("set line %i visible false\n", a);
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
  _mce_update_line_details(function_editor, &node->layout->__bounds);

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
    mcr_issue_render_command_text(image_render_queue,
                                  (unsigned int)(node->layout->__bounds.x + fedit->lines.padding.left +
                                                 fedit->font_horizontal_stride * ((float)fedit->cursor.col - 0.5f)),
                                  (unsigned int)(node->layout->__bounds.y + fedit->lines.padding.top +
                                                 fedit->lines.vertical_stride * fedit->cursor.line),
                                  "|", 0U, cursor_color);
  }
}

void _mce_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mce_handle_input %p %p\n", node, input_event);
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

    if ((input_event->input_state->ctrl_function & BUTTON_STATE_DOWN) && input_event->button_code == KEY_CODE_L) {
      mce_move_cursor_right(fedit);
    }
    if ((input_event->input_state->ctrl_function & BUTTON_STATE_DOWN) && input_event->button_code == KEY_CODE_J) {
      mce_move_cursor_left(fedit);
    }
    if ((input_event->input_state->ctrl_function & BUTTON_STATE_DOWN) && input_event->button_code == KEY_CODE_I) {
      mce_move_cursor_up(fedit);
    }
    if ((input_event->input_state->ctrl_function & BUTTON_STATE_DOWN) && input_event->button_code == KEY_CODE_K) {
      mce_move_cursor_down(fedit);
    }
  }
  // printf("obd %i\n", input_event->type);
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
  layout->handle_input_event = (void *)&_mce_handle_input;

  // layout->preferred_width = 980;
  layout->preferred_height = 720;
  layout->padding.left = 20;
  layout->padding.top = 20;
  layout->padding.right = 520;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  function_editor->background_color = COLOR_NEARLY_BLACK;
  function_editor->node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  function_editor->node->children->alloc = 0;
  function_editor->node->children->count = 0;

  function_editor->code.lines.capacity = 0;
  function_editor->code.lines.count = 0;

  // init_c_str(&function_editor->code.rtf);
  // function_editor->code.syntax = NULL;

  function_editor->lines.count = 0;
  function_editor->lines.capacity = 0;
  function_editor->lines.display_offset_index = 0;

  *p_function_editor = function_editor;
}

// int _mce_convert_syntax_node_to_rtf(c_str *rtf, mc_syntax_node *syntax_node)
// {
//   // printf("csntr-0\n");
//   if ((mc_token_type)syntax_node->type < MC_TOKEN_EXCLUSIVE_MAX_VALUE) {

//     // printf("csntr-2\n");
//     // RTF - prepend
//     switch ((mc_token_type)syntax_node->type) {
//     case MC_TOKEN_LINE_COMMENT:
//       append_to_c_str(rtf, "[color=22,162,18]");
//       break;
//     default:
//       break;
//     }
//     // printf("csntr-3\n");

//     // Content
//     int s = 0;
//     for (int i = 0;; ++i) {
//       if (syntax_node->text[i] == '\0') {
//         if (i - s) {
//           append_to_c_strn(rtf, syntax_node->text + s, i - s);
//         }
//         break;
//       }
//       else if (syntax_node->text[i] == '[') {
//         // Copy to this point
//         append_to_c_strn(rtf, syntax_node->text + s, i - s);
//         // MCcall(append_to_c_str(rtf, "["));
//         append_to_c_str(rtf, "[[");
//         s = i + 1;
//       }
//     }
//     // printf("csntr-6\n");

//     // RTF - append
//     switch ((mc_token_type)syntax_node->type) {
//     case MC_TOKEN_LINE_COMMENT:
//       append_to_c_str(rtf, "[color=156,219,253]");
//       break;
//     default:
//       break;
//     }

//     // printf("csntr-7\n");
//     return 0;
//   }

//   // printf("csntr-8\n");
//   // Syntax Node -- convert each child
//   for (int a = 0; a < syntax_node->children->count; ++a) {
//     mc_syntax_node *child = syntax_node->children->items[a];

//     _mce_convert_syntax_node_to_rtf(rtf, child);
//   }
//   // printf("csntr-9\n");

//   return 0;
// }

// int mce_convert_semantic_tokens_to_rtf(c_str *code_rtf, mc_syntax_node *syntax_node)
// {
//   const char *DEFAULT_CODE_COLOR = "[color=156,219,253]";

//   set_c_str(code_rtf, DEFAULT_CODE_COLOR);
//   _mce_convert_syntax_node_to_rtf(code_rtf, syntax_node);

//   return 0;
// }

void _mce_update_function_editor_line_displays(mce_function_editor *function_editor)
{
  // printf("_mce_update_function_editor_line_displays\n");
  function_editor->lines.utilized = 0;

  int line = 0;
  printf("function_editor->lines.count:%u \n", function_editor->lines.count);
  for (; line < function_editor->lines.count; ++line) {
    int code_line_index = line + function_editor->lines.display_offset_index;

    printf("code_line_index:%u displayoffsetindex:%u \n", code_line_index, function_editor->lines.display_offset_index);
    if (code_line_index > function_editor->code.lines.count) {
      break;
    }

    // Set
    mce_source_line *source_line = function_editor->lines.items[line];
    mce_source_token_list *line_token_list = function_editor->code.lines.items[code_line_index];

    source_line->node->layout->visible = true;
    // printf("set line %i visible true\n", line);

    printf("setting line:%i with %i source_tokens\n", line, line_token_list->count);
    source_line->source_list = line_token_list;
  }

  // Mark the rest of the lines invisible
  for (; line < function_editor->lines.count; ++line) {
    printf("setting line:%i invisible\n", line);
    function_editor->lines.items[line]->node->layout->visible = false;
  }
}

void _mce_obtain_source_token_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token **token)
{
  if (!source_editor_pool->source_tokens.count) {
    *token = (mce_source_token *)malloc(sizeof(mce_source_token));
    init_c_str(&(*token)->str);
    return;
  }

  --source_editor_pool->source_tokens.count;
  *token = source_editor_pool->source_tokens.items[source_editor_pool->source_tokens.count];
}

void _mce_obtain_source_token_list_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token_list **list)
{
  if (!source_editor_pool->source_token_lists.count) {
    *list = (mce_source_token_list *)malloc(sizeof(mce_source_token_list));
    (*list)->capacity = 0U;
    (*list)->count = 0U;
    return;
  }

  --source_editor_pool->source_token_lists.count;
  *list = source_editor_pool->source_token_lists.items[source_editor_pool->source_token_lists.count];
}

void _mce_return_source_token_lists_to_editor_pool(mce_source_editor_pool *source_editor_pool,
                                                   mce_source_token_list **lists, unsigned int count)
{
  for (int a = 0; a < count; ++a) {
    mce_source_token_list *list = lists[a];

    for (int b = 0; b < list->count; ++b) {
      // Return the source tokens to the pool
      append_to_collection((void ***)&source_editor_pool->source_tokens.items,
                           &source_editor_pool->source_tokens.capacity, &source_editor_pool->source_tokens.count,
                           list->items[b]);
    }

    // Return the list to the pool
    append_to_collection((void ***)&source_editor_pool->source_token_lists.items,
                         &source_editor_pool->source_token_lists.capacity,
                         &source_editor_pool->source_token_lists.count, list);
  }
}

void _mce_set_function_editor_lines_with_plain_text(mce_function_editor *function_editor, const char *code)
{
  // printf("_mce_set_function_editor_lines_with_plain_text\n");
  // Clear previous lines and return items to the pool
  for (int a = 0; a < function_editor->code.lines.count; ++a) {
    _mce_return_source_token_lists_to_editor_pool(function_editor->source_editor_pool,
                                                  function_editor->code.lines.items, function_editor->code.lines.count);
  }
  function_editor->code.lines.count = 0;

  int i = 0, s;
  while (code[i] != '\0') {

    mce_source_token_list *line_token_list;
    _mce_obtain_source_token_list_from_pool(function_editor->source_editor_pool, &line_token_list);
    append_to_collection((void ***)&function_editor->code.lines.items, &function_editor->code.lines.capacity,
                         &function_editor->code.lines.count, line_token_list);
    line_token_list->count = 0;
    line_token_list->line_len = 0;

    // printf("code:(%i):'%s'\n", i, code);

    while (code[i] != '\0' && code[i] != '\n') {
      s = i;

      mce_source_token *token;
      _mce_obtain_source_token_from_pool(function_editor->source_editor_pool, &token);
      append_to_collection((void ***)&line_token_list->items, &line_token_list->capacity, &line_token_list->count,
                           token);

      // Set token
      switch (code[i]) {
      case ' ': {
        while (1) {
          if (code[i] == ' ') {
            ++i;
          }
          else if (code[i] == '\t') {
            i += 2;
          }
          else
            break;
        }

        token->type = MCE_SRC_EDITOR_EMPTY;
        set_c_str(token->str, ""); // TODO a set_c_strn(str, 'c', times_num)?
        for (; s < i; ++s)
          append_to_c_str(token->str, " ");
      } break;
      default: {
        bool loop = true;
        while (loop) {
          ++i;
          switch (code[i]) {
          case ' ':
          case '\t':
          case '\n':
          case '\0':
            loop = false;
            break;
          default:
            break;
          }
        }

        token->type = MCE_SRC_EDITOR_NON_SEMANTIC_TEXT;
        set_c_str(token->str, ""); // TODO a set_c_strn?
        append_to_c_strn(token->str, code + s, i - s);

      } break;
      }

      // Add to the line total length
      line_token_list->line_len += token->str->len;
    }

    if (code[i] == '\n') {
      ++i;
    }
  }

  // printf("function_editor->code.lines.count:%i\n", function_editor->code.lines.count);
  _mce_update_function_editor_line_displays(function_editor);
}

// Do not call directly, prefer calling through mce_activate_source_editor_for_definition
int _mce_set_definition_to_function_editor(mce_function_editor *function_editor, function_info *function)
{
  // Set
  function_editor->function = function;

  _mce_set_function_editor_lines_with_plain_text(function_editor, function->source->code);

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

  // TODO -- initially set to somewhere in the code - end of first or last line or something (visibility trumps?)
  function_editor->lines.display_offset_index = 0;
  function_editor->cursor.line = 0;
  function_editor->cursor.col = 0;

  // mce_convert_syntax_to_rtf(function_editor->code.rtf, function_editor->code.syntax);
  // printf("code.rtf:\n%s||\n", function_editor->code.rtf->text);

  mca_focus_node(function_editor->node);

  return 0;
}