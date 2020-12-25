#ifndef MO_TYPES_H
#define MO_TYPES_H

#include "core/core_definitions.h"

#include "modules/collections/hash_table.h"

#define MO_OP_PROCESS_STACK_SIZE 16

typedef enum mo_op_step_action_type {
  MO_OPPA_NULL = 0,
  MO_OPPA_CREATE_PROCESS_STEP_DIALOG,
  MO_OPPA_FILE_DIALOG,
  MO_OPPA_OPEN_FOLDER_DIALOG,
  MO_OPPA_TEXT_INPUT_DIALOG,
  MO_OPPA_OPTIONS_DIALOG,
  MO_OPPA_USER_FUNCTION,
} mo_op_step_action_type;

typedef enum mo_op_step_context_arg_type {
  MO_OPPC_NULL = 0,
  MO_OPPC_DELEGATE,
  MO_OPPC_CSTR,
  MO_OPPC_ACTIVE_PROJECT_SRC_PATH,
  MO_OPPC_PROCESS_CONTEXT_PROPERTY,
  MO_OPPC_CURRENT_WORKING_DIRECTORY,
} mo_op_step_context_arg_type;

typedef struct mo_op_step_context_arg {
  mo_op_step_context_arg_type type;
  char *data;
} mo_op_step_context_arg;

typedef struct mo_operational_step {
  struct mo_operational_step *next;

  mo_op_step_action_type action;
  union {
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
    } options_dialog;
    struct {
      char *message;
      mo_op_step_context_arg initial_folder;
      char *initial_filename;
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

struct mo_operational_process;
typedef struct mo_operational_process_parameter {
  char *name;
  struct mo_operational_process *obtain_value_subprocess;
} mo_operational_process_parameter;

struct mc_mo_process_stack;

typedef struct mo_operational_process {
  struct mc_mo_process_stack *stack;

  char *name;

  int nb_parameters;
  mo_operational_process_parameter *parameters;

  mo_operational_step *first;
} mo_operational_process;

typedef struct mc_mo_process_stack {
  int index;
  void *state_arg;

  hash_table_t context_maps[MO_OP_PROCESS_STACK_SIZE];
  mo_operational_process *processes[MO_OP_PROCESS_STACK_SIZE];
  mo_operational_step *steps[MO_OP_PROCESS_STACK_SIZE];
  mo_operational_process_parameter *argument_subprocesses[MO_OP_PROCESS_STACK_SIZE];
} mc_mo_process_stack;

#endif // MO_TYPES_H