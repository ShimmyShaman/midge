#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/c_parser_lexer.h"
#include "core/midge_app.h"
#include "env/environment_definitions.h"
#include "mc_error_handling.h"

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_source_extensions.h"
#include "modules/ui_elements/ui_elements.h"

#include "modules/source_editor/modification_watcher.h"

typedef enum _mc_smw_mdf_state {
  MC_SMW_MDF_STATE_NULL = 0,
  MC_SMW_MDF_STATE_DISCOVERED,
  MC_SMW_MDF_STATE_PARSE_ERROR,
  MC_SMW_MDF_STATE_COMPILATION_ERROR,
} _mc_smw_mdf_state;

typedef struct _mc_smw_tracked_modification {
  mc_source_file_info *source_file;
  _mc_smw_mdf_state state;
  function_info *function_info;
  char *disk_code;
} _mc_smw_tracked_modification;

typedef struct _mc_smw_ui_entry {
  mcu_panel *panel;
  mcu_textblock *label_textblock;
  mcu_button *button;
} _mc_smw_ui_entry;

typedef struct mc_source_modification_data {
  mc_node *node;

  render_color shade_color;

  mcu_panel *panel;

  struct {
    int count, size;
    _mc_smw_tracked_modification *items;
  } modified_functions;

  int ui_entries_size;
  _mc_smw_ui_entry *ui_entries;
  //   struct {
  //     unsigned int capacity, count, utilized;
  //     mcu_button **items;
  //   } displayed_items;

  //   // struct {
  //   //   unsigned int width, height;
  //   //   mcr_texture_image *image;
  //   // } render_target;

  //   mc_str *current_directory;
  //   mc_source_modification_display_mode mode;
  //   struct {
  //     void *state;
  //     void *result_delegate;
  //   } callback;

} mc_source_modification_data;

typedef struct _mc_smw_button_tag {
  mc_source_modification_data *md;
  _mc_smw_tracked_modification *mfi;
  mcu_textblock *label_textblock;
} _mc_smw_button_tag;

int _mc_smw_update_node_layout(mc_node *node, const mc_rectf *available_area)
{
  mc_source_modification_data *md = (mc_source_modification_data *)node->data;
  node->layout->__requires_layout_update = false;

  // Useless Check
  if (!md->modified_functions.count) {
    node->layout->visible = false;
    return 0;
  }

  // Set the node data appropriately
  int a;
  char rel[256];
  _mc_smw_tracked_modification *mfi;
  _mc_smw_ui_entry *entry;
  mcu_button *button;
  mcu_textblock *textblock;
  for (a = 0; a < md->ui_entries_size && a < md->modified_functions.count; ++a) {
    // printf("a=%i\n", a);
    mfi = &md->modified_functions.items[a];
    MCcall(mcf_obtain_path_relative_to_cwd(mfi->source_file->filepath, rel, 256));

    entry = &md->ui_entries[a];
    textblock = entry->label_textblock;
    button = entry->button;

    // Set Visible
    entry->panel->node->layout->visible = true;

    // Update textblock
    MCcall(mc_set_str(textblock->str, ""));
    textblock->node->layout->__requires_layout_update = true;

    ((_mc_smw_button_tag *)button->tag)->mfi = mfi;
    switch (mfi->state) {
    case MC_SMW_MDF_STATE_DISCOVERED: {
      entry->panel->background_color = COLOR_MIDNIGHT_EXPRESS;
      MCcall(mc_append_to_strf(textblock->str, "%s :: %s", rel, mfi->function_info->name));
      button->enabled = true;
    } break;
    case MC_SMW_MDF_STATE_PARSE_ERROR: {
      entry->panel->background_color = COLOR_TYRIAN_PURPLE;
      MCcall(mc_append_to_strf(textblock->str, "%s", rel));
      button->enabled = false;
    } break;
    case MC_SMW_MDF_STATE_COMPILATION_ERROR: {
      entry->panel->background_color = COLOR_MAROON;
      MCcall(mc_append_to_strf(textblock->str, "%s :: %s", rel, mfi->function_info->name));
      button->enabled = false;
    } break;
    default:
      MCerror(9952, "TODO : %i", mfi->state);
    }
    MCcall(mca_set_node_requires_rerender(button->node));
    MCcall(mca_set_node_requires_rerender(textblock->node));
  }

  // Hide the rest
  for (; a < md->ui_entries_size; ++a) {
    entry = &md->ui_entries[a];

    entry->panel->node->layout->visible = false;
  }

  // Update
  MCcall(mca_update_typical_node_layout(node, available_area));

  return 0;
}

void _mc_render_source_modification_headless(render_thread_info *render_thread, mc_node *node)
{
  mc_source_modification_data *md = (mc_source_modification_data *)node->data;

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

  //   // // Render the render target
  //   // midge_app_info *app_info;
  //   // mc_obtain_midge_app_info(&app_info);

  //   // // Children
  //   // for (int a = 0; a < node->children->count; ++a) {
  //   //   mc_node *child = node->children->items[a];
  //   //   if (child->layout && child->layout->visible && child->layout->render_present) {
  //   //     // TODO fptr casting
  //   //     void (*render_node_present)(image_render_details *, mc_node *) =
  //   //         (void (*)(image_render_details *, mc_node *))child->layout->render_present;
  //   //     render_node_present(irq, child);
  //   //   }
  //   // }

  //   // mcr_submit_image_render_request(app_info->render_thread, irq);
}

void _mc_render_source_modification_present(image_render_details *image_render_queue, mc_node *node)
{
  mc_source_modification_data *md = (mc_source_modification_data *)node->data;

  // mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  // midge_app_info *app_info;
  // mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, md->shade_color);

  //   mcr_issue_render_command_colored_quad(
  //       image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
  //       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
  //       md->background_color);

  // Children
  mca_render_node_list_present(image_render_queue, node->children);
}

void _mc_handle_source_modification_input(mc_node *node, mci_input_event *input_event)
{
  // printf("_mco_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

// void _mc_md_open_current_clicked(mci_input_event *input_event, mcu_button *button)
// {
//   mc_source_modification_data *md = (mc_source_modification_data *)button->tag;

//   // Wrap Up
//   md->node->layout->visible = false;

//   mca_set_node_requires_rerender(md->node);
// }

int _mc_smw_exit_dialog_clicked(mci_input_event *input_event, mcu_button *button)
{
  mc_source_modification_data *md = (mc_source_modification_data *)button->tag;

  // Wrap Up
  md->node->layout->visible = false;

  mca_set_node_requires_rerender(md->node);

  return 0;
}

int _mc_smw_info_button_clicked(mci_input_event *input_event, mcu_button *button)
{
  _mc_smw_button_tag *tag = (_mc_smw_button_tag *)button->tag;

  mc_source_modification_data *md = tag->md;
  _mc_smw_tracked_modification *mfi = tag->mfi;

  int res = mc_redefine_function_provisionally(mfi->function_info, mfi->disk_code);
  if (res) {
    printf("Code for function '%s' was not successfully interpreted.\n", mfi->function_info->name);

    // tag->label_textblock->background_color = COLOR_MAROON;
    mfi->state = MC_SMW_MDF_STATE_COMPILATION_ERROR;

    // button->background_color = COLOR_RED;
    // MCcall(mc_set_str(&button->str, "Reinterpret"));
  }
  else {
    // Remove the mfi
    int a = md->modified_functions.count - (tag->mfi - md->modified_functions.items) - 1;
    for (; a > 0; ++a) {
      *mfi = *(mfi + 1);
      ++mfi;
    }
    --md->modified_functions.count;
  }

  MCcall(mca_set_node_requires_layout_update(md->node));

  return 0;
}

int _mc_smw_check_modfuncary_capacity(mc_source_modification_data *md)
{
  if (md->modified_functions.count + 1 > md->modified_functions.size) {
    // Capacity Check
    md->modified_functions.size *= 2;
    md->modified_functions.items =
        realloc(md->modified_functions.items, sizeof(_mc_smw_tracked_modification) * md->modified_functions.size);
    if (!md->modified_functions.items) {
      MCerror(5981, "realloc error TODO");
    }
  }
  return 0;
}

int _mc_smw_analyze_function_differences(mc_source_modification_data *md, mc_source_file_info *sf, mc_syntax_node *fast)
{
  int a, b;
  char *exc;
  mc_source_file_code_segment *seg;
  _mc_smw_tracked_modification *mfn;

  // Find the function info in the source file
  for (a = 0; a < sf->segments.count; ++a) {
    seg = sf->segments.items[a];

    if (seg->type != MC_SOURCE_SEGMENT_FUNCTION_DEFINITION) {
      continue;
    }
    if (strcmp(seg->function->name, fast->function.name->text))
      continue;

    // TODO -- parameters

    MCcall(mcs_copy_syntax_node_to_text(fast->function.code_block, &exc));
    if (!strcmp(seg->function->code, exc)) {
      free(exc);
    mc_smw_loop_continue:
      continue;
    }

    printf("SMW Difference found in function '%s' in file '%s' %i<>%i\n", seg->function->name, sf->filepath,
           strlen(seg->function->code), strlen(exc));

    // Check modification isn't already tracked
    for (b = 0; b < md->modified_functions.count; ++b) {
      mfn = &md->modified_functions.items[b];
      if (!strcmp(mfn->function_info->name, seg->function->name)) {
        // Already exists in list
        // Update the code and state
        if (mfn->disk_code)
          free(mfn->disk_code);
        mfn->state = MC_SMW_MDF_STATE_DISCOVERED;
        mfn->disk_code = exc;
        goto mc_smw_loop_continue;
      }
    }

    // Set Anew
    MCcall(_mc_smw_check_modfuncary_capacity(md));

    mfn = &md->modified_functions.items[md->modified_functions.count++];
    mfn->source_file = sf;
    mfn->state = MC_SMW_MDF_STATE_DISCOVERED;
    mfn->function_info = seg->function;
    mfn->disk_code = exc;
  }

  return 0;
}

int _mc_smw_find_equivalent_child_in_info(mc_source_file_info *source_file, mc_source_file_code_segment_type type,
                                          int int1, void *data1, void **info)
{
  int a;
  mc_source_file_code_segment *seg;

  for (a = 0; a < source_file->segments.count; ++a) {
    seg = source_file->segments.items[a];

    if (seg->type != type)
      continue;

    switch (seg->type) {
    case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE: {
      if (seg->include->is_system_search != (bool)int1)
        continue;
      if (strcmp(seg->include->filepath, (const char *)data1))
        continue;

      *info = seg;
      return 0;
    }
    default:
      break;
    }
  }

  *info = NULL;

  return 0;
}

int _mc_smw_find_equivalent_child_in_ast(mc_syntax_node_list *children, mc_syntax_node_type type, int int1, void *data1,
                                         mc_syntax_node **node)
{
  int a;
  mc_syntax_node *child;

  for (a = 0; a < children->count; ++a) {
    child = children->items[a];

    if (child->type != type)
      continue;

    switch (child->type) {
    case MC_SYNTAX_PP_DIRECTIVE_INCLUDE: {
      if (child->include_directive.is_system_header_search != (bool)int1)
        continue;
      if (strcmp(child->include_directive.filepath->text, (const char *)data1))
        continue;

      *node = child;
      return 0;
    }
    default:
      break;
    }
  }

  *node = NULL;

  return 0;
}

int _mc_smw_analyze_children_differences(mc_source_modification_data *md, mc_source_file_info *sf,
                                         mc_syntax_node_list *children)
{
  int a;
  mc_syntax_node *child;
  mc_source_file_code_segment *seg;

  // Search for removals
  for (a = 0; a < sf->segments.count; ++a) {
    seg = sf->segments.items[a];

    switch (seg->type) {
    case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE: {
      MCcall(_mc_smw_find_equivalent_child_in_ast(children, MC_SYNTAX_PP_DIRECTIVE_INCLUDE,
                                                  (int)seg->include->is_system_search, (void *)seg->include->filepath,
                                                  &child));
      if (child) {
        // printf("found include:'%s'\n", seg->include->filepath);
        continue;
      }

      printf("TODO -- removal of include from file. How to handle this? :'%s'\n", seg->include->filepath);
    } break;
    default:
      break;
    }
  }

  // Search for modifications and additions
  for (a = 0; a < children->count; ++a) {
    child = children->items[a];
    switch (child->type) {
    case MC_SYNTAX_FUNCTION: {
      // printf("childfunc:'%s'\n", child->function.name->text);
      if (child->function.code_block) {
        // More than just a declaration
        MCcall(_mc_smw_analyze_function_differences(md, sf, child));
      }
    } break;
    case MC_SYNTAX_TYPE_ALIAS: {
      switch (child->type_alias.type_descriptor->type) {
      case MC_SYNTAX_UNION_DECL:
      case MC_SYNTAX_STRUCT_DECL:
      case MC_SYNTAX_ENUM_DECL: {
        // TODO...?
      } break;
      default:
        MCcall(print_syntax_node(child->type_alias.type_descriptor, 0));
        MCerror(3547, "Unhandled type_alias-descriptor-syntax-type:%s",
                get_mc_syntax_token_type_name(child->type_alias.type_descriptor->type));
        break;
      }
    } break;
    case MC_SYNTAX_UNION_DECL:
    case MC_SYNTAX_STRUCT_DECL:
    case MC_SYNTAX_ENUM_DECL: {
      // TODO...?
    } break;
    case MC_SYNTAX_GLOBAL_VARIABLE_DECLARATION:
    case MC_SYNTAX_PP_DIRECTIVE_DEFINE:
    case MC_SYNTAX_PP_DIRECTIVE_UNDEFINE: {
      // TODO...?
    } break;
    // TODO
    case MC_SYNTAX_PP_DIRECTIVE_IFDEF: {
      // Assume all ifndefs (for the moment TODO ) to be true
      MCcall(_mc_smw_analyze_children_differences(md, sf, child->preprocess_ifdef.groupopt));
    } break;
    case MC_SYNTAX_PP_DIRECTIVE_IFNDEF: {
      // Assume all ifndefs (for the moment TODO ) to be true
      MCcall(_mc_smw_analyze_children_differences(md, sf, child->preprocess_ifndef.groupopt));
    } break;
    case MC_SYNTAX_PP_DIRECTIVE_INCLUDE: {
      // Determine if this is an addition
      void *info;
      MCcall(_mc_smw_find_equivalent_child_in_info(sf, MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE,
                                                   child->include_directive.is_system_header_search,
                                                   (void *)child->include_directive.filepath->text, &info));
      if (info)
        continue;

      printf("TODO : include '%s' is new to file -- recompile whole thing\n", child->include_directive.filepath->text);

    } break;
    default: {
      switch ((mc_token_type)child->type) {
      case MC_TOKEN_PP_KEYWORD_IFDEF:
      case MC_TOKEN_PP_KEYWORD_IFNDEF:
      case MC_TOKEN_PP_KEYWORD_ENDIF:
        MCerror(5313, "TODO");
      case MC_TOKEN_NEW_LINE:
      case MC_TOKEN_SPACE_SEQUENCE:
      case MC_TOKEN_LINE_COMMENT:
      case MC_TOKEN_MULTI_LINE_COMMENT: {
        break;
      }
      default: {
        print_syntax_node(child, 0);
        MCerror(576, "Unhandled root-syntax-type:%s", get_mc_syntax_token_type_name(child->type));
      }
      }
    } break;
    }
  }

  return 0;
}

int _mc_smw_analyze_modified_source_file(mc_source_modification_data *md, mc_source_file_info *sf)
{
  int a, b;
  char *file_text;
  mc_syntax_node *ast;

  // Remove all entries related to this source file
  _mc_smw_tracked_modification *mfi;
  for (a = md->modified_functions.count - 1; a >= 0; --a) {
    mfi = &md->modified_functions.items[a];

    if (mfi->source_file != sf)
      continue;

    // Move down one
    for (b = a + 1; b < md->modified_functions.count; ++b) {
      md->modified_functions.items[b - 1] = md->modified_functions.items[b];
    }
    --md->modified_functions.count;
  }

  // Read & Parse the source file
  MCcall(read_file_text(sf->filepath, &file_text));
  int res = mcs_parse_file_to_syntax_tree(file_text, &ast);
  if (res) {
    // Track the WHOLE source File (not just a function)
    MCcall(_mc_smw_check_modfuncary_capacity(md));
    mfi = &md->modified_functions.items[md->modified_functions.count];
    ++md->modified_functions.count;

    mfi->source_file = sf;
    mfi->state = MC_SMW_MDF_STATE_PARSE_ERROR;
    mfi->disk_code = NULL;
    mfi->function_info = NULL;

    return 0;
  }

  free(file_text);
  // printf("ast:'%s'\n", child->function.name->text);
  // print_syntax_node(ast, 1);
  MCcall(_mc_smw_analyze_children_differences(md, sf, ast->children));

  return 0;
}

int _mc_smw_source_file_modified(void *handler_state, void *event_args)
{
  mc_source_file_info *sf = (mc_source_file_info *)event_args;
  mc_source_modification_data *md = (mc_source_modification_data *)handler_state;

  printf("sf-modified:%s\n", sf->filepath);
  MCcall(_mc_smw_analyze_modified_source_file(md, sf));

  md->node->layout->visible = true;
  mca_set_node_requires_layout_update(md->node);

  return 0;
}

int mc_smw_init_data(mc_node *module_node)
{
  mc_source_modification_data *md = (mc_source_modification_data *)malloc(sizeof(mc_source_modification_data));
  module_node->data = md;
  md->node = module_node;

  md->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  md->modified_functions.count = 0;
  md->modified_functions.size = 4;
  md->modified_functions.items =
      (_mc_smw_tracked_modification *)malloc(sizeof(_mc_smw_tracked_modification) * md->modified_functions.size);
  // TODO--malloc check just like everywhere

  //   MCcall(mc_alloc_str(&md->current_directory));
  //   md->callback.state = NULL;
  //   md->callback.result_delegate = NULL;

  //   md->displayed_items.capacity = md->displayed_items.count = 0U;

  //   // mo_data->render_target.image = NULL;
  //   // mo_data->render_target.width = module_node->layout->preferred_width;
  //   // mo_data->render_target.height = module_node->layout->preferred_height;
  //   // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  //   //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  //   //   TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  //   //   while (!md->render_target.image) {
  //   //     // puts("wait");
  //   //     usleep(100);
  //   //   }

  return 0;
}

int mc_smw_init_ui(mc_node *module_node)
{
  mc_source_modification_data *md = (mc_source_modification_data *)module_node->data;

  //   // Locals
  //   char buf[64];
  int a;
  mca_node_layout *layout;
  mcu_panel *panel;
  mcu_button *button;
  mcu_textblock *textblock;

  // Panel
  MCcall(mcu_init_panel(module_node, &md->panel));

  layout = md->panel->node->layout;
  layout->max_width = 840;
  layout->padding = (mc_paddingf){40, 40, 40, 40};
  layout->max_height = 420;

  md->panel->background_color = (render_color){0.35f, 0.35f, 0.35f, 1.f};

  // Info Text-Blocks
  md->ui_entries_size = 11;
  md->ui_entries = (_mc_smw_ui_entry *)malloc(sizeof(_mc_smw_ui_entry) * md->ui_entries_size);
  _mc_smw_button_tag *tags = (_mc_smw_button_tag *)malloc(sizeof(_mc_smw_button_tag) * md->ui_entries_size);
  for (a = 0; a < md->ui_entries_size; ++a) {
    MCcall(mcu_init_panel(md->panel->node, &panel));
    md->ui_entries[a].panel = panel;

    panel->background_color = COLOR_DARK_SLATE_GRAY;

    layout = panel->node->layout;
    layout->visible = false;
    layout->padding = (mc_paddingf){2, 32 + 31 * a, 2, 2};
    layout->preferred_height = 30;
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

    MCcall(mcu_init_textblock(panel->node, &textblock));
    md->ui_entries[a].label_textblock = textblock;

    layout = textblock->node->layout;
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
    layout->padding = (mc_paddingf){2, 2, 148, 2};

    MCcall(mcu_init_button(panel->node, &button));
    md->ui_entries[a].button = button;
    MCcall(mc_set_str(&button->str, "Interpret"));
    button->left_click = &_mc_smw_info_button_clicked;

    _mc_smw_button_tag *tag = tags + a;
    button->tag = tag;
    tag->md = md;
    tag->mfi = NULL;
    tag->label_textblock = textblock;

    layout = button->node->layout;
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
    layout->padding = (mc_paddingf){2, 2, 2, 2};
    layout->preferred_width = 120;
  }

  // Exit Button
  MCcall(mcu_init_button(md->panel->node, &button));

  layout = button->node->layout;
  layout->preferred_width = 16;
  layout->preferred_height = 16;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(mc_set_str(&button->str, "X"));
  button->tag = md;
  button->left_click = (void *)&_mc_smw_exit_dialog_clicked;

  //   MCcall(mcu_init_textblock(md->panel->node, &md->message_textblock));

  //   layout = md->message_textblock->node->layout;
  //   layout->padding = (mc_paddingf){4, 4, 4, 4};
  //   layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  //   layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  //   layout->preferred_width = 0.f;

  //   md->message_textblock->background_color = COLOR_GRAPE;

  //   // Open Button
  //   MCcall(mcu_init_button(md->panel->node, &button));

  //   layout = button->node->layout;
  //   layout->preferred_width = 200;
  //   layout->preferred_height = 26;
  //   layout->padding = (mc_paddingf){4, 4, 4, 4};
  //   layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  //   layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  //   button->background_color = COLOR_MIDNIGHT_EXPRESS;
  //   MCcall(mc_set_str(&button->str, "Select Current Folder"));
  //   button->tag = md;
  //   button->left_click = (void *)&_mc_md_open_current_clicked;

  //   // Textblocks to display items
  //   for (int a = 0; a < 12; ++a) {
  //     MCcall(mcu_init_button(md->panel->node, &button));

  //     if (button->node->name) {
  //       free(button->node->name);
  //       button->node->name = NULL;
  //     }
  //     sprintf(buf, "folder-dialog-item-button-%i", a);
  //     button->node->name = strdup(buf);

  //     button->node->layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;
  //     button->node->layout->padding = (mc_paddingf){6, 24 + 8 + a * 27, 6, 0};
  //     button->node->layout->max_width = 0U;
  //     button->node->layout->visible = false;

  //     button->tag = md;
  //     button->left_click = (void *)&_mc_md_item_selected;

  //     MCcall(mc_set_str(&button->str, "button"));

  //     MCcall(append_to_collection((void ***)&md->displayed_items.items, &md->displayed_items.capacity,
  //                                 &md->displayed_items.count, button));
  //   }

  return 0;
}

int mc_se_init_modification_watcher(mc_node *app_root)
{
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  // TODO -- get rid of node type

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "external-modification-dialog", &node));
  MCcall(mca_init_node_layout(&node->layout));
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  node->layout->visible = false;
  node->layout->z_layer_index = 9;
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&_mc_smw_update_node_layout;
  node->layout->render_headless = (void *)&_mc_render_source_modification_headless;
  node->layout->render_present = (void *)&_mc_render_source_modification_present;
  node->layout->handle_input_event = (void *)&_mc_handle_source_modification_input;

  MCcall(mc_smw_init_data(node));
  MCcall(mc_smw_init_ui(node));

  MCcall(mca_register_event_handler(MC_APP_EVENT_SOURCE_FILE_MODIFIED_EXTERNALLY, &_mc_smw_source_file_modified,
                                    node->data));

  MCcall(mca_attach_node_to_hierarchy(app_root, node));

  return 0;
}