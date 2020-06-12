/* midge_core.h */
#ifndef MIDGE_CORE_H
#define MIDGE_CORE_H

#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __cplusplus
typedef unsigned char bool;
static const bool false = 0;
static const bool true = 1;
#endif

typedef void **midgeo;
typedef void **midgeary;
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

#define MCcall(function)                      \
  {                                           \
    int mc_res = function;                    \
    if (mc_res) {                             \
      printf("--" #function ":%i\n", mc_res); \
      return mc_res;                          \
    }                                         \
  }

#define MCerror(error_code, error_message, ...)                          \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return error_code;

#define cprintf(dest, format, ...)                            \
  {                                                           \
    int cprintf_n = snprintf(NULL, 0, format, ##__VA_ARGS__); \
    dest = (char *)malloc(sizeof(char) * (cprintf_n + 1));    \
    sprintf(dest, format, ##__VA_ARGS__);                     \
  }

typedef enum {

  PROCESS_ACTION_NULL = 1,
  PROCESS_ACTION_NONE,

  // User Initiated
  PROCESS_ACTION_USER_UNPROVOKED_COMMAND,
  PROCESS_ACTION_USER_SCRIPT_ENTRY,
  PROCESS_ACTION_USER_SCRIPT_RESPONSE,
  PROCESS_ACTION_USER_CREATED_SCRIPT_NAME,

  PROCESS_ACTION_DEMO_INITIATION,
  PROCESS_ACTION_DEMO_CONCLUSION,

  // Process Manager Initiated
  PROCESS_ACTION_PM_IDLE,
  PROCESS_ACTION_PM_SEQUENCE_RESOLVED,
  PROCESS_ACTION_PM_UNRESOLVED_COMMAND,
  PROCESS_ACTION_PM_SCRIPT_REQUEST,
  PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME,

  // Script
  PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS,
  PROCESS_ACTION_SCRIPT_QUERY,

  PROCESS_ACTION_MAX_VALUE = 999,
} process_action_type;

typedef enum {
  PROCESS_MOVEMENT_NULL = 1,
  PROCESS_MOVEMENT_CONTINUE,
  PROCESS_MOVEMENT_INDENT,
  PROCESS_MOVEMENT_RESOLVE,
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

typedef struct mc_struct_id_v1 {
  const char *identifier;
  unsigned int version;
} mc_struct_id_v1;

typedef struct mc_void_collection_v1 {
  mc_struct_id_v1 *struct_id;
  unsigned int allocated;
  unsigned int count;
  void **items;
} mc_void_collection_v1;

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

typedef struct mc_struct_info_v1 {
  mc_struct_id_v1 *struct_id;
  const char *name;
  unsigned int version;
  const char *declared_mc_name;
  unsigned int field_count;
  void **fields;
  const char *sizeof_cstr;
} mc_struct_info_v1;

typedef struct mc_parameter_info_v1 {
  mc_struct_id_v1 *struct_id;
  const char *type_name;
  unsigned int type_version;
  unsigned int type_deref_count;
  const char *name;
} mc_parameter_info_v1;

typedef struct mc_function_info_v1 {
  mc_struct_id_v1 *struct_id;
  const char *name;
  unsigned int latest_iteration;
  const char *return_type;
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

typedef struct mc_command_hub_v1 {
  mc_struct_id_v1 *struct_id;
  mc_node_v1 *global_node;
  mc_node_v1 *nodespace;
  void *template_collection;
  mc_process_unit_v1 *process_matrix;
  unsigned int focused_issue_stack_alloc;
  unsigned int focused_issue_stack_count;
  void **focused_issue_stack;
  unsigned int scripts_alloc;
  unsigned int scripts_count;
  void **scripts;
  unsigned int script_instances_alloc;
  unsigned int script_instances_count;
  void **script_instances;
  bool focused_issue_activated;
  unsigned int uid_counter;
  mc_process_action_v1 *demo_issue;
} mc_command_hub_v1;

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

// typedef struct mc_process_matrix_node_v1 {
//   mc_struct_id_v1 *struct_id;

//   mc_process_action_detail_v1 *action;
//   // The action previous in the current sequence back to after a demo_initiation or idle/resolution action
//   mc_process_action_detail_v1 *sequence_root_issue;
//   mc_process_action_detail_v1 *previous_issue;
//   mc_process_action_detail_v1 *contextual_issue;

//   mc_process_action_detail_v1 *continuance;

//   union {
//     mc_void_collection_v1 *consensus_process_units;
//     mc_void_collection_v1 *branches;
//   };
// } mc_process_matrix_node_v1;

int (*find_function_info)(int, void **);

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
  case PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS:
    return "PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS";
  case PROCESS_ACTION_SCRIPT_QUERY:
    return "PROCESS_ACTION_SCRIPT_QUERY";
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
  case PROCESS_ACTION_DEMO_CONCLUSION:
    *result = PROCESS_ORIGINATOR_USER;
    return 0;
    // Process Manager Initiated
  case PROCESS_ACTION_PM_IDLE:
  case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
  case PROCESS_ACTION_PM_SCRIPT_REQUEST:
  case PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME:
  case PROCESS_ACTION_PM_SEQUENCE_RESOLVED:
    *result = PROCESS_ORIGINATOR_PM;
    return 0;
    // Script
  case PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS:
  case PROCESS_ACTION_SCRIPT_QUERY:
    *result = PROCESS_ORIGINATOR_SCRIPT;
    return 0;
  default:
    MCerror(3152, "get_process_originator:TODO for type:%i", (int)action_type);
    break;
  }
}
#endif // MIDGE_CORE_H