/* midge_core.h */
#ifndef MIDGE_CORE_H
#define MIDGE_CORE_H

#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "rendering/node_render.h"

#define _POSIX_C_SOURCE 200809L
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#ifndef __cplusplus
typedef unsigned char bool;
static const bool false = 0;
static const bool true = 1;
#endif

typedef unsigned int uint;

#define SCRIPT_NAME_PREFIX "mc_script_"

/*
 * @field a (void **) variable to store the created value in.
 */
#define allocate_from_intv(field, val)                                                      \
  *field = (void *)malloc(sizeof(int) * 1);                                                 \
  if (!*field) {                                                                            \
    printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
    return -1;                                                                              \
  }                                                                                         \
  **(int **)field = val;

#define allocate_from_uintv(field, val)                                                     \
  *field = (void *)malloc(sizeof(uint) * 1);                                                \
  if (!*field) {                                                                            \
    printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
    return -1;                                                                              \
  }                                                                                         \
  **(uint **)field = val;

#define allocate_from_cstringv(field, cstr)                                                 \
  *field = (void *)malloc(sizeof(char) * (strlen(cstr) + 1));                               \
  if (!*field) {                                                                            \
    printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
    return -1;                                                                              \
  }                                                                                         \
  strcpy((char *)(*field), cstr);

#define allocate_and_copy_cstr(dest, src)                                 \
  if (src == NULL) {                                                      \
    dest = NULL;                                                          \
  }                                                                       \
  else {                                                                  \
    char *mc_tmp_cstr = (char *)malloc(sizeof(char) * (strlen(src) + 1)); \
    strcpy(mc_tmp_cstr, src);                                             \
    mc_tmp_cstr[strlen(src)] = '\0';                                      \
    dest = mc_tmp_cstr;                                                   \
  }

#define allocate_and_copy_cstrn(dest, src, n)                   \
  if (src == NULL) {                                            \
    dest = NULL;                                                \
  }                                                             \
  else {                                                        \
    char *mc_tmp_cstr = (char *)malloc(sizeof(char) * (n + 1)); \
    strncpy(mc_tmp_cstr, src, n);                               \
    mc_tmp_cstr[n] = '\0';                                      \
    dest = mc_tmp_cstr;                                         \
  }

#define cprintf(dest, format, ...)                                       \
  {                                                                      \
    int cprintf_n = snprintf(NULL, 0, format, ##__VA_ARGS__);            \
    char *mc_temp_cstr = (char *)malloc(sizeof(char) * (cprintf_n + 1)); \
    sprintf(mc_temp_cstr, format, ##__VA_ARGS__);                        \
    dest = mc_temp_cstr;                                                 \
  }

#define dprintf(format, ...)       \
  {                                \
    printf(format, ##__VA_ARGS__); \
  }

typedef enum {

  PROCESS_ACTION_NULL = 1,
  PROCESS_ACTION_NONE,

  // User Initiated
  PROCESS_ACTION_USER_UNPROVOKED_COMMAND,
  PROCESS_ACTION_USER_SCRIPT_ENTRY,
  PROCESS_ACTION_USER_SCRIPT_RESPONSE,
  PROCESS_ACTION_USER_CREATED_SCRIPT_NAME,
  PROCESS_ACTION_USER_VARIABLE_RESPONSE,
  PROCESS_ACTION_USER_TEMPLATE_COMMAND,

  // Demo
  PROCESS_ACTION_DEMO_INITIATION,
  PROCESS_ACTION_USER_DEMO_COMMAND,
  PROCESS_ACTION_DEMO_CONCLUSION,

  // Process Manager Initiated
  PROCESS_ACTION_PM_IDLE,
  PROCESS_ACTION_PM_SEQUENCE_RESOLVED,
  PROCESS_ACTION_PM_UNRESOLVED_COMMAND,
  PROCESS_ACTION_PM_VARIABLE_REQUEST,
  PROCESS_ACTION_PM_SCRIPT_REQUEST,
  PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME,

  // Script
  PROCESS_ACTION_SCRIPT_EXECUTION,
  PROCESS_ACTION_SCRIPT_QUERY,

  PROCESS_ACTION_MAX_VALUE = 999,
} process_action_type;

typedef enum {
  PROCESS_MOVEMENT_NULL = 1,
  PROCESS_MOVEMENT_CONTINUE,
  PROCESS_MOVEMENT_INDENT,
  PROCESS_MOVEMENT_RESOLVE,
  PROCESS_MOVEMENT_DOUBLE_RESOLVE,
} process_action_indent_movement;

typedef enum {
  PROCESS_ORIGINATOR_NULL = 1,
  PROCESS_ORIGINATOR_USER,
  PROCESS_ORIGINATOR_PM,
  PROCESS_ORIGINATOR_SCRIPT,
} process_originator_type;

typedef enum {
  BRANCHING_INTERACTION_IGNORE_DATA = 1,
  BRANCHING_INTERACTION_INCR_DPTR,
  BRANCHING_INTERACTION,
  BRANCHING_INTERACTION_INVOKE,
} branching_interaction_type;

typedef enum {
  INTERACTION_PROCESS_STATE_INITIAL = 1,
  INTERACTION_PROCESS_STATE_POSTREPLY,
} interaction_process_state;

typedef enum {
  PROCESS_CONTEXTUAL_DATA_NODESPACE = 1,
} process_contextual_data;

typedef enum {
  SCRIPT_PROCESS_NOT_STARTED = 1,
  SCRIPT_PROCESS_EXECUTION,
  SCRIPT_PROCESS_QUERY_USER,
} script_process_state;

typedef enum {
  PROCESS_VIEW_DEMONSTRATED = 1,
  PROCESS_VIEW_EXHIBITED,
} process_unit_view_type;

typedef enum {
  PROCESS_MATRIX_SAMPLE = 1,
  PROCESS_MATRIX_NODE,
  // PROCESS_MATRIX_CONSENSUS_CONTAINER,
} process_matrix_unit_type;

typedef enum source_file_type {
  SOURCE_FILE_NULL = 0,
  SOURCE_FILE_MC_DEFINITIONS,
  SOURCE_FILE_TEXT,
  SOURCE_FILE_EXCLUSIVE_MAX = 100,
} source_file_type;

typedef enum source_definition_type {
  SOURCE_DEFINITION_NULL = SOURCE_FILE_EXCLUSIVE_MAX,
  SOURCE_DEFINITION_FUNCTION,
  SOURCE_DEFINITION_STRUCT,
} source_definition_type;

#define PROCESS_UNIT_FIELD_COUNT 8
typedef enum {
  PROCESS_UNIT_FIELD_ACTION_TYPE = 1,
  PROCESS_UNIT_FIELD_ACTION_DIALOGUE,
  PROCESS_UNIT_FIELD_PREVIOUS_TYPE,
  PROCESS_UNIT_FIELD_PREVIOUS_DIALOGUE,
  PROCESS_UNIT_FIELD_CONTEXTUAL_TYPE,
  PROCESS_UNIT_FIELD_CONTEXTUAL_DIALOGUE,
  PROCESS_UNIT_FIELD_SEQUENCE_ROOT_TYPE,
  PROCESS_UNIT_FIELD_SEQUENCE_ROOT_DIALOGUE,
} process_unit_field_type;

struct mc_struct_id_v1;
struct mc_void_collection_v1;
struct mc_cstring_list_v1;
struct mc_key_value_pair_v1;
struct mc_script_local_v1;
struct mc_struct_info_v1;
struct mc_parameter_info_v1;
struct mc_function_info_v1;
struct mc_script_v1;
struct mc_script_instance_v1;
struct mc_command_hub_v1;
struct mc_node_v1;
struct mc_process_action_v1;
struct mc_process_action_detail_v1;
struct mc_process_unit_v1;
struct mc_template_v1;
struct mc_procedure_template_v1;

typedef struct mc_cstring_list_v1 {
  mc_struct_id_v1 *struct_id;
  unsigned int lines_alloc;
  unsigned int lines_count;
  char **lines;
} mc_cstring_list_v1;

typedef struct mc_key_value_pair_v1 {
  mc_struct_id_v1 *struct_id;
  char *key;
  void *value;
} mc_key_value_pair_v1;

typedef struct mc_script_local_v1 {
  mc_struct_id_v1 *struct_id;
  char *type;
  char *identifier;
  unsigned int locals_index;
  char *replacement_code;
  void *struct_info;
  int scope_depth;
} mc_script_local_v1;

typedef struct mc_source_definition_v1 {
  source_definition_type type;
  union {
    void *data;
    mc_struct_info_v1 *structure_info;
    mc_function_info_v1 *func_info;
  };
} mc_source_definition_v1;

typedef struct mc_source_file_info_v1 {
  char *filepath;
  struct {
    uint alloc;
    uint count;
    mc_source_definition_v1 **items;
  } definitions;
} mc_source_file_info_v1;

typedef struct mc_struct_info_v1 {
  mc_struct_id_v1 *struct_id;
  mc_source_file_info_v1 *source_file;
  char *name;
  unsigned int version;
  char *declared_mc_name;
  unsigned int field_count;
  mc_parameter_info_v1 **fields;
  const char *sizeof_cstr;
} mc_struct_info_v1;

typedef struct mc_parameter_info_v1 {
  mc_struct_id_v1 *struct_id;
  const char *type_name;
  const char *mc_declared_type;
  unsigned int type_version;
  unsigned int type_deref_count;
  const char *name;
} mc_parameter_info_v1;

typedef struct mc_function_info_v1 {
  mc_struct_id_v1 *struct_id;
  mc_source_file_info_v1 *source_file;
  const char *name;
  unsigned int latest_iteration;
  struct {
    char *name;
    unsigned int deref_count;
  } return_type;
  char *mc_code;
  unsigned int parameter_count;
  mc_parameter_info_v1 **parameters;
  int variable_parameter_begin_index;
  unsigned int struct_usage_count;
  mc_struct_info_v1 **struct_usage;
} mc_function_info_v1;

typedef struct mc_script_v1 {
  mc_struct_id_v1 *struct_id;
  char *name;
  unsigned int script_uid;
  unsigned int local_count;
  unsigned int segment_count;
  char *created_function_name;
} mc_script_v1;

typedef struct mc_script_instance_v1 {
  mc_struct_id_v1 *struct_id;
  mc_script_v1 *script;
  unsigned int sequence_uid;
  mc_command_hub_v1 *command_hub;
  mc_node_v1 *nodespace;
  mc_process_action_v1 *contextual_action;
  void **locals;
  char *response;
  unsigned int segments_complete;
  int awaiting_data_set_index;
} mc_script_instance_v1;

typedef struct mc_workflow_process_v1 {

  mc_process_action_v1 *initial_issue;
  mc_process_action_v1 *current_issue;
  bool requires_activation;
} mc_workflow_process_v1;

typedef struct mc_ui_element_v1 {
  uint resource_uid;
} mc_ui_element_v1;

typedef struct mc_interactive_console_v1 {
  struct {
    int x, y;
    unsigned int width, height;
  } bounds;

  int (*logic_delegate)(int argc, void **args);
  struct {
    int action_count;
  } logic;

  int (*handle_input_delegate)(int argc, void **args);
  struct {
    uint image_resource_uid;
    bool requires_render_update;
    int (*render_delegate)(int argc, void **args);
  } visual;
  uint font_resource_uid;

  struct {
    uint image_resource_uid;
    uint width, height;
    const char *text;
    bool requires_render_update;
  } input_line;
} mc_interactive_console_v1;

typedef struct update_callback_timer {
  struct timespec next_update;
  struct timespec period;
  bool reset_timer_on_update;
  int (*update_delegate)(int, void **);
  void *state;
} update_callback_timer;

typedef struct mc_command_hub_v1 {
  mc_struct_id_v1 *struct_id;
  mc_node_v1 *global_node;
  mc_node_v1 *nodespace;
  mc_ui_element_v1 *ui_elements;
  mc_void_collection_v1 *template_collection;
  mc_void_collection_v1 *render_sequence_pool;

  // char *selected_text; TODO for xcb implicit pasting
  char *clipboard_text;

  struct {
    resource_queue *resource_queue;
    render_queue *render_queue;
  } renderer;
  struct {
    uint count, allocated;
    update_callback_timer **callbacks;
  } update_timers;
  struct {
    uint count, alloc;
    mc_source_file_info_v1 **items;
  } source_files;
  mc_process_unit_v1 *process_matrix;
  mc_workflow_process_v1 *focused_workflow;
  unsigned int scripts_alloc;
  unsigned int scripts_count;
  void **scripts;
  unsigned int uid_counter;
  mc_process_action_v1 *demo_issue;
  mc_interactive_console_v1 *interactive_console;

  unsigned int error_definition_index;
} mc_command_hub_v1;

typedef enum node_type {
  NODE_TYPE_NONE = 1,
  NODE_TYPE_GLOBAL_ROOT,
  NODE_TYPE_VISUAL,
  NODE_TYPE_ABSTRACT,
} node_type;

typedef struct mc_input_event_v1 {
  bool shiftDown, ctrlDown, altDown;
  window_input_event_type type;
  window_input_event_detail detail;
  bool handled;
} mc_input_event_v1;

typedef struct node_visual_info {

  bool requires_render_update;
  bool hidden;
  uint image_resource_uid;
  struct {
    int x, y;
    uint width, height;
  } bounds;
  int (**render_delegate)(int, void **);
  int (**input_handler)(int, void **);

} node_visual_info;

typedef struct mc_node_v1 {
  mc_struct_id_v1 *struct_id;
  const char *name;
  mc_node_v1 *parent;
  unsigned int functions_alloc;
  unsigned int function_count;
  mc_function_info_v1 **functions;
  unsigned int structs_alloc;
  unsigned int struct_count;
  mc_struct_info_v1 **structs;
  unsigned int children_alloc;
  unsigned int child_count;
  mc_node_v1 **children;

  node_type type;
  union {
    node_visual_info visual;
    struct {
      uint image_resource_uid;
      int (*render_delegate)(int, void **);
    } global_root;
  } data;

  void *extra;
} mc_node_v1;

typedef struct mc_process_action_v1 {
  mc_struct_id_v1 *struct_id;
  unsigned int object_uid;
  unsigned int sequence_uid;

  process_action_indent_movement process_movement;

  /* The root issue this exists under. eg. */
  /* Demo/root-unprovoked-command*/
  mc_process_action_v1 *contextual_issue;
  /* The action previous in the chain */
  mc_process_action_v1 *previous_issue;
  /* The action next in the chain */
  mc_process_action_v1 *next_issue;

  process_action_type type;
  char *dialogue;
  void *data;

  mc_procedure_template_v1 const *queued_procedures;
  mc_void_collection_v1 *contextual_data;
} mc_process_action_v1;

typedef struct mc_process_action_detail_v1 {
  process_action_indent_movement process_movement;
  process_action_type type;
  char *dialogue;
  bool dialogue_has_pattern;
  process_originator_type origin;
} mc_process_action_detail_v1;

typedef struct mc_process_unit_v1 {
  mc_struct_id_v1 *struct_id;

  process_matrix_unit_type type;
  unsigned int process_unit_field_differentiation_index;

  mc_process_action_detail_v1 *action;
  // The action previous in the current sequence back to after a demo_initiation or idle/resolution action
  mc_process_action_detail_v1 *sequence_root_issue;
  mc_process_action_detail_v1 *previous_issue;
  mc_process_action_detail_v1 *contextual_issue;

  mc_process_action_detail_v1 *continuance;

  union {
    mc_void_collection_v1 *children;
  };
} mc_process_unit_v1;
struct mc_template_v1;
struct mc_procedure_template_v1;

typedef struct mc_process_template_v1 {
  mc_struct_id_v1 *struct_id;

  char *dialogue;
  bool dialogue_has_pattern;

  mc_procedure_template_v1 *initial_procedure;

} mc_process_template_v1;

typedef struct mc_procedure_template_v1 {
  mc_struct_id_v1 *struct_id;

  mc_procedure_template_v1 *next;

  process_action_type type;
  char const *command;
  void *data;

} mc_procedure_template_v1;

// FunctionLiveDebugger Structs
typedef struct fld_variable_snapshot {
  int line_index;
  char *type;
  char *mc_declared_type;
  unsigned int type_deref_count;
  char *name;
  char *value_text;
} fld_variable_snapshot;

typedef enum fld_code_type {
  FLD_CODE_NONE,
  FLD_CODE_CSTRING, // TODO -- type/function-name/field-name/literal/number/newline/etc
  FLD_CODE_SNAPSHOT,
} fld_code_type;

typedef struct fld_visual_code_element {
  fld_code_type type;
  void *data;
} fld_visual_code_element;

typedef struct fld_view_state {
  unsigned int declare_incremental_uid;

  int (**ptr_function_ptr)(int, void **);
  int (*previous_function_address)(int, void **);

  struct {
    uint alloc;
    uint count;
    fld_variable_snapshot **items;
  } arguments;

  struct {
    uint alloc;
    uint count;
    fld_visual_code_element **items;
  } visual_code;

} fld_view_state;

#define CODE_EDITOR_RENDERED_CODE_LINES 37
typedef struct rendered_code_line {
  uint index;
  bool requires_render_update;
  uint image_resource_uid;
  char *text;
  uint width, height;
} rendered_code_line;
struct mc_syntax_node;
typedef struct mc_code_editor_state_v1 {
  mc_node_v1 *visual_node;

  rendered_code_line **render_lines;
  uint font_resource_uid;

  int line_display_offset;
  uint cursorLine;
  uint cursorCol;
  bool selection_exists;
  uint selection_begin_line;
  uint selection_begin_col;
  bool cursor_requires_render_update;

  source_definition_type source_data_type;
  void *source_data;
  struct {
    union {
      mc_syntax_node *function_ast;
    };
  } source_interpretation;

  mc_cstring_list_v1 *text;

  bool in_view_function_live_debugger;
  fld_view_state *fld_view;

  struct {
    struct {
      uint x, y, width, height;
    } bounds;
    uint image_resource_uid;
    char *message;
    bool requires_render_update;
  } status_bar;

} mc_code_editor_state_v1;
int read_editor_text_into_cstr(mc_code_editor_state_v1 *state, char **output);
int define_struct_from_code_editor(mc_code_editor_state_v1 *state);
int register_update_timer(int (*fnptr_update_callback)(int, void **), uint usecs_period, bool reset_timer_on_update,
                          void *state);

int print_parse_error(const char *const text, int index, const char *const function_name, const char *section_id);
int parse_past(const char *text, int *index, const char *sequence);
int parse_past_variable_name(const char *text, int *index, char **output);
int parse_past_type_declaration_text(const char *code, int *i, char **type_declaration_text);
int parse_past_dereference_sequence(const char *text, int *i, unsigned int *deref_count);
int parse_past_empty_text(char const *const code, int *i);
int parse_past_number(const char *text, int *index, char **output);
int parse_past_character_literal(const char *text, int *index, char **output);
int mc_parse_past_literal_string(const char *text, int *index, char **output);
int parse_past_identifier(const char *text, int *index, char **identifier, bool include_member_access,
                          bool include_referencing);
int parse_past_type_identifier(const char *text, int *index, char **identifier);
int append_to_cstrn(unsigned int *allocated_size, char **cstr, const char *extra, int chars_of_extra);
int append_to_cstr(unsigned int *allocated_size, char **cstr, const char *extra);
int increment_time_spec(struct timespec *time, struct timespec *amount, struct timespec *outTime);

// MC_PARSER_LEXER
struct mc_syntax_node;

typedef enum mc_token_type {
  MC_TOKEN_NULL = 0,
  // One or more '*'
  MC_TOKEN_STAR_OPERATOR,
  MC_TOKEN_IDENTIFIER,
  MC_TOKEN_SQUARE_OPEN_BRACKET,
  MC_TOKEN_SQUARE_CLOSE_BRACKET,
  MC_TOKEN_OPEN_BRACKET,
  MC_TOKEN_CLOSING_BRACKET,
  MC_TOKEN_SEMI_COLON,
  MC_TOKEN_DECREMENT_OPERATOR,
  MC_TOKEN_POINTER_OPERATOR,
  MC_TOKEN_ASSIGNMENT_OPERATOR,
  MC_TOKEN_SUBTRACT_OPERATOR,
  MC_TOKEN_IF_KEYWORD,
  MC_TOKEN_ELSE_KEYWORD,
  MC_TOKEN_WHILE_KEYWORD,
  MC_TOKEN_FOR_KEYWORD,
  MC_TOKEN_SWITCH_KEYWORD,
  MC_TOKEN_RETURN_KEYWORD,
  MC_TOKEN_CONST_KEYWORD,
  MC_TOKEN_CURLY_OPEN_BRACKET,
  MC_TOKEN_CURLY_CLOSING_BRACKET,
  MC_TOKEN_NEW_LINE,
  MC_TOKEN_TAB_SEQUENCE,
  MC_TOKEN_SPACE_SEQUENCE,
  MC_TOKEN_LINE_COMMENT,
  MC_TOKEN_DECIMAL_POINT,
  MC_TOKEN_NUMERIC_LITERAL,
  MC_TOKEN_STRING_LITERAL,
  MC_TOKEN_COMMA,
  MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR,
  MC_TOKEN_LESS_THAN_OPERATOR,
  MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR,
  MC_TOKEN_MORE_THAN_OPERATOR,
  MC_TOKEN_EQUALITY_OPERATOR,
  MC_TOKEN_INEQUALITY_OPERATOR,
  MC_TOKEN_CASE_KEYWORD,
  MC_TOKEN_DEFAULT_KEYWORD,
  MC_TOKEN_STRUCT_KEYWORD,
  MC_TOKEN_VOID_KEYWORD,
  MC_TOKEN_INT_KEYWORD,
  MC_TOKEN_UNSIGNED_KEYWORD,
  MC_TOKEN_BOOL_KEYWORD,
  MC_TOKEN_FLOAT_KEYWORD,
  MC_TOKEN_LONG_KEYWORD,
  MC_TOKEN_STANDARD_MAX_VALUE = 200,
} mc_token_type;

typedef enum mc_syntax_node_type {
  MC_SYNTAX_ROOT = MC_TOKEN_STANDARD_MAX_VALUE + 1,
  MC_SYNTAX_FUNCTION,
  MC_SYNTAX_BLOCK,
  MC_SYNTAX_LOCAL_DECLARATION_STATEMENT,
  MC_SYNTAX_LOCAL_DECLARATION_ASSIGN_STATEMENT,
  MC_SYNTAX_ASSIGNMENT_STATEMENT,
  MC_SYNTAX_INVOKE_STATEMENT,

  MC_SYNTAX_SUPERNUMERARY,
  MC_SYNTAX_STRING_LITERAL_EXPRESSION,
  MC_SYNTAX_CONDITIONAL_EXPRESSION,
  MC_SYNTAX_DEREFERENCE_SEQUENCE,
  MC_SYNTAX_MEMBER_ACCESS_EXPRESSION,
  MC_SYNTAX_PARAMETER_DECLARATION,
} mc_syntax_node_type;

typedef struct mc_syntax_node_list {
  uint alloc;
  uint count;
  mc_syntax_node **items;
} mc_syntax_node_list;

typedef struct mc_syntax_node {
  mc_syntax_node_type type;
  struct {
    int line;
    int col;
  } begin;
  union {
    // Text -- only used by MC_TOKENS -- All syntax tokens use below fields (as appropriate)
    char *text;
    struct {
      mc_syntax_node_list *children;
      union {
        struct {
          mc_syntax_node *return_type_identifier;
          mc_struct_info_v1 *return_mc_type;
          // May be null indicating no dereference operators
          mc_syntax_node *return_type_dereference;
          mc_syntax_node *name;
          mc_syntax_node_list *parameters;
          mc_syntax_node *code_block;
        } function;
        struct {
          mc_syntax_node *type_identifier;
          mc_struct_info_v1 *mc_type;
          // May be null indicating no dereference operators
          mc_syntax_node *type_dereference;
          mc_syntax_node *name;
        } parameter;
        struct {
          mc_syntax_node *type_identifier;
          mc_struct_info_v1 *mc_type;
          // May be null indicating no dereference operators
          mc_syntax_node *type_dereference;
          mc_syntax_node *variable_name;
        } local_declaration;
        struct {
          mc_syntax_node *type_identifier;
          mc_struct_info_v1 *mc_type;
          // May be null indicating no dereference operators
          mc_syntax_node *type_dereference;
          mc_syntax_node *variable_name;
          mc_syntax_node *assignment_expression;
        } local_declaration_assignment;
        struct {
          mc_syntax_node *function_identity;
          mc_syntax_node_list *arguments;
        } invocation;
        struct {
          mc_syntax_node *variable;
          mc_syntax_node *value_expression;
        } assignment;
        struct {
          mc_syntax_node *left;
          mc_syntax_node *conditional_operator;
          mc_syntax_node *right;
        } conditional_expression;
        struct {
          mc_syntax_node *initialization;
          mc_syntax_node *conditional;
          mc_syntax_node *update_expression;
        } for_loop;
      };
    };
  };
} mc_syntax_node;
int (*parse_mc_to_syntax_tree)(char *mcode, mc_syntax_node **function_block_ast);

int (*parse_and_process_function_definition)(char *function_definition_text, mc_function_info_v1 **function_definition,
                                             bool skip_clint_declaration);
int (*parse_struct_definition)(char *code_definition, mc_struct_info_v1 **structure_info);
int (*declare_struct_from_info)(mc_struct_info_v1 *structure_info);

int (*transcribe_c_block_to_mc)(mc_function_info_v1 *owner, char *code, int *i, uint *transcription_alloc,
                                char **transcription);
int (*code_editor_toggle_view)(mc_code_editor_state_v1 *state);

// OTHER
int (*begin_debug_automation)(int, void **);
int (*load_existing_function_into_code_editor)(int, void **);

// char *filepath, [out]char **output
int (*read_file_text)(int, void **);
int (*declare_function_pointer)(int, void **);
int (*instantiate_function)(int, void **);
// function_info **result (may be NULL); node **nodespace, char **function_name
int (*find_function_info)(int, void **);
int (*build_initial_workspace)(int, void **);
int (*build_interactive_console)(int, void **);
int (*build_code_editor)(int, void **);
int (*code_editor_update)(int, void **);
int (*code_editor_render)(int, void **);
int (*render_global_node)(int, void **);

int (*build_core_display)(int, void **);
int (*core_display_handle_input)(int, void **);
int (*core_display_entry_handle_input)(int, void **);
int (*core_display_render)(int, void **);

int (*build_function_live_debugger)(int, void **);
int (*function_live_debugger_handle_input)(int, void **);
int (*function_live_debugger_render)(int, void **);

// mc_struct_info_v1 *find_struct_info(mc_node_v1 *nodespace, char *struct_name) -- result is last
int (*find_struct_info)(int, void **);
// int (*special_modification)(int, void **);
int (*special_update)(int, void **);
// int (*move_cursor_up)(int, void **);
// int (*save_function_to_file)(int, void **);
// int (*save_struct_to_file)(int, void **);
// int (*insert_text_into_editor)(int, void **);
// int (*delete_selection)(int, void **);
// int (*read_selected_editor_text)(int, void **);
int (*load_existing_struct_into_code_editor)(int, void **);
// int (*code_editor_handle_keyboard_input)(int, void **);
int (*code_editor_handle_input)(int, void **);

#define allocate_anon_struct(ptr_to_struct, size) \
  mc_dvp = (void **)&ptr_to_struct;               \
  *mc_dvp = malloc(size);

#define declare_and_allocate_anon_struct(struct, ptr_to_struct, size) \
  struct *ptr_to_struct;                                              \
  mc_dvp = (void **)&ptr_to_struct;                                   \
  *mc_dvp = malloc(size);

#define declare_and_assign_anon_struct(struct, ptr_to_struct, voidassignee) \
  struct *ptr_to_struct;                                                    \
  mc_dvp = (void **)&ptr_to_struct;                                         \
  *mc_dvp = (void *)voidassignee;

#define assign_anon_struct(ptr_to_struct, voidassignee) \
  mc_dvp = (void **)&ptr_to_struct;                     \
  *mc_dvp = (void *)voidassignee;

int clint_process(const char *str);
int clint_declare(const char *str);
int clint_loadfile(const char *path);
int clint_loadheader(const char *path);

const char *get_action_type_string(process_action_type action_type)
{
  switch (action_type) {
  case PROCESS_ACTION_NONE:
    return "PROCESS_ACTION_NONE";
  case PROCESS_ACTION_NULL:
    return "PROCESS_ACTION_NULL";
  case PROCESS_ACTION_USER_UNPROVOKED_COMMAND:
    return "PROCESS_ACTION_USER_UNPROVOKED_COMMAND";
  case PROCESS_ACTION_USER_SCRIPT_ENTRY:
    return "PROCESS_ACTION_USER_SCRIPT_ENTRY";
  case PROCESS_ACTION_USER_SCRIPT_RESPONSE:
    return "PROCESS_ACTION_USER_SCRIPT_RESPONSE";
  case PROCESS_ACTION_USER_CREATED_SCRIPT_NAME:
    return "PROCESS_ACTION_USER_CREATED_SCRIPT_NAME";
  case PROCESS_ACTION_USER_TEMPLATE_COMMAND:
    return "PROCESS_ACTION_USER_TEMPLATE_COMMAND";
  case PROCESS_ACTION_PM_IDLE:
    return "PROCESS_ACTION_PM_IDLE";
  case PROCESS_ACTION_PM_SEQUENCE_RESOLVED:
    return "PROCESS_ACTION_PM_SEQUENCE_RESOLVED";
  case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
    return "PROCESS_ACTION_PM_UNRESOLVED_COMMAND";
  case PROCESS_ACTION_DEMO_INITIATION:
    return "PROCESS_ACTION_DEMO_INITIATION";
  case PROCESS_ACTION_DEMO_CONCLUSION:
    return "PROCESS_ACTION_DEMO_CONCLUSION";
  case PROCESS_ACTION_PM_SCRIPT_REQUEST:
    return "PROCESS_ACTION_PM_SCRIPT_REQUEST";
  case PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME:
    return "PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME";
  case PROCESS_ACTION_SCRIPT_EXECUTION:
    return "PROCESS_ACTION_SCRIPT_EXECUTION";
  case PROCESS_ACTION_SCRIPT_QUERY:
    return "PROCESS_ACTION_SCRIPT_QUERY";
  case PROCESS_ACTION_USER_DEMO_COMMAND:
    return "PROCESS_ACTION_USER_DEMO_COMMAND";
  case PROCESS_ACTION_PM_VARIABLE_REQUEST:
    return "PROCESS_ACTION_PM_VARIABLE_REQUEST";
  case PROCESS_ACTION_USER_VARIABLE_RESPONSE:
    return "PROCESS_ACTION_USER_VARIABLE_RESPONSE";
  default: {
    char *type;
    cprintf(type, "UNSPECIFIED_PROCESS_ACTION_TYPE:%i", (int)action_type);
    return type;
  }
  }
}

int get_process_originator_type(process_action_type action_type, process_originator_type *result)
{
  switch (action_type) {
    // User Initiated
  case PROCESS_ACTION_USER_UNPROVOKED_COMMAND:
  case PROCESS_ACTION_USER_SCRIPT_ENTRY:
  case PROCESS_ACTION_USER_SCRIPT_RESPONSE:
  case PROCESS_ACTION_USER_CREATED_SCRIPT_NAME:
  case PROCESS_ACTION_DEMO_INITIATION:
  case PROCESS_ACTION_USER_DEMO_COMMAND:
  case PROCESS_ACTION_DEMO_CONCLUSION:
  case PROCESS_ACTION_USER_VARIABLE_RESPONSE:
  case PROCESS_ACTION_USER_TEMPLATE_COMMAND:
    *result = PROCESS_ORIGINATOR_USER;
    return 0;
    // Process Manager Initiated
  case PROCESS_ACTION_PM_IDLE:
  case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
  case PROCESS_ACTION_PM_SCRIPT_REQUEST:
  case PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME:
  case PROCESS_ACTION_PM_SEQUENCE_RESOLVED:
  case PROCESS_ACTION_PM_VARIABLE_REQUEST:
    *result = PROCESS_ORIGINATOR_PM;
    return 0;
    // Script
  case PROCESS_ACTION_SCRIPT_EXECUTION:
  case PROCESS_ACTION_SCRIPT_QUERY:
    *result = PROCESS_ORIGINATOR_SCRIPT;
    return 0;
  default:
    MCerror(477, "get_process_originator:TODO for type:%s", get_action_type_string(action_type));
    break;
  }
}

typedef struct mc_token {
  mc_token_type type;
  char *text;
  unsigned int start_index;
} mc_token;

const char *get_mc_token_type_name(mc_token_type type);

#endif // MIDGE_CORE_H