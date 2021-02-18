/* file_dialog.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/app_modules.h"
#include "core/midge_app.h"
#include "render/render_common.h"

#include "modules/mc_io/mc_file.h"
#include "modules/ui_elements/ui_elements.h"

// #include "env/environment_definitions.h"
// #include "render/render_thread.h"
// #include "ui/ui_definitions.h"

typedef enum mc_file_dialog_display_mode {
  MC_FILEDIALOG_DISPLAY_NULL = 0,
  MC_FILEDIALOG_DISPLAY_ALL = 0,
} mc_file_dialog_display_mode;

typedef struct mc_file_dialog_data {
  mc_node *node;

  render_color shade_color;

  bool specified_path_set_from_folder_item;

  mcu_panel *panel;
  mcu_textblock *message_textblock;
  struct {
    unsigned int capacity, count, utilized;
    mcu_button **items;
  } displayed_items;
  mcu_textbox *input_textbox;

  // struct {
  //   unsigned int width, height;
  //   mcr_texture_image *image;
  // } render_target;

  mc_str *current_directory;
  mc_file_dialog_display_mode mode;
  struct {
    void *state;
    void *result_delegate;
  } callback;

} mc_file_dialog_data;

void _mc_render_file_dialog_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)node->data;

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

void _mc_render_file_dialog_present(image_render_details *image_render_queue, mc_node *node)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)node->data;

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, fd->shade_color);

  // Children
  mca_render_node_list_present(image_render_queue, node->children);
}

// TODO -- because 'private static' symbols have to be public in TCC for now -- these names are longer and more
// descriptive than they should be
// When its no longer the case - rename them
void _mc_handle_file_dialog_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mco_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

int _mc_file_dialog_submit(mc_file_dialog_data *fd)
{
  char buf[256];
  strcpy(buf, fd->current_directory->text);
  mcf_concat_filepath(buf, 256, fd->input_textbox->contents->text);

  // Activate Callback
  // TODO -- const this char *selected_path delegate for all users
  int (*result_delegate)(void *invoker_state, char *selected_path) =
      (int (*)(void *, char *))fd->callback.result_delegate;
  if (result_delegate) {
    MCcall(result_delegate(fd->callback.state, buf));
  }
  fd->callback.state = NULL;
  fd->callback.result_delegate = NULL;

  // Wrap Up
  fd->node->layout->visible = false;
  MCcall(mca_set_node_requires_rerender(fd->node));

  return 0;
}

void _mc_file_dialog_textbox_submit(mci_input_event *input_event, mcu_textbox *textbox)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)textbox->tag;

  _mc_file_dialog_submit(fd);
}

void _mc_file_dialog_submit_clicked(mci_input_event *input_event, mcu_button *button)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)button->tag;

  _mc_file_dialog_submit(fd);
}

int _mc_file_dialog_open_directory(mc_file_dialog_data *fd, const char *starting_directory)
{
  fd->displayed_items.utilized = 0U;
  char *c, path[256], ext[16];
  int len;
  mcu_button *button;

  if (starting_directory) {
    MCcall(mc_set_str(fd->current_directory, starting_directory));
  }
  else {
    if (fd->current_directory->len == 0) {
      if (!getcwd(path, 256)) {
        MCerror(9135, "Current Working Directory too large for this pretty big buffer");
      }
      MCcall(mc_set_str(fd->current_directory, path));
    }
  }

  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(fd->current_directory->text)) != NULL) {
    // printf("_mc_fd_open_directory:'%s'\n", fd->current_directory->text);

    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL && fd->displayed_items.utilized < fd->displayed_items.count) {
      // Ignore the "." entry
      if (!strcmp(ent->d_name, "."))
        continue;

      // Create the full path of the entry
      strcpy(path, fd->current_directory->text);
      len = strlen(path);
      if (path[len - 1] != '\\' && path[len - 1] != '/') {
        strcat(path, "/");
      }
      strcat(path, ent->d_name);

      // printf("path:%s", path);
      // if (fd->mode == MC_FILEDIALOG_DISPLAY_ALL) {
      // Obtain the path entry type
      struct stat stats;
      int res = stat(path, &stats);
      if (res) {
        MCerror(1919, "Odd? TODO print errno");
      }

      bool is_dir = S_ISDIR(stats.st_mode);

      // Assign entry
      button = fd->displayed_items.items[fd->displayed_items.utilized++];
      button->font_color = is_dir ? COLOR_LIGHT_YELLOW : COLOR_WHITE;
      button->node->layout->visible = true;
      MCcall(mc_set_str(&button->str, ent->d_name));
    }
    closedir(dir);
  }
  else {
    /* could not open directory */
    perror("");
    MCerror(8528, "Could not open directory '%s'", fd->current_directory->text);
  }

  for (int a = fd->displayed_items.utilized; a < fd->displayed_items.count; ++a) {
    fd->displayed_items.items[a]->node->layout->visible = false;
  }

  MCcall(mca_set_node_requires_rerender(fd->node));

  return 0;
}

// TODO -- all events app-wide need int returning success etc
void _mc_file_dialog_item_selected(mci_input_event *input_event, mcu_button *button)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)button->tag;

  char buf[256];
  if (!strcmp(button->str.text, "..")) {
    mcf_get_parent_directory(buf, 256, fd->current_directory->text);
    _mc_file_dialog_open_directory(fd, buf);

    if (fd->specified_path_set_from_folder_item) {
      mc_set_str(fd->input_textbox->contents, "");
      mca_set_node_requires_rerender(fd->input_textbox->node);
    }
  }
  else {
    strcpy(buf, fd->current_directory->text);
    mcf_concat_filepath(buf, 256, button->str.text);

    struct stat stats;
    int res = stat(buf, &stats);
    if (res) {
      // MCerror(2234, "Odd? TODO print errno");
      puts("ERROR TODO 2234");
    }

    bool is_dir = S_ISDIR(stats.st_mode);
    if (is_dir) {
      _mc_file_dialog_open_directory(fd, buf);

      if (fd->specified_path_set_from_folder_item) {
        mc_set_str(fd->input_textbox->contents, "");
        mca_set_node_requires_rerender(fd->input_textbox->node);
      }
    }
    else {
      if (access(buf, F_OK) == -1) {
        // File doesn't exist!
        puts("ERROR TODO 8244");
      }

      // Set the file name to the textbox
      mc_set_str(fd->input_textbox->contents, button->str.text);
      mca_set_node_requires_rerender(fd->input_textbox->node);
      fd->specified_path_set_from_folder_item = true;
    }
  }
}

int _mc_file_dialog_requested(void *handler_state, void *event_args)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)handler_state;

  void **vary = (void **)event_args;
  char *message = (char *)vary[0];
  char *starting_path = (char *)vary[1];

  // Set Callback Info
  fd->callback.state = vary[2];
  fd->callback.result_delegate = vary[3];

  // Set the message
  MCcall(mcu_set_textblock_text(fd->message_textblock, message == NULL ? "" : message));

  // TODO -- set starting filename
  fd->specified_path_set_from_folder_item = false;

  // Open The Dialog at the starting path
  fd->mode = MC_FILEDIALOG_DISPLAY_ALL;
  MCcall(_mc_file_dialog_open_directory(fd, starting_path));

  // Display
  fd->node->layout->visible = true;
  MCcall(mca_focus_node(fd->input_textbox->node));

  return 0;
}

int _mc_init_file_dialog_data(mc_node *module_node)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)malloc(sizeof(mc_file_dialog_data));
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

int _mc_init_file_dialog_ui(mc_node *module_node)
{
  mc_file_dialog_data *fd = (mc_file_dialog_data *)module_node->data;

  // Locals
  char buf[64];
  mca_node_layout *layout;
  mcu_button *button;

  // Panel
  MCcall(mcu_init_panel(module_node, &fd->panel));

  layout = fd->panel->node->layout;
  layout->max_width = 580;
  layout->padding = (mc_paddingf){40, 40, 40, 40};
  layout->max_height = 340;

  fd->panel->background_color = (render_color){0.35f, 0.35f, 0.35f, 1.f};

  // Message Block
  MCcall(mcu_init_textblock(fd->panel->node, &fd->message_textblock));

  layout = fd->message_textblock->node->layout;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  layout->preferred_width = 0.f;

  fd->message_textblock->background_color = COLOR_TRANSPARENT;

  // Buttons to display items
  for (int a = 0; a < 18; ++a) {
    MCcall(mcu_init_button(fd->panel->node, &button));

    if (button->node->name) {
      free(button->node->name);
      button->node->name = NULL;
    }
    sprintf(buf, "file-dialog-item-button-%i", a);
    button->node->name = strdup(buf);

    button->node->layout->preferred_width = 280;
    button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    button->node->layout->horizontal_alignment = (a % 2) ? HORIZONTAL_ALIGNMENT_RIGHT : HORIZONTAL_ALIGNMENT_LEFT;
    button->node->layout->padding = (mc_paddingf){6 + (a % 2) * 224, 24 + 8 + (a / 2) * 27, 6, 0};
    button->node->layout->max_width = 0U;
    button->node->layout->visible = false;

    button->tag = fd;
    button->left_click = (void *)&_mc_file_dialog_item_selected;

    MCcall(mc_set_str(&button->str, "button"));

    MCcall(append_to_collection((void ***)&fd->displayed_items.items, &fd->displayed_items.capacity,
                                &fd->displayed_items.count, button));
  }

  // Input Textbox
  MCcall(mcu_init_textbox(fd->panel->node, &fd->input_textbox));

  layout = fd->input_textbox->node->layout;
  layout->preferred_width = 420;
  layout->preferred_height = 26;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  fd->input_textbox->tag = fd;
  fd->input_textbox->submit = (void *)&_mc_file_dialog_textbox_submit;

  // Open Button
  MCcall(mcu_init_button(fd->panel->node, &button));

  layout = button->node->layout;
  layout->preferred_width = 120;
  layout->preferred_height = 26;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(mc_set_str(&button->str, "Open"));
  button->tag = fd;
  button->left_click = (void *)&_mc_file_dialog_submit_clicked;

  return 0;
}

int mc_fd_init_file_dialog(mc_node *app_root)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // TODO -- get rid of node type
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "file-dialog", &node));
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
  node->layout->render_headless = (void *)&_mc_render_file_dialog_headless;
  node->layout->render_present = (void *)&_mc_render_file_dialog_present;
  node->layout->handle_input_event = (void *)&_mc_handle_file_dialog_input;

  MCcall(_mc_init_file_dialog_data(node));
  MCcall(_mc_init_file_dialog_ui(node));

  MCcall(mca_register_event_handler(MC_APP_EVENT_FILE_DIALOG_REQUESTED, _mc_file_dialog_requested, node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  return 0;
}