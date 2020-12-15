/* init_modus_operandi */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "core/midge_app.h"
#include "render/render_common.h"

#include "modules/collections/hash_table.h"
#include "modules/info_transcription/info_transcription.h"
#include "modules/mc_io/mc_io.h"
#include "modules/render_utilities/render_util.h"
#include "modules/ui_elements/ui_elements.h"

// #include "env/environment_definitions.h"
// #include "render/render_thread.h"
// #include "ui/ui_definitions.h"
// TODO -- may not need these after removing user functions
#include "core/c_parser_lexer.h"
#include "core/mc_source.h"

typedef enum mo_op_step_action_type {
  MO_OPPA_NULL = 0,
  MO_OPPA_OPEN_FILE_DIALOG,
  MO_OPPA_OPEN_FOLDER_DIALOG,
  MO_OPPA_TEXT_INPUT_DIALOG,
  MO_OPPA_USER_FUNCTION,
} mo_op_step_action_type;

typedef enum mo_op_step_context_arg_type {
  MO_OPPC_NULL = 0,
  MO_OPPC_CSTR,
  MO_OPPC_ACTIVE_PROJECT_SRC_PATH,
  MO_OPPC_PROCESS_CONTEXT_PROPERTY,
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
      mo_op_step_context_arg initial_folder;
      char *target_context_property;
    } file_dialog;
    struct {
      char *message;
      mo_op_step_context_arg default_text;
      char *target_context_property;
    } text_input_dialog;
    struct {
      void *fptr;
    } delegate;
  };
} mo_operational_step;

struct modus_operandi_data;
typedef struct mo_operational_process {
  struct modus_operandi_data *mod;
  char *title;
  hash_table_t context;
  mo_operational_step *first;
} mo_operational_process;

typedef struct modus_operandi_data {
  mc_node *node;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

  struct {
    unsigned int capacity, count;
    mo_operational_process **items;
  } all_ops;
  mo_operational_process *active_process;
  mo_operational_step *active_step;

  mcu_textbox *search_textbox;
  struct {
    unsigned int capacity, count;
    mcu_button **items;
  } options_buttons;

} modus_operandi_data;

// Forward Declaration
int _mc_mo_activate_active_step(modus_operandi_data *mod);

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

int _mc_mo_dialog_input_text_entered(void *invoker_state, char *input_text)
{
  modus_operandi_data *mod = (modus_operandi_data *)invoker_state;

  // printf("_mc_mo_dialog_path_selected:'%s'\n", selected_folder);
  if (mod->active_step->action != MO_OPPA_TEXT_INPUT_DIALOG) {
    MCerror(8140, "TODO - state error");
  }
  if (!input_text) {
    MCerror(8142, "TODO - canceled process section");
  }

  // printf("step completed:'%s'\n", input_text);

  // Set the path to the target context property
  hash_table_set(mod->active_step->text_input_dialog.target_context_property, strdup(input_text),
                 &mod->active_process->context);

  // Move to the next step
  mod->active_step = mod->active_step->next;
  MCcall(_mc_mo_activate_active_step(mod));

  return 0;
}

int _mc_mo_dialog_path_selected(void *invoker_state, char *selected_folder)
{
  modus_operandi_data *mod = (modus_operandi_data *)invoker_state;

  // printf("_mc_mo_dialog_path_selected:'%s'\n", selected_folder);
  if (mod->active_step->action != MO_OPPA_OPEN_FOLDER_DIALOG) {
    MCerror(8141, "TODO - state error");
  }

  // Set the path to the target context property
  hash_table_set(mod->active_step->file_dialog.target_context_property, strdup(selected_folder),
                 &mod->active_process->context);

  // Move to the next step
  mod->active_step = mod->active_step->next;
  MCcall(_mc_mo_activate_active_step(mod));

  return 0;
}

int _mc_mo_obtain_context_arg(modus_operandi_data *mod, mo_op_step_context_arg *context_arg, void **result)
{
  switch (context_arg->type) {
  case MO_OPPC_CSTR: {
    *result = (void *)context_arg->data;
  } break;
  case MO_OPPC_PROCESS_CONTEXT_PROPERTY: {
    *result = hash_table_get((char *)context_arg->data, &mod->active_process->context);
    if (!*result) {
      MCerror(9215, "_mc_mo_obtain_context_arg:Process Context Property '%s' does not exist!",
              (char *)context_arg->data);
    }
    // printf("MO_OPPC_PROCESS_CONTEXT_PROPERTY:'%s'\n", (char *)*result);
  } break;
  // TODO -- implement when theres some way to inform the calling method to free the set cstr when its done with it.
  // case MO_OPPC_CURRENT_WORKING_DIRECTORY: {
  //   char buf[256];
  //   getcwd(buf, 256);
  //   vary[0] = strdup(buf);
  // } break;
  case MO_OPPC_ACTIVE_PROJECT_SRC_PATH:
    puts("ERROR TODO -- MO_OPPC_ACTIVE_PROJECT_SRC_PATH");
  default:
    MCerror(8220, "_mc_mo_obtain_context_arg:Unsupported type>%i", context_arg->type);
  }

  return 0;
}

int _mc_mo_activate_active_step(modus_operandi_data *mod)
{
  if (!mod->active_step) {
    // MCerror(7186, "No further Steps TODO");
    return 0;
  }

  switch (mod->active_step->action) {
  case MO_OPPA_TEXT_INPUT_DIALOG: {
    void **vary = (void **)malloc(sizeof(void *) * 4);

    // TODO -- make a define or const of project label
    vary[0] = mod->active_step->text_input_dialog.message;
    MCcall(_mc_mo_obtain_context_arg(mod, &mod->active_step->text_input_dialog.default_text, &vary[1]));
    vary[2] = (void *)mod;
    vary[3] = (void *)&_mc_mo_dialog_input_text_entered;

    MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_TEXT_INPUT_DIALOG_REQUESTED, vary, 1, vary));
  } break;
  case MO_OPPA_OPEN_FOLDER_DIALOG: {
    void **vary = (void **)malloc(sizeof(void *) * 3);

    puts("WARNING TODO -- MO_OPPA_OPEN_FOLDER_DIALOG-context_arg");
    // TODO -- make sure memory free is handled (TODO)
    // TODO -- make a define or const of project label
    MCcall(_mc_mo_obtain_context_arg(mod, &mod->active_step->file_dialog.initial_folder, &vary[0]));
    vary[1] = (void *)mod;
    vary[2] = (void *)&_mc_mo_dialog_path_selected;

    MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_FOLDER_DIALOG_REQUESTED, vary, 2, vary[0], vary));
  } break;
  case MO_OPPA_USER_FUNCTION: {
    int (*user_function)(hash_table_t * context) = (int (*)(hash_table_t *))mod->active_step->delegate.fptr;

    MCcall(user_function(&mod->active_process->context));

    // Move to the next step -- TODO -- not sure if this is the right move, maybe have user-function return a flag
    // indicating if it is complete or something
    mod->active_step = mod->active_step->next;
    MCcall(_mc_mo_activate_active_step(mod));

  } break;
  default:
    MCerror(8142, "Unsupported action:%i", mod->active_step->action);
  }

  return 0;
}

void _mc_mo_operational_process_selected(mci_input_event *input_event, mcu_button *button)
{
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
    mo_operational_process *mopp = (mo_operational_process *)button->tag;

    // Begin the process
    modus_operandi_data *mod = mopp->mod;
    mod->active_process = mopp;
    mod->active_step = mopp->first;

    // Reset the context
    // TODO -- can't just clear these properties need memory releasing
    hash_table_clear(&mod->active_process->context);

    // Temp -- project context TODO
    struct_info *si;
    find_struct_info("fs_world", &si);
    const char *const project_3d_root_context_property = "project-3d-root";
    hash_table_set(project_3d_root_context_property, si, &mod->active_process->context);

    _mc_mo_activate_active_step(mod);
  }
}

int mc_mo_update_options_display(modus_operandi_data *mod)
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

    MCcall(set_mc_str(button->str, mopp->title));

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

int mc_mo_load_resources(mc_node *module_node)
{
  // cube_template
  modus_operandi_data *mod = (modus_operandi_data *)malloc(sizeof(modus_operandi_data));
  module_node->data = mod;
  mod->node = module_node;

  mod->options_buttons.capacity = mod->options_buttons.count = 0U;
  mod->all_ops.capacity = mod->all_ops.count = 0U;

  mod->render_target.image = NULL;
  mod->render_target.width = module_node->layout->preferred_width;
  mod->render_target.height = module_node->layout->preferred_height;
  MCcall(mcr_create_texture_resource(mod->render_target.width, mod->render_target.height,
                                     MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mod->render_target.image));

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
  mod->search_textbox->node->layout->padding = (mc_paddingf){6, 2, 6, 2};

  char buf[64];

  // unsigned int y = (unsigned int)(24 + 8 + 4);
  for (int a = 0; a < 12; ++a) {
    mcu_button *button;
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

  // // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  // while (!mod->render_target.image) {
  //   // puts("wait");
  //   usleep(100);
  // }

  return 0;
}

int _user_function_gen_source_files(hash_table_t *context)
{
  const char *const gen_source_name_context_property = "gen-source-name";
  const char *const gen_source_folder_context_property = "gen-source-folder";
  const char *const created_header_context_property = "gen-source-header-file";

  char *name = (char *)hash_table_get(gen_source_name_context_property, context);
  char *folder = (char *)hash_table_get(gen_source_folder_context_property, context);
  printf("this is where I would generate source files named '%s.c' and '%s.h' in the folder '%s'\n", name, name,
         folder);

  char path[256];
  strcpy(path, folder);
  MCcall(mcf_concat_filepath(path, 256, name));
  strcat(path, ".h");

  mc_app_itp_data *itp;
  mc_obtain_app_itp_data(&itp);

  int a;
  mc_source_file_info *source_file = NULL;
  for (a = itp->source_files.count - 1; a >= 0; --a) {
    // printf("%s<>%s\n", itp->source_files.items[a]->filepath, path);
    if (!strcmp(itp->source_files.items[a]->filepath, path)) {
      source_file = itp->source_files.items[a];
      break;
    }
  }
  if (source_file) {
    puts("WARNING header source file already existed");
    // MCerror(8438, "TODO");
  }
  else {
    source_file = (mc_source_file_info *)malloc(sizeof(mc_source_file_info));
    source_file->filepath = strdup(path);
    source_file->definitions.alloc = source_file->definitions.count = 0U;
    source_file->includes.alloc = source_file->includes.count = 0U;

    MCcall(mc_save_source_file_from_updated_info(source_file));
  }

  hash_table_set(created_header_context_property, (void *)source_file, context);

  return 0;
}

int _user_util_insert_standard_field_in_struct(struct_info *structure, const char *type_name, unsigned int deref_count,
                                               const char *field_name)
{
  field_info *f;

  // TODO -- checking of any kind???

  f = (field_info *)malloc(sizeof(field_info));
  f->field_type = FIELD_KIND_STANDARD;
  f->field.type_name = strdup(type_name);
  f->field.declarators = (field_declarator_info_list *)malloc(sizeof(field_declarator_info_list));
  f->field.declarators->alloc = f->field.declarators->count = 0U;

  field_declarator_info *d = (field_declarator_info *)malloc(sizeof(field_declarator_info));
  d->deref_count = deref_count;
  d->name = strdup(field_name);
  MCcall(append_to_collection((void ***)&f->field.declarators->items, &f->field.declarators->alloc,
                              &f->field.declarators->count, d));

  MCcall(append_to_collection((void ***)&structure->fields->items, &structure->fields->alloc, &structure->fields->count,
                              f));

  return 0;
}

int _user_util_insert_struct_in_file(mc_source_file_info *source_file, const char *struct_name, bool error_if_exists,
                                     struct_info **result)
{
  mc_source_definition *sdef = NULL, *d;
  for (int a = 0; a < source_file->definitions.count; ++a) {
    d = source_file->definitions.items[a];
    if (d->type != mc_source_definition_STRUCTURE)
      continue;
    if (!strcmp(d->data.structure_info->name, struct_name)) {
      sdef = d;
      break;
    }
  }
  if (!sdef) {
    // -- Create a new source definition
    sdef = (mc_source_definition *)malloc(sizeof(mc_source_definition));
    sdef->type_id = NULL; // TODO -- remove
    sdef->type = mc_source_definition_STRUCTURE;
    sdef->code = "";
    sdef->source_file = source_file;

    struct_info *si = sdef->data.structure_info = (struct_info *)malloc(sizeof(struct_info));
    si->is_defined = true;
    si->is_union = false;
    si->name = strdup(struct_name);
    si->source = sdef;
    si->fields = (field_info_list *)malloc(sizeof(field_info_list));
    si->fields->alloc = si->fields->count = 0;

    MCcall(append_to_collection((void ***)&source_file->definitions.items, &source_file->definitions.alloc,
                                &source_file->definitions.count, sdef));
  }

  return 0;
}

int _user_function_insert_data_struct(hash_table_t *context)
{
  // Add a struct declaration for the data type
  // -- Obtain the source file
  const char *const created_header_context_property = "gen-source-header-file";
  mc_source_file_info *header_file = (mc_source_file_info *)hash_table_get(created_header_context_property, context);

  const char *const gen_data_name_context_property = "gen-data-name";
  const char *data_struct_name = (const char *)hash_table_get(gen_data_name_context_property, context);

  const char *const gen_source_name_context_property = "gen-source-name";
  const char *gen_source_name = (const char *)hash_table_get(gen_source_name_context_property, context);

  // -- Check structure does not already exist
  struct_info *si;
  MCcall(_user_util_insert_struct_in_file(header_file, data_struct_name, false, &si));

  // Add a reference to this struct in the projects root 3D display module
  const char *const project_3d_root_context_property = "project-3d-root";
  struct_info *project_3d_root_data = (struct_info *)hash_table_get(project_3d_root_context_property, context);

  if (!project_3d_root_data) {
    MCerror(9517, "TODO no 3d root for project..?");
  }

  MCcall(_user_util_insert_standard_field_in_struct(project_3d_root_data, data_struct_name, 1, gen_source_name));

  // -- Integrate
  MCcall(mc_save_source_file_from_updated_info(project_3d_root_data->source->source_file));

  return 0;
}

int mc_mo_load_operations(mc_node *module_node)
{
  // TODO -- temp - needs to be more persistable way to load save processes
  modus_operandi_data *mod = (modus_operandi_data *)module_node->data;

  // Script an operational process for scripting operational processes
  {
    // Create
    mo_operational_process *script_operational_process;
    mo_operational_step *step;

    // script_operational_process = (mo_operational_process *)malloc(sizeof(mo_operational_process));
    // script_operational_process->mod = mod;

    // create_hash_table(64, &script_operational_process->context);
  }

  // Add Renderable System
  {
    mo_operational_process *add_renderable_system;
    mo_operational_step *step;

    add_renderable_system = (mo_operational_process *)malloc(sizeof(mo_operational_process));
    add_renderable_system->mod = mod;

    create_hash_table(64, &add_renderable_system->context);

    // Create the source files
    const char *const gen_source_name_context_property = "gen-source-name";
    const char *const gen_source_folder_context_property = "gen-source-folder";
    const char *const gen_data_name_context_property = "gen-data-name";
    {
      // Obtain their name
      add_renderable_system->title = strdup("add-3d-renderable-system");
      step = add_renderable_system->first = (mo_operational_step *)malloc(sizeof(mo_operational_step));

      // Obtain the name of the source module to create
      step->action = MO_OPPA_TEXT_INPUT_DIALOG;
      step->text_input_dialog.message = strdup("Enter the name of the system source files:");
      step->text_input_dialog.default_text = (mo_op_step_context_arg){MO_OPPC_CSTR, NULL};
      step->text_input_dialog.target_context_property = strdup(gen_source_name_context_property);
      step->next = NULL;
    }
    {
      // Obtain the folder to create them in
      step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
      step = step->next;

      // -- Open Open-Path-Dialog - Seek a folder to store the source
      step->action = MO_OPPA_OPEN_FOLDER_DIALOG;
      step->file_dialog.message = strdup("Select the folder to create the system source files in:");
      // -- -- Folder to begin dialog with -- should be the src folder of the active project
      step->file_dialog.initial_folder =
          (mo_op_step_context_arg){MO_OPPC_CSTR, strdup("/home/jason/midge/projects/fs/src")}; // TODO
      step->file_dialog.target_context_property = strdup(gen_source_folder_context_property);
      step->next = NULL;
    }
    {
      // Create them
      step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
      step = step->next;

      // -- Open Open-Path-Dialog - Seek a folder to store the source
      step->action = MO_OPPA_USER_FUNCTION;
      step->delegate.fptr = &_user_function_gen_source_files;
      step->next = NULL;
    }
    {
      // Confirm the data name
      step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
      step = step->next;

      // Obtain the name of the source module to create
      step->action = MO_OPPA_TEXT_INPUT_DIALOG;
      step->text_input_dialog.message = strdup("Enter the name of the data structure:");
      step->text_input_dialog.default_text =
          (mo_op_step_context_arg){MO_OPPC_PROCESS_CONTEXT_PROPERTY, strdup(gen_source_name_context_property)};
      step->text_input_dialog.target_context_property = strdup(gen_data_name_context_property);
      step->next = NULL;
    }
    {
      // Generate them
      step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
      step = step->next;

      // -- Open Open-Path-Dialog - Seek a folder to store the source
      step->action = MO_OPPA_USER_FUNCTION;
      step->delegate.fptr = &_user_function_insert_data_struct;
      step->next = NULL;
    }
    // {
    //   // Generate the data
    //   step->next = (mo_operational_step *)malloc(sizeof(mo_operational_step));
    //   step = step->next;

    //   // -- Open Open-Path-Dialog - Seek a folder to store the source
    //   step->action = MO_OPPA_USER_FUNCTION;
    //   step->delegate.fptr = &_user_function_gen_source_files;
    //   step->next = NULL;
    // }

    MCcall(append_to_collection((void ***)&mod->all_ops.items, &mod->all_ops.capacity, &mod->all_ops.count,
                                add_renderable_system));
  }

  MCcall(mc_mo_update_options_display(mod));

  return 0;
}

int init_modus_operandi(mc_node *app_root)
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

  MCcall(mc_mo_init_ui(node));

  MCcall(mc_mo_load_operations(node));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  // Panel
  // mcu_panel *panel;
  // mcu_init_panel(global_data->global_node, &panel);

  // panel->node->layout->padding = {300, 400, 800, 60};
  // panel->background_color = COLOR_GREEN;

  // mcu_set_element_update(panel->element);

  // Text Block
  // mcu_text_block *text_block;
  // mcu_init_text_block(global_data->global_node, &text_block);

  // mca_node_layout *layout = text_block->element->layout;
  // layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  // layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  // layout->padding = {150, 300, 0, 0};

  // set_mc_str(text_block->str, "");
  // for (int a = 32; a < 128; ++a) {
  //   char buf[2];
  //   buf[0] = (char)a;
  //   buf[1] = '\0';
  //   append_to_mc_str(text_block->str, buf);
  // }
  // text_block->font_color = COLOR_LIGHT_YELLOW;

  return 0;
}