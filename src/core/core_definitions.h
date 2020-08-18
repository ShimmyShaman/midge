/* core_definitions.h */

#ifndef CORE_DEFINITIONS_H
#define CORE_DEFINITIONS_H

#include <stddef.h>

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
  SOURCE_DEFINITION_STRUCT,
  SOURCE_DEFINITION_ENUMERATION,
} source_definition_type;

typedef enum node_type {
  NODE_TYPE_NONE = 1,
  NODE_TYPE_GLOBAL_ROOT,
  NODE_TYPE_CONSOLE_APP,
  NODE_TYPE_VISUAL_APP,
  NODE_TYPE_VISUAL,
  NODE_TYPE_ABSTRACT,
} node_type;

typedef struct struct_id {
  char *identifier;
  unsigned short version;
} struct_id;

struct source_file_info;
struct struct_info;
struct function_info;
struct enumeration_info;
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

typedef struct enum_member {
  struct_id *type_id;
  char *identity;
  char *value;
} enum_member;

typedef struct enumeration_info {
  struct_id *type_id;
  source_definition *source;
  char *name;
  unsigned int latest_iteration;
  struct {
    unsigned int count, alloc;
    enum_member **items;
  } members;
} enumeration_info;

typedef struct field_info {
  struct_id *type_id;
  char *type_name;
  struct_info *type;
  unsigned int type_deref_count;
  char *name;
} field_info;

typedef struct struct_info {
  struct_id *type_id;
  source_definition *source;
  char *name;
  unsigned int version;
  char *declared_name;
  struct {
    unsigned int count, alloc;
    field_info **items;
  } fields;
} struct_info;

typedef struct parameter_info {
  struct_id *type_id;
  bool is_function_pointer;
  union {
    struct {
      char *type_name;
      char *declared_type;
      unsigned int type_version;
    };
    struct {
      char *function_type;
      char *full_function_pointer_declaration;
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

typedef struct node {
  struct_id *type_id;
  node_type type;
  char *name;

  node *parent;
  struct {
    unsigned int alloc, count;
    node **items;
  } children;

  void *data;
} node;

typedef struct global_root_data {
  node *global_node;
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
    struct_info **items;
  } structs;
  struct {
    unsigned int alloc, count;
    enumeration_info **items;
  } enumerations;
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

int find_function_info(char *function_name, function_info **funct_info);
int find_struct_info(char *function_name, struct_info **structure_info);
int find_enumeration_info(char *function_name, enumeration_info **enum_info);

int release_field_info(field_info *ptr);
};

#endif // CORE_DEFINITIONS_H