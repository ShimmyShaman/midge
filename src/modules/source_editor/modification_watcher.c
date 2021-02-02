#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/c_parser_lexer.h"
#include "core/midge_app.h"
#include "env/environment_definitions.h"

#include "modules/ui_elements/ui_elements.h"

#include "modules/source_editor/modification_watcher.h"

typedef enum _mc_smw_mdf_state {
  MC_SMW_MDF_STATE_NULL = 0,
  MC_SMW_MDF_STATE_DISCOVERED,
} _mc_smw_mdf_state;

typedef struct _mc_smw_mdf_func_info {
  mc_source_file_info *source_file;
  _mc_smw_mdf_state state;
  function_info *function_info;
  char *disk_code;
} _mc_smw_mdf_func_info;

typedef struct mc_source_modification_data {
  mc_node *node;

  render_color shade_color;

  mcu_panel *panel;

  struct {
    int count, size;
    _mc_smw_mdf_func_info *items;
  } modified_functions;
  //   mcu_textblock *message_textblock;
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

int _mc_smw_analyze_function_differences(mc_source_modification_data *md, mc_source_file_info *sf, mc_syntax_node *fast)
{
  int a, b, n;
  char *exc;
  mc_source_file_code_segment *seg;
  _mc_smw_mdf_func_info *mfn;

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
    n = strlen(exc);
    if (n == strlen(seg->function->code)) {
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
    if (md->modified_functions.count >= md->modified_functions.size) {
      // Capacity Check
      md->modified_functions.size *= 2;
      md->modified_functions.items =
          realloc(md->modified_functions.items, sizeof(_mc_smw_mdf_func_info) * md->modified_functions.size);
      if (!md->modified_functions.items) {
        MCerror(5981, "realloc error TODO");
      }
    }

    mfn = &md->modified_functions.items[md->modified_functions.count++];
    mfn->source_file = sf;
    mfn->state = MC_SMW_MDF_STATE_DISCOVERED;
    mfn->function_info = seg->function;
    mfn->disk_code = exc;
  }

  return 0;
}

int _mc_smw_analyze_children_differences(mc_source_modification_data *md, mc_source_file_info *sf,
                                         mc_syntax_node_list *children)
{
  mc_syntax_node *child;
  for (int a = 0; a < children->count; ++a) {
    child = children->items[a];
    switch (child->type) {
    case MC_SYNTAX_FUNCTION: {
      printf("childfunc:'%s'\n", child->function.name->text);
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
        print_syntax_node(child->type_alias.type_descriptor, 0);
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
      // TODO...??
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
  char *file_text;
  MCcall(read_file_text(sf->filepath, &file_text));

  mc_syntax_node *ast;
  MCcall(mcs_parse_file_to_syntax_tree(file_text, &ast));
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

  // printf("sf-modified:%s\n", sf->filepath);
  // MCcall(_mc_smw_analyze_modified_source_file(md, sf));

  // md->node->layout->visible = true;
  // mca_set_node_requires_layout_update(md->node);

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
  md->modified_functions.items = malloc(sizeof(_mc_smw_mdf_func_info) * md->modified_functions.size);
  // TODO--malloc check just like everywhere

  //   MCcall(init_mc_str(&md->current_directory));
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
  mca_node_layout *layout;
  //   mcu_button *button;

  // Panel
  MCcall(mcu_init_panel(module_node, &md->panel));

  layout = md->panel->node->layout;
  layout->max_width = 640;
  layout->padding = (mc_paddingf){40, 40, 40, 40};
  layout->max_height = 480;

  md->panel->background_color = (render_color){0.35f, 0.35f, 0.35f, 1.f};

  //   // Message Block
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
  //   MCcall(set_mc_str(button->str, "Select Current Folder"));
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

  //     MCcall(set_mc_str(button->str, "button"));

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
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
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