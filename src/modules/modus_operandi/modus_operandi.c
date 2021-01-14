/* init_modus_operandi */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <unistd.h>

#include "core/midge_app.h"
#include "midge_error_handling.h"
#include "render/render_common.h"

#include "modules/collections/hash_table.h"
#include "modules/mc_io/mc_file.h"
#include "modules/render_utilities/render_util.h"
#include "modules/ui_elements/ui_elements.h"

#include "modules/modus_operandi/create_process_dialog.h"
#include "modules/modus_operandi/mo_serialization.h"
#include "modules/modus_operandi/mo_types.h"
#include "modules/modus_operandi/mo_util.h"
#include "modules/modus_operandi/process_step_dialog.h"

// #include "env/environment_definitions.h"
// #include "render/render_thread.h"
// #include "ui/ui_definitions.h"
// TODO -- may not need these after removing user functions
#include "core/c_parser_lexer.h"
#include "core/mc_source.h"

typedef struct modus_operandi_data {
  mc_node *node;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

  mc_create_process_dialog_data *create_process_dialog;
  mc_process_step_dialog_data *create_step_dialog;

  mc_mo_process_stack process_stack;

  struct {
    unsigned int capacity, count;
    mo_operational_process **items;
  } all_ops;

  mcu_textbox *search_textbox;
  struct {
    unsigned int capacity, count;
    mcu_button **items;
  } options_buttons;

} modus_operandi_data;

// Forward Declaration
int _mc_mo_activate_next_stack_step(mc_mo_process_stack *process_stack);
int _mc_mo_begin_op_process(mo_operational_process *process, void *args);
int _mc_mo_update_options_display(modus_operandi_data *mod);

void _mc_mo_render_mod_headless(render_thread_info *render_thread, mc_node *node)
{
  modus_operandi_data *mod = (modus_operandi_data *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(render_thread_info *, mc_node *) =
          (void (*)(render_thread_info *, mc_node *))child->layout->render_headless;
      render_node_headless(render_thread, child);
    }
  }

  // Render the render target
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  image_render_details *irq;
  mcr_obtain_image_render_request(global_data->render_thread, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = COLOR_BLACKCURRANT;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = mod->render_target.width;   // TODO
  irq->image_height = mod->render_target.height; // TODO
  irq->data.target_image.image = mod->render_target.image;
  irq->data.target_image.screen_offset_coordinates.x = (unsigned int)node->layout->__bounds.x;
  irq->data.target_image.screen_offset_coordinates.y = (unsigned int)node->layout->__bounds.y;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_present) {
      // TODO fptr casting
      void (*render_node_present)(image_render_details *, mc_node *) =
          (void (*)(image_render_details *, mc_node *))child->layout->render_present;
      render_node_present(irq, child);
    }
  }

  mcr_submit_image_render_request(global_data->render_thread, irq);
}

void _mc_mo_render_mod_present(image_render_details *image_render_queue, mc_node *node)
{
  modus_operandi_data *mod = (modus_operandi_data *)node->data;

  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, mod->render_target.width,
                                         mod->render_target.height, mod->render_target.image);
}

void _mc_mo_handle_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mc_mo_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

int _mc_mo_process_step_dialog_result(void *invoker_state, mc_process_step_dialog_result result)
{
  MCerror(8157, "TODO");
  return 0;
}

int _mc_mo_dialog_input_text_entered(void *invoker_state, char *input_text)
{
  modus_operandi_data *mod = (modus_operandi_data *)invoker_state;

  mo_operational_step *step = mod->process_stack.steps[mod->process_stack.index];

  // printf("_mc_mo_dialog_folder_selected:'%s'\n", selected_folder);
  if (step->action != MO_STEP_TEXT_INPUT_DIALOG) {
    MCerror(8140, "TODO - state error");
  }
  if (!input_text) {
    MCerror(8142, "TODO - canceled process section");
  }

  // Set the path to the target context property
  MCcall(mc_mo_set_top_context_cstr(&mod->process_stack, step->folder_dialog.target_context_property, input_text));

  // Move to the next step
  MCcall(_mc_mo_activate_next_stack_step(&mod->process_stack));

  return 0;
}

int _mc_mo_dialog_options_text_selected(void *invoker_state, char *selected_text)
{
  modus_operandi_data *mod = (modus_operandi_data *)invoker_state;

  mo_operational_step *step = mod->process_stack.steps[mod->process_stack.index];

  // printf("_mc_mo_dialog_folder_selected:'%s'\n", selected_folder);
  if (step->action != MO_STEP_OPTIONS_DIALOG) {
    MCerror(5960, "TODO - state error %i", step->action);
  }
  if (!selected_text) {
    MCerror(8223, "TODO - canceled process section");
  }

  printf("step completed:'%s' %p \n", selected_text, step->options_dialog.target_context_property);

  // Set the selected option to the target context property
  MCcall(mc_mo_set_top_context_cstr(&mod->process_stack, step->options_dialog.target_context_property, selected_text));

  // Move to the next step
  MCcall(_mc_mo_activate_next_stack_step(&mod->process_stack));

  return 0;
}

int _mc_mo_dialog_folder_selected(void *invoker_state, char *selected_folder)
{
  modus_operandi_data *mod = (modus_operandi_data *)invoker_state;
  mo_operational_step *step = mod->process_stack.steps[mod->process_stack.index];

  // printf("_mc_mo_dialog_folder_selected:'%s'\n", selected_folder);
  if (step->action != MO_STEP_OPEN_FOLDER_DIALOG) {
    MCerror(8214, "TODO - state error");
  }

  // Set the path to the target context property
  MCcall(mc_mo_set_top_context_cstr(&mod->process_stack, step->folder_dialog.target_context_property, selected_folder));

  // Move to the next step
  MCcall(_mc_mo_activate_next_stack_step(&mod->process_stack));

  return 0;
}

int _mc_mo_dialog_filepath_selected(void *invoker_state, char *selected_path)
{
  modus_operandi_data *mod = (modus_operandi_data *)invoker_state;

  mo_operational_step *step = mod->process_stack.steps[mod->process_stack.index];

  // printf("_mc_mo_dialog_folder_selected:'%s'\n", selected_folder);
  if (step->action != MO_STEP_FILE_DIALOG) {
    MCerror(8141, "TODO - state error");
  }

  // printf("here %p\n", step->file_dialog.target_context_property);
  // printf("_mc_mo_dialog_filepath_selected:'%s'='%s'\n", step->file_dialog.target_context_property, selected_path);

  // Set the path to the target context property
  MCcall(mc_mo_set_top_context_cstr(&mod->process_stack, step->file_dialog.target_context_property, selected_path));

  // Move to the next step
  MCcall(_mc_mo_activate_next_stack_step(&mod->process_stack));

  return 0;
}

int _mc_mo_obtain_context_arg(mo_op_step_context_arg *context_arg, bool *requires_mem_free, void **result)
{
  switch (context_arg->type) {
  case MO_STEP_CTXARG_CSTR: {
    *result = (void *)context_arg->data;
    *requires_mem_free = false;
  } break;
  case MO_STEP_CTXARG_PROCESS_CONTEXT_PROPERTY: {
    MCerror(8162, "TODO");
    // *result = hash_table_get((char *)context_arg->data, &mod->active_process->context);
    // if (!*result) {
    //   MCerror(9215, "_mc_mo_obtain_context_arg:Process Context Property '%s' does not exist!",
    //           (char *)context_arg->data);
    // }
    // *requires_mem_free = false;
    // // printf("MO_STEP_CTXARG_PROCESS_CONTEXT_PROPERTY:'%s'\n", (char *)*result);
  } break;
  // TODO -- implement when theres some way to inform the calling method to free the set cstr when its done with it.
  case MO_STEP_CTXARG_CURRENT_WORKING_DIRECTORY: {
    char buf[256];
    getcwd(buf, 256);
    *result = (void *)strdup(buf);
    *requires_mem_free = true;
  } break;
  case MO_STEP_CTXARG_ACTIVE_PROJECT_SRC_PATH:
    puts("ERROR TODO -- MO_STEP_CTXARG_ACTIVE_PROJECT_SRC_PATH");
    *requires_mem_free = false;
  default:
    MCerror(8227, "_mc_mo_obtain_context_arg:Unsupported type>%i", context_arg->type);
  }

  return 0;
}

int _mc_mo_return_from_stack_process(mc_mo_process_stack *process_stack)
{
  int a, sidx;
  bool e;
  mo_operational_process *op;
  hash_table_t *ctx, *ctxb;
  const char *pv;

  // Update the process process_stack info
  --process_stack->index;
  sidx = process_stack->index;
  // printf("process_stack->index reduced to %i\n", process_stack->index);

  if (sidx < 0) {
    // process_stack is empty
    return 0;
  }

  mo_operational_step *step = process_stack->steps[sidx];
  if (step && step->action == MO_STEP_CONTEXT_PARAMETER) {
    // Obtain the parameter from the previous process_stack context
    MCcall(mc_mo_get_specific_context_cstr(&process_stack->context_maps[sidx + 1], step->context_parameter.key, &pv));

    if (!pv) {
      MCerror(5294, "subprocess param '%s' was not retrieved", step->context_parameter.key);
    }

    // printf("previous sequence was op_param subprocess: setting '%s' with '%s'\n", step->context_parameter.key, pv);

    // Set it to the now-current context
    MCcall(mc_mo_set_top_context_cstr(process_stack, step->context_parameter.key, pv));

    // MCcall(mc_mo_get_specific_context_cstr(&process_stack->context_maps[sidx], step->context_parameter.key, &pv));
    // printf("got '%s' using '%s' from %p\n", pv, step->context_parameter.key, &process_stack->context_maps[sidx]);
  }

  // Continue on to the first step
  MCcall(_mc_mo_activate_next_stack_step(process_stack));

  return 0;
}

int _mc_mo_activate_next_stack_step(mc_mo_process_stack *process_stack)
{
  const char *str;
  void **vary;
  int sidx = process_stack->index;
  printf("activate-step: %i\n", sidx);
  mo_operational_step *step = process_stack->steps[sidx];

  if (!step) {
    // First step
    step = process_stack->steps[sidx] = process_stack->processes[sidx]->first;
    printf("first step:%i %p\n", step->action, step);
  }
  else {
    if (!step->next) {
      // No Next step
      puts("No Next step\n");
      MCcall(_mc_mo_return_from_stack_process(process_stack));
      return 0;
    }

    // Continue onto next linked step
    step = process_stack->steps[sidx] = step->next;
    printf("next step:%i\n", step->action);
  }

  bool free_ctx_arg;
  switch (step->action) {
  case MO_STEP_CONTEXT_PARAMETER: {
    // printf("MO_STEP_CONTEXT_PARAMETER : '%s'\n", step->context_parameter.key);
    // Ensure the context value exists for the parameter
    // Find in context tree
    MCcall(mc_mo_get_context_cstr(process_stack, step->context_parameter.key, true, &str));
    if (str) {
      if (step->context_parameter.presence == MO_STEP_CTXP_PRESENCE_EMPTY_OBTAIN) {
        MCerror(8582, "TODO -- context parameter must not be set...");
      }

      MCcall(_mc_mo_activate_next_stack_step(process_stack));
      break;
    }

    switch (step->context_parameter.presence) {
    case MO_STEP_CTXP_PRESENCE_EMPTY_OBTAIN:
    case MO_STEP_CTXP_PRESENCE_OBTAIN_AVAILABLE: {
      if (!step->context_parameter.obtain_value_subprocess) {
        MCerror(8774, "A means should be provided to obtain the value for parameter '%s'", step->context_parameter.key);
      }

      // Activate the subprocess to obtain the argument value
      // printf("activating subprocess to get param value for '%s'\n", step->context_parameter.key);
      MCcall(_mc_mo_begin_op_process(step->context_parameter.obtain_value_subprocess, NULL));
    } break;
    default:
      MCerror(8572, "TODO : %i", step->context_parameter.presence);
    }
  } break;
  case MO_STEP_TEXT_INPUT_DIALOG: {
    void **vary = (void **)malloc(sizeof(void *) * 4);

    // TODO -- make a define or const of project label
    vary[0] = step->text_input_dialog.message;
    MCcall(_mc_mo_obtain_context_arg(&step->text_input_dialog.default_text, &free_ctx_arg, &vary[1]));
    vary[2] = (void *)process_stack->state_arg;
    vary[3] = (void *)&_mc_mo_dialog_input_text_entered;

    if (free_ctx_arg) {
      MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_TEXT_INPUT_DIALOG_REQUESTED, vary, 2, vary[1], vary));
    }
    else {
      MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_TEXT_INPUT_DIALOG_REQUESTED, vary, 1, vary));
    }
  } break;
  case MO_STEP_OPTIONS_DIALOG: {
    void **vary = (void **)malloc(sizeof(void *) * 5);

    vary[0] = step->options_dialog.message;
    vary[1] = &step->options_dialog.option_count;
    vary[2] = step->options_dialog.options;
    vary[3] = (void *)process_stack->state_arg;
    vary[4] = (void *)&_mc_mo_dialog_options_text_selected;

    MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_OPTIONS_DIALOG_REQUESTED, vary, 1, vary));
  } break;
  case MO_STEP_OPEN_FOLDER_DIALOG: {
    void **vary = (void **)malloc(sizeof(void *) * 4);

    puts("WARNING TODO -- MO_STEP_OPEN_FOLDER_DIALOG-context_arg");
    // TODO -- make sure memory free is handled (TODO)
    // TODO -- make a define or const of project label
    vary[0] = (void *)step->folder_dialog.message;
    MCcall(_mc_mo_obtain_context_arg(&step->folder_dialog.initial_folder, &free_ctx_arg, &vary[1]));
    vary[2] = (void *)process_stack->state_arg;
    vary[3] = (void *)&_mc_mo_dialog_folder_selected;

    if (free_ctx_arg) {
      MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_FOLDER_DIALOG_REQUESTED, vary, 2, vary[1], vary));
    }
    else {
      MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_FOLDER_DIALOG_REQUESTED, vary, 2, vary));
    }
  } break;
  case MO_STEP_FILE_DIALOG: {
    vary = (void **)malloc(sizeof(void *) * 4);

    // Construct the default filename -- TODO
    vary[0] = (void *)step->file_dialog.message;
    MCcall(_mc_mo_obtain_context_arg(&step->file_dialog.initial_folder, &free_ctx_arg, &vary[1]));
    vary[2] = (void *)process_stack->state_arg;
    vary[3] = (void *)&_mc_mo_dialog_filepath_selected;

    if (free_ctx_arg) {
      MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_FILE_DIALOG_REQUESTED, vary, 2, vary[1], vary));
    }
    else {
      MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_FILE_DIALOG_REQUESTED, vary, 2, vary));
    }
  } break;
  case MO_STEP_DELEGATE_FUNCTION: {

    int (*user_function)(mc_mo_process_stack * context, void *farg) =
        (int (*)(mc_mo_process_stack *, void *))step->delegate.fptr;

    MCcall(user_function(process_stack, step->delegate.farg));

    // Move to the next step -- TODO -- not sure if this is the right move, maybe have user-function return a flag
    // indicating if it is complete or something
    MCcall(_mc_mo_activate_next_stack_step(process_stack));

  } break;
  // case MO_STEP_CREATE_PROCESS_STEP_DIALOG: {
  //   // TODO -- unlike the other events which will eventually initiate on the main UI thread
  //   // -- this is not activated from that position -- figure it out
  //   MCcall(mc_mocsd_activate_process_step_dialog(mod->create_step_dialog,
  //   mod->active_step->process_step_dialog.message,
  //                                                mod, (void *)&_mc_mo_process_step_dialog_result));
  // } break;
  default:
    MCerror(8305, "Unsupported action:%i", step->action);
  }

  return 0;
}

int mc_mo_clear_top_context(mc_mo_process_stack *process_stack)
{
  hash_table_t *ctx = &process_stack->context_maps[process_stack->index];

  hash_table_clear(ctx);

  puts("WARNING TODO 4827 -- clearing hashtable without freeing mc_strs");

  return 0;
}

int _mc_mo_begin_op_process(mo_operational_process *process, void *args)
{
  mc_mo_process_stack *process_stack = process->stack;
  hash_table_t *ctx;
  int a, sidx;
  mo_operational_step *step;
  const char *pv;

  // Increment the process process_stack
  sidx = ++process_stack->index;
  if (sidx >= MO_OP_PROCESS_STACK_SIZE) {
    MCerror(5385, "modus operandi exceeded process_stack capacity");
  }

  // Set the process process_stack info
  process_stack->processes[sidx] = process;
  MCcall(mc_mo_clear_top_context(process_stack));
  process_stack->steps[sidx] = NULL;

  // Set all arguments
  if (args) {
    MCerror(6532, "TODO process to fill parameters from args");
  }

  MCcall(_mc_mo_activate_next_stack_step(process_stack));

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

int _mc_mo_load_operations(mc_node *module_node)
{
  // TODO -- temp - needs to be more persistable way to load save processes
  modus_operandi_data *mod = (modus_operandi_data *)module_node->data;

  // // Define Struct Process
  // mo_operational_step *step;
  // mo_operational_process_parameter *op_param;

  // mo_operational_process *define_struct_process;
  // {
  //   define_struct_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
  //   define_struct_process->stack = &mod->process_stack;
  //   define_struct_process->name = strdup("define-struct");
  //   define_struct_process->nb_parameters = 0;
  //   {
  //     MCcall(mc_grow_array((void **)&define_struct_process->parameters, &define_struct_process->nb_parameters,
  //                          sizeof(mo_operational_process_parameter), (void **)&op_param));
  //     op_param->name = strdup("header-path");
  //     {
  //       // Obtain process
  //       mo_operational_process *sub_process;

  //       sub_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
  //       sub_process->stack = &mod->process_stack;
  //       sub_process->name = strdup("define-struct::header-path");
  //       sub_process->nb_parameters = 0;

  //       // Save-File
  //       const char *const ctxpropid = "header-path";
  //       {
  //         // Obtain their name
  //         step = sub_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

  //         // Obtain the name of the source module to create
  //         step->action = MO_STEP_FILE_DIALOG;
  //         step->file_dialog.message = strdup("Choose or create a header file");
  //         step->file_dialog.initial_filename = strdup("header.h");
  //         step->file_dialog.initial_folder.type = MO_STEP_CTXARG_CURRENT_WORKING_DIRECTORY;
  //         step->file_dialog.initial_folder.data = NULL;
  //         step->file_dialog.target_context_property = strdup(ctxpropid);
  //       }

  //       step->next = NULL;
  //       op_param->obtain_value_subprocess = sub_process;
  //     }

  //     MCcall(mc_grow_array((void **)&define_struct_process->parameters, &define_struct_process->nb_parameters,
  //                          sizeof(mo_operational_process_parameter), (void **)&op_param));
  //     op_param->name = strdup("struct-name");
  //     {
  //       // Obtain process
  //       mo_operational_process *sub_process;

  //       sub_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
  //       sub_process->stack = &mod->process_stack;
  //       sub_process->name = strdup("define-struct::struct-name");
  //       sub_process->nb_parameters = 0;

  //       const char *const ctxpropid = "struct-name";
  //       {
  //         // Obtain their name
  //         step = sub_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

  //         // Obtain the name of the source module to create
  //         step->action = MO_STEP_TEXT_INPUT_DIALOG;
  //         step->text_input_dialog.message = strdup("Specify the struct name");
  //         step->text_input_dialog.default_text.type = MO_STEP_CTXARG_CSTR;
  //         step->text_input_dialog.default_text.data = NULL;
  //         step->text_input_dialog.target_context_property = strdup(ctxpropid);
  //       }

  //       step->next = NULL;
  //       op_param->obtain_value_subprocess = sub_process;
  //     }
  //   }

  //   {
  //     // Generate them
  //     step = define_struct_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

  //     // -- Open Open-Path-Dialog - Seek a folder to store the source
  //     step->action = MO_STEP_DELEGATE_FUNCTION;
  //     step->delegate.fptr = &_user_function_add_struct;
  //     step->delegate.farg = NULL;
  //   }

  //   step->next = NULL;
  //   MCcall(append_to_collection((void ***)&mod->all_ops.items, &mod->all_ops.capacity, &mod->all_ops.count,
  //                               define_struct_process));
  // }

  // Add Render System Process
  {
      //   mo_operational_process *add_render_system_process;
      //   mo_operational_process_parameter *op_param;
      //   mo_operational_step *step;

      //   add_render_system_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
      //   add_render_system_process->mod = mod;
      //   add_render_system_process->name = strdup("add-render-system");
      //   add_render_system_process->nb_parameters = 0;
      //   {
      //     MCcall(mc_grow_array((void **)&add_render_system_process->parameters,
      //     &add_render_system_process->nb_parameters,
      //                          sizeof(mo_operational_process_parameter), (void **)&op_param));
      //     op_param->name = strdup("source-path");
      //     {
      //       // Obtain process
      //       mo_operational_process *sub_process;

      //       sub_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
      //       sub_process->mod = mod;
      //       sub_process->name = strdup("add-render-system::source-path");
      //       sub_process->nb_parameters = 0;

      //       const char *const ctxpropid = "source-path";
      //       {
      //         // Obtain their name
      //         step = sub_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

      //         // Obtain the name of the source module to create
      //         step->action = MO_STEP_OPEN_FOLDER_DIALOG;
      //         step->folder_dialog.initial_folder.type = MO_STEP_CTXARG_CURRENT_WORKING_DIRECTORY;
      //         step->folder_dialog.initial_folder.data = NULL;
      //         step->folder_dialog.message = strdup("Set Source Folder");
      //         step->folder_dialog.target_context_property = strdup(ctxpropid);
      //       }

      //       step->next = NULL;
      //       op_param->obtain_value_subprocess = sub_process;
      //     }

      //     MCcall(mc_grow_array((void **)&add_render_system_process->parameters,
      //     &add_render_system_process->nb_parameters,
      //                          sizeof(mo_operational_process_parameter), (void **)&op_param));
      //     op_param->name = strdup("source-name");
      //     {
      //       // Obtain process
      //       mo_operational_process *sub_process;

      //       sub_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
      //       sub_process->mod = mod;
      //       sub_process->name = strdup("add-render-system::source-name");
      //       sub_process->nb_parameters = 0;

      //       const char *const ctxpropid = "source-name";
      //       {
      //         // Obtain their name
      //         step = sub_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

      //         // Obtain the name of the source module to create
      //         step->action = MO_STEP_TEXT_INPUT_DIALOG;
      //         step->text_input_dialog.message = strdup("Please Specify Source Name");
      //         step->text_input_dialog.default_text.type = MO_STEP_CTXARG_CSTR;
      //         step->text_input_dialog.default_text.data = NULL;
      //         step->text_input_dialog.target_context_property = strdup(ctxpropid);
      //       }

      //       step->next = NULL;
      //       op_param->obtain_value_subprocess = sub_process;
      //     }
      //   }

      //   const char *const ctxprop_data_name = "data-name";
      //   {
      //     // Obtain the data name
      //     step = add_render_system_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

      //     step->action = MO_STEP_TEXT_INPUT_DIALOG;
      //     step->text_input_dialog.message = strdup("Name of the Data Struct");
      //     step->text_input_dialog.default_text =
      //         (mo_op_step_context_arg){MO_STEP_CTXARG_DELEGATE, (char *)&_context_delegate_set_default_data_name};
      //     step->text_input_dialog.target_context_property = strdup(ctxprop_data_name);
      //   }

      //   // {
      //   //   // Invoke declare struct
      //   //   step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
      //   //   step = step->next;

      //   //   step->action = MO_STEP_
      //   // }

      //   // {
      //   //   // Generate them
      //   //   step = add_render_system_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

      //   //   // -- Open Open-Path-Dialog - Seek a folder to store the source
      //   //   step->action = MO_STEP_DELEGATE_FUNCTION;
      //   //   step->delegate.fptr = &_user_function_add_struct;
      //   //   step->delegate.farg = NULL;
      //   // }

      //   step->next = NULL;
      //   MCcall(append_to_collection((void ***)&mod->all_ops.items, &mod->all_ops.capacity, &mod->all_ops.count,
      //                               define_struct_process));
  }

  // Steps Example Process
  {
    //   mo_operational_process *steps_example_process;
    //   mo_operational_step *step;

    //   steps_example_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
    //   steps_example_process->mod = mod;
    //   steps_example_process->name = strdup("steps-example-process");
    //   init_hash_table(64, &steps_example_process->context);

    //   // Save-File
    //   const char *const ctxprop_file = "sep-example-file";
    //   {
    //     // Obtain their name
    //     step = steps_example_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

    //     // Obtain the name of the source module to create
    //     step->action = MO_STEP_FILE_DIALOG;
    //     step->file_dialog.message = strdup("save-file-dialog-example");
    //     step->file_dialog.initial_filename = strdup("example_file.txt");
    //     step->folder_dialog.initial_folder.type = MO_STEP_CTXARG_CURRENT_WORKING_DIRECTORY;
    //     step->folder_dialog.initial_folder.data = NULL;
    //     step->text_input_dialog.target_context_property = strdup(ctxprop_file);
    //   }
    //   {
    //     // Generate them
    //     step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
    //     step = step->next;

    //     // -- Open Open-Path-Dialog - Seek a folder to store the source
    //     step->action = MO_STEP_DELEGATE_FUNCTION;
    //     step->delegate.fptr = &_user_function_print_result;
    //     step->delegate.farg = strdup(ctxprop_file);
    //   }

    //   // Options Input
    //   const char *const ctxprop_options_dialog = "sep-example-options-dialog";
    //   {
    //     // Obtain their name
    //     step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
    //     step = step->next;

    //     // Obtain the name of the source module to create
    //     step->action = MO_STEP_OPTIONS_DIALOG;
    //     step->options_dialog.message = strdup("text-input-dialog-example");
    //     step->options_dialog.option_count = 3U;
    //     step->options_dialog.options = (char **)malloc(sizeof(char *) * step->options_dialog.option_count);
    //     step->options_dialog.options[0] = strdup("option A");
    //     step->options_dialog.options[1] = strdup("option B");
    //     step->options_dialog.options[2] = strdup("option C");
    //     step->text_input_dialog.target_context_property = strdup(ctxprop_options_dialog);
    //   }
    //   {
    //     // Generate them
    //     step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
    //     step = step->next;

    //     // -- Open Open-Path-Dialog - Seek a folder to store the source
    //     step->action = MO_STEP_DELEGATE_FUNCTION;
    //     step->delegate.fptr = &_user_function_print_result;
    //     step->delegate.farg = strdup(ctxprop_options_dialog);
    //   }

    //   // Text Input
    //   const char *const ctxprop_text_input = "sep-example-text-input";
    //   {
    //     // Obtain their name
    //     step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
    //     step = step->next;

    //     // Obtain the name of the source module to create
    //     step->action = MO_STEP_TEXT_INPUT_DIALOG;
    //     step->text_input_dialog.message = strdup("text-input-dialog-example");
    //     step->text_input_dialog.default_text = (mo_op_step_context_arg){MO_STEP_CTXARG_CSTR, "first example"};
    //     step->text_input_dialog.target_context_property = strdup(ctxprop_text_input);
    //   }
    //   {
    //     // Generate them
    //     step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
    //     step = step->next;

    //     // -- Open Open-Path-Dialog - Seek a folder to store the source
    //     step->action = MO_STEP_DELEGATE_FUNCTION;
    //     step->delegate.fptr = &_user_function_print_result;
    //     step->delegate.farg = strdup(ctxprop_text_input);
    //   }

    //   // Folder
    //   const char *const ctxprop_folder = "sep-example-folder";
    //   {
    //     // Generate them
    //     step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
    //     step = step->next;

    //     // -- Open Open-Path-Dialog - Seek a folder to store the source
    //     step->action = MO_STEP_OPEN_FOLDER_DIALOG;
    //     step->folder_dialog.initial_folder.type = MO_STEP_CTXARG_CURRENT_WORKING_DIRECTORY;
    //     step->folder_dialog.initial_folder.data = NULL;
    //     step->folder_dialog.message = strdup("folder-dialog-example");
    //     step->folder_dialog.target_context_property = strdup(ctxprop_folder);
    //   }
    //   {
    //     // Generate them
    //     step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
    //     step = step->next;

    //     // -- Open Open-Path-Dialog - Seek a folder to store the source
    //     step->action = MO_STEP_DELEGATE_FUNCTION;
    //     step->delegate.fptr = &_user_function_print_result;
    //     step->delegate.farg = strdup(ctxprop_folder);
    //   }

    //   step->next = NULL;
    //   MCcall(append_to_collection((void ***)&mod->all_ops.items, &mod->all_ops.capacity, &mod->all_ops.count,
    //                               steps_example_process));
  }

  // Create Operational Process
  // {
  //   mo_operational_process *create_op_process;

  //   create_op_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
  //   create_op_process->mod = mod;
  //   create_op_process->name = strdup("create-op-process");
  //   create_op_process->nb_parameters = 0;
  //   {
  //     MCcall(mc_grow_array((void **)&create_op_process->parameters, &create_op_process->nb_parameters,
  //                          sizeof(mo_operational_process_parameter), (void **)&op_param));
  //     op_param->name = strdup("header-path");
  //     {
  //       // Obtain process
  //       mo_operational_process *sub_process;

  //       sub_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
  //       sub_process->mod = mod;
  //       sub_process->name = strdup("define-struct::header-path");
  //       sub_process->nb_parameters = 0;

  //       // Save-File
  //       const char *const ctxpropid = "header-path";
  //       {
  //         // Obtain their name
  //         step = sub_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

  //         // Obtain the name of the source module to create
  //         step->action = MO_STEP_TEXT_INPUT_DIALOG;
  //         step->text_input_dialog.message = strdup("The name of the process");
  //         step->text_input_dialog.default_text.type = MO_STEP_CTXARG_CSTR;
  //         step->text_input_dialog.default_text.data = NULL;
  //         step->text_input_dialog.target_context_property = strdup(ctxpropid);
  //       }

  //       step->next = NULL;
  //       op_param->obtain_value_subprocess = sub_process;
  //     }
  //   }

  //   const char *const ctxprop_folder = "sep-example-folder";
  //   {
  //     // Generate them
  //     step = create_op_process->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

  //     // -- Open Open-Path-Dialog - Seek a folder to store the source
  //     step->action = MO_STEP_CREATE_PROCESS_STEP_DIALOG;
  //     step->process_step_dialog.message = strdup("Design the first step");
  //   }

  //   step->next = NULL;
  //   MCcall(append_to_collection((void ***)&mod->all_ops.items, &mod->all_ops.capacity, &mod->all_ops.count,
  //                               create_op_process));
  // }

  MCcall(_mc_mo_update_options_display(mod));

  return 0;
}

void _mc_mod_create_process_completed(void *invoker_state, void *created_process)
{
  // Do nothing
  return;
}

void _mc_mo_create_process_clicked(mci_input_event *input_event, mcu_button *button)
{
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    modus_operandi_data *mod = (modus_operandi_data *)button->tag;
    mc_mo_activate_create_process_dialog(mod->create_process_dialog, mod, &_mc_mod_create_process_completed);

    input_event->handled = true;
  }
}

void _mc_mo_operational_process_selected(mci_input_event *input_event, mcu_button *button)
{
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    mo_operational_process *mopp = (mo_operational_process *)button->tag;

    _mc_mo_begin_op_process(mopp, NULL);

    input_event->handled = true;
  }
}

int _mc_mo_update_options_display(modus_operandi_data *mod)
{
  // Order according to search text match score
  // if(!mod->search_textbox->contents->len) {

  // }

  int a;
  mcu_button *button;
  mo_operational_process *mopp;
  for (a = 0; a < mod->all_ops.count && a < mod->options_buttons.count; ++a) {
    button = mod->options_buttons.items[a];
    mopp = mod->all_ops.items[a];

    MCcall(set_mc_str(button->str, mopp->name));

    button->tag = mopp;

    button->node->layout->visible = true;
    MCcall(mca_set_node_requires_rerender(button->node));
  }
  for (; a < mod->options_buttons.count; ++a) {
    button = mod->options_buttons.items[a];

    button->node->layout->visible = false;
  }

  MCcall(mca_set_node_requires_rerender(mod->node));

  return 0;
}

int _mc_mo_parse_directory_for_mop_files(modus_operandi_data *mod, const char *modir)
{
  DIR *dir;
  struct dirent *ent;
  char buf[256];
  if ((dir = opendir(modir)) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL) {
      // Ignore the . / .. entries
      if (!strncmp(ent->d_name, ".", 1))
        continue;

      // Ignore the context data file
      if (!strcmp(ent->d_name, "context"))
        continue;

      // Read the file
      sprintf(buf, "%s/%s", modir, ent->d_name);
      char *file_text;
      MCcall(read_file_text(buf, &file_text));

      // Parse the process
      mo_operational_process *process;
      printf("interpreting mo process...'%s'\n", ent->d_name);
      MCcall(mc_mo_parse_serialized_process(&mod->process_stack, file_text, &process));
      free(file_text);

      // Attach the process to the system
      MCcall(append_to_collection((void ***)&mod->all_ops.items, &mod->all_ops.capacity, &mod->all_ops.count, process));

      MCcall(_mc_mo_update_options_display(mod));
    }
    closedir(dir);
  }
  else {
    // Directory doesn't exist
    // -- there are no MOs for this project

    // perror("could not open directory");
    // MCerror(7918, "Could not open directory '%s'", modir);
    // return EXIT_FAILURE;
  }

  return 0;
}

int _mc_mo_project_created(void *handler_state, void *event_args)
{
  // Parameters
  void **evargs = (void **)event_args;
  const char *project_dir = (const char *)evargs[0];
  const char *project_name = (const char *)evargs[1];

  char modir[256], buf[256], fnb[64];
  const char *property_name;

  // Create the mo dir
  strcpy(modir, project_dir);
  MCcall(mcf_concat_filepath(modir, 256, ".mprj/mo"));
  MCcall(mcf_ensure_directory_exists(modir));

  // Create the context file and its initial data
  mc_str *str;
  MCcall(init_mc_str(&str));

  // Project info context
  MCcall(append_to_mc_strf(str, "%s=%s\n", "project-name", project_name));
  MCcall(append_to_mc_strf(str, "%s=%s\n", "project-dir", project_dir));
  MCcall(append_to_mc_strf(str, "%s=%s_data\n", "project-data", project_name));
  MCcall(append_to_mc_strf(str, "%s=_%s_render_present\n", "project-render-function-name", project_name));

  strcpy(buf, project_dir);
  MCcall(mcf_concat_filepath(buf, 256, "src"));
  MCcall(mcf_concat_filepath(buf, 256, "app"));
  MCcall(mcf_concat_filepath(buf, 256, project_name));
  strcat(buf, ".c");
  MCcall(append_to_mc_strf(str, "%s=%s\n", "project-init-source-filepath", buf));

  strcpy(buf, project_dir);
  MCcall(mcf_concat_filepath(buf, 256, "src"));
  MCcall(mcf_concat_filepath(buf, 256, "app"));
  MCcall(mcf_concat_filepath(buf, 256, project_name));
  strcat(buf, ".h");
  MCcall(append_to_mc_strf(str, "%s=%s\n", "project-init-header-filepath", buf));

  MCcall(append_to_mc_strf(str, "%s=initialize_%s\n", "project-init-function-name", project_name));

  MCcall(mcf_concat_filepath(modir, 256, "context"));
  MCcall(save_text_to_file(modir, str->text));

  release_mc_str(str, true);

  return 0;
}

int _mc_mo_load_project_context(modus_operandi_data *mod, mc_project_info *project)
{
  char path[256];
  strcpy(path, project->path_mprj_data);
  MCcall(mcf_concat_filepath(path, 256, "mo/context"));

  bool exists;
  MCcall(mcf_file_exists(path, &exists));
  if (!exists) {
    MCerror(8527, "TODO");
  }

  // Obtain the project context
  hash_table_t *project_context = (hash_table_t *)hash_table_get(project->name, &mod->process_stack.project_contexts);
  if (!project_context) {
    // Create it
    project_context = (hash_table_t *)malloc(sizeof(hash_table_t));
    MCcall(init_hash_table(32, project_context));
    hash_table_set(project->name, (void *)project_context, &mod->process_stack.project_contexts);
  }

  // Load persisted context into the project context
  char *ft;
  MCcall(read_file_text(path, &ft));
  MCcall(mc_mo_parse_context_file(project_context, ft));
  free(ft);

  // Set ptr data
  printf("setting %p to ptr_project_node\n", project->root_node);
  MCcall(mc_mo_set_specific_context_ptr(project_context, "project-node", project->root_node));

  return 0;
}

int _mc_mo_project_loaded(void *handler_state, void *event_args)
{
  modus_operandi_data *mod = (modus_operandi_data *)handler_state;
  mc_project_info *project = (mc_project_info *)event_args;

  // Load project specific mo's
  char modir[256];
  strcpy(modir, project->path_mprj_data);
  MCcall(mcf_concat_filepath(modir, 256, "mo"));

  // Each file exists as a process
  MCcall(_mc_mo_parse_directory_for_mop_files(mod, modir));

  // Set Global Context
  // TODO -- one day optimize this by not creating/freeing strings every time contexts change
  // char *value = (char *)hash_table_get("key-project", &mod->process_stack.global_context);
  // if (value) {
  //   free(value);
  // }
  // hash_table_set("key-project", strdup(project->name), &mod->process_stack.global_context);

  // Load project context
  MCcall(_mc_mo_load_project_context(mod, project));

  return 0;
}

int mc_mo_load_resources(mc_node *module_node)
{
  int a;

  // Initialize
  modus_operandi_data *mod = (modus_operandi_data *)malloc(sizeof(modus_operandi_data));
  module_node->data = mod;
  mod->node = module_node;

  mod->options_buttons.capacity = mod->options_buttons.count = 0U;
  mod->all_ops.capacity = mod->all_ops.count = 0U;

  mod->process_stack.index = -1;
  mod->process_stack.state_arg = (void *)mod;
  MCcall(init_hash_table(64, &mod->process_stack.global_context));
  MCcall(init_hash_table(4, &mod->process_stack.project_contexts));
  for (a = 0; a < MO_OP_PROCESS_STACK_SIZE; ++a) {
    MCcall(init_hash_table(8, &mod->process_stack.context_maps[a]));
    mod->process_stack.processes[a] = NULL;
    mod->process_stack.steps[a] = NULL;
  }

  mod->render_target.image = NULL;
  mod->render_target.width = module_node->layout->preferred_width;
  mod->render_target.height = module_node->layout->preferred_height;
  MCcall(mcr_create_texture_resource(mod->render_target.width, mod->render_target.height,
                                     MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mod->render_target.image));

  const char *global_mop_directory = "res/mo";
  mcf_directory_exists(global_mop_directory, (bool *)&a);
  if (!a) {
    MCerror(4827, "couldn't find it");
  }
  MCcall(_mc_mo_parse_directory_for_mop_files(mod, global_mop_directory));

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  while (!mod->render_target.image) {
    // puts("wait");
    usleep(100);
  }

  return 0;
}

int mc_mo_init_ui(mc_node *module_node)
{
  modus_operandi_data *mod = (modus_operandi_data *)module_node->data;

  MCcall(mcu_init_textbox(module_node, &mod->search_textbox));
  mod->search_textbox->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  mod->search_textbox->node->layout->padding = (mc_paddingf){6, 2, 42, 2};

  char buf[64];
  mcu_button *button;

  // unsigned int y = (unsigned int)(24 + 8 + 4);
  for (int a = 0; a < 12; ++a) {
    MCcall(mcu_init_button(module_node, &button));

    if (button->node->name) {
      free(button->node->name);
      button->node->name = NULL;
    }
    sprintf(buf, "mo-options-button-%i", a);
    button->node->name = strdup(buf);

    button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    button->node->layout->padding = (mc_paddingf){6, 24 + 8 + a * 27, 6, 0};
    button->node->layout->max_width = 0U;
    button->node->layout->visible = false;

    button->left_click = (void *)&_mc_mo_operational_process_selected;

    MCcall(set_mc_str(button->str, "button"));

    MCcall(append_to_collection((void ***)&mod->options_buttons.items, &mod->options_buttons.capacity,
                                &mod->options_buttons.count, button));
  }

  // Add process button
  MCcall(mcu_init_button(module_node, &button));

  if (button->node->name) {
    free(button->node->name);
    button->node->name = NULL;
  }
  button->node->name = strdup("mo-create-process-button");

  button->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  button->node->layout->padding = (mc_paddingf){6, 2, 6, 2};
  button->node->layout->max_width = 32U;
  button->tag = mod;

  button->left_click = (void *)&_mc_mo_create_process_clicked;

  MCcall(set_mc_str(button->str, "+"));

  // // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  // while (!mod->render_target.image) {
  //   // puts("wait");
  //   usleep(100);
  // }

  return 0;
}

int init_modus_operandi_system(mc_node *app_root)
{
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  mc_node *node;
  mca_init_mc_node(NODE_TYPE_ABSTRACT, "mod-op-root", &node);
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 299;
  node->layout->preferred_height = 360;

  node->layout->padding.left = 1;
  node->layout->padding.bottom = 1;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_mo_render_mod_headless;
  node->layout->render_present = (void *)&_mc_mo_render_mod_present;
  node->layout->handle_input_event = (void *)&_mc_mo_handle_input;

  // TODO
  // node->layout->visible = false;
  // TODO

  MCcall(mc_mo_load_resources(node));
  modus_operandi_data *mod = (modus_operandi_data *)node->data;

  MCcall(mc_mo_init_ui(node));

  MCcall(_mc_mo_load_operations(node));

  // Create Process Step Dialog
  MCcall(mc_mo_init_create_process_dialog(app_root, &mod->create_process_dialog));
  MCcall(mc_mocsd_init_process_step_dialog(app_root, &mod->create_step_dialog));

  // Event Registers
  MCcall(mca_register_event_handler(MC_APP_EVENT_PROJECT_STRUCTURE_CREATION, &_mc_mo_project_created, node->data));
  MCcall(mca_register_event_handler(MC_APP_EVENT_PROJECT_LOADED, &_mc_mo_project_loaded, node->data));
  // TODO -- register for project closed/shutdown

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  return 0;
}