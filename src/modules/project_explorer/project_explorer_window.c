/* project_explorer_window.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/midge_app.h"
#include "env/environment_definitions.h"

#include "modules/collections/hash_table.h"
#include "modules/mc_io/mc_io.h"
#include "modules/project_explorer/project_explorer_window.h"
#include "modules/ui_elements/ui_elements.h"

#define MCM_PJXP_ENTRY_HEIGHT 12
#define MCM_PJXP_ENTRY_INDENT 10

typedef enum mcm_pjxp_entry_type {
  PJXP_NULL = 0,
  PJXP_ENTRY_UNKNOWN,
  PJXP_ENTRY_PROJECT,
  PJXP_ENTRY_DIRECTORY,
  PJXP_ENTRY_HEADER,
  PJXP_ENTRY_SOURCE,
} mcm_pjxp_entry_type;

typedef struct mcm_pjxp_entry {
  unsigned long hash;
  mc_str *path;
  mcm_pjxp_entry_type type;
  int indent;
  bool collapsed;
} mcm_pjxp_entry;

typedef struct project_explorer_data {
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
    mcm_pjxp_entry **items;
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

} project_explorer_data;

typedef struct explored_project {
  mc_project_info *project;
  mcu_panel *root_panel;

} explored_project;

// int _mcm_pjxp_determine_typical_node_extents(mc_node *node, layout_extent_restraints restraints)
// {
//   const float MAX_EXTENT_VALUE = 100000.f;
//   mca_node_layout *layout = node->layout;

//   // Children
//   if (node->children) {
//     for (int a = 0; a < node->children->count; ++a) {
//       mc_node *child = node->children->items[a];
//       if (child->layout && child->layout->determine_layout_extents) {
//         // TODO fptr casting
//         void (*determine_layout_extents)(mc_node *, layout_extent_restraints) =
//             (void (*)(mc_node *, layout_extent_restraints))child->layout->determine_layout_extents;
//         determine_layout_extents(child, LAYOUT_RESTRAINT_VERTICAL);
//       }
//     }
//   }

//   if (!layout->preferred_width || !layout->preferred_height) {
//     MCerror(5798, "ERROR -- preferred width and height should always be set, as it is a root-adjacent node");
//   }

//   // Set to preferred values
//   layout->determined_extents.width = layout->preferred_width;
//   layout->determined_extents.height = layout->preferred_height;

//   return 0;
// }

void _mcm_pjxp_render_headless(mc_node *node)
{
  // Toggle
  node->layout->__requires_rerender = false;

  // pjxp
  project_explorer_data *pjxp = (project_explorer_data *)node->data;

  // Children
  for (int a = 0; a < node->children->count; ++a) {
    mc_node *child = node->children->items[a];
    if (child->layout && child->layout->visible && child->layout->render_headless &&
        child->layout->__requires_rerender) {
      // TODO fptr casting
      void (*render_node_headless)(mc_node *) = (void (*)(mc_node *))child->layout->render_headless;
      render_node_headless(child);
    }
  }

  // Render the render target
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  image_render_details *irq;
  mcr_obtain_image_render_request(global_data->render_thread, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = pjxp->render_target.background;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = pjxp->render_target.width;   // TODO
  irq->image_height = pjxp->render_target.height; // TODO
  irq->data.target_image.image = pjxp->render_target.image;
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

void _mcm_pjxp_render_present(image_render_details *image_render_queue, mc_node *node)
{
  // Toggle
  node->layout->__requires_rerender = false;

  // pjxp
  project_explorer_data *pjxp = (project_explorer_data *)node->data;

  // Render Image
  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                         (unsigned int)node->layout->__bounds.y, pjxp->render_target.width,
                                         pjxp->render_target.height, pjxp->render_target.image);
}

void _mcm_pjxp_handle_input(mc_node *node, mci_input_event *input_event)
{
  // pjxp
  project_explorer_data *pjxp = (project_explorer_data *)node->data;

  if (input_event->type == INPUT_EVENT_MOUSE_PRESS) {
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
    input_event->handled = true;
  }
}

mcm_pjxp_entry_type _mcm_pjxp_parse_entry_type(char *extension)
{
  // printf("_mcm_pjxp_parse_entry_type:'%s'\n", extension);
  switch (extension[0]) {
  case '\0':
    return PJXP_ENTRY_UNKNOWN;
    break;
  case 'h': {
    if (strlen(extension) == 1) {
      return PJXP_ENTRY_HEADER;
    }
    else {
      return PJXP_ENTRY_UNKNOWN;
    }
  } break;
  case 'c': {
    if (strlen(extension) == 1) {
      return PJXP_ENTRY_SOURCE;
    }
    else {
      return PJXP_ENTRY_UNKNOWN;
    }
  } break;
  default:
    return PJXP_ENTRY_UNKNOWN;
    break;
  }
}

// int _mcm_pjxp_add_entry(project_explorer_data *xp, const char *full_path, const char *filename,
//                         mcm_pjxp_entry_type entry_type, int indent)
// {
//   mcm_pjxp_entry *entry = (mcm_pjxp_entry *)malloc(
//       sizeof(mcm_pjxp_entry)); // TODO -- use a pool for when projects can be closed & opened & updated
//   MCcall(init_mc_str(&entry->name));
//   MCcall(set_mc_str(entry->name, filename));
//   MCcall(init_mc_str(&entry->path));
//   MCcall(set_mc_str(entry->path, full_path));
//   entry->type = entry_type;
//   entry->indent = indent;
//   entry->collapsed = false;

//   MCcall(append_to_collection((void ***)&xp->entries.items, &xp->entries.capacity, &xp->entries.count, entry));

//   return 0;
// }

// int _mcm_pjxp_add_project_file_structure_recursive(project_explorer_data *xp, const char *directory, int indent)
// {
//   // Obtain a list of all items in the directory
//   mc_str *str;
//   MCcall(init_mc_str(&str));

//   char *c, ext[16];
//   int a;

//   DIR *dir;
//   struct dirent *ent;
//   if ((dir = opendir(directory)) != NULL) {
//     /* print all the files and directories within directory */
//     while ((ent = readdir(dir)) != NULL) {
//       // Remove the . / .. entries
//       if (!strncmp(ent->d_name, ".", 1))
//         continue;

//       // Create the full path of the directory entry
//       MCcall(set_mc_str(str, directory));
//       if (str->text[str->len - 1] != '\\' && str->text[str->len - 1] != '/') {
//         MCcall(append_char_to_mc_str(str, '/'));
//       }
//       MCcall(append_to_mc_str(str, ent->d_name));

//       // Obtain the path entry type
//       struct stat stats;
//       int res = stat(str->text, &stats);
//       if (res) {
//         MCerror(1919, "Odd? TODO print errno");
//       }

//       if (S_ISDIR(stats.st_mode)) {
//         MCcall(_mcm_pjxp_add_entry(xp, str->text, ent->d_name, PJXP_ENTRY_DIRECTORY, indent));

//         // printf("[D]%s\n", str->text);
//         // Obtain the directorys sub-entries and add those too
//         MCcall(_mcm_pjxp_add_project_file_structure_recursive(xp, str->text, indent + 1));
//       }
//       else {
//         // Obtain the entry type from the file extension
//         MCcall(mcf_obtain_file_extension(ent->d_name, ext, 16));
//         mcm_pjxp_entry_type entry_type = _mcm_pjxp_parse_entry_type(ext);

//         MCcall(_mcm_pjxp_add_entry(xp, str->text, ent->d_name, entry_type, indent));
//       }
//     }
//     closedir(dir);
//   }
//   else {
//     /* could not open directory */
//     perror("");
//     MCerror(8528, "Could not open directory '%s'", directory);
//     // return EXIT_FAILURE;
//   }

//   // Add to parent
//   return 0;
// }

// int _mcm_pjxp_update_entries_representation(project_explorer_data *pjxp) {

//   // Create an entry for each project and all items within that project
//   // -- maintaining previous entry information as much as possible
//   mcm_pjxp_entry *prev_entries[pjxp->entries.capacity]

//   return 0;
// }

render_color _mcm_pjxp_get_entry_font_color(mcm_pjxp_entry_type entry_type)
{
  switch (entry_type) {
  case PJXP_ENTRY_UNKNOWN:
    return COLOR_SILVER;
  case PJXP_ENTRY_PROJECT:
    return COLOR_ISLAMIC_GREEN;
  case PJXP_ENTRY_DIRECTORY:
    return COLOR_LIGHT_YELLOW;
  case PJXP_ENTRY_HEADER:
    return COLOR_NODE_ORANGE;
  case PJXP_ENTRY_SOURCE:
    return COLOR_POWDER_BLUE;
  default: {
    break;
  }
  }

  // ERROR
  return COLOR_RED;
}

int _mcm_pjxp_set_entry_textblock(project_explorer_data *pjxp, mcu_textblock *tb, mcm_pjxp_entry_type type,
                                  const char *name, const char *full_path, int indent)
{
  // printf("_mcm_pjxp_set_entry_textblock tb:%p type:%i name:'%s' full_path:'%s'\n", tb, type, name, full_path);
  MCcall(set_mc_str(tb->str, name));
  tb->node->layout->visible = true;
  tb->node->layout->padding.left = MCM_PJXP_ENTRY_INDENT * indent;
  tb->font_color = _mcm_pjxp_get_entry_font_color(type);

  mcm_pjxp_entry *ent = (mcm_pjxp_entry *)tb->tag;
  ent->type = type;
  ent->indent = indent;
  ent->hash = hash_djb2(full_path);
  MCcall(set_mc_str(ent->path, full_path));
  ent->collapsed = (hash_table_find(ent->hash, &pjxp->collapsed_entries) != NULL) ? true : false;

  return 0;
}

int _mcm_pjxp_update_entries_display_from_subdirectory(project_explorer_data *pjxp, bool *awaiting_offset_entry,
                                                       int *evaluated_count, int indent, const char *directory)
{
  // Obtain a list of all items in the directory
  char *c, path[256], ext[16];
  int len;
  unsigned long h;

  // directory = "/home/jason/midge/projects/cube";
  // printf("_mcm_pjxp_update_entries_display_from_subdirectory: utilized:%u count:%u\n", pjxp->textblocks.utilized,
  //        pjxp->textblocks.count);
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(directory)) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL && pjxp->textblocks.utilized < pjxp->textblocks.count) {
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
        *awaiting_offset_entry = (h != pjxp->scroll_offset.entry_hash ? true : false);
      }
      ++*evaluated_count;

      // Obtain the path entry type
      struct stat stats;
      int res = stat(path, &stats);
      if (res) {
        MCerror(1919, "Odd? TODO print errno");
      }

      if (S_ISDIR(stats.st_mode)) {
        if (!*awaiting_offset_entry)
          MCcall(_mcm_pjxp_set_entry_textblock(pjxp, pjxp->textblocks.items[pjxp->textblocks.utilized++],
                                               PJXP_ENTRY_DIRECTORY, ent->d_name, path, indent));

        // printf("[D]%s\n", path);
        // Obtain the directorys sub-entries and add those too
        MCcall(_mcm_pjxp_update_entries_display_from_subdirectory(pjxp, awaiting_offset_entry, evaluated_count,
                                                                  indent + 1, path));
      }
      else if (!*awaiting_offset_entry) {
        // Obtain the entry type from the file extension
        MCcall(mcf_obtain_file_extension(ent->d_name, ext, 16));
        mcm_pjxp_entry_type entry_type = _mcm_pjxp_parse_entry_type(ext);

        MCcall(_mcm_pjxp_set_entry_textblock(pjxp, pjxp->textblocks.items[pjxp->textblocks.utilized++], entry_type,
                                             ent->d_name, path, indent));
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

int _mcm_pjxp_update_entries_display(project_explorer_data *pjxp)
{
  bool awaiting_offset_entry = pjxp->scroll_offset.entry_hash;

  pjxp->textblocks.utilized = 0;

  mc_project_info *project;
  unsigned long h;
  mcu_textblock *tb;
  mcm_pjxp_entry *ent;
  // TODO -- use this in the future to keep roughly the same scroll position as before (in the case the scroll offset
  // entry hash is never found
  int evaluated = 0;

  for (int p = 0; p < pjxp->projects.count && pjxp->textblocks.utilized < pjxp->textblocks.count; ++p) {
    project = pjxp->projects.items[p];

    // Project Entry
    if (awaiting_offset_entry) {
      h = hash_djb2(project->path);
      awaiting_offset_entry = (h != pjxp->scroll_offset.entry_hash);
    }
    if (!awaiting_offset_entry) {
      // Set
      MCcall(_mcm_pjxp_set_entry_textblock(pjxp, pjxp->textblocks.items[pjxp->textblocks.utilized++],
                                           PJXP_ENTRY_PROJECT, project->name, project->path, 0));

      if (pjxp->textblocks.utilized >= pjxp->textblocks.count)
        break;
    }
    ++evaluated;

    // Recursively work through the items that can be added
    _mcm_pjxp_update_entries_display_from_subdirectory(pjxp, &awaiting_offset_entry, &evaluated, 1, project->path);
  }

  // Disappear the remainder textblocks
  for (int a = pjxp->textblocks.utilized; a < pjxp->textblocks.count; ++a) {
    tb = pjxp->textblocks.items[a];

    tb->node->layout->visible = false;
  }

  // Finish Up
  MCcall(mca_set_node_requires_layout_update(pjxp->node));
  return 0;
}

int _mcm_pjxp_project_loaded(void *handler_state, void *event_args)
{
  // Args
  project_explorer_data *pjxp = (project_explorer_data *)handler_state;
  mc_project_info *project = (mc_project_info *)event_args;

  // Ensure node is shown - now at least one project is being tracked
  pjxp->node->layout->visible = true;

  MCcall(
      append_to_collection((void ***)&pjxp->projects.items, &pjxp->projects.capacity, &pjxp->projects.count, project));

  // // Update Entry Info
  // MCcall(_mcm_pjxp_update_entries_representation(pjxp));
  // printf("project_explorer: %u entries loaded\n", pjxp->entries.count);

  // Reset the entries displayed
  MCcall(_mcm_pjxp_update_entries_display(pjxp));
  // printf("_mcm_pjxp_update_entries_display\n");

  return 0;
}

void _mcm_pjxp_textblock_left_click(mci_input_event *input_event, mcu_textblock *textblock)
{
  if (!textblock->tag) {
    MCVerror(7389, "_mcm_pjxp_textblock_left_click missing tag");
  }

  // Obtain Attached Data
  project_explorer_data *pjxp = (project_explorer_data *)textblock->node->parent->data;
  mcm_pjxp_entry *entry = (mcm_pjxp_entry *)textblock->tag;

  // printf("LEFT-CLICK-PROJECT_EXPLORER-ITEM:'%s'\n", entry->path->text);
  switch (entry->type) {
  case PJXP_ENTRY_PROJECT:
  case PJXP_ENTRY_DIRECTORY: {
    // Toggle the collapse
    if (hash_table_find(entry->hash, &pjxp->collapsed_entries) != NULL) {
      hash_table_remove(entry->hash, &pjxp->collapsed_entries);
    }
    else {
      int res = hash_table_insert(entry->hash, 1, &pjxp->collapsed_entries);
      if (res) {
        MCVerror(8530, "TODO?");
      }
    }

    mca_set_node_requires_layout_update(pjxp->node);
    // TODO -- why does it also require rerender set
    TROUBLE
    // mca_set_node_requires_rerender(pjxp->node);
  } break;
  default:
    break;
  }
}

int _mcm_pjxp_init_ui(mc_node *pjxp_node)
{
  project_explorer_data *pjxp = (project_explorer_data *)pjxp_node->data;

  // Textblocks
  int initial_entry_count = pjxp_node->layout->preferred_width / MCM_PJXP_ENTRY_HEIGHT;
  for (int a = 0; a < initial_entry_count; ++a) {
    mcu_textblock *tb;
    MCcall(mcu_init_textblock(pjxp_node, &tb));

    tb->node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
    tb->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    tb->node->layout->padding.left = 6;
    tb->node->layout->padding.top = 4 + a * 24;

    tb->background_color = COLOR_TRANSPARENT;
    tb->font_color = COLOR_BLACK;

    MCcall(set_mc_str(tb->str, "(null)"));

    tb->left_click = (void *)&_mcm_pjxp_textblock_left_click;
    mcm_pjxp_entry *ent = tb->tag = (void *)malloc(sizeof(mcm_pjxp_entry));
    MCcall(init_mc_str(&ent->path));

    MCcall(append_to_collection((void ***)&pjxp->textblocks.items, &pjxp->textblocks.capacity, &pjxp->textblocks.count,
                                tb));
  }

  return 0;
}

int _mcm_pjxp_init_data(mc_node *root)
{
  project_explorer_data *pjxp = (project_explorer_data *)malloc(sizeof(project_explorer_data));
  pjxp->node = root;
  root->data = pjxp;

  pjxp->render_target.width = root->layout->preferred_width;
  pjxp->render_target.height = root->layout->preferred_height;
  pjxp->render_target.background = COLOR_BLACKCURRANT;

  pjxp->entries.capacity = pjxp->entries.count = 0;
  pjxp->projects.capacity = pjxp->projects.count = 0;
  pjxp->textblocks.capacity = pjxp->textblocks.count = pjxp->textblocks.utilized = 0;

  create_hash_table(256, &pjxp->collapsed_entries);
  pjxp->scroll_offset.entry_hash = 0LU;
  pjxp->scroll_offset.rough_position = 0;

  MCcall(mcr_create_texture_resource(pjxp->render_target.width, pjxp->render_target.height,
                                     MVK_IMAGE_USAGE_RENDER_TARGET_2D, &pjxp->render_target.image));

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  while (!pjxp->render_target.image) {
    // puts("wait");
    usleep(100);
  }
  return 0;
}

int mcm_init_project_explorer(mc_node *app_root)
{
  //   midge_app_info *global_data;
  //   mc_obtain_midge_app_info(&global_data);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  // TODO -- get rid of node type
  mc_node *node;
  mca_init_mc_node(NODE_TYPE_ABSTRACT, "project-explorer-root", &node);
  mca_init_node_layout(&node->layout);
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  node->layout->preferred_width = 240;
  node->layout->preferred_height = 420;

  node->layout->visible = false;

  node->layout->padding.left = 4;
  node->layout->padding.top = 4;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mcm_pjxp_render_headless;
  node->layout->render_present = (void *)&_mcm_pjxp_render_present;
  node->layout->handle_input_event = (void *)&_mcm_pjxp_handle_input;

  MCcall(_mcm_pjxp_init_data(node));

  MCcall(_mcm_pjxp_init_ui(node));

  MCcall(mca_register_event_handler(MC_APP_EVENT_PROJECT_LOADED, (void *)_mcm_pjxp_project_loaded, (void *)node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));
  return 0;
}