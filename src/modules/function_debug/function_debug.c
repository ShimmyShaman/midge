#include "control/mc_controller.h"

#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "modules/app_modules.h"
#include "modules/source_editor/source_editor.h"
#include "render/render_common.h"

void _mce_determine_function_debug_extents(mc_node *node, layout_extent_restraints restraints)
{
  mca_determine_typical_node_extents(node, restraints);
  //   // const float MAX_EXTENT_VALUE = 100000.f;

  //   // mce_function_debug *function_debug = (mce_function_debug *)node->data;

  //   // Width
  //   if (node->layout->preferred_width) {
  //     node->layout->determined_extents.width = node->layout->preferred_width;
  //   }
  //   else {
  //     // MCerror(7295, "NotYetSupported");
  //   }

  //   // Height
  //   if (node->layout->preferred_height) {
  //     node->layout->determined_extents.height = node->layout->preferred_height;
  //   }
  //   else {
  //     MCerror(7301, "NotYetSupported");
  //   }
}

void _mce_update_function_debug_layout(mc_node *node, mc_rectf *available_area)
{
  mca_update_typical_node_layout(node, available_area);
  //   mce_function_debug *function_debug = (mce_function_debug *)node->data;

  // printf("function_debug-available %.3f %.3f %.3f*%.3f\n", available_area->x, available_area->y,
  // available_area->width,
  //        available_area->height);
  // printf("function_debug-padding %.3f %.3f %.3f*%.3f\n", node->layout->padding.left, node->layout->padding.top,
  //        node->layout->padding.right, node->layout->padding.bottom);
  // printf("function_debug-bounds %.3f %.3f %.3f*%.3f\n", node->layout->__bounds.x, node->layout->__bounds.y,
  //        node->layout->__bounds.width, node->layout->__bounds.height);

  // Align text lines to fit to the container
  //   _mce_update_line_positions(function_debug, &node->layout->__bounds);

  //   // Children
  //   for (int a = 0; a < node->children->count; ++a) {
  //     mc_node *child = node->children->items[a];
  //     if (child->layout && child->layout->update_layout) {
  //       // TODO fptr casting
  //       void (*update_layout)(mc_node *, mc_rectf *) = (void (*)(mc_node *, mc_rectf *))child->layout->update_layout;
  //       update_layout(child, &node->layout->__bounds);
  //     }
  //   }

  //   node->layout->__requires_layout_update = false;

  //   // Set rerender anyway because lazy TODO--maybe
  //   mca_set_node_requires_rerender(node);
}

void _mce_render_function_debug_headless(mc_node *node)
{
  mce_function_debug *function_debug = (mce_function_debug *)node->data;

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
  // printf("FunctionDebugHeadless: rendered %i children took %.2fms\n", debug_count,
  //        1000.f * (debug_end_time.tv_sec - debug_start_time.tv_sec) +
  //            1e-6 * (debug_end_time.tv_nsec - debug_start_time.tv_nsec));
}

void mcr_sprint_variable_report_value(char *buf, mce_function_debug_variable_report *debug_var, int max_chars)
{
  if (debug_var->deref_count) {
    if (debug_var->deref_count == 1) {
      if (!strcmp(debug_var->type_identity, "const char") || !strcmp(debug_var->type_identity, "char")) {
        const char *str = (const char *)debug_var->value;
        if (strlen(str) >= max_chars) {
          for (int i = 0; i + 1 < max_chars; ++i) {
            buf[i] = str[i];
          }
          buf[max_chars - 1] = '\0';
        }
        else {
          sprintf(buf, "%s", str);
        }
      }
      else {
        sprintf(buf, "%p", debug_var->value);
      }
    }
    else {
      sprintf(buf, "%p", debug_var->value);
    }
  }
  else {
    if (!strcmp(debug_var->type_identity, "bool")) {
      sprintf(buf, "%s", (*(bool *)debug_var->value) ? "true" : "false");
    }
    else if (!strcmp(debug_var->type_identity, "window_input_event_type")) {
      sprintf(buf, "%i", *(int *)debug_var->value);
    }
    else {
      sprintf(buf, "unhandled %s TODO-9401", debug_var->type_identity);
    }
  }
}

void _mce_render_function_debug_present(image_render_details *image_render_queue, mc_node *node)
{
  mce_function_debug *fdebug = (mce_function_debug *)node->data;

  mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, fdebug->background_color);
  //   printf("_mce_render_function_debug_present %u %u %u %u\n", (unsigned int)node->layout->__bounds.x,
  //          (unsigned int)node->layout->__bounds.y, (unsigned int)node->layout->__bounds.width,
  //          (unsigned int)node->layout->__bounds.height);

  // Most Recent Variable Values
  if (fdebug->displayed_call) {
    render_color color = {24.f / 255.f, 36.f / 255.f, 64.f / 255.f, 1.f};

    for (int i = 0; i < fdebug->displayed_call->variable_reports.count; ++i) {
      mce_function_debug_variable_report *debug_var = fdebug->displayed_call->variable_reports.items[i];
      // printf("debug_var->length:%i\n", debug_var->length);
      mcr_issue_render_command_colored_quad(
          image_render_queue,
          (unsigned int)(node->layout->__bounds.x + fdebug->lines.padding.left +
                         fdebug->font_horizontal_stride * debug_var->col),
          (unsigned int)(node->layout->__bounds.y + fdebug->lines.padding.top - 3.f + 22.f * debug_var->line),
          (unsigned int)(fdebug->font_horizontal_stride * debug_var->length), 22U, color);
    }
  }

  // // Children
  // for (int a = 0; a < node->children->count; ++a) {
  //   mc_node *child = node->children->items[a];
  //   if (child->layout && child->layout->visible && child->layout->render_present) {
  //     // TODO fptr casting
  //     void (*render_node_presentation)(image_render_details *, mc_node *) =
  //         (void (*)(image_render_details *, mc_node *))child->layout->render_present;
  //     render_node_presentation(image_render_queue, child);
  //   }
  // }
  render_color font_color = COLOR_LIGHT_SKY_BLUE;

  float horizontal_offset = fdebug->lines.padding.left;
  mce_function_debug_line *line = fdebug->code.first_line;
  for (unsigned int y = (unsigned int)(node->layout->__bounds.y + fdebug->lines.padding.top);
       line && y < (unsigned int)(node->layout->__bounds.y + node->layout->__bounds.height); y += 22U) {

    mcr_issue_render_command_text(image_render_queue, (unsigned int)(node->layout->__bounds.x + horizontal_offset), y,
                                  line->str->text, 0, font_color);
    line = line->next;
  }

  //   if (fdebug->cursor.visible) {
  //     render_color cursor_color = COLOR_GHOST_WHITE;
  //     mcr_issue_render_command_text(
  //         image_render_queue,
  //         (unsigned int)(node->layout->__bounds.x + fdebug->lines.padding.left +
  //                        fdebug->font_horizontal_stride * ((float)fdebug->cursor.col - 0.5f)),
  //         (unsigned int)(node->layout->__bounds.y + fdebug->lines.padding.top +
  //                        fdebug->lines.vertical_stride * (fdebug->cursor.line -
  //                        fdebug->lines.display_index_offset)),
  //         "|", 0U, cursor_color);
  //   }

  render_color title_color = COLOR_NODE_ORANGE;
  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)fdebug->lines.padding.top - 4, title_color);

  {
    // Border
    mcr_issue_render_command_colored_quad(
        image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
        (unsigned int)node->layout->__bounds.width, (unsigned int)fdebug->border.thickness, fdebug->border.color);
    mcr_issue_render_command_colored_quad(
        image_render_queue, (unsigned int)node->layout->__bounds.x,
        (unsigned int)node->layout->__bounds.y + fdebug->border.thickness, (unsigned int)fdebug->border.thickness,
        (unsigned int)node->layout->__bounds.height - fdebug->border.thickness, fdebug->border.color);
    mcr_issue_render_command_colored_quad(
        image_render_queue,
        (unsigned int)node->layout->__bounds.x + node->layout->__bounds.width - fdebug->border.thickness,
        (unsigned int)node->layout->__bounds.y + fdebug->border.thickness, (unsigned int)fdebug->border.thickness,
        (unsigned int)node->layout->__bounds.height - fdebug->border.thickness, fdebug->border.color);
    mcr_issue_render_command_colored_quad(
        image_render_queue, (unsigned int)node->layout->__bounds.x + fdebug->border.thickness,
        (unsigned int)node->layout->__bounds.y + node->layout->__bounds.height - fdebug->border.thickness,
        (unsigned int)node->layout->__bounds.width - fdebug->border.thickness * 2,
        (unsigned int)fdebug->border.thickness, fdebug->border.color);
  }

  // TODO render the texture window instead
  if (fdebug->displayed_variable_window.item) {
    render_color back_color = {48.f / 255.f, 52.f / 255.f, 32.f / 255.f, 1.f};

    char buf[128];
    mcr_sprint_variable_report_value(buf, fdebug->displayed_variable_window.item, 64);
    int buf_strlen = strlen(buf);

    unsigned int x_draw_pos =
        (unsigned int)(node->layout->__bounds.x + fdebug->lines.padding.left +
                       fdebug->font_horizontal_stride * (fdebug->displayed_variable_window.item->col + 2));
    unsigned int y_draw_pos = (unsigned int)(node->layout->__bounds.y + fdebug->lines.padding.top +
                                             22.f * (fdebug->displayed_variable_window.item->line + 1));
    mcr_issue_render_command_colored_quad(image_render_queue, x_draw_pos, y_draw_pos,
                                          (unsigned int)(8 + fdebug->font_horizontal_stride * buf_strlen),
                                          (unsigned int)30, back_color);

    font_color = COLOR_GHOST_WHITE;
    mcr_issue_render_command_text(image_render_queue, x_draw_pos + 4, y_draw_pos + 4 + 22U * 0, buf, 0, font_color);
  }
}

void _mce_function_debug_handle_input(mc_node *node, mci_input_event *input_event)
{
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    mca_focus_node(node);

    mce_function_debug *fdebug = (mce_function_debug *)node->data;

    float mouse_x_offset =
        (float)input_event->input_state->mouse.x - node->layout->__bounds.x - fdebug->lines.padding.left;
    float mouse_y_offset =
        (float)input_event->input_state->mouse.y - node->layout->__bounds.y - fdebug->lines.padding.top + 3.f;

    fdebug->displayed_variable_window.item = NULL;
    if (mouse_x_offset >= 0 && mouse_y_offset >= 0 && mouse_x_offset < node->layout->__bounds.width &&
        mouse_y_offset < node->layout->__bounds.height) {
      int mouse_col = (int)(mouse_x_offset / fdebug->font_horizontal_stride);
      int mouse_line = (int)(mouse_y_offset / 22U);

      for (int i = 0; i < fdebug->displayed_call->variable_reports.count; ++i) {
        mce_function_debug_variable_report *var_report = fdebug->displayed_call->variable_reports.items[i];
        if (mouse_line == var_report->line && mouse_col >= var_report->col &&
            mouse_col < var_report->col + var_report->length) {
          // printf("hit %s\n", var_report->name);
          fdebug->displayed_variable_window.item = var_report;
          break;
        }
      }
    }
  }

  // TODO -- asdf
}

void mce_function_debug_update_report_display(mce_function_debug *fdebug)
{
  mca_set_node_requires_rerender(fdebug->node);
}

// // int _mce_report_variable_value()
int _mce_report_variable_value(mct_function_variable_report_index *report_index, unsigned int call_uid,
                               mct_expression_type_info *type_info, const char *variable_name, int line, int col,
                               void *p_value)
{
  // printf("_mce_report_variable_value\n");
  mce_function_debug *fdebug = (mce_function_debug *)report_index->tag_data;

  // Obtain the call report
  mce_function_debug_function_call *function_call = NULL;
  for (int i = 0; i < fdebug->call_reports.count; ++i) {
    if (call_uid == fdebug->call_reports.items[i]->call_uid) {
      function_call = fdebug->call_reports.items[i];
      break;
    }
  }
  if (!function_call) {
    function_call = (mce_function_debug_function_call *)malloc(sizeof(mce_function_debug_function_call));
    function_call->call_uid = call_uid;
    function_call->variable_reports.capacity = 0U;
    function_call->variable_reports.count = 0U;

    append_to_collection((void ***)&fdebug->call_reports.items, &fdebug->call_reports.capacity,
                         &fdebug->call_reports.count, function_call);

    // Set the displayed call if most recent
    if (!fdebug->displayed_call || fdebug->displayed_call->call_uid < call_uid) {
      fdebug->displayed_call = function_call;
    }
  }

  mce_function_debug_variable_report *variable_report =
      (mce_function_debug_variable_report *)malloc(sizeof(mce_function_debug_variable_report));
  variable_report->type_identity = strdup(type_info->type_name);
  variable_report->deref_count = type_info->deref_count;
  variable_report->is_array = type_info->is_array;
  variable_report->name = strdup(variable_name);
  variable_report->line = line;
  variable_report->col = col;
  variable_report->length = strlen(variable_report->name);
  variable_report->value_display_text = NULL;
  // printf("variable_report:%p\n", variable_report);
  append_to_collection((void ***)&function_call->variable_reports.items, &function_call->variable_reports.capacity,
                       &function_call->variable_reports.count, variable_report);
  // printf("variable_report added to function call\n");

  // TODO -- Adjustment that needs to be made because variable col is set at the column of the access operator just
  // before the identifier of any member-access syntax
  {
    for (int c = strlen(variable_report->name) - 1; c > 0; --c) {
      if (variable_report->name[c] == '.') {
        variable_report->col = col - c;
        break;
      }
      else if (variable_report->name[c] == '>' && c > 1) {
        variable_report->col = col - (c - 1);
        break;
      }
    }
  }

  // printf("_mce_report_variable_value:call_uid:'%u'\n", call_uid);
  // printf("_mce_report_variable_value:type_identifier:'%s'\n", type_info->type_name);
  // printf("_mce_report_variable_value:variable_name:'%s'\n", variable_name);
  // printf("_mce_report_variable_value:line:'%i'\n", line);
  // printf("_mce_report_variable_value:col:'%i'\n", col);

  if (type_info->deref_count) {
    if (type_info->deref_count == 1 && !strcmp(type_info->type_name, "const char *")) {
      // printf("FLD:%s:%s[%i]:'%s'\n", variable_name, type_info->type_name, type_info->deref_count,
      //        *(const char **)p_value);
      variable_report->value = strdup(*(const char **)p_value);
    }
    else if (type_info->deref_count == 1 && !strcmp(type_info->type_name, "char *")) {
      // printf("FLD:%s:%s[%i]:'%s'\n", variable_name, type_info->type_name, type_info->deref_count, *(char **)p_value);
      variable_report->value = strdup(*(char **)p_value);
    }
    else {
      // printf("FLD:%s:%s[%i]:value:'%p'\n", variable_name, type_info->type_name, type_info->deref_count,
      //        *(void **)p_value);
      variable_report->value = *(void **)p_value;
    }
  }
  else {
    if (!strcmp(type_info->type_name, "bool")) {
      // printf("FLD:%s:%s[%i]:value:'%s'\n", variable_name, type_info->type_name, type_info->deref_count,
      //        (*(bool *)p_value) ? "true" : "false");
      variable_report->value = (void *)malloc(sizeof(bool));
      *((bool *)variable_report->value) = *(bool *)p_value;
    }
    else if (!strcmp(type_info->type_name, "float")) {
      // printf("FLD:%s:%s[%i]:value:'%f'\n", variable_name, type_info->type_name, type_info->deref_count,
      //        *(float *)p_value);
      variable_report->value = (void *)malloc(sizeof(float));
      *((float *)variable_report->value) = *(float *)p_value;
    }
    else if (!strcmp(type_info->type_name, "int")) {
      // printf("FLD:%s:%s[%i]:value:'%i'\n", variable_name, type_info->type_name, type_info->deref_count,
      //        *(int *)p_value);
      variable_report->value = (void *)malloc(sizeof(int));
      *((int *)variable_report->value) = *(int *)p_value;
    }
    else if (!strcmp(type_info->type_name, "char")) {
      // printf("FLD:%s:%s[%i]:value:'%c'\n", variable_name, type_info->type_name, type_info->deref_count,
      //        *(char *)p_value);
      variable_report->value = (void *)malloc(sizeof(char));
      *((char *)variable_report->value) = *(char *)p_value;
    }
    else if (!strcmp(type_info->type_name, "unsigned int")) {
      // printf("FLD:%s:%s[%i]:value:'%u'\n", variable_name, type_info->type_name, type_info->deref_count,
      //        *(unsigned int *)p_value);
      variable_report->value = (void *)malloc(sizeof(unsigned int));
      *((unsigned int *)variable_report->value) = *(unsigned int *)p_value;
    }
    else {
      enumeration_info *enum_info;
      find_enumeration_info(type_info->type_name, &enum_info);
      if (enum_info) {
        // printf("FLD:%s:%s[%i]:%i\n", variable_name, type_info->type_name, type_info->deref_count, *(int *)p_value);
        variable_report->value = (void *)malloc(sizeof(int));
        *((int *)variable_report->value) = *(int *)p_value;
      }
      else {
        // printf("Do not know how to handle unknown type. FLD:%s::'%s'[%i]\n", variable_name, type_info->type_name,
        //        type_info->deref_count);
        variable_report->value = NULL;
      }
    }
  }

  // printf("/_mce_report_variable_value\n");
  return 0;
}

int _mce_end_report_variable_values(mct_function_variable_report_index *report_index, unsigned int call_uid)
{
  mce_function_debug *fdebug = (mce_function_debug *)report_index->tag_data;

  mce_function_debug_update_report_display(fdebug);
  printf("_mce_end_report_variable_values\n");

  return 0;
}

void mce_init_function_debug_instance(mce_function_debug **p_function_debugger)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mce_function_debug *function_debug = (mce_function_debug *)malloc(sizeof(mce_function_debug));

  mca_init_mc_node(global_data->global_node, NODE_TYPE_ABSTRACT, &function_debug->node);
  function_debug->node->data = function_debug;

  function_debug->lines.vertical_stride = 22.f;
  function_debug->lines.padding.left = 6.f;
  function_debug->lines.padding.top = 18.f;
  // TODO
  function_debug->font_horizontal_stride = 9.2794f;

  // Layout
  mca_init_node_layout(&function_debug->node->layout);
  mca_node_layout *layout = function_debug->node->layout;
  layout->determine_layout_extents = (void *)&_mce_determine_function_debug_extents;
  layout->update_layout = (void *)&_mce_update_function_debug_layout;
  layout->render_headless = (void *)&_mce_render_function_debug_headless;
  layout->render_present = (void *)&_mce_render_function_debug_present;
  layout->handle_input_event = (void *)&_mce_function_debug_handle_input;

  // layout->preferred_width = 980;
  layout->preferred_height = 720;
  layout->padding.left = 120;
  layout->padding.top = 280;
  layout->padding.right = 240;
  layout->padding.bottom = 180;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  function_debug->background_color = COLOR_NEARLY_BLACK;
  function_debug->border.color = COLOR_GHOST_WHITE;
  function_debug->border.thickness = 2U;

  function_debug->font_horizontal_stride = 9.2794f;
  function_debug->lines.display_index_offset = 0;

  function_debug->node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  function_debug->node->children->alloc = 0;
  function_debug->node->children->count = 0;

  function_debug->variable_value_report_index =
      (mct_function_variable_report_index *)malloc(sizeof(mct_function_variable_report_index));
  function_debug->variable_value_report_index->call_uid_counter = 1;
  function_debug->variable_value_report_index->tag_data = (void *)function_debug;
  // function_debug->variable_value_report_index->call_track_limit = 0;
  function_debug->variable_value_report_index->calls.capacity = 0;
  function_debug->variable_value_report_index->calls.count = 0;
  // function_debug->variable_value_report_index->begin_report_variable_values_delegate =
  //     (void *)&_mce_begin_report_variable_values;
  function_debug->variable_value_report_index->report_variable_value_delegate = (void *)&_mce_report_variable_value;
  function_debug->variable_value_report_index->end_report_variable_values_delegate = NULL; // TODO -- this causes crash
  //     (void *)&_mce_end_report_variable_values;
  printf("function_debug=%p\n", function_debug);
  printf("function_debug->variable_value_report_index=%p\n", function_debug->variable_value_report_index);

  function_debug->call_reports.capacity = 0U;
  function_debug->call_reports.count = 0U;
  function_debug->displayed_call = NULL;

  function_debug->displayed_variable_window.item = NULL;
  function_debug->displayed_variable_window.image_resource_uid = 0U;
  function_debug->displayed_variable_window.width = 240;
  function_debug->displayed_variable_window.height = 150;

  mcr_create_texture_resource(function_debug->displayed_variable_window.width,
                              function_debug->displayed_variable_window.height, MVK_IMAGE_USAGE_RENDER_TARGET_2D,
                              &function_debug->displayed_variable_window.image_resource_uid);

  *p_function_debugger = function_debug;
}

void _mce_transcribe_syntax_to_function_debug_display(mc_syntax_node *syntax_node, mce_function_debug_line **line)
{
  if ((mc_token_type)syntax_node->type < MC_TOKEN_EXCLUSIVE_MAX_VALUE) {

    switch ((mc_token_type)syntax_node->type) {
    // case MC_TOKEN_LINE_COMMENT:
    case MC_TOKEN_NEW_LINE: {
      mce_function_debug_line *new_line = (mce_function_debug_line *)malloc(sizeof(mce_function_debug_line));
      init_c_str(&new_line->str);
      new_line->next = NULL;

      (*line)->next = new_line;
      *line = new_line;
    } break;
    default:
      append_to_c_str((*line)->str, syntax_node->text);
      break;
      // print_syntax_node(syntax_node, 0);
      // MCerror(9216, "TODO:%i", syntax_node->type);
    }

    return;
  }

  // Syntax Node -- convert each child
  for (int a = 0; a < syntax_node->children->count; ++a) {
    mc_syntax_node *child = syntax_node->children->items[a];

    _mce_transcribe_syntax_to_function_debug_display(child, line);
  }
}

void _mce_set_function_to_function_debugger(mce_function_debug *fdebug, function_info *function)
{
  mc_syntax_node *function_syntax;
  int result = parse_definition_to_syntax_tree(function->source->code, &function_syntax);
  if (result) {
    // printf("cees-4\n");
    printf("PARSE_ERROR:8154\n");
    // mc_pprintf(&function_debug->status_bar.message, "ERR[%i]: read console output", result);
    return;
  }
  else {

    printf("function_debug: loaded %s(...)\n", function->name);
    // print_syntax_node(code_syntax, 0);
    // mc_pprintf(&function_debug->status_bar.message, "loaded %s(...)", function->name);
    // function_debug->status_bar.requires_render_update = true;
  }
  // printf("cees-7\n");

  // Clear any code

  // Transcribe
  mce_function_debug_line *line = (mce_function_debug_line *)malloc(sizeof(mce_function_debug_line));
  init_c_str(&line->str);
  line->next = NULL;

  fdebug->code.first_line = line;
  _mce_transcribe_syntax_to_function_debug_display(function_syntax, &line);

  // TODO -- Clear fdebug->variable_value_report_index

  char *mc_transcription;
  mct_function_transcription_options options = {};
  options.report_invocations_to_error_stack = true;
  options.report_simple_args_to_error_stack = true;
  options.check_mc_functions_not_null = true;
  options.tag_on_function_entry = false;
  options.tag_on_function_exit = false;
  options.report_variable_values = fdebug->variable_value_report_index;
  mct_transcribe_function_to_mc(function, function_syntax, &options, &mc_transcription);

  // TODO
  // Keep a pointer of old function delegate
  fdebug->previous_debug_fptr = (void *)*function->ptr_declaration;

  // Instantiate new transcription
  c_str *str;
  init_c_str(&str); // TODO -- maybe init with char*
  set_c_str(str, mc_transcription);
  free(mc_transcription);

  int i;
  for (i = 0;; ++i) {
    if (str->text[i] == '(') {
      break;
    } // TODO -- replace with method call
  }
  char buf[256];
  sprintf(buf, "_fld%u", fdebug->fld_instantiation_uid);
  insert_into_c_str(str, buf, i);

  printf("fdebug.mc_transcription:\n%s||\n", str->text);
  clint_declare(str->text);

  sprintf(buf, "%s = &%s_mc_v%u_fld%u;", function->name, function->name, function->latest_iteration,
          fdebug->fld_instantiation_uid);
  ++fdebug->fld_instantiation_uid;
  // printf("idfc-6\n");
  clint_process(buf);
}

void mce_activate_function_debugging(function_info *func_info)
{
  // global_root_data *global_data;
  // obtain_midge_global_root(&global_data);

  // mce_function_debug_pool *debug_pool = (mce_function_debug_pool *)global_data->ui_state->function_debug_pool;

  // // Determine if an editor already exists with this definition;
  // mce_function_debug *function_debugger = NULL;

  // _mce_obtain_function_debug_instance(debug_pool, &function_debugger);

  // _mce_set_function_to_function_debug(function_debugger, definition->data.func_info);

  // function_debugger->node->layout->visible = true;
  // mca_set_node_requires_layout_update(function_debug->node);
  mce_function_debug *function_debugger;
  mce_init_function_debug_instance(&function_debugger);

  _mce_set_function_to_function_debugger(function_debugger, func_info);
}