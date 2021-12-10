/* hierarchy_viewer_window.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/midge_app.h"
#include "env/environment_definitions.h"

#include "modules/collections/hash_table.h"
#include "modules/mc_io/mc_file.h"
// #include "modules/hierarchy_viewer/hierarchy_viewer.h"
#include "modules/ui_elements/ui_elements.h"

#define MCM_hvwr_ENTRY_HEIGHT 24
#define MCM_hvwr_ENTRY_INDENT 10

typedef enum mcm_hvwr_entry_type {
  hvwr_NULL = 0,
  hvwr_ENTRY_UNKNOWN,
  hvwr_ENTRY_PROJECT,
  hvwr_ENTRY_DIRECTORY,
  hvwr_ENTRY_HEADER,
  hvwr_ENTRY_SOURCE,
} mcm_hvwr_entry_type;

typedef struct mcm_hvwr_entry {
  unsigned long hash;
  mc_str *path, *name;
  mcm_hvwr_entry_type type;
  int indent;
  bool collapsed;
} mcm_hvwr_entry;

typedef struct hierarchy_viewer_data {
  mc_node *node;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
    render_color background;
  } render_target;

  struct {
    unsigned int capacity, count;
    mc_project_info **items;
  } projects;

  struct {
    unsigned int capacity, count;
    mcm_hvwr_entry **items;
  } entries;

  hash_table_t collapsed_entries;
  struct {
    unsigned long entry_hash;
    int rough_position; // The 'index' the entry hash was last time
  } scroll_offset;

  struct {
    unsigned int capacity, count, utilized;
    mcu_textblock **items;
  } textblocks;

} hierarchy_viewer_data;

typedef struct explored_project {
  mc_project_info *project;
  mcu_panel *root_panel;

} explored_project;

mcm_hvwr_entry_type _mcm_hvwr_parse_entry_type(char *extension)
{
  // printf("_mcm_hvwr_parse_entry_type:'%s'\n", extension);
  switch (extension[0]) {
  case '\0':
    return hvwr_ENTRY_UNKNOWN;
    break;
  case 'h': {
    if (strlen(extension) == 1) {
      return hvwr_ENTRY_HEADER;
    }
    else {
      return hvwr_ENTRY_UNKNOWN;
    }
  } break;
  case 'c': {
    if (strlen(extension) == 1) {
      return hvwr_ENTRY_SOURCE;
    }
    else {
      return hvwr_ENTRY_UNKNOWN;
    }
  } break;
  default:
    return hvwr_ENTRY_UNKNOWN;
    break;
  }
}

render_color _mcm_hvwr_get_entry_font_color(mcm_hvwr_entry_type entry_type)
{
  switch (entry_type) {
  case hvwr_ENTRY_UNKNOWN:
    return COLOR_SILVER;
  case hvwr_ENTRY_PROJECT:
    return COLOR_ISLAMIC_GREEN;
  case hvwr_ENTRY_DIRECTORY:
    return COLOR_LIGHT_YELLOW;
  case hvwr_ENTRY_HEADER:
    return COLOR_NODE_ORANGE;
  case hvwr_ENTRY_SOURCE:
    return COLOR_POWDER_BLUE;
  default: {
    break;
  }
  }

  // ERROR
  return COLOR_RED;
}

int _mcm_hvwr_set_entry_textblock(hierarchy_viewer_data *hvwr, mcu_textblock *tb, mcm_hvwr_entry_type type,
                                  const char *name, const char *full_path, int indent, bool collapsed)
{
  mcm_hvwr_entry *ent = (mcm_hvwr_entry *)tb->tag;
  MCcall(mc_set_str(ent->name, name));
  ent->type = type;
  ent->indent = indent;
  ent->hash = hash_djb2(full_path);
  MCcall(mc_set_str(ent->path, full_path));
  ent->collapsed = collapsed;

  // printf("_mcm_hvwr_set_entry_textblock tb:%p type:%i name:'%s' full_path:'%s'\n", tb, type, name, full_path);
  tb->node->layout->visible = true;
  tb->node->layout->padding.left = MCM_hvwr_ENTRY_INDENT * indent;
  tb->font_color = _mcm_hvwr_get_entry_font_color(type);

  // Display Name
  char dsp_name[64];
  switch (ent->type) {
  case hvwr_ENTRY_PROJECT:
  case hvwr_ENTRY_DIRECTORY: {
    if (ent->collapsed)
      strcpy(dsp_name, "+ ");
    else
      strcpy(dsp_name, "- ");
  } break;
  case hvwr_ENTRY_UNKNOWN:
  case hvwr_ENTRY_HEADER:
  case hvwr_ENTRY_SOURCE:
    strcpy(dsp_name, "  ");
    break;
  default: {
    MCerror(9152, "Unsupported type:%i", ent->type);
  }
  }
  strcat(dsp_name, name);
  MCcall(mcu_set_textblock_text(tb, dsp_name));

  return 0;
}

int _mcm_hvwr_update_entries_display_from_subdirectory(hierarchy_viewer_data *hvwr, bool *awaiting_offset_entry,
                                                       int *evaluated_count, int indent, const char *directory)
{
  // Obtain a list of all items in the directory
  char *c, path[256], ext[16];
  int len;
  unsigned long h;

  // directory = "projects/cube";
  // printf("_mcm_hvwr_update_entries_display_from_subdirectory: utilized:%u count:%u\n", hvwr->textblocks.utilized,
  //        hvwr->textblocks.count);
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(directory)) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL && hvwr->textblocks.utilized < hvwr->textblocks.count) {
      // Ignore the . / .. entries
      if (!strncmp(ent->d_name, ".", 1))
        continue;

      // Create the full path of the directory entry
      strcpy(path, directory);
      len = strlen(path);
      if (path[len - 1] != '\\' && path[len - 1] != '/') {
        strcat(path, "/");
      }
      strcat(path, ent->d_name);

      // Awaiting Check
      if (*awaiting_offset_entry) {
        h = hash_djb2((unsigned char *)path);
        *awaiting_offset_entry = (h != hvwr->scroll_offset.entry_hash ? true : false);
      }
      ++*evaluated_count;

      // Obtain the path entry type
      struct stat stats;
      int res = stat(path, &stats);
      if (res) {
        MCerror(7464, "Odd? TODO print errno");
      }

      if (S_ISDIR(stats.st_mode)) {
        bool collapsed = hash_table_exists(path, &hvwr->collapsed_entries) ? true : false;

        if (!*awaiting_offset_entry) {
          MCcall(_mcm_hvwr_set_entry_textblock(hvwr, hvwr->textblocks.items[hvwr->textblocks.utilized++],
                                               hvwr_ENTRY_DIRECTORY, ent->d_name, path, indent, collapsed));
        }

        if (!collapsed) {
          // printf("[D]%s\n", path);
          // Obtain the directorys sub-entries and add those too
          MCcall(_mcm_hvwr_update_entries_display_from_subdirectory(hvwr, awaiting_offset_entry, evaluated_count,
                                                                    indent + 1, path));
        }
      }
      else if (!*awaiting_offset_entry) {
        // Obtain the entry type from the file extension
        MCcall(mcf_obtain_file_extension(ent->d_name, ext, 16));
        mcm_hvwr_entry_type entry_type = _mcm_hvwr_parse_entry_type(ext);

        MCcall(_mcm_hvwr_set_entry_textblock(hvwr, hvwr->textblocks.items[hvwr->textblocks.utilized++], entry_type,
                                             ent->d_name, path, indent, false));
      }
    }
    closedir(dir);
  }
  else {
    /* could not open directory */
    perror("");
    MCerror(8528, "Could not open directory '%s'", directory);
    // return EXIT_FAILURE;
  }

  return 0;
}

int _mcm_hvwr_update_entries_display(hierarchy_viewer_data *hvwr)
{
  bool awaiting_offset_entry = hvwr->scroll_offset.entry_hash;

  hvwr->textblocks.utilized = 0;

  mc_project_info *project;
  unsigned long h;
  mcu_textblock *tb;
  mcm_hvwr_entry *ent;
  // TODO -- use this in the future to keep roughly the same scroll position as before (in the case the scroll offset
  // entry hash is never found
  int evaluated = 0;

  for (int p = 0; p < hvwr->projects.count && hvwr->textblocks.utilized < hvwr->textblocks.count; ++p) {
    project = hvwr->projects.items[p];

    bool collapsed = hash_table_exists(project->path, &hvwr->collapsed_entries) ? true : false;

    // Project Entry
    if (awaiting_offset_entry) {
      h = hash_djb2(project->path);
      awaiting_offset_entry = (h != hvwr->scroll_offset.entry_hash);
    }
    if (!awaiting_offset_entry) {
      // Set
      MCcall(_mcm_hvwr_set_entry_textblock(hvwr, hvwr->textblocks.items[hvwr->textblocks.utilized++],
                                           hvwr_ENTRY_PROJECT, project->name, project->path, 0, collapsed));

      if (hvwr->textblocks.utilized >= hvwr->textblocks.count)
        break;
    }
    ++evaluated;

    if (!collapsed) {
      // Recursively work through the items that can be added
      _mcm_hvwr_update_entries_display_from_subdirectory(hvwr, &awaiting_offset_entry, &evaluated, 1, project->path);
    }
  }

  // Disappear the remainder textblocks
  for (int a = hvwr->textblocks.utilized; a < hvwr->textblocks.count; ++a) {
    tb = hvwr->textblocks.items[a];

    tb->node->layout->visible = false;
  }
  return 0;
}

int _mcm_hvwr_update_node_layout(mc_node *node, mc_rectf const *available_area)
{
  // hvwr
  hierarchy_viewer_data *hvwr = (hierarchy_viewer_data *)node->data;

  MCcall(_mcm_hvwr_update_entries_display(hvwr));
  MCcall(mca_set_node_requires_rerender(node));

  // printf("available_area; %i %i %i %i  {children_count=%i}\n", (int)available_area->x, (int)available_area->y,
  //        (int)(available_area->x + available_area->width), (int)(available_area->y + available_area->height),
  //        node->children->count);
  MCcall(mca_update_typical_node_layout(node, available_area));

  return 0;
}

void _mcm_hvwr_render_headless(render_thread_info *render_thread, mc_node *node)
{
  // Toggle
  node->layout->__requires_rerender = false;

  // hvwr
  hierarchy_viewer_data *hvwr = (hierarchy_viewer_data *)node->data;

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
  irq->clear_color = hvwr->render_target.background;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = hvwr->render_target.width;   // TODO
  irq->image_height = hvwr->render_target.height; // TODO
  irq->data.target_image.image = hvwr->render_target.image;
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

void _mcm_hvwr_render_present(image_render_details *image_render_queue, mc_node *node)
{
  // Toggle
  node->layout->__requires_rerender = false;

  // hvwr
  hierarchy_viewer_data *hvwr = (hierarchy_viewer_data *)node->data;

  // Render Image
  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, hvwr->render_target.width,
                                         hvwr->render_target.height, hvwr->render_target.image);
}

int _mcm_hvwr_activate_entry(hierarchy_viewer_data *hvwr, mcm_hvwr_entry *entry)
{
  switch (entry->type) {
  case hvwr_ENTRY_PROJECT:
  case hvwr_ENTRY_DIRECTORY: {
    // Toggle the collapse
    if (hash_table_find(entry->hash, &hvwr->collapsed_entries) != NULL) {
      hash_table_remove(entry->hash, &hvwr->collapsed_entries);
    }
    else {
      int res = hash_table_insert(entry->hash, NULL, &hvwr->collapsed_entries);
      if (res) {
        MCerror(8530, "TODO?");
      }
    }

    mca_set_node_requires_layout_update(hvwr->node);
  } break;
  case hvwr_ENTRY_HEADER:
  case hvwr_ENTRY_SOURCE: {
    char *path = strdup(entry->path->text);
    // printf("path %p created\n", path);
    MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_SOURCE_FILE_OPEN_REQ, path, 1, path));
  } break;
  case hvwr_ENTRY_UNKNOWN: {
    char buf[8];
    MCcall(mcf_obtain_file_extension(entry->path->text, buf, 8));
    if (!strcmp(buf, "vert") || !strcmp(buf, "frag") || !strcmp(buf, "md")) {
      char *path = strdup(entry->path->text);
      // printf("path %p created\n", path);
      MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_SOURCE_FILE_OPEN_REQ, path, 1, path));
    }
    else if (!strcmp(buf, "png") || !strcmp(buf, "jpeg") || !strcmp(buf, "jpg")) {
      char buf[256];
      sprintf(buf, "gimp %s", entry->path->text);
      system(buf);
    }
    else {
      puts("NotYetImplemented - a means to open the file requested");
    }
  } break;
  default:
    puts("NotYetImplemented - a means to open the file requested");
    break;
  }

  return 0;
}

void _mcm_hvwr_handle_input(mc_node *node, mci_input_event *input_event)
{
  // hvwr
  hierarchy_viewer_data *hvwr = (hierarchy_viewer_data *)node->data;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {

    switch (input_event->button_code) {
    case MOUSE_BUTTON_SCROLL_DOWN:
      break;
    case MOUSE_BUTTON_SCROLL_UP:
      break;
    case MOUSE_BUTTON_LEFT: {
      int relative_y = input_event->input_state->mouse.y - node->layout->__bounds.y - 4;
      if (relative_y >= 0) {
        relative_y /= MCM_hvwr_ENTRY_HEIGHT;

        if (relative_y < hvwr->textblocks.utilized) {
          mcu_textblock *tb = (mcu_textblock *)hvwr->textblocks.items[relative_y]->node->data;
          _mcm_hvwr_activate_entry(hvwr, (mcm_hvwr_entry *)tb->tag);
        }
      }
    } break;
    default:
      break;
    }

    // printf("click was at %u %u\n", input_event->input_state->mouse.x, input_event->input_state->mouse.y);
    // mc_rectf *b = &hvwr->textblocks.items[3]->node->layout->__bounds;
    // printf("item 4 is %.2f %.2f %.2f %.2f\n", b->x, b->y, b->width, b->height);
    // b = &hvwr->textblocks.items[4]->node->layout->__bounds;
    // printf("item 5 is %.2f %.2f %.2f %.2f\n", b->x, b->y, b->width, b->height);
    // b = &hvwr->textblocks.items[5]->node->layout->__bounds;
    // printf("item 6 is %.2f %.2f %.2f %.2f\n", b->x, b->y, b->width, b->height);

    // printf("obb\n");
    // if (input_event->button_code == MOUSE_BUTTON_LEFT) {

    //   // printf("left_click:offset=%i %.3f  line_index:%i\n", input_event->input_state->mouse.y,
    //   // node->layout->__bounds.y,
    //   //        (int)((input_event->input_state->mouse.y - node->layout->__bounds.y - fedit->lines.padding.top) /
    //   //              fedit->lines.vertical_stride));
    //   int line_index = -1;
    //   int click_relative_y =
    //       input_event->input_state->mouse.y - (int)(node->layout->__bounds.y + fedit->lines.padding.top);
    //   if (click_relative_y >= 0) {
    //     line_index = (int)((float)click_relative_y / fedit->lines.vertical_stride);
    //   }
    //   if (line_index >= 0) {
    //     // Find the column index
    //     int click_relative_x =
    //         input_event->input_state->mouse.x -
    //         (int)(node->layout->__bounds.x + fedit->lines.padding.left - fedit->font_horizontal_stride * 0.5f);
    //     if (click_relative_x < 0 && click_relative_x > -3)
    //       click_relative_x = 0;
    //     if (click_relative_x >= 0) {

    //       mce_set_function_editor_cursor_position(fedit, line_index,
    //                                               (int)((float)click_relative_x / fedit->font_horizontal_stride));
    //     }
    //   }
    // }

    // mca_focus_node(node);
    // input_event->handled = true;
  }
}

int _mcm_hvwr_print_node(mc_node *node, bool print_name, bool print_type, int print_children_depth, int _recur) {
  
  char buf[512];
  strcpy(buf, ">");
  int r = _recur;
  while(r > 0) {
    strcat(buf, "  ");
    --r;
  }
  strcat(buf, "--");

  strcat(buf, " ");
  if(print_name)
    strcat(buf, node->name);

  if(print_type) {
    char n[33];
    sprintf(n, "%i", (int)node->type);
    if(print_name)
      strcat(buf, ":");
    strcat(buf, n);
  }
  puts(buf);

  // printf("node->children:%p\n", node->children);
  // if(node->children)
  //   printf("node->children->count:%i\n", node->children->count);
  if(print_children_depth > 0 && node->children && node->children->count) {
    for(int i = 0; i < node->children->count; ++i) {
      mc_node *child = node->children->items[i];
      if(child->layout && child->layout->visible) {
        MCcall(_mcm_hvwr_print_node(node->children->items[i], print_name, print_type, print_children_depth - 1, _recur + 1));
      }
    }
  }

  return 0;
}

int __mcm_hvwr_hierarchy_updated(void *handler_state, void *event_args) {
  mc_node *last_attached_node = (mc_node *)event_args;

  hierarchy_viewer_data *hvwr = (hierarchy_viewer_data *)handler_state;

  mc_node *root = hvwr->node;
  while(root->parent)
    root = root->parent;

  puts("hierarchy-updated:");
  printf("[node trigger:%s-%i\n", last_attached_node->name, last_attached_node->type);
  MCcall(_mcm_hvwr_print_node(root, true, true, INT32_MAX, 0));

  return 0;
}

int _mcm_hvwr_init_ui(mc_node *hvwr_node)
{
  hierarchy_viewer_data *hvwr = (hierarchy_viewer_data *)hvwr_node->data;

  // Textblocks
  int initial_entry_count = hvwr_node->layout->preferred_height / MCM_hvwr_ENTRY_HEIGHT;
  for (int a = 0; a < initial_entry_count; ++a) {
    mcu_textblock *tb;
    MCcall(mcu_init_textblock(hvwr_node, &tb));

    if (tb->node->name)
      free(tb->node->name);
    char tb_name[64];
    sprintf(tb_name, "textblock-entry-%i", a);
    tb->node->name = strdup(tb_name);

    tb->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
    tb->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    tb->node->layout->padding.left = 6;
    tb->node->layout->padding.top = 4 + a * MCM_hvwr_ENTRY_HEIGHT;

    tb->background_color = COLOR_TRANSPARENT;
    tb->font_color = COLOR_BLACK;

    MCcall(mcu_set_textblock_text(tb, "(null)"));

    // tb->left_click = (void *)&_mcm_hvwr_textblock_left_click;
    mcm_hvwr_entry *ent = tb->tag = (void *)malloc(sizeof(mcm_hvwr_entry));
    MCcall(mc_alloc_str(&ent->path));
    MCcall(mc_alloc_str(&ent->name)); // TODO -- think this is still a redundant field - probably some others are too

    MCcall(append_to_collection((void ***)&hvwr->textblocks.items, &hvwr->textblocks.capacity, &hvwr->textblocks.count,
                                tb));
  }

  return 0;
}

int _mcm_hvwr_init_data(mc_node *root)
{
  hierarchy_viewer_data *hvwr = (hierarchy_viewer_data *)malloc(sizeof(hierarchy_viewer_data));
  hvwr->node = root;
  root->data = hvwr;

  hvwr->render_target.width = root->layout->preferred_width;
  hvwr->render_target.height = root->layout->preferred_height;
  hvwr->render_target.background = COLOR_BLACKCURRANT;

  hvwr->entries.capacity = hvwr->entries.count = 0;
  hvwr->projects.capacity = hvwr->projects.count = 0;
  hvwr->textblocks.capacity = hvwr->textblocks.count = hvwr->textblocks.utilized = 0;

  MCcall(init_hash_table(256, &hvwr->collapsed_entries));
  hvwr->scroll_offset.entry_hash = 0LU;
  hvwr->scroll_offset.rough_position = 0;

  MCcall(mcr_create_texture_resource(hvwr->render_target.width, hvwr->render_target.height,
                                     MVK_IMAGE_USAGE_RENDER_TARGET_2D, &hvwr->render_target.image));

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  while (!hvwr->render_target.image) {
    // puts("wait");
    usleep(100);
  }
  return 0;
}

int mcm_init_hierarchy_viewer(mc_node *app_root)
{
  //   midge_app_info *global_data;
  //   mc_obtain_midge_app_info(&global_data);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  // TODO -- get rid of node type
  mc_node *node;
  mca_init_mc_node(NODE_TYPE_ABSTRACT, "hierarchy-viewer-root", &node);
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 299;
  node->layout->preferred_height = 440;

  node->layout->visible = true;

  node->layout->padding.left = 1;
  node->layout->padding.top = 1;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&_mcm_hvwr_update_node_layout;
  node->layout->render_headless = (void *)&_mcm_hvwr_render_headless;
  node->layout->render_present = (void *)&_mcm_hvwr_render_present;
  node->layout->handle_input_event = (void *)&_mcm_hvwr_handle_input;

  MCcall(_mcm_hvwr_init_data(node));

  MCcall(_mcm_hvwr_init_ui(node));

  // MCcall(mca_register_event_handler(MC_APP_EVENT_VISUAL_HIERARCHY_UPDATED, (void *)__mcm_hvwr_hierarchy_updated, (void *)node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));
  return 0;
}