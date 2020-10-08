#ifndef SOURCE_EDITOR_H
#define SOURCE_EDITOR_H

#include "core/core_definitions.h"
#include "render/render_common.h"

typedef enum mce_source_token_type {
  MCE_SRC_EDITOR_EMPTY = 0,
  MCE_SRC_EDITOR_NON_SEMANTIC_TEXT,
} mce_source_token_type;

typedef struct mce_source_token {
  mce_source_token_type type;
  c_str *str;
} mce_source_token;

typedef struct mce_source_token_list {
  unsigned int char_len;
  unsigned int capacity, count;
  mce_source_token **items;
} mce_source_token_list;

typedef struct mce_source_line {
  mc_node *node;

  mce_source_token_list *source_list;

  unsigned int font_resource_uid;
  float font_horizontal_stride;
  struct {
    unsigned int width, height;
    unsigned int resource_uid;
  } render_target;
} mce_source_line;

struct mce_source_editor_pool;
typedef struct mce_function_editor {
  mc_node *node;
  mce_source_editor_pool *source_editor_pool;

  function_info *function;
  render_color background_color;

  struct {
    struct {
      unsigned int capacity, count;
      mce_source_token_list **items;
    } lines;
  } code;

  struct {
    float vertical_stride;
    struct {
      float top, left;
    } padding;

    int display_offset_index;

    unsigned int utilized, count, capacity;
    mce_source_line **items;

  } lines;
  float font_horizontal_stride;

  struct {
    bool visible;

    int zen_col;
    int line, col;
  } cursor;

} mce_function_editor;

typedef struct mce_source_editor_pool {
  unsigned int max_instance_count;
  struct {
    unsigned int size;
    mce_function_editor **items;
  } function_editor;

  struct {
    unsigned int capacity, count;
    mce_source_token_list **items;
  } source_token_lists;

  struct {
    unsigned int capacity, count;
    mce_source_token **items;
  } source_tokens;

} mce_source_editor_pool;

extern "C" {

// source_editor/source_editor.c
void mce_init_source_editor_pool();
void mce_activate_source_editor_for_definition(source_definition *definition);

// source_editor/source_line.c
void mce_init_source_line(mc_node *parent_node, mce_source_line **source_line);

// source_editor/function_editor.c
void mce_init_function_editor(mc_node *parent_node, mce_source_editor_pool *source_editor_pool,
                              mce_function_editor **p_function_editor);
}
#endif // SOURCE_EDITOR_H
