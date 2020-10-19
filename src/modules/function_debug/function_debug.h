#ifndef FUNCTION_DEBUG_H
#define FUNCTION_DEBUG_H

#include "core/c_parser_lexer.h"
#include "core/mc_code_transcription.h"

typedef struct mce_function_debug_line {
  c_str *str;
  // TODO -- future colored text parts

  mce_function_debug_line *next;
} mce_function_debug_line;

typedef struct mce_function_debug_variable_report {
  char *name;
  char *type_identity;
  unsigned int deref_count;
  bool is_array, is_fptr;
  int line, col, length;
  void *value;
  char *value_display_text;

} mce_function_debug_variable_report;

typedef struct mce_function_debug_function_call {
  unsigned int call_uid;

  struct {
    unsigned int capacity, count;
    mce_function_debug_variable_report **items;
  } variable_reports;
} mce_function_debug_function_call;

typedef struct mce_function_debug {

  mc_node *node;
  render_color background_color;

  struct {
    render_color color;
    unsigned int thickness;
  } border;

  struct {
    mc_syntax_node *syntax;
    mce_function_debug_line *first_line;
  } code;

  struct {
    struct {
      float left, top;
    } padding;

    int display_index_offset;
    float vertical_stride;
  } lines;

  mce_function_debug_function_call *displayed_call;
  struct {
    unsigned int image_resource_uid;
    unsigned int width, height;
    mce_function_debug_variable_report *item;
  } displayed_variable_window;

  struct {
    unsigned int capacity, count;
    mce_function_debug_function_call **items;
  } call_reports;

  mct_function_variable_report_index *variable_value_report_index;
  void *previous_debug_fptr;
  unsigned int fld_instantiation_uid;

  float font_horizontal_stride;
} mce_function_debug;

extern "C" {
void mce_init_function_debug_instance(mce_function_debug **function_debugger);
void _mce_set_function_to_function_debugger(mce_function_debug *function_debugger, function_info *func_info);
}

#endif // FUNCTION_DEBUG_H
