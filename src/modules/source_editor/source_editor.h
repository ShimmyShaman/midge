

typedef struct mcm_source_line {
  mc_node *node;

  c_str *rtf;

  unsigned int font_resource_uid;
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
    c_str *rtf;
    mc_syntax_node *syntax;
  } code;

  struct {
    unsigned int count, alloc;
    mcm_source_line **items;
  } lines;

  int line_display_offset;

} mcm_function_editor;

typedef struct mcm_source_editor_pool {
  unsigned int max_instance_count;
  struct {
    unsigned int size;
    mcm_function_editor **instances;
  } function_editor;
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
