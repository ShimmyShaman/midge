
#include "control/mc_controller.h"
#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "modules/app_modules.h"
#include "modules/source_editor/source_editor.h"
#include "render/render_common.h"

// void move_cursor_right(mcm_function_editor *state)
// {
//   // Adjust the cursor index & col
//   char *code = state->code.rtf->text;

//   int i = state->cursor.rtf_index;

//   if (code[i] == '\0') {
//     return;
//   }

//   state->cursor.zen_col = 0;

//   print_parse_error(code, i, "mcu-initial", "");
//   // Move
//   while (code[i] == '[') {
//     if (code[i + 1] == '[') {
//       ++i;
//       break;
//     }

//     while (code[i] != ']') {
//       ++i;
//     }
//     ++i;
//   }
//   if (code[i] == '\0') {
//     --i;
//   }
//   ++i;
//   // Move
//   while (code[i] == '[') {
//     if (code[i + 1] == '[') {
//       ++i;
//       break;
//     }

//     while (code[i] != ']') {
//       ++i;
//     }
//     ++i;
//   }
//   if (code[i] == '\0') {
//     --i;
//   }
//   print_parse_error(code, i, "mcu-end", "");

//   state->cursor.rtf_index = i;
//   update_code_editor_cursor_line_and_column(state);

//   // Update the cursor visual
//   state->cursor.requires_render_update = true;
//   state->visual_node->data.visual.requires_render_update = true;

//   // Adjust display offset
//   if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
//     // Move display offset down
//     state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
//   }
// }

int _mcm_update_line_details(mcm_function_editor *fedit, mc_rectf *available_area)
{
  int y_index = 0;

  while (y_index * fedit->lines.vertical_stride < available_area->height) {

    // Obtain the line control
    mcm_source_line *line;
    if (y_index >= fedit->lines.count) {
      // Construct a new one
      mcm_init_source_line(fedit->node, &line);
      append_to_collection((void ***)&fedit->lines.items, &fedit->lines.capacity, &fedit->lines.count, line);
    }
    else {
      line = fedit->lines.items[y_index];
    }

    // Set line layout
    line->node->layout->visible = true;
    line->node->layout->padding = {fedit->lines.padding.left,
                                   fedit->lines.padding.top + fedit->lines.vertical_stride * y_index, 0.f, 0.f};
    line->node->layout->preferred_height = fedit->lines.vertical_stride;

    // Attach line source (if it exists)
    if (y_index + fedit->lines.display_offset_index < fedit->code.lines.count) {
      line->source_list = fedit->code.lines.items[y_index + fedit->lines.display_offset_index];
    }
    else
      line->source_list = NULL;

    // Continue
    ++y_index;
  }

  for (int a = y_index; a < fedit->lines.count; ++a) {
    // Dont render the rest of the lines
    fedit->lines.items[a]->node->layout->visible = false;
  }

  return 0;
}

void _mcm_determine_function_editor_extents(mc_node *node, layout_extent_restraints restraints)
{
  // const float MAX_EXTENT_VALUE = 100000.f;

  // mcm_function_editor *function_editor = (mcm_function_editor *)node->data;

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

void _mcm_update_function_editor_layout(mc_node *node, mc_rectf *available_area)
{
  mcm_function_editor *function_editor = (mcm_function_editor *)node->data;

  mca_update_typical_node_layout(node, available_area);
  // printf("function_editor-available %.3f %.3f %.3f*%.3f\n", available_area->x, available_area->y, available_area->width,
  //        available_area->height);
  // printf("function_editor-padding %.3f %.3f %.3f*%.3f\n", node->layout->padding.left, node->layout->padding.top,
  //        node->layout->padding.right, node->layout->padding.bottom);
  // printf("function_editor-bounds %.3f %.3f %.3f*%.3f\n", node->layout->__bounds.x, node->layout->__bounds.y,
  //        node->layout->__bounds.width, node->layout->__bounds.height);

  // Align text lines to fit to the container
  _mcm_update_line_details(function_editor, &node->layout->__bounds);

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

void _mcm_render_function_editor_headless(mc_node *node)
{
  mcm_function_editor *function_editor = (mcm_function_editor *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }
}

void _mcm_render_function_editor_present(image_render_details *image_render_queue, mc_node *node)
{
  mcm_function_editor *fedit = (mcm_function_editor *)node->data;

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, fedit->background_color);
  //   printf("_mcm_render_function_editor_present %u %u %u %u\n", (unsigned int)node->layout->__bounds.x,
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

void mcm_set_function_editor_cursor_position(mcm_function_editor *fedit, int preferred_line, int preferred_col)
{
  // fedit->cursor.rtf_index = 0;

  // const char *code = fedit->code.rtf;

  // // Find the start of the new line
  // int line = 0, col = 0, rtf_index;
  // for (;; ++rtf_index) {
  //   if (code[i] == '\0') {
  //     --i;
  //     break;
  //   }
  //   if (code[i] == '\n') {
  //     ++line;
  //     if (line < preferred_line)
  //       break;
  //   }
  // }

  // if (line < preferred_line)
  //   preferred_col = INT32_MAX;

  // // Move to the preferred column
  // for (int c = 0; c < preferred_col; ++c) {
  //   ++i;
  //   if (code[i] == '\0') {
  //     --i;
  //     break;
  //   }
  //   if (code[i] == '\n') {
  //     break;
  //   }
  // }

  // fedit->cursor.rtf_index = i;

  // // printf("Cursor should be placed at {%i,%i}\n", line_index, column_index);
  // fedit->cursor.visible = true;
  // mca_set_node_requires_rerender(node);
}

void _mcm_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mcm_handle_input %p %p\n", node, input_event);
  mcm_function_editor *fedit = (mcm_function_editor *)node->data;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    printf("obb\n");
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
      if (line_index) {
        // Find the column index
        int click_relative_x =
            input_event->input_state->mouse.x - (int)(node->layout->__bounds.x - fedit->lines.padding.left + 0.4f);
        if (click_relative_x < 0 && click_relative_x > -3)
          click_relative_x = 0;
        if (click_relative_x >= 0) {

          mcm_set_function_editor_cursor_position(fedit, line_index,
                                                  (int)((float)click_relative_x / fedit->font_horizontal_stride));
        }
      }
    }

    input_event->handled = true;
  }
  else if (input_event->type == INPUT_EVENT_KEY_RELEASE) {
    printf("obc\n");
    printf("function-editor: Key_Release: %i\n", input_event->button_code);

    input_event->handled = true;
  }
  printf("obd %i\n", input_event->type);
}

void mcm_init_function_editor(mc_node *parent_node, mcm_source_editor_pool *source_editor_pool,
                              mcm_function_editor **p_function_editor)
{
  mcm_function_editor *function_editor = (mcm_function_editor *)malloc(sizeof(mcm_function_editor));
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
  layout->determine_layout_extents = (void *)&_mcm_determine_function_editor_extents;
  layout->update_layout = (void *)&_mcm_update_function_editor_layout;
  layout->render_headless = (void *)&_mcm_render_function_editor_headless;
  layout->render_present = (void *)&_mcm_render_function_editor_present;
  layout->handle_input_event = (void *)&_mcm_handle_input;

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

// int _mcm_convert_syntax_node_to_rtf(c_str *rtf, mc_syntax_node *syntax_node)
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

//     _mcm_convert_syntax_node_to_rtf(rtf, child);
//   }
//   // printf("csntr-9\n");

//   return 0;
// }

// int mcm_convert_semantic_tokens_to_rtf(c_str *code_rtf, mc_syntax_node *syntax_node)
// {
//   const char *DEFAULT_CODE_COLOR = "[color=156,219,253]";

//   set_c_str(code_rtf, DEFAULT_CODE_COLOR);
//   _mcm_convert_syntax_node_to_rtf(code_rtf, syntax_node);

//   return 0;
// }

void _mcm_update_function_editor_line_displays(mcm_function_editor *function_editor)
{
  // printf("_mcm_update_function_editor_line_displays\n");
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
    mcm_source_line *source_line = function_editor->lines.items[line];
    mcm_source_token_list *line_token_list = function_editor->code.lines.items[code_line_index];

    source_line->node->layout->visible = true;

    printf("setting line:%i with %i source_tokens\n", line, line_token_list->count);
    source_line->source_list = line_token_list;
  }

  // Mark the rest of the lines invisible
  for (; line < function_editor->lines.count; ++line) {
    printf("setting line:%i invisible\n", line);
    function_editor->lines.items[line]->node->layout->visible = false;
  }
}

void _mcm_obtain_source_token_from_pool(mcm_source_editor_pool *source_editor_pool, mcm_source_token **token)
{
  if (!source_editor_pool->source_tokens.count) {
    *token = (mcm_source_token *)malloc(sizeof(mcm_source_token));
    init_c_str(&(*token)->str);
    return;
  }

  --source_editor_pool->source_tokens.count;
  *token = source_editor_pool->source_tokens.items[source_editor_pool->source_tokens.count];
}

void _mcm_obtain_source_token_list_from_pool(mcm_source_editor_pool *source_editor_pool, mcm_source_token_list **list)
{
  if (!source_editor_pool->source_token_lists.count) {
    *list = (mcm_source_token_list *)malloc(sizeof(mcm_source_token_list));
    (*list)->capacity = 0U;
    (*list)->count = 0U;
    return;
  }

  --source_editor_pool->source_token_lists.count;
  *list = source_editor_pool->source_token_lists.items[source_editor_pool->source_token_lists.count];
}

void _mcm_return_source_token_lists_to_editor_pool(mcm_source_editor_pool *source_editor_pool,
                                                   mcm_source_token_list **lists, unsigned int count)
{
  for (int a = 0; a < count; ++a) {
    mcm_source_token_list *list = lists[a];

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

void _mcm_set_function_editor_lines_with_plain_text(mcm_function_editor *function_editor, const char *code)
{
  // printf("_mcm_set_function_editor_lines_with_plain_text\n");
  // Clear previous lines and return items to the pool
  for (int a = 0; a < function_editor->code.lines.count; ++a) {
    _mcm_return_source_token_lists_to_editor_pool(function_editor->source_editor_pool,
                                                  function_editor->code.lines.items, function_editor->code.lines.count);
  }
  function_editor->code.lines.count = 0;

  int i = 0, s;
  while (code[i] != '\0') {

    mcm_source_token_list *line_token_list;
    _mcm_obtain_source_token_list_from_pool(function_editor->source_editor_pool, &line_token_list);
    append_to_collection((void ***)&function_editor->code.lines.items, &function_editor->code.lines.capacity,
                         &function_editor->code.lines.count, line_token_list);
    line_token_list->count = 0;

    // printf("code:(%i):'%s'\n", i, code);

    while (code[i] != '\0' && code[i] != '\n') {
      s = i;

      mcm_source_token *token;
      _mcm_obtain_source_token_from_pool(function_editor->source_editor_pool, &token);
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

        token->type = MCM_SRC_EDITOR_EMPTY;
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

        token->type = MCM_SRC_EDITOR_NON_SEMANTIC_TEXT;
        set_c_str(token->str, ""); // TODO a set_c_strn?
        append_to_c_strn(token->str, code + s, i - s);

      } break;
      }
    }

    if (code[i] == '\n') {
      ++i;
    }
  }

  // printf("function_editor->code.lines.count:%i\n", function_editor->code.lines.count);
  _mcm_update_function_editor_line_displays(function_editor);
}

// Do not call directly, prefer calling through mca_activate_source_editor_for_definition
int _mcm_set_definition_to_function_editor(mcm_function_editor *function_editor, function_info *function)
{
  // Set
  function_editor->function = function;

  _mcm_set_function_editor_lines_with_plain_text(function_editor, function->source->code);

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

  // mcm_convert_syntax_to_rtf(function_editor->code.rtf, function_editor->code.syntax);
  // printf("code.rtf:\n%s||\n", function_editor->code.rtf->text);

  mca_focus_node(function_editor->node);

  return 0;
}