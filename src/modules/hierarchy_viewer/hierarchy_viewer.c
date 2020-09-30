
#include "env/environment_definitions.h"
#include "modules/app_modules.h"
#include "render/render_common.h"
#include "render/render_thread.h"
#include "ui/ui_definitions.h"

void mc_hv_add_file_to_hierarchy_state(mc_hv_hierarchy_view_state *hv_state, mc_source_file_info *source_file)
{
  mc_hv_source_path_state *parent_section = NULL;
  int path_len = strlen(source_file->filepath);
  int i = 0;
  for (; i < path_len; ++i) {
    int s = i;
    bool end_of_path = true;
    for (; source_file->filepath[i] != '\0'; ++i) {
      if (source_file->filepath[i] == '\\' || source_file->filepath[i] == '/') {
        end_of_path = false;
        break;
      }
    }

    // printf("afths-0\n");
    char *path_item_name = (char *)malloc(sizeof(char) * (i - s + 1));
    strncpy(path_item_name, source_file->filepath + s, i - s);
    path_item_name[i - s] = '\0';

    // TODO -- check whether it exists?

    // printf("afths-1\n");
    mc_hv_source_path_state *sps = NULL;
    if (!parent_section) {
      // Search if it already exists in the root path states
      for (int i = 0; i < hv_state->path_states.count; ++i) {
        if (!strcmp(hv_state->path_states.items[i]->item_name, path_item_name)) {
          sps = hv_state->path_states.items[i];
          free(path_item_name);
          // printf("--%s\n", hv_state->path_states.items[i]->item_name);
          break;
        }
      }
      // printf("afths-2\n");
      if (!sps) {
        // printf("afths-3\n");
        sps = (mc_hv_source_path_state *)malloc(sizeof(mc_hv_source_path_state));
        sps->collapsed = false;
        if (end_of_path) {
          char last_character = path_item_name[strlen(path_item_name) - 1];
          switch (last_character) {
          case 'c':
            sps->item_type = MC_HV_HIERARCHICAL_C_SOURCE;
            break;
          case 'h':
            sps->item_type = MC_HV_HIERARCHICAL_C_HEADER;
            break;
          default:
            MCerror(8657, "TODO:'%c'", last_character);
          }
        }
        else {
          sps->item_type = MC_HV_HIERARCHICAL_FOLDER;
        }
        sps->item_name = path_item_name;
        sps->children.count = 0;
        sps->children.alloc = 0;

        // printf("++%s\n", sps->item_name);
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
        // printf("afths-5c %s \n", parent_section->children.items[i]->item_name);
        // printf("afths-5d %s \n", path_item_name);
        if (!strcmp(parent_section->children.items[i]->item_name, path_item_name)) {
          already_exists = true;
          free(path_item_name);

          // Set parent for next loop
          parent_section = parent_section->children.items[i];
          // printf(">>%s\n", parent_section->item_name);
          break;
        }
      }
      // printf("afths-5z\n");
      if (!already_exists) {
        // printf("afths-6\n");
        sps = (mc_hv_source_path_state *)malloc(sizeof(mc_hv_source_path_state));
        sps->collapsed = false;
        if (end_of_path) {
          char last_character = path_item_name[strlen(path_item_name) - 1];
          switch (last_character) {
          case 'c':
            sps->item_type = MC_HV_HIERARCHICAL_C_SOURCE;
            break;
          case 'h':
            sps->item_type = MC_HV_HIERARCHICAL_C_HEADER;
            break;
          default:
            MCerror(8987, "TODO:'%c'", last_character);
          }
        }
        else {
          sps->item_type = MC_HV_HIERARCHICAL_FOLDER;
        }
        sps->item_name = path_item_name;
        sps->children.count = 0;
        sps->children.alloc = 0;
        // printf("==%s\n", path_item_name);

        // Add to the parent
        append_to_collection((void ***)&parent_section->children.items, &parent_section->children.alloc,
                             &parent_section->children.count, sps);

        // Set parent for next loop
        parent_section = sps;
      }
      // printf("afths-7\n");
    }

    // printf("afths-8\n");
    // printf("path_item_name:'%s'\n", path_item_name);
  }

  // Attach the definitions for the source file also
  for (int i = 0; i < source_file->definitions.count; ++i) {
    mc_hv_source_path_state *sps = (mc_hv_source_path_state *)malloc(sizeof(mc_hv_source_path_state));
    sps->collapsed = true;
    source_definition *definition = source_file->definitions.items[i];
    sps->data = definition;
    switch (definition->type) {
    case SOURCE_DEFINITION_STRUCTURE: {
      sps->item_type = MC_HV_HIERARCHICAL_STRUCT_DEFINITION;
      sps->item_name = strdup(definition->data.structure_info->name);
    } break;
    case SOURCE_DEFINITION_FUNCTION: {
      sps->item_type = MC_HV_HIERARCHICAL_FUNCTION_DEFINITION;
      sps->item_name = strdup(definition->data.func_info->name);
    } break;
    case SOURCE_DEFINITION_ENUMERATION: {
      sps->item_type = MC_HV_HIERARCHICAL_ENUM_DEFINITION;
      sps->item_name = strdup(definition->data.enum_info->name);
    } break;
    default:
      MCerror(8109, "definition->type:%i", definition->type);
      break;
    }
    // sps->item_name = definition->;
    sps->children.count = 0;
    sps->children.alloc = 0;
    // printf("==%s\n", path_item_name);

    // Add to the parent
    append_to_collection((void ***)&parent_section->children.items, &parent_section->children.alloc,
                         &parent_section->children.count, sps);
  }
}

void __mc_hv_update_hierarchy_view_text_lines(mc_hv_hierarchy_view_state *hv_state, mc_hv_source_path_state *sp_state,
                                              int *text_line_index, int depth)
{
  mc_node *tl_node = hv_state->text_lines.items[*text_line_index];
  *text_line_index += 1;

  mui_button *tl_button = (mui_button *)tl_node->data;

  // Make visible and adjust horizontal indent
  tl_node->layout->padding.left = depth * 16;
  tl_node->layout->visible = true;

  // Set path section
  tl_button->tag = (void *)sp_state;
  if (sp_state->children.count) {
    if (sp_state->collapsed)
      set_c_str(tl_button->str, "+ ");
    else
      set_c_str(tl_button->str, "- ");
  }
  else {
    set_c_str(tl_button->str, "  ");
  }
  append_to_c_str(tl_button->str, sp_state->item_name);
  switch (sp_state->item_type) {
  case MC_HV_HIERARCHICAL_STRUCT_DEFINITION: {
    tl_button->font_color = COLOR_MACARONI_AND_CHEESE;
  } break;
  case MC_HV_HIERARCHICAL_FUNCTION_DEFINITION: {
    tl_button->font_color = COLOR_FUNCTION_GREEN;
  } break;
  case MC_HV_HIERARCHICAL_ENUM_DEFINITION: {
    tl_button->font_color = COLOR_TEAL;
  } break;
  case MC_HV_HIERARCHICAL_C_SOURCE: {
    tl_button->font_color = COLOR_LIGHT_SKY_BLUE;
  } break;
  case MC_HV_HIERARCHICAL_C_HEADER: {
    tl_button->font_color = COLOR_NODE_ORANGE;
  } break;
  case MC_HV_HIERARCHICAL_FOLDER: {
    tl_button->font_color = COLOR_LIGHT_YELLOW;
  } break;
  default:
    MCerror(173, "NotSupported:%i", sp_state->item_type);
  }

  mca_set_node_requires_layout_update(tl_node);

  if (!sp_state->collapsed) {
    for (int i = 0; i < sp_state->children.count && *text_line_index < hv_state->text_lines.size; ++i) {
      __mc_hv_update_hierarchy_view_text_lines(hv_state, sp_state->children.items[i], text_line_index, depth + 1);
    }
  }
}

void mc_hv_update_hierarchy_view_text_lines(mc_hv_hierarchy_view_state *hv_state)
{
  // printf("mc_hv_update_hierarchy_view_text_lines\n");
  int tl_index = 0;

  for (int i = 0; i < hv_state->path_states.count && tl_index < hv_state->text_lines.size; ++i) {
    __mc_hv_update_hierarchy_view_text_lines(hv_state, hv_state->path_states.items[i], &tl_index, 0);
  }

  hv_state->text_lines.visible_count = tl_index;
  for (int i = tl_index; i < hv_state->text_lines.size; ++i) {
    hv_state->text_lines.items[i]->layout->visible = false;
  }

  mca_set_node_requires_layout_update(hv_state->root_node);
}

void __mc_hv_text_line_left_click_handler(mui_button *button, mc_point click_location)
{
  // printf("__mc_hv_text_line_left_click_handler\n");
  mc_hv_source_path_state *sp_state = (mc_hv_source_path_state *)button->tag;

  if (sp_state->item_type == MC_HV_HIERARCHICAL_FUNCTION_DEFINITION) {
  }
  switch (sp_state->item_type) {
  case MC_HV_HIERARCHICAL_ENUM_DEFINITION:
  case MC_HV_HIERARCHICAL_STRUCT_DEFINITION:
    // Do nothing
    break;
  case MC_HV_HIERARCHICAL_FUNCTION_DEFINITION: {
    // Activate the code editor with the function
    source_definition *definition = (source_definition *)sp_state->data;

    mca_activate_source_editor_for_definition(definition);
  } break;
  case MC_HV_HIERARCHICAL_C_HEADER:
  case MC_HV_HIERARCHICAL_C_SOURCE:
  case MC_HV_HIERARCHICAL_FOLDER: {
    sp_state->collapsed = !sp_state->collapsed;
    printf("sp_state->collapsed:%i %s\n", sp_state->collapsed, sp_state->item_name);

    mca_set_node_requires_layout_update(button->node);
  } break;
  default:
    MCerror(257, "Unsupported: %i", sp_state->item_type);
  }
  // printf("sp_state->collapsed:%i\n", sp_state->collapsed);
}

void _mc_hv_update_hierarchy_viewer_layout(mc_node *node, mc_rectf *available_area)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Update the text lines (collapsed/text/etc)
  mc_hv_update_hierarchy_view_text_lines((mc_hv_hierarchy_view_state *)global_data->ui_state->hierarchy_viewer_state);

  // Continue with the underlying panel update
  __mui_update_panel_layout(node, available_area);
}

void init_hierarchy_viewer()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // Panel
  mui_panel *panel;
  mui_init_panel(global_data->global_node, &panel);

  panel->node->layout->visible = false;
  panel->node->layout->padding = {20, 20, 0, 0};
  panel->node->layout->preferred_width = 360;
  panel->node->layout->preferred_height = 720;
  panel->node->layout->update_layout = (void *)&_mc_hv_update_hierarchy_viewer_layout;
  panel->background_color = COLOR_DEEP_FIR;

  // Data
  mc_hv_hierarchy_view_state *hv_state = (mc_hv_hierarchy_view_state *)malloc(sizeof(mc_hv_hierarchy_view_state));
  global_data->ui_state->hierarchy_viewer_state = (void *)hv_state;

  hv_state->root_node = panel->node;
  hv_state->path_states.count = 0;
  hv_state->path_states.alloc = 0;
  hv_state->path_states.items = NULL;

  // Rest of the UI
  const int TEXT_LINE_COUNT = 26;
  hv_state->text_lines.size = 0;
  hv_state->text_lines.visible_count = TEXT_LINE_COUNT;
  reallocate_collection((void ***)&hv_state->text_lines.items, &hv_state->text_lines.size, TEXT_LINE_COUNT, 0);

  for (int i = 0; i < hv_state->text_lines.size; ++i) {
    mui_button *button;
    mui_init_button(hv_state->root_node, &button);
    hv_state->text_lines.items[i] = button->node;
    button->left_click = &__mc_hv_text_line_left_click_handler;

    mca_node_layout *layout = button->node->layout;
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