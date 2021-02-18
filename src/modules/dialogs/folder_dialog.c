/* folder_dialog.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/app_modules.h"
#include "core/midge_app.h"
#include "render/render_common.h"

#include "modules/ui_elements/ui_elements.h"
#include "modules/mc_io/mc_file.h"

// #include "env/environment_definitions.h"
// #include "render/render_thread.h"
// #include "ui/ui_definitions.h"

typedef enum mc_folder_dialog_display_mode {
  MC_FD_MODE_NULL = 0,
  MC_FD_MODE_DIRECTORIES_ONLY,
} mc_folder_dialog_display_mode;

typedef struct mc_folder_dialog_data {
  mc_node *node;

  render_color shade_color;

  mcu_panel *panel;
  mcu_textblock *message_textblock;
  struct {
    unsigned int capacity, count, utilized;
    mcu_button **items;
  } displayed_items;

  // struct {
  //   unsigned int width, height;
  //   mcr_texture_image *image;
  // } render_target;

  mc_str *current_directory;
  mc_folder_dialog_display_mode mode;
  struct {
    void *state;
    void *result_delegate;
  } callback;

} mc_folder_dialog_data;

void _mc_render_folder_dialog_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_folder_dialog_data *fd = (mc_folder_dialog_data *)node->data;

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

  // // Render the render target
  // midge_app_info *app_info;
  // mc_obtain_midge_app_info(&app_info);

  // // Children
  // for (int a = 0; a < node->children->count; ++a) {
  //   mc_node *child = node->children->items[a];
  //   if (child->layout && child->layout->visible && child->layout->render_present) {
  //     // TODO fptr casting
  //     void (*render_node_present)(image_render_details *, mc_node *) =
  //         (void (*)(image_render_details *, mc_node *))child->layout->render_present;
  //     render_node_present(irq, child);
  //   }
  // }

  // mcr_submit_image_render_request(app_info->render_thread, irq);
}

void _mc_render_folder_dialog_present(image_render_details *image_render_queue, mc_node *node)
{
  mc_folder_dialog_data *fd = (mc_folder_dialog_data *)node->data;

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, fd->shade_color);

  //   mcr_issue_render_command_colored_quad(
  //       image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
  //       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
  //       fd->background_color);

  // Children
  mca_render_node_list_present(image_render_queue, node->children);
}

void _mc_handle_folder_dialog_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mco_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

int _mc_fd_open_directory(mc_folder_dialog_data *fd, const char *starting_directory)
{
  fd->displayed_items.utilized = 0U;
  char *c, path[256], ext[16];
  int len;
  mcu_button *button;

  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(starting_directory)) != NULL) {
    if (starting_directory) {
      MCcall(mc_set_str(fd->current_directory, starting_directory));
    }
    else {
      if (!getcwd(path, 256)) {
        MCerror(9135, "Current Working Directory too large for this pretty big buffer");
      }
      MCcall(mc_set_str(fd->current_directory, path));
    }
    // printf("_mc_fd_open_directory:'%s'\n", fd->current_directory->text);

    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL && fd->displayed_items.utilized < fd->displayed_items.count) {
      // Ignore the "." entry
      if (!strcmp(ent->d_name, "."))
        continue;

      // Create the full path of the directory entry
      strcpy(path, starting_directory);
      len = strlen(path);
      if (path[len - 1] != '\\' && path[len - 1] != '/') {
        strcat(path, "/");
      }
      strcat(path, ent->d_name);

      // printf("path:%s", path);
      if (fd->mode == MC_FD_MODE_DIRECTORIES_ONLY) {
        // Obtain the path entry type
        struct stat stats;
        int res = stat(path, &stats);
        if (res) {
          MCerror(1919, "Odd? TODO print errno");
        }

        if (!S_ISDIR(stats.st_mode)) {
          // puts("--NOT dir");
          continue;
        }
      }
      // puts("");

      // Assign entry
      button = fd->displayed_items.items[fd->displayed_items.utilized++];
      button->node->layout->visible = true;
      MCcall(mc_set_str(&button->str, ent->d_name));
    }
    closedir(dir);
  }
  else {
    /* could not open directory */
    perror("");
    MCerror(8528, "Could not open directory '%s'", starting_directory);
  }

  for (int a = fd->displayed_items.utilized; a < fd->displayed_items.count; ++a) {
    fd->displayed_items.items[a]->node->layout->visible = false;
  }

  MCcall(mca_set_node_requires_rerender(fd->node));

  return 0;
}

void _mc_fd_open_current_clicked(mci_input_event *input_event, mcu_button *button)
{
  mc_folder_dialog_data *fd = (mc_folder_dialog_data *)button->tag;

  // Wrap Up
  fd->node->layout->visible = false;

  // printf("_mc_fd_open_current_clicked:'%s\n", fd->current_directory->text);

  // Activate Callback
  int (*result_delegate)(void *invoker_state, char *selected_folder) =
      (int (*)(void *, char *))fd->callback.result_delegate;
  if (result_delegate) {
    result_delegate(fd->callback.state, fd->current_directory->text);
  }
  fd->callback.state = NULL;
  fd->callback.result_delegate = NULL;

  mca_set_node_requires_rerender(fd->node);
}

// TODO -- all events app-wide need int returning success etc
void _mc_fd_item_selected(mci_input_event *input_event, mcu_button *button)
{
  mc_folder_dialog_data *fd = (mc_folder_dialog_data *)button->tag;

  char buf[256];
  if (!strcmp(button->str.text, "..")) {
    mcf_get_parent_directory(buf, 256, fd->current_directory->text);
  }
  else {
    strcpy(buf, fd->current_directory->text);
    mcf_concat_filepath(buf, 256, button->str.text);
    if (access(buf, F_OK) == -1) {
      // File doesn't exist!
      puts("ERROR TODO 8195");
    }
  }

  _mc_fd_open_directory(fd, buf);
}

int _mc_fd_on_folder_dialog_request(void *handler_state, void *event_args)
{
  mc_folder_dialog_data *fd = (mc_folder_dialog_data *)handler_state;

  void **vary = (void **)event_args;
  char *message = (char *)vary[0];
  char *starting_path = (char *)vary[1];

  // Set Callback Info
  fd->callback.state = vary[2];
  fd->callback.result_delegate = vary[3];

  // Set the message
  MCcall(mcu_set_textblock_text(fd->message_textblock,  message == NULL ? "" : message));

  // Open The Dialog at the starting path
  fd->mode = MC_FD_MODE_DIRECTORIES_ONLY;
  MCcall(_mc_fd_open_directory(fd, starting_path));

  // Display
  fd->node->layout->visible = true;

  return 0;
}

int mc_fd_init_data(mc_node *module_node)
{
  mc_folder_dialog_data *fd = (mc_folder_dialog_data *)malloc(sizeof(mc_folder_dialog_data));
  module_node->data = fd;
  fd->node = module_node;

  fd->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  MCcall(mc_alloc_str(&fd->current_directory));
  fd->callback.state = NULL;
  fd->callback.result_delegate = NULL;

  fd->displayed_items.capacity = fd->displayed_items.count = 0U;

  // mo_data->render_target.image = NULL;
  // mo_data->render_target.width = module_node->layout->preferred_width;
  // mo_data->render_target.height = module_node->layout->preferred_height;
  // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  //   TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  //   while (!fd->render_target.image) {
  //     // puts("wait");
  //     usleep(100);
  //   }

  return 0;
}

int mc_fd_init_ui(mc_node *module_node)
{
  mc_folder_dialog_data *fd = (mc_folder_dialog_data *)module_node->data;

  // Locals
  char buf[64];
  mca_node_layout *layout;
  mcu_button *button;

  // Panel
  MCcall(mcu_init_panel(module_node, &fd->panel));

  layout = fd->panel->node->layout;
  layout->max_width = 320;
  layout->padding = (mc_paddingf){40, 40, 40, 40};
  layout->max_height = 540;

  fd->panel->background_color = (render_color){0.35f, 0.35f, 0.35f, 1.f};

  // Message Block
  MCcall(mcu_init_textblock(fd->panel->node, &fd->message_textblock));

  layout = fd->message_textblock->node->layout;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  layout->preferred_width = 0.f;

  fd->message_textblock->background_color = COLOR_GRAPE;

  // Open Button
  MCcall(mcu_init_button(fd->panel->node, &button));

  layout = button->node->layout;
  layout->preferred_width = 200;
  layout->preferred_height = 26;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(mc_set_str(&button->str, "Select Current Folder"));
  button->tag = fd;
  button->left_click = (void *)&_mc_fd_open_current_clicked;

  // Textblocks to display items
  for (int a = 0; a < 12; ++a) {
    MCcall(mcu_init_button(fd->panel->node, &button));

    if (button->node->name) {
      free(button->node->name);
      button->node->name = NULL;
    }
    sprintf(buf, "folder-dialog-item-button-%i", a);
    button->node->name = strdup(buf);

    button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    button->node->layout->padding = (mc_paddingf){6, 24 + 8 + a * 27, 6, 0};
    button->node->layout->max_width = 0U;
    button->node->layout->visible = false;

    button->tag = fd;
    button->left_click = (void *)&_mc_fd_item_selected;

    MCcall(mc_set_str(&button->str, "button"));

    MCcall(append_to_collection((void ***)&fd->displayed_items.items, &fd->displayed_items.capacity,
                                &fd->displayed_items.count, button));
  }

  return 0;
}

int mc_fd_init_folder_dialog(mc_node *app_root)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  // TODO -- get rid of node type

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "folder-dialog", &node));
  MCcall(mca_init_node_layout(&node->layout));
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  node->layout->visible = false;
  node->layout->z_layer_index = 9;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_render_folder_dialog_headless;
  node->layout->render_present = (void *)&_mc_render_folder_dialog_present;
  node->layout->handle_input_event = (void *)&_mc_handle_folder_dialog_input;

  MCcall(mc_fd_init_data(node));
  MCcall(mc_fd_init_ui(node));

  MCcall(mca_register_event_handler(MC_APP_EVENT_FOLDER_DIALOG_REQUESTED, _mc_fd_on_folder_dialog_request, node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  return 0;
}