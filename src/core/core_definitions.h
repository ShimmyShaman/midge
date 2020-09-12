/* core_definitions.h */

#ifndef CORE_DEFINITIONS_H
#define CORE_DEFINITIONS_H

#include <stddef.h>

#ifndef bool
#define bool unsigned char
#endif
#ifndef true
#define true ((unsigned char)1)
#endif
#ifndef false
#define false ((unsigned char)0)
#endif

typedef enum source_file_type {
  SOURCE_FILE_NULL = 0,
  SOURCE_FILE_NODE,
  SOURCE_FILE_MC_DEFINITIONS,
  SOURCE_FILE_TEXT,
  SOURCE_FILE_EXCLUSIVE_MAX = 100,
} source_file_type;

typedef enum source_definition_type {
  SOURCE_DEFINITION_NULL = SOURCE_FILE_EXCLUSIVE_MAX,
  SOURCE_DEFINITION_FUNCTION,
  SOURCE_DEFINITION_STRUCTURE,
  SOURCE_DEFINITION_ENUMERATION,
} source_definition_type;

typedef enum node_type {
  NODE_TYPE_NONE = 1,
  NODE_TYPE_GLOBAL_ROOT,
  NODE_TYPE_CONSOLE_APP,
  NODE_TYPE_VISUAL_APP,
  NODE_TYPE_UI,
  NODE_TYPE_ABSTRACT,
  NODE_TYPE_MIDGE_EXCLUSIVE_MAXIMUM = 1000,
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

typedef struct struct_id {
  char *identifier;
  unsigned short version;
} struct_id;

struct source_file_info;
struct struct_info;
struct function_info;
struct enumeration_info;
struct field_info_list;

typedef struct source_definition {
  struct_id *type_id;
  source_definition_type type;
  source_file_info *source_file;
  union {
    void *p_data;
    struct_info *structure_info;
    function_info *func_info;
    enumeration_info *enum_info;
  } data;
  char *code;
} source_definition;

typedef struct source_file_info {
  struct_id *type_id;
  char *filepath;
  struct {
    unsigned int alloc;
    unsigned int count;
    source_definition **items;
  } definitions;
} source_file_info;

typedef struct enum_member_info {
  struct_id *type_id;
  char *identity;
  char *value;
} enum_member_info;

typedef struct enumeration_info {
  struct_id *type_id;
  source_definition *source;
  char *name;
  unsigned int latest_iteration;
  char *mc_declared_name;
  bool is_defined;
  struct {
    unsigned int count, alloc;
    enum_member_info **items;
  } members;
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
} field_declarator_info;

typedef struct field_declarator_info_list {
  unsigned int alloc, count;
  field_declarator_info **items;
} field_declarator_info_list;

typedef struct field_info {
  struct_id *type_id;
  field_kind field_type;

  union {
    struct {
      char *type_name;
      struct_info *type_info;
      field_declarator_info_list *declarators;
    } field;
    struct {
      bool is_union, is_anonymous;
      char *type_name;
      field_info_list *fields;
      field_declarator_info_list *declarators;
    } sub_type;
  };
} field_info;

typedef struct field_info_list {
  unsigned int alloc, count;
  field_info **items;
} field_info_list;

typedef struct struct_info {
  struct_id *type_id;
  bool is_union;
  source_definition *source;
  char *name;
  unsigned int latest_iteration;
  char *mc_declared_name;
  bool is_defined;
  field_info_list *fields;
} struct_info;

typedef struct parameter_info {
  struct_id *type_id;
  parameter_kind parameter_type;
  union {
    struct {
      char *type_name;
      unsigned int type_version;
    };
    struct {
      char *return_type;
      unsigned int return_deref_count;
      char *fptr_name;
      // parameter_info_list *parameters;//TODO
    };
  };
  unsigned int type_deref_count;
  char *name;
} parameter_info;

typedef struct function_info {
  struct_id *type_id;
  source_definition *source;
  char *name;
  unsigned int latest_iteration;
  int (**ptr_declaration)(int, void **);
  struct {
    char *name;
    unsigned int deref_count;
  } return_type;
  // char *code;
  unsigned int parameter_count;
  parameter_info **parameters;
  int variable_parameter_begin_index;
  unsigned int struct_usage_count;
  struct_info **struct_usage;
} function_info;

typedef struct event_handler_array {
  unsigned int event_type;
  unsigned int count;
  unsigned int alloc;
  int (***handlers)(int, void **);
} event_handler_array;

typedef struct mc_node {
  node_type type;
  char *name;

  mc_node *parent;

  void *data;
} mc_node;

typedef struct mc_node_list {
  unsigned int alloc, count;
  mc_node **items;
} mc_node_list;

// Incomplete Structure declarations
struct render_thread_info;
struct mui_ui_state;

typedef struct global_root_data {
  struct timespec *app_begin_time;
  struct render_thread_info *render_thread;

  mc_node *global_node;
  mc_node_list *children;

  struct {
    unsigned int width, height;
  } screen;

  struct mui_ui_state *ui_state;
  bool requires_rerender;
  unsigned int present_image_resource_uid;
  unsigned int default_font_resource;

  // struct {
  //   pthread_mutex_t mutex;
  //   unsigned int index;
  // } uid_counter;

  struct {
    unsigned int alloc, count;
    source_file_info **items;
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
  struct {
    unsigned int alloc, count;
    event_handler_array **items;
  } event_handlers;
} global_root_data;

extern "C" {

int obtain_midge_global_root(global_root_data **root_data);

int read_file_text(const char *filepath, char **output);

int append_to_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
                         void *item);
int insert_in_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
                         int insertion_index, void *item);
int remove_from_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
                           int index);
int remove_ptr_from_collection(void ***collection, unsigned int *collection_count, bool return_error_on_failure,
                               void *ptr);

int find_function_info(char *function_name, function_info **result);
int find_function_info_by_ptr(void *function_ptr, function_info **result);
int find_struct_info(char *function_name, struct_info **structure_info);
int find_enumeration_info(char *function_name, enumeration_info **enum_info);
int find_enum_member_info(char *name, enumeration_info **result_type, enum_member_info **result);

int release_struct_id(struct_id *ptr);
int release_field_declarator_info(field_declarator_info *declarator);
int release_field_declarator_info_list(field_declarator_info_list *declarator_list);
int release_field_info(field_info *ptr);
int release_field_info_list(field_info_list *ptr);
int release_parameter_info(parameter_info *ptr);
};

#endif // CORE_DEFINITIONS_H