#ifndef SOURCE_EDITOR_H
#define SOURCE_EDITOR_H

#include "core/core_definitions.h"
#include "render/render_common.h"

typedef enum mcm_source_token_type {
  MCM_SRC_EDITOR_EMPTY = 0,
  MCM_SRC_EDITOR_NON_SEMANTIC_TEXT,
} mcm_source_token_type;

typedef struct mcm_source_token {
  mcm_source_token_type type;
  c_str *str;
} mcm_source_token;

typedef struct mcm_source_token_list {
  unsigned int capacity, count;
  mcm_source_token **items;
} mcm_source_token_list;

typedef struct mcm_source_line {
  mc_node *node;

  mcm_source_token_list *source_list;

  unsigned int font_resource_uid;
  float font_horizontal_stride;
  struct {
    unsigned int width, height;
    unsigned int resource_uid;
  } render_target;
} mcm_source_line;

struct mcm_source_editor_pool;
typedef struct mcm_function_editor {
  mc_node *node;
  mcm_source_editor_pool *source_editor_pool;

  function_info *function;
  render_color background_color;

  struct {
    struct {
      unsigned int capacity, count;
      mcm_source_token_list **items;
    } lines;
  } code;

  struct {
    float vertical_stride;
    struct {
      float top, left;
    } padding;

    int display_offset_index;

    unsigned int utilized, count, capacity;
    mcm_source_line **items;

  } lines;
  float font_horizontal_stride;

  struct {
    bool visible;

    int rtf_index;
    int zen_col;
    int line, col;
  } cursor;

} mcm_function_editor;

typedef struct mcm_source_editor_pool {
  unsigned int max_instance_count;
  struct {
    unsigned int size;
    mcm_function_editor **items;
  } function_editor;

  struct {
    unsigned int capacity, count;
    mcm_source_token_list **items;
  } source_token_lists;

  struct {
    unsigned int capacity, count;
    mcm_source_token **items;
  } source_tokens;

} mcm_source_editor_pool;

extern "C" {

// source_editor/source_editor.c
void mca_init_source_editor_pool();
void mca_activate_source_editor_for_definition(source_definition *definition);

// source_editor/source_line.c
void mcm_init_source_line(mc_node *parent_node, mcm_source_line **source_line);

// source_editor/function_editor.c
void mcm_init_function_editor(mc_node *parent_node, mcm_source_editor_pool *source_editor_pool,
                              mcm_function_editor **p_function_editor);
}
#endif // SOURCE_EDITOR_H
