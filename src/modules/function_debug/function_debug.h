#ifndef FUNCTION_DEBUG_H
#define FUNCTION_DEBUG_H

#include "core/c_parser_lexer.h"
#include "core/mc_code_transcription.h"

typedef struct mce_function_debug_line {
  c_str *str;
  // TODO -- future colored text parts

  mce_function_debug_line *next;
} mce_function_debug_line;

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
