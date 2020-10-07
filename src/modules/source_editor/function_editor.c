
#include "control/mc_controller.h"
#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "modules/app_modules.h"
#include "render/render_common.h"

void move_cursor_right(mc_code_editor_state_v1 *state)
{
  // Adjust the cursor index & col
  char *code = state->code.rtf->text;
  int i = state->cursor.rtf_index;

  if (code[i] == '\0') {
    return;
  }

  state->cursor.zen_col = 0;

  print_parse_error(code, i, "mcu-initial", "");
  // Move
  while (code[i] == '[') {
    if (code[i + 1] == '[') {
      ++i;
      break;
    }

    while (code[i] != ']') {
      ++i;
    }
    ++i;
  }
  if (code[i] == '\0') {
    --i;
  }
  ++i;
  // Move
  while (code[i] == '[') {
    if (code[i + 1] == '[') {
      ++i;
      break;
    }

    while (code[i] != ']') {
      ++i;
    }
    ++i;
  }
  if (code[i] == '\0') {
    --i;
  }
  print_parse_error(code, i, "mcu-end", "");

  state->cursor.rtf_index = i;
  update_code_editor_cursor_line_and_column(state);

  // Update the cursor visual
  state->cursor.requires_render_update = true;
  state->visual_node->data.visual.requires_render_update = true;

  // Adjust display offset
  if (state->cursor.line >= state->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES) {
    // Move display offset down
    state->line_display_offset = state->cursor.line - CODE_EDITOR_RENDERED_CODE_LINES + 1;
  }
}

int _mcm_update_line_details(mcm_function_editor *fedit, mc_rectf *available_area)
{
  int y_index = 0;

  int rtf_index = 0;

  char buf[512];

  while (y_index * fedit->lines.vertical_stride < available_area->height && rtf_index < fedit->code.rtf->len) {

    // Obtain the line control
    mcm_source_line *line;
    if (y_index >= fedit->lines.count) {
      // Construct a new one
      mcm_init_source_line(fedit->node, &line);
      append_to_collection((void ***)&fedit->lines.items, &fedit->lines.alloc, &fedit->lines.count, line);
    }
    else {
      line = fedit->lines.items[y_index];
    }

    // Set line layout
    line->node->layout->visible = true;
    line->node->layout->padding = {fedit->lines.padding.left,
                                   fedit->lines.padding.top + fedit->lines.vertical_stride * y_index, 0.f, 0.f};
    line->node->layout->preferred_height = fedit->lines.vertical_stride;

    // Obtain the line rtf
    const char *code = fedit->code.rtf->text;
    int bi = 0;
    while (rtf_index < fedit->code.rtf->len && code[rtf_index] != '\n') {

      buf[bi++] = fedit->code.rtf->text[rtf_index];

      ++rtf_index;
    }
    ++rtf_index;
    buf[bi] = '\0';
    // printf("bufbi:'%s'\n", buf);
    set_c_str(line->rtf, buf);

    // Continue
    ++y_index;
  }

  for (int a = y_index; a < fedit->lines.count; ++a) {
    // Dont render the rest of the lines
    fedit->lines.items[a]->node->layout->visible = false;
  }

  // const int CODE_EDITOR_RENDERED_CODE_LINES = 12;
  // // Copies the rtf text to the rendered lines
  // // printf("urt-0\n");
  // char *code = function_editor->code.rtf->text;
  // int i = 0;
  // int s = i;
  // int line_index = 0;
  // int max_line_index = function_editor->line_display_offset + CODE_EDITOR_RENDERED_CODE_LINES;
  // char *color_from_previous_line = NULL;
  // char *color_at_end_of_current_line = NULL;
  // for (; i < function_editor->code.rtf->len && line_index < max_line_index;) {
  //   // Obtain the line rtf
  //   bool eof = false;
  //   // printf("urt-1\n");
  //   for (;; ++i) {
  //     if (code[i] == '\0') {
  //       eof = true;
  //       break;
  //     }
  //     else if (code[i] == '\n') {
  //       break;
  //     }
  //     else if (code[i] == '[') {
  //       if (code[i + 1] == '[') {
  //         ++i;
  //         continue;
  //       }
  //       else if (!strncmp(code + i, "[color=", 7)) {
  //         int j = i + 7;
  //         for (;; ++j) {
  //           if (code[j] == ']') {
  //             ++j;
  //             break;
  //           }
  //         }
  //         if (color_at_end_of_current_line) {
  //           free(color_at_end_of_current_line);
  //         }
  //         allocate_and_copy_cstrn(color_at_end_of_current_line, code + i, j - i);
  //         i = j - 1;
  //         continue;
  //       }
  //       print_parse_error(code, i, "mce_update_rendered_text", "Unhandled rtf?");
  //       MCerror(1465, "Unhandled");
  //     }
  //   }
  //   // register_midge_error_tag("mce_update_rendered_text-4");
  //   // printf("urt-4\n");

  //   if (line_index >= function_editor->line_display_offset) {
  //     rendered_code_line *rendered_code_line =
  //         function_editor->render_lines[line_index - function_editor->line_display_offset];
  //     rendered_code_line->visible = true;

  //     bool text_differs;
  //     if (color_from_previous_line) {
  //       int potential_line_len = (color_from_previous_line ? strlen(color_from_previous_line) : 0) + i - s;
  //       text_differs =
  //           rendered_code_line->rtf->len != potential_line_len ||
  //           strncmp(rendered_code_line->rtf->text, color_from_previous_line, strlen(color_from_previous_line)) ||
  //           strncmp(rendered_code_line->rtf->text + strlen(color_from_previous_line), code + s, s - i);
  //     }
  //     else {
  //       text_differs =
  //           rendered_code_line->rtf->len != (i - s) || strncmp(rendered_code_line->rtf->text, code + s, s - i);
  //     }

  //     if (text_differs) {
  //       // printf("urt-5a\n");
  //       // printf("line-%i was:'%s'\n", line_index - function_editor->line_display_offset,
  //       // rendered_code_line->rtf->text);

  //       // Set previous rtf settings
  //       MCcall(set_c_str(rendered_code_line->rtf, ""));
  //       if (color_from_previous_line) {
  //         MCcall(append_to_c_str(rendered_code_line->rtf, color_from_previous_line));
  //       }

  //       // printf("urt-6\n");
  //       // Set the line text
  //       MCcall(append_to_c_strn(rendered_code_line->rtf, code + s, i - s));
  //       rendered_code_line->requires_render_update = true;
  //       function_editor->visual_node->data.visual.requires_render_update = true;

  //       // printf("line-%i now:'%s'\n", line_index - function_editor->line_display_offset,
  //       // rendered_code_line->rtf->text);
  //     }
  //   }
  //   // register_midge_error_tag("mce_update_rendered_text-7");
  //   // printf("urt-8\n");

  //   ++line_index;
  //   if (eof) {
  //     break;
  //   }
  //   ++i; // Past the new-line
  //   s = i;

  //   if (color_at_end_of_current_line) {
  //     if (color_from_previous_line) {
  //       free(color_from_previous_line);
  //     }
  //     color_from_previous_line = color_at_end_of_current_line;
  //     color_at_end_of_current_line = NULL;
  //   }
  //   // printf("urt-9\n");
  // }

  // // Turn off visibility of the rest of the lines
  // if (line_index - function_editor->line_display_offset < 0) {
  //   line_index -= line_index - function_editor->line_display_offset;
  // }
  // for (; line_index - function_editor->line_display_offset < CODE_EDITOR_RENDERED_CODE_LINES; ++line_index) {
  //   rendered_code_line *rendered_code_line =
  //       function_editor->render_lines[line_index - function_editor->line_display_offset];
  //   rendered_code_line->visible = false;
  // }

  // register_midge_error_tag("mce_update_rendered_text(~)");
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
  printf("function_editor-available %.3f %.3f %.3f*%.3f\n", available_area->x, available_area->y, available_area->width,
         available_area->height);
  printf("function_editor-padding %.3f %.3f %.3f*%.3f\n", node->layout->padding.left, node->layout->padding.top,
         node->layout->padding.right, node->layout->padding.bottom);
  printf("function_editor-bounds %.3f %.3f %.3f*%.3f\n", node->layout->__bounds.x, node->layout->__bounds.y,
         node->layout->__bounds.width, node->layout->__bounds.height);

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
          fedit->cursor.line = line_index;
          fedit->cursor.col = (int)((float)click_relative_x / fedit->font_horizontal_stride);
          // printf("Cursor should be placed at {%i,%i}\n", line_index, column_index);
          fedit->cursor.visible = true;
          mca_set_node_requires_rerender(node);
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

  init_c_str(&function_editor->code.rtf);
  function_editor->code.syntax = NULL;

  function_editor->lines.count = 0;
  function_editor->lines.alloc = 0;
  function_editor->line_display_offset = 0;

  *p_function_editor = function_editor;
}

int _mcm_convert_syntax_node_to_rtf(c_str *rtf, mc_syntax_node *syntax_node)
{
  // printf("csntr-0\n");
  if ((mc_token_type)syntax_node->type < MC_TOKEN_EXCLUSIVE_MAX_VALUE) {

    // printf("csntr-2\n");
    // RTF - prepend
    switch ((mc_token_type)syntax_node->type) {
    case MC_TOKEN_LINE_COMMENT:
      append_to_c_str(rtf, "[color=22,162,18]");
      break;
    default:
      break;
    }
    // printf("csntr-3\n");

    // Content
    int s = 0;
    for (int i = 0;; ++i) {
      if (syntax_node->text[i] == '\0') {
        if (i - s) {
          append_to_c_strn(rtf, syntax_node->text + s, i - s);
        }
        break;
      }
      else if (syntax_node->text[i] == '[') {
        // Copy to this point
        append_to_c_strn(rtf, syntax_node->text + s, i - s);
        // MCcall(append_to_c_str(rtf, "["));
        append_to_c_str(rtf, "[[");
        s = i + 1;
      }
    }
    // printf("csntr-6\n");

    // RTF - append
    switch ((mc_token_type)syntax_node->type) {
    case MC_TOKEN_LINE_COMMENT:
      append_to_c_str(rtf, "[color=156,219,253]");
      break;
    default:
      break;
    }

    // printf("csntr-7\n");
    return 0;
  }

  // printf("csntr-8\n");
  // Syntax Node -- convert each child
  for (int a = 0; a < syntax_node->children->count; ++a) {
    mc_syntax_node *child = syntax_node->children->items[a];

    _mcm_convert_syntax_node_to_rtf(rtf, child);
  }
  // printf("csntr-9\n");

  return 0;
}

int mcm_convert_syntax_to_rtf(c_str *code_rtf, mc_syntax_node *syntax_node)
{
  const char *DEFAULT_CODE_COLOR = "[color=156,219,253]";

  set_c_str(code_rtf, DEFAULT_CODE_COLOR);
  _mcm_convert_syntax_node_to_rtf(code_rtf, syntax_node);

  return 0;
}

int _mcm_set_definition_to_function_editor(mcm_function_editor *function_editor, function_info *function)
{
  // Set
  function_editor->function = function;

  // Parse
  int result = parse_definition_to_syntax_tree(function->source->code, &function_editor->code.syntax);
  if (result) {
    // printf("cees-4\n");
    printf("PARSE_ERROR:8154\n");
    // mc_pprintf(&function_editor->status_bar.message, "ERR[%i]: read console output", result);
    return 0;
  }
  else {

    printf("function_editor: loaded %s(...)\n", function->name);
    // print_syntax_node(code_syntax, 0);
    // mc_pprintf(&function_editor->status_bar.message, "loaded %s(...)", function->name);
    // function_editor->status_bar.requires_render_update = true;
  }
  // printf("cees-7\n");

  mcm_convert_syntax_to_rtf(function_editor->code.rtf, function_editor->code.syntax);
  // printf("code.rtf:\n%s||\n", function_editor->code.rtf->text);

  mca_focus_node(function_editor->node);

  return 0;
}