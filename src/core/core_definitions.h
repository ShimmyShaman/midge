/* core_definitions.h */

#ifndef CORE_DEFINITIONS
#define CORE_DEFINITIONS

#include <stdbool.h>
#include <time.h>

#include "tinycc/libtccinterp.h"

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

// TODO seperate node and event stuff from source stuff

typedef enum mc_source_entity_focus_options {
  MC_SRC_FOC_ENT_NONE = 0,
  MC_SRC_FOC_ENT_REFACTOR_RENAME = 1 << 0,
} mc_source_entity_focus_options;

// TODO -- identify & document the parameters that need releasing by the event handler functions
typedef enum mc_app_event_type {
  MC_APP_EVENT_NULL = 0,
  MC_APP_EVENT_POST_INITIALIZATION,
  // int (*event_handler)(void *handler_state, void *event_args) {event_args is NULL}
  MC_APP_EVENT_INITIAL_MODULES_PROJECTS_LOADED,
  // int (*event_handler)(void *handler_state, void *event_args) {event_args is mc_node *last_attached_node (NULL if removed)}
  MC_APP_EVENT_HIERARCHY_UPDATED,
  // int (*event_handler)(void *handler_state, void *event_args)
  // - event_args is void *[] { const char *project_directory, const char *project_name }
  MC_APP_EVENT_PROJECT_STRUCTURE_CREATION,
  // int (*event_handler)(void *handler_state, void *event_args) {event_args is mc_project_info *project}
  MC_APP_EVENT_PROJECT_LOADED,
  // int (*event_handler)(void *handler_state, void *event_args) {event_args is const char *path}
  MC_APP_EVENT_SOURCE_FILE_OPEN_REQ,
  // int (*event_handler)(void *handler_state, void *event_args) {event_args is mc_source_file_info *modified_file}
  MC_APP_EVENT_SOURCE_FILE_MODIFIED_EXTERNALLY,
  // int (*event_handler)(void *handler_state, void *event_args)
  // - event_args is void *[] { const char *entity_name, mc_source_entity_focus_options *options}
  MC_APP_EVENT_SOURCE_ENTITY_FOCUS_REQ,
  // int (*event_handler)(void *handler_state, void *event_args)
  // - event_args is void*[] { const char *dialog_msg, const char *starting_directory, void *invoker_state,
  //    int (*invoker_result_delegate)(void *invoker_state, char *selected_folder)}
  // -* starting_directory may be NULL indicating use of current-working-directory
  // -** selected_folder may be NULL if user cancels
  MC_APP_EVENT_FOLDER_DIALOG_REQUESTED,
  // int (*event_handler)(void *handler_state, void *event_args)
  // - event_args is void*[] { const char *dialog_msg, const char *starting_directory, void *invoker_state,
  //    int (*invoker_result_delegate)(void *invoker_state, char *filepath)}
  // -* starting_directory may be NULL indicating use of current-working-directory
  // -** filepath may be NULL if user cancels
  MC_APP_EVENT_FILE_DIALOG_REQUESTED,
  // int (*event_handler)(void *handler_state, void *event_args)
  // - event_args is void*[] { const char *prompt_message, const char *default_value, void *invoker_state,
  //    int (*invoker_result_delegate)(void *invoker_state, char *input_text)}
  // -* prompt_message and default_value may be NULL indicating an empty message and value
  // -** input_text may be NULL if user cancels
  MC_APP_EVENT_TEXT_INPUT_DIALOG_REQUESTED,
  // int (*event_handler)(void *handler_state, void *event_args)
  // - event_args is void*[] { const char *prompt_message, unsigned int *option_count, const char **options, void
  // *invoker_state,
  //    int (*invoker_result_delegate)(void *invoker_state, char *selected_option)}
  // -* prompt_message may be NULL indicating an empty message
  // -** selected_option may be NULL if user cancels
  MC_APP_EVENT_OPTIONS_DIALOG_REQUESTED,
  MC_APP_EXCLUSIVE_MAX,
} mc_app_event_type;

typedef enum source_file_type {
  SOURCE_FILE_NULL = 0,
  SOURCE_FILE_NODE,
  SOURCE_FILE_MC_DEFINITIONS,
  SOURCE_FILE_TEXT,
  SOURCE_FILE_EXCLUSIVE_MAX = 100,
} source_file_type;

typedef enum mc_source_definition_type {
  mc_source_definition_NULL = SOURCE_FILE_EXCLUSIVE_MAX,
  mc_source_definition_FUNCTION,
  mc_source_definition_STRUCTURE,
  mc_source_definition_ENUMERATION,
} mc_source_definition_type;

typedef enum node_type {
  NODE_TYPE_NONE = 1,
  NODE_TYPE_GLOBAL_ROOT,
  NODE_TYPE_CONSOLE_APP,
  NODE_TYPE_VISUAL_PROJECT,
  NODE_TYPE_MCU_PANEL,
  NODE_TYPE_MCU_CONTEXT_MENU,
  NODE_TYPE_MCU_BUTTON,
  NODE_TYPE_MCU_TEXT_BLOCK,

  NODE_TYPE_FUNCTION_EDITOR,
  NODE_TYPE_STRUCT_EDITOR,
  NODE_TYPE_ENUM_EDITOR,
  NODE_TYPE_MCM_SOURCE_LINE,

  NODE_TYPE_3D_PORTAL,

  NODE_TYPE_ABSTRACT,
  NODE_TYPE_DOESNT_MATTER,
  NODE_TYPE_MIDGE_EXCLUSIVE_MAXIMUM = 3000,
} node_type;

typedef enum parameter_kind {
  PARAMETER_KIND_NULL = 0,
  PARAMETER_KIND_STANDARD,
  PARAMETER_KIND_FUNCTION_POINTER,
  PARAMETER_KIND_VARIABLE_ARGS,
} parameter_kind;

typedef enum field_kind {
  FIELD_KIND_NULL = 0,
  FIELD_KIND_STANDARD,
  FIELD_KIND_NESTED_STRUCT,
  FIELD_KIND_NESTED_UNION,
} field_kind;

typedef enum preprocessor_define_type {
  PREPROCESSOR_DEFINE_NULL = 0,
  // #define identifier
  PREPROCESSOR_DEFINE_REMOVAL,
  // #define identifier token-string
  PREPROCESSOR_DEFINE_REPLACEMENT,
  // #define identifier(identifier...opt) token-string-opt
  PREPROCESSOR_DEFINE_FUNCTION_LIKE,
} preprocessor_define_type;

// TODO -- remove this and all uses, remnant of a different interpreter
typedef struct struct_id {
  char *identifier;
  unsigned short version;
} struct_id;

struct mc_source_file_info;
struct struct_info;
struct function_info;
struct enumeration_info;
struct field_info_list;

// typedef struct mc_source_definition {
//   struct_id *type_id;
//   mc_source_definition_type type;
//   struct mc_source_file_info *source_file;
//   union {
//     void *p_data;
//     struct struct_info *structure_info;
//     struct function_info *func_info;
//     struct enumeration_info *enum_info;
//   } data;
//   char *code;
// } mc_source_definition;

typedef struct mc_include_directive_info {
  bool is_system_search;
  char *filepath;
} mc_include_directive_info;

typedef struct mc_define_directive_info {
  char *statement;
} mc_define_directive_info;

typedef enum mc_source_file_code_segment_type {
  MC_SOURCE_SEGMENT_NULL = 0,
  MC_SOURCE_SEGMENT_FUNCTION_DECLARATION,
  MC_SOURCE_SEGMENT_STRUCTURE_DECLARATION,
  MC_SOURCE_SEGMENT_ENUMERATION_DECLARATION,
  MC_SOURCE_SEGMENT_FUNCTION_DEFINITION,
  MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION,
  MC_SOURCE_SEGMENT_ENUMERATION_DEFINITION,
  MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE,
  MC_SOURCE_SEGMENT_DEFINE_DIRECTIVE,
  MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR,
  MC_SOURCE_SEGMENT_SINGLE_LINE_COMMENT,
  MC_SOURCE_SEGMENT_MULTI_LINE_COMMENT,
} mc_source_file_code_segment_type;

typedef struct enum_member_info {
  struct_id *type_id;
  char *identity;
  char *value;
} enum_member_info;

typedef struct enumeration_info {
  char *name;
  bool is_defined;

  struct {
    unsigned int count, alloc;
    enum_member_info **items;
  } members;

  struct mc_source_file_info *source;
} enumeration_info;

typedef struct preprocess_define_info {
  preprocessor_define_type statement_type;
  char *identifier;
  char *replacement;
} preprocess_define_info;

typedef struct field_declarator_info {
  unsigned int deref_count;
  union {
    char *name;
    struct {
      char *identifier;
      unsigned int fp_deref_count;
    } function_pointer;
  };

  /* Will be NULL if declarator is not an array; Otherwise, first value will be the array size, the array will dimension
   * size listings will commence at array_dimensions[1]
   */
  struct {
    unsigned int dimension_count;
    char **dimensions;
  } array;
} field_declarator_info;

typedef struct field_declarator_info_list {
  unsigned int alloc, count;
  field_declarator_info **items;
} field_declarator_info_list;

typedef struct field_info {
  field_kind field_type;

  union {
    struct {
      char *type_name;
    } std;
    struct {
      bool is_union, is_anonymous;
      char *type_name;
      struct field_info_list *fields;
    } sub_type;
  };

  field_declarator_info_list declarators;
} field_info;

typedef struct field_info_list {
  unsigned int alloc, count;
  field_info **items;
} field_info_list;

typedef struct struct_info {
  char *name;
  bool is_union;
  bool is_defined;

  /* The file this structure is defined in */
  struct mc_source_file_info *source_file;

  field_info_list fields;
} struct_info;

// TODO -- make this more understandable divide into type & declarator maybe
typedef struct parameter_info {
  struct_id *type_id;
  parameter_kind parameter_type;
  union {
    struct {
      char *type_name;
    };
    struct {
      char *return_type;
      unsigned int return_deref_count;
      char *fptr_name;
      // parameter_info_list *parameters;//TODO
    } fptr;
  };
  unsigned int type_deref_count;
  char *name;
} parameter_info;

typedef struct function_info {
  // struct_id *type_id;
  struct mc_source_file_info *source;
  bool is_defined;
  char *name;
  // int (**ptr_declaration)(int, void **);
  struct {
    char *name;
    unsigned int deref_count;
  } return_type;
  char *code;
  struct {
    unsigned int alloc, count;
    parameter_info **items;
  } parameters;
  // int variable_parameter_begin_index;

  unsigned int nb_dependents;
  struct function_info **dependents;
  unsigned int nb_dependencies;
  struct function_info **dependencies;
} function_info;

typedef struct mc_source_file_code_segment {
  mc_source_file_code_segment_type type;
  union {
    void *data;
    function_info *function;
    struct_info *structure;
    enumeration_info *enumeration;
    mc_include_directive_info *include;
    mc_define_directive_info *define;
    // mc_source_comment_info *comment; TODO
    // mc_pp_directive_info *pp_directive; TODO -- other directives
  };
} mc_source_file_code_segment;

typedef struct mc_source_file_code_segment_list {
  unsigned int capacity;
  unsigned int count;
  mc_source_file_code_segment **items;
} mc_source_file_code_segment_list;

typedef struct mc_source_file_info {
  struct_id *type_id;
  struct timespec recent_disk_sync;
  char *filepath;
  mc_source_file_code_segment_list segments;
} mc_source_file_info;

struct mc_node;
typedef struct mc_node_list {
  unsigned int alloc, count;
  struct mc_node **items;
} mc_node_list;

struct mca_node_layout;
typedef struct mc_node {
  node_type type;
  char *name;

  struct mc_node *parent;
  mc_node_list *children;

  struct mca_node_layout *layout;

  void *data;
  void (*destroy_data)(void *data);
} mc_node;

typedef struct mc_app_itp_data {
  struct timespec *app_begin_time;

  TCCInterpState *interpreter;

  mc_node *global_node;

  struct {
    unsigned int alloc, count;
    mc_source_file_info **items;
  } source_files;
  struct {
    unsigned int alloc, count;
    function_info **items;
  } functions;
  struct {
    unsigned int alloc, count;
    function_info **items;
  } function_declarations;
  struct {
    unsigned int alloc, count;
    struct_info **items;
  } structs;
  struct {
    unsigned int alloc, count;
    enumeration_info **items;
  } enumerations;
  struct {
    unsigned int alloc, count;
    preprocess_define_info **items;
  } preprocess_defines;
} mc_app_itp_data;

typedef struct mc_project_info {
  char *name;
  char *path, *path_src, *path_mprj_data;

  struct {
    unsigned int capacity, count;
    mc_source_file_info *items;
  } source_files;

  mc_node *root_node;
} mc_project_info;

int mc_obtain_app_itp_data(mc_app_itp_data **p_data);

int mc_throw_delayed_error(int error_no, const char *error_message, int year, int month, int day);

int read_file_text(const char *filepath, char **output);
int save_text_to_file(const char *filepath, char *text);

int append_to_collection(void ***collection, unsigned int *collection_capacity, unsigned int *collection_count,
                         void *item);
int insert_in_collection(void ***collection, unsigned int *collection_capacity, unsigned int *collection_count,
                         int insertion_index, void *item);
int remove_from_collection(void ***collection, unsigned int *collection_count, int index);
int remove_ptr_from_collection(void ***collection, unsigned int *collection_count, bool return_error_on_failure,
                               void *ptr);

int find_function_info(const char *name, function_info **result);
// int find_function_info_by_ptr(void *function_ptr, function_info **result);
int find_struct_info(const char *name, struct_info **structure_info);
int find_enumeration_info(const char *name, enumeration_info **enum_info);
int find_enum_member_info(const char *name, enumeration_info **result_type, enum_member_info **result);

int release_struct_id(struct_id *ptr);
int release_field_declarator_info(field_declarator_info *declarator);
int release_field_declarator_info_list(field_declarator_info_list *declarator_list);
int release_field_info(field_info *ptr);
int release_field_info_list(field_info_list *ptr);
int release_parameter_info(parameter_info *ptr);

#endif /* CORE_DEFINITIONS */
