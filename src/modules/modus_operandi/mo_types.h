#ifndef MO_TYPES_H
#define MO_TYPES_H

#include "core/core_definitions.h"

#include "modules/collections/hash_table.h"

#define MO_OP_PROCESS_STACK_SIZE 16

typedef enum mo_op_step_action_type {
  MO_STEP_NULL = 0,
  MO_STEP_CONTEXT_PARAMETER,
  MO_STEP_CREATE_PROCESS_STEP_DIALOG,
  MO_STEP_MESSAGE_BOX,
  MO_STEP_SYMBOL_DIALOG,
  MO_STEP_FILE_DIALOG,
  MO_STEP_OPEN_FOLDER_DIALOG,
  MO_STEP_TEXT_INPUT_DIALOG,
  MO_STEP_OPTIONS_DIALOG,
  MO_STEP_DELEGATE_FUNCTION,
} mo_op_step_action_type;

typedef enum mo_op_step_context_arg_type {
  MO_STEP_CTXARG_NULL = 0,
  MO_STEP_CTXARG_DELEGATE,
  MO_STEP_CTXARG_CSTR,
  MO_STEP_CTXARG_ACTIVE_PROJECT_SRC_PATH,
  MO_STEP_CTXARG_PROCESS_CONTEXT_PROPERTY,
  MO_STEP_CTXARG_CURRENT_WORKING_DIRECTORY,
} mo_op_step_context_arg_type;

typedef enum mo_op_step_context_parameter_presence_type {
  MO_STEP_CTXP_PRESENCE_NULL = 0,
  MO_STEP_CTXP_PRESENCE_REQUIRED,
  MO_STEP_CTXP_PRESENCE_EMPTY_OBTAIN,
  MO_STEP_CTXP_PRESENCE_EMPTY_DEFAULT,
  MO_STEP_CTXP_PRESENCE_DEFAULT_AVAILABLE,
  MO_STEP_CTXP_PRESENCE_OBTAIN_AVAILABLE,
} mo_op_step_context_parameter_presence_type;

typedef struct mo_op_step_context_arg {
  mo_op_step_context_arg_type type;
  char *data;
} mo_op_step_context_arg;

struct mo_operational_process;
typedef struct mo_operational_step {
  struct mo_operational_step *next;

  mo_op_step_action_type action;
  union {
    struct {
      char *key;
      mo_op_step_context_parameter_presence_type presence;
      struct mo_operational_process *obtain_value_subprocess;
    } context_parameter;
    struct {
      char *message;
    } message_box_dialog;
    struct {
      char *message;
      char *symbol_type;
      char *target_context_property;
      char *target_result_type;
    } symbol_dialog;
    struct {
      char *message;
    } process_step_dialog;
    struct {
      char *message;
      mo_op_step_context_arg initial_folder;
      char *target_context_property;
    } folder_dialog;
    struct {
      char *message;
      unsigned int option_count;
      char **options;
      char *target_context_property;
    } options_dialog;
    struct {
      char *message;
      mo_op_step_context_arg initial_folder;
      mo_op_step_context_arg initial_filename;
      char *target_context_property;
    } file_dialog;
    struct {
      char *message;
      mo_op_step_context_arg default_text;
      char *target_context_property;
    } text_input_dialog;
    struct {
      void *fptr;
      void *farg;
    } delegate;
  };
} mo_operational_step;

struct mc_mo_process_stack;

typedef struct mo_operational_process {
  struct mc_mo_process_stack *stack;

  char *name;

  // unsigned int nb_parameters;
  // mo_operational_process_parameter *parameters;

  mo_operational_step *first;
} mo_operational_process;

typedef struct mc_mo_process_stack {
  int index;
  void *state_arg;

  hash_table_t global_context;
  hash_table_t project_contexts;
  hash_table_t context_maps[MO_OP_PROCESS_STACK_SIZE];
  mo_operational_process *processes[MO_OP_PROCESS_STACK_SIZE];
  mo_operational_step *steps[MO_OP_PROCESS_STACK_SIZE];
  // mo_operational_process_parameter *argument_subprocesses[MO_OP_PROCESS_STACK_SIZE];
} mc_mo_process_stack;

#endif // MO_TYPES_H