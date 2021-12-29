#ifndef SOURCE_EDITOR_H
#define SOURCE_EDITOR_H

#include "core/core_definitions.h"
#include "mc_str.h"
#include "render/render_common.h"

#include "modules/collections/hash_table.h"

// int (*event_handler)(void *handler_state, void *event_args) {event_args is const char *path}
// const char *MC_APP_EVENT_SOURCE_FILE_OPEN_REQ = "MC_APP_EVENT_SOURCE_FILE_OPEN_REQ";
#define MC_APP_EVENT_SOURCE_FILE_OPEN_REQ "MC_APP_EVENT_SOURCE_FILE_OPEN_REQ"

// int (*event_handler)(void *handler_state, void *event_args) {event_args is const char *space_delimited_search_str}
#define MCM_SE_EVENT_FIND_IN_FILE "MCM_SE_EVENT_FIND_IN_FILE"

// int (*event_handler)(void *handler_state, void *event_args) {event_args is const char *filepath}
// - filepath may be NULL or partially complete indicating the best match will be displayed in a file search window
//     on top of the source editor.
#define MCM_SE_FIND_SOURCE_FILE "MCM_SE_FIND_SOURCE_FILE"


// typedef enum mce_source_token_type {
//   MCE_SE_NULL = 0,
//   MCE_SE_UNPROCESSED_TEXT,
//   // MCE_SE_EMPTY_SPACE,
// } mce_source_token_type;

// struct mce_source_token;
// struct mce_source_line_token;

// typedef struct mce_source_token {
//   mce_source_token_type type;
//   mc_str *str;

//   struct mce_source_token *next;
// } mce_source_token;

// typedef struct mce_source_line_token {
//   unsigned int len;

//   mce_source_token *first;
//   struct mce_source_line_token *prev, *next;
// } mce_source_line_token;

// typedef struct mce_source_token_list {
//   unsigned int line_len;
//   unsigned int capacity, count;
//   mce_source_token **items;
// } mce_source_token_list;

// typedef struct mce_source_line {
//   mc_node *node;

//   mce_source_line_token *line_token;

//   mcr_font_resource *font;
//   float font_horizontal_stride;
//   struct {
//     unsigned int width, height;
//     mcr_texture_image *image;
//   } render_target;
// } mce_source_line;

typedef struct mc_source_editor_file {
  mc_source_file_info *sf;

  int scroll_offset;
  struct {
    int count, size;
    mc_str *items;
  } lines;
} mc_source_editor_file;

typedef struct mc_source_editor_line {
  bool active;
  unsigned int draw_offset_y;
  unsigned int width, height;
  mcr_texture_image *image;
} mc_source_editor_line;

typedef struct mc_source_editor_line_group {
  hash_table_t cache;
  int lines_size;
  mc_source_editor_line *lines;

} mc_source_editor_line_group;

// struct mce_source_editor_pool;
// struct mce_source_editor_pool *source_editor_pool;
typedef struct mc_source_editor {
  mc_node *node;

  struct {
    unsigned int size, used;
    mc_source_editor_file *items;
    mc_source_editor_file *focus;
  } source_files;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
    bool requires_rerender;
  } tab_index;

  struct {
    unsigned int left, top, bottom;
  } content_padding;

  struct {
    render_color color;
    unsigned int size;
  } border;

  mc_source_editor_line_group line_images;

  // struct {
  //   struct {
  //     unsigned int capacity, count;
  //     mce_source_token_list **items;
  //   } source_token_lists;

  //   struct {
  //     unsigned int capacity, count;
  //     mce_source_token **items;
  //   } source_tokens;
  // } pool;

  render_color background_color;
  // struct {
  //   render_color color;
  //   unsigned int thickness;
  // } border;

  // struct {
  //   mce_source_line_token *first_line;
  //   unsigned int capacity, count;
  //   mce_source_line_token **line_tokens;
  // } code;

  // struct {
  //   float vertical_stride;
  //   struct {
  //     float top, left;
  //   } padding;

  //   int display_index_offset;

  //   unsigned int utilized, count, capacity;
  //   mce_source_line **items;

  // } lines;
  // float font_horizontal_stride;

  // struct {
    bool cursor_visible;

  //   int zen_col;
  //   int line, col;
  // } cursor;

  // struct {
  //   bool exists;
  //   int line, col;
  // } selection;

} mc_source_editor;

// typedef struct mce_source_editor_pool {
//   unsigned int max_instance_count;
//   struct {
//     unsigned int size;
//     mce_function_editor **items;
//   } function_editor;

//   struct {
//     unsigned int capacity, count;
//     mce_source_token_list **items;
//   } source_token_lists;

//   struct {
//     unsigned int capacity, count;
//     mce_source_token **items;
//   } source_tokens;

// } mce_source_editor_pool;

// extern "C" {

// source_editor/source_editor.c
int mc_se_init_source_editor(mc_node *app_root);

int mcm_se_open_source_file(mc_node *source_editor_node, const char *filepath);
// int mce_activate_source_editor_for_definition(mc_source_definition *definition);

// // source_editor/source_line.c
// int mce_init_source_line(mc_node *parent_node, mce_source_line **source_line);

// // source_editor/function_editor.c
// int mce_init_function_editor(mc_node *parent_node, mce_source_editor_pool *source_editor_pool,
//                               mce_function_editor **p_function_editor);

// int mce_obtain_source_token_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token **token);
// int mce_obtain_source_token_list_from_pool(mce_source_editor_pool *source_editor_pool, mce_source_token_list **list);
// int mce_return_source_token_lists_to_editor_pool(mce_source_editor_pool *source_editor_pool,
//                                                   mce_source_token_list **lists, unsigned int count);
// }
#endif // SOURCE_EDITOR_H
