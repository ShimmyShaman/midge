#ifndef FUNCTION_DEBUG_H
#define FUNCTION_DEBUG_H

#include "core/c_parser_lexer.h"

typedef struct mce_function_debug {

  mc_node *node;
  render_color background_color;

  struct {
    render_color color;
    unsigned int thickness;
  } border;

  struct {
    mc_syntax_node *syntax;
  } code;

  struct {
    struct {
      float left, top;
    } padding;

    int display_index_offset;
    float vertical_stride;
  } lines;

  float font_horizontal_stride;
} mce_function_debug;

extern "C" {
void mce_init_function_debug_instance(mce_function_debug **function_debugger);
void _mce_set_function_to_function_debugger(mce_function_debug *function_debugger, function_info *func_info);
}

#endif // FUNCTION_DEBUG_H
