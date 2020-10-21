#ifndef SOURCE_EDITOR_H
#define SOURCE_EDITOR_H

#include "core/core_definitions.h"
#include "render/render_common.h"

typedef enum mce_source_token_type {
  MCE_SE_NULL = 0,
  MCE_SE_UNPROCESSED_TEXT,
  // MCE_SE_EMPTY_SPACE,
} mce_source_token_type;

typedef struct mce_source_token {
  mce_source_token_type type;
  c_str *str;

  mce_source_token *next;
} mce_source_token;

typedef struct mce_source_line_token {
  unsigned int len;

  mce_source_token *first;
  mce_source_line_token *prev, *next;
} mce_source_line_token;

typedef struct mce_source_token_list {
  unsigned int line_len;
  unsigned int capacity, count;
  mce_source_token **items;
} mce_source_token_list;

typedef struct mce_source_line {
  mc_node *node;

  mce_source_line_token *line_token;

  font_resource *font;
  float font_horizontal_stride;
  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;
} mce_source_line;

struct mce_source_editor_pool;
typedef struct mce_function_editor {
  mc_node *node;
  mce_source_editor_pool *source_editor_pool;

  function_info *function;
  render_color background_color;
  struct {
    render_color color;
    unsigned int thickness;
  } border;

  struct {
    mce_source_line_token *first_line;
    unsigned int capacity, count;
    mce_source_line_token **line_tokens;
  } code;

  struct {
    float vertical_stride;
    struct {
      float top, left;
    } padding;

    int display_index_offset;

    unsigned int utilized, count, capacity;
    mce_source_line **items;

  } lines;
  float font_horizontal_stride;

  struct {
    bool visible;

    int zen_col;
    int line, col;
  } cursor;

  struct {
    bool exists;
    int line, col;
  } selection;

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

void mce_obtain_source_token_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token **token);
void mce_obtain_source_token_list_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token_list **list);
void mce_return_source_token_lists_to_editor_pool(mce_source_editor_pool *source_editor_pool,
                                                  mce_source_token_list **lists, unsigned int count);
}
#endif // SOURCE_EDITOR_H
