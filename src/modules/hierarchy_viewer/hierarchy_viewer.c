
#include "env/environment_definitions.h"
#include "modules/app_modules.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void mc_hv_add_file_to_hierarchy_state(mc_hv_hierarchy_view_state *hv_state, source_file_info *source_file)
{
  mc_hv_source_path_state *parent_section = NULL;
  int path_len = strlen(source_file->filepath);
  int i = 0;
  for (; i < path_len; ++i) {
    int s = i;
    for (; source_file->filepath[i] != '\0'; ++i) {
      if (source_file->filepath[i] == '\\' || source_file->filepath[i] == '/') {
        break;
      }
    }

    // printf("afths-0\n");
    char *path_section_name = (char *)malloc(sizeof(char) * (i - s + 1));
    strncpy(path_section_name, source_file->filepath + s, i - s);
    path_section_name[i - s] = '\0';

    // TODO -- check whether it exists?

    // printf("afths-1\n");
    mc_hv_source_path_state *sps = NULL;
    if (!parent_section) {
      // Search if it already exists in the root path states
      for (int i = 0; i < hv_state->path_states.count; ++i) {
        if (!strcmp(hv_state->path_states.items[i]->section_name, path_section_name)) {
          sps = hv_state->path_states.items[i];
          free(path_section_name);
          // printf("--%s\n", hv_state->path_states.items[i]->section_name);
          break;
        }
      }
      // printf("afths-2\n");
      if (!sps) {
        // printf("afths-3\n");
        sps = (mc_hv_source_path_state *)malloc(sizeof(mc_hv_source_path_state));
        sps->collapsed = false;
        sps->section_name = path_section_name;
        sps->children.count = 0;
        sps->children.alloc = 0;

        // printf("++%s\n", sps->section_name);
        append_to_collection((void ***)&hv_state->path_states.items, &hv_state->path_states.alloc,
                             &hv_state->path_states.count, sps);
      }

      // Set parent for next loop
      parent_section = sps;
      // printf("afths-4\n");
    }
    else {
      // printf("afths-5\n");
      // Search that it doesn't already exist in the parent
      bool already_exists = false;
      // printf("afths-5a  %i \n", parent_section->children.count);
      for (int i = 0; i < parent_section->children.count; ++i) {
        // printf("afths-5b %i %p \n", i, parent_section->children.items[i]);
        // printf("afths-5c %s \n", parent_section->children.items[i]->section_name);
        // printf("afths-5d %s \n", path_section_name);
        if (!strcmp(parent_section->children.items[i]->section_name, path_section_name)) {
          already_exists = true;
          free(path_section_name);

          // Set parent for next loop
          parent_section = parent_section->children.items[i];
          // printf(">>%s\n", parent_section->section_name);
          break;
        }
      }
      // printf("afths-5z\n");
      if (!already_exists) {
        // printf("afths-6\n");
        sps = (mc_hv_source_path_state *)malloc(sizeof(mc_hv_source_path_state));
        sps->collapsed = false;
        sps->section_name = path_section_name;
        sps->children.count = 0;
        sps->children.alloc = 0;
        // printf("==%s\n", path_section_name);

        // Add to the parent
        append_to_collection((void ***)&parent_section->children.items, &parent_section->children.alloc,
                             &parent_section->children.count, sps);

        // Set parent for next loop
        parent_section = sps;
      }
      // printf("afths-7\n");
    }

    // printf("afths-8\n");
    // printf("path_section_name:'%s'\n", path_section_name);
  }
}

void __mc_hv_update_hierarchy_view_text_lines(mc_hv_hierarchy_view_state *hv_state, mc_hv_source_path_state *sp_state,
                                              int *text_line_index, int depth)
{
  mc_node *tl_node = hv_state->text_lines.items[*text_line_index];
  *text_line_index += 1;

  mui_ui_element *tl_element = (mui_ui_element *)tl_node->data;
  mui_button *tl_button = (mui_button *)tl_element->data;

  tl_node->visible = true;
  mca_set_node_requires_layout_update(tl_node);

  tl_element->layout->padding.left = depth * 16;

  if (sp_state->collapsed)
    set_c_str(tl_button->str, "+ ");
  else
    set_c_str(tl_button->str, "- ");
  append_to_c_str(tl_button->str, sp_state->section_name);

  if (!sp_state->collapsed) {
    for (int i = 0; i < sp_state->children.count && *text_line_index < hv_state->text_lines.size; ++i) {
      __mc_hv_update_hierarchy_view_text_lines(hv_state, sp_state->children.items[i], text_line_index, depth + 1);
    }
  }
}

void mc_hv_update_hierarchy_view_text_lines(mc_hv_hierarchy_view_state *hv_state)
{
  int tl_index = 0;

  for (int i = 0; i < hv_state->path_states.count && tl_index < hv_state->text_lines.size; ++i) {
    __mc_hv_update_hierarchy_view_text_lines(hv_state, hv_state->path_states.items[i], &tl_index, 0);
  }

  hv_state->text_lines.visible_count = tl_index;
  for (int i = tl_index; i < hv_state->text_lines.size; ++i) {
    hv_state->text_lines.items[i]->visible = false;
  }

  mca_set_node_requires_layout_update(hv_state->root_node);
}

void init_hierarchy_viewer()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Panel
  mui_panel *panel;
  mui_init_panel(global_data->global_node, &panel);

  panel->element->layout->padding = {100, 100, 0, 0};
  panel->element->layout->preferred_width = 400;
  panel->element->layout->preferred_height = 720;
  panel->background_color = COLOR_DEEP_FIR;

  // Data
  mc_hv_hierarchy_view_state *hv_state = (mc_hv_hierarchy_view_state *)malloc(sizeof(mc_hv_hierarchy_view_state));
  global_data->ui_state->hierarchy_viewer_state = (void *)hv_state;

  hv_state->root_node = panel->element->visual_node;
  hv_state->path_states.count = 0;
  hv_state->path_states.alloc = 0;
  hv_state->path_states.items = NULL;

  // Rest of the UI
  const int TEXT_LINE_COUNT = 16;
  hv_state->text_lines.size = 0;
  hv_state->text_lines.visible_count = TEXT_LINE_COUNT;
  reallocate_collection((void ***)&hv_state->text_lines.items, &hv_state->text_lines.size, TEXT_LINE_COUNT, 0);

  for (int i = 0; i < hv_state->text_lines.size; ++i) {
    mui_button *button;
    mui_init_button(hv_state->root_node, &button);
    hv_state->text_lines.items[i] = button->element->visual_node;

    mca_node_layout *layout = button->element->layout;
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
    layout->padding = {4, (float)(i * 24), 0, 0};

    set_c_str(button->str, "text");
    append_to_c_strf(button->str, "%i", i);
    button->background_color = COLOR_TRANSPARENT;
    button->font_color = COLOR_GHOST_WHITE;
  }

  // Display all source files
  // printf("source file count:%i\n", global_data->source_files.count);
  for (int i = 0; i < global_data->source_files.count; ++i) {
    mc_hv_add_file_to_hierarchy_state(hv_state, global_data->source_files.items[i]);
  }

  mc_hv_update_hierarchy_view_text_lines(hv_state);
}