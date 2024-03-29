/* mo_context_viewer.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/midge_app.h"
#include "env/environment_definitions.h"

#include "modules/modus_operandi/mo_context_viewer.h"
#include "modules/render_utilities/render_util.h"
#include "modules/ui_elements/ui_elements.h"

typedef struct _mc_mo_cv_context_row {
  mcu_panel *panel;
  mcu_textblock *key_textblock, *value_textblock;
} _mc_mo_cv_context_row;

typedef struct mc_mo_context_viewer_data {
  mc_node *node;

  mc_mo_process_stack *process_stack;

  struct {
    int size;
    _mc_mo_cv_context_row *items;
  } rows;

  render_color background_color;
} mc_mo_context_viewer_data;

static void _mc_mo_cv_render_headless(render_thread_info *render_thread, mc_node *node)
{
  // mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)node->data;

  // Children
  mca_render_node_list_headless(render_thread, node->children);

  // mcr_submit_image_render_request(app_info->render_thread, irq);
}

void _mc_mo_cv_render_present(image_render_details *irq, mc_node *node)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)node->data;

  // printf("_mc_mo_cv_render_present : %s\n", node->layout->visible ? "visible" : "not-visible");

  // mcr_issue_render_command_textured_quad(irq, (unsigned int)node->layout->__bounds.x,
  //                                        (unsigned int)node->layout->__bounds.y, modata->render_target.width,
  //                                        modata->render_target.height, modata->render_target.image);

  // Render the render target
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  mcr_issue_render_command_colored_quad(
      irq, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, cv->background_color);

  mcr_render_border(irq, node, 1, COLOR_GHOST_WHITE);

  //   mcr_issue_render_command_colored_quad(
  //       irq, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
  //       (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height,
  //       cv->background_color);

  // Children
  mca_render_node_list_present(irq, node->children);

  // _mc_mo_cv_context_row *row = cv->rows.items;
  // mc_rectf *ba, *bb;
  // while (row->panel->node->layout->visible) {
  //   ba = &row->key_textblock->node->layout->__bounds;
  //   bb = &row->value_textblock->node->layout->__bounds;
  //   mcr_issue_render_command_colored_quad(irq, (unsigned int)(ba->x + ba->width), (unsigned int)ba->y,
  //                                         (unsigned int)(bb->y - (ba->x + ba->width)), (unsigned int)ba->height,
  //                                         cv->background_color);
  // }

  // mc_mo_cv_step_data *cell;
  // for (cell = cv->cells.ary; cell < &cv->cells.ary[cv->cells.size]; ++cell) {

  //   if (!cell->panel->node->layout->visible)
  //     break;

  //   printf("cell %i %i: %.2f %.2f %.2f %.2f\n", cell - cv->cells.ary, cell->type,
  //          cell->panel->node->layout->__bounds.x, cell->panel->node->layout->__bounds.y,
  //          cell->panel->node->layout->__bounds.width, cell->panel->node->layout->__bounds.height);
  // }
}

int _mc_mo_cv_set_context_row(mc_mo_context_viewer_data *cv, _mc_mo_cv_context_row **p_row, const char *key,
                              const char *value)
{
  _mc_mo_cv_context_row *row = *p_row;

  // Show
  row->panel->node->layout->visible = true;

  // Set
  MCcall(mcu_set_textblock_text(row->key_textblock, key));

  MCcall(mcu_set_textblock_text(row->value_textblock, value));

  *p_row = row + 1;

  return 0;
}

int _mc_mo_cv_iterate_fill_context_rows(mc_mo_context_viewer_data *cv, _mc_mo_cv_context_row **row,
                                        hash_table_entry_t *cte, hash_table_entry_t *ctn)
{
  mc_mo_context_data *ctx;
  for (; cte < ctn; ++cte) {
    if (!cte->filled)
      continue;

    ctx = (mc_mo_context_data *)cte->value;

    // printf("--'%s':", ctx->key);
    switch (ctx->value_type) {
    case MC_MO_CONTEXT_DATA_VALUE_MC_STR:
      MCcall(_mc_mo_cv_set_context_row(cv, row, ctx->key, ctx->str.text));
      break;
    default:
      MCcall(_mc_mo_cv_set_context_row(cv, row, ctx->key, "{data}"));
      break;
    }

    if (*row >= cv->rows.items + cv->rows.size)
      break;
  }

  return 0;
}

int _mc_mo_refresh_context_viewer_display(mc_mo_context_viewer_data *cv)
{
  // puts("_mc_mo_refresh_context_viewer_display");
  _mc_mo_cv_context_row *row = cv->rows.items;

  hash_table_entry_t *pje, *pjn;
  hash_table_t *pjc;
  mc_mo_context_data *ctx;

  // Search global
  MCcall(_mc_mo_cv_iterate_fill_context_rows(cv, &row, cv->process_stack->global_context.entries,
                                             cv->process_stack->global_context.entries +
                                                 cv->process_stack->global_context.capacity));

  // Project Context
  if (row < cv->rows.items + cv->rows.size) {
    // Search through projects
    pje = cv->process_stack->project_contexts.entries;
    pjn = cv->process_stack->project_contexts.entries + cv->process_stack->project_contexts.capacity;

    for (; pje < pjn; ++pje) {
      if (!pje->filled)
        continue;

      pjc = (hash_table_t *)pje->value;

      MCcall(_mc_mo_cv_iterate_fill_context_rows(cv, &row, pjc->entries, pjc->entries + pjc->capacity));
      if (row >= cv->rows.items + cv->rows.size)
        break;
    }
  }

  // Hide any remaining rows
  for (; row < cv->rows.items + cv->rows.size; ++row) {
    row->panel->node->layout->visible = false;
  }

  return 0;
}

////////////////////////////////////////////////////////////
////////////////      Input Handling      //////////////////
////////////////////////////////////////////////////////////

void _mc_mo_cv_handle_input(mc_node *node, mci_input_event *input_event)
{
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)node->data;
  }
  input_event->handled = true;
}

void _mc_mo_cv_cancel_clicked(mci_input_event *input_event, mcu_button *button)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)button->tag;

  cv->node->layout->visible = false;
  mca_set_node_requires_rerender(cv->node);
}

////////////////////////////////////////////////////////////
////////////////      Initialization      //////////////////
////////////////////////////////////////////////////////////

int _mc_mo_cv_init_data(mc_node *module_node, mc_mo_process_stack *process_stack)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)malloc(sizeof(mc_mo_context_viewer_data));
  module_node->data = cv;
  cv->node = module_node;

  cv->process_stack = process_stack;

  // cv->shade_color = (render_color){0.13f, 0.12f, 0.17f, 0.8f};

  // cv->callback.state = NULL;
  // cv->callback.result_delegate = NULL;

  // cv->cells.size = 32;
  // cv->cells.ary = (mc_mo_cv_step_data *)malloc(sizeof(mc_mo_cv_step_data) * cv->cells.size);

  // // mo_data->render_target.image = NULL;
  // // mo_data->render_target.width = module_node->layout->preferred_width;
  // // mo_data->render_target.height = module_node->layout->preferred_height;
  // // mcr_create_texture_resource(mo_data->render_target.width, mo_data->render_target.height,
  // //                             MVK_IMAGE_USAGE_RENDER_TARGET_2D, &mo_data->render_target.image);

  // //   TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  // //   while (!cv->render_target.image) {
  // //     // puts("wait");
  // //     usleep(100);
  // //   }

  return 0;
}

int _mc_mo_cv_init_ui(mc_node *module_node)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)module_node->data;

  cv->background_color = COLOR_MIDNIGHT_EXPRESS;

  // Locals
  int a, b;
  // char buf[64];
  mca_node_layout *layout;
  mcu_panel *panel;
  mcu_button *button;
  // mcu_textbox *textbox;
  mcu_textblock *textblock;
  // mcu_dropdown *dropdown;
  // mc_mo_cv_step_data *cell;
  _mc_mo_cv_context_row *row;

  // Rows
  cv->rows.size = 16;
  cv->rows.items = (_mc_mo_cv_context_row *)malloc(sizeof(_mc_mo_cv_context_row) * cv->rows.size);
  for (a = 0; a < cv->rows.size; ++a) {
    row = cv->rows.items + a;

    MCcall(mcu_init_panel(module_node, &panel));
    row->panel = panel;

    panel->background_color = COLOR_DARK_SLATE_GRAY;

    layout = panel->node->layout;
    // layout->visible = false;
    layout->padding = (mc_paddingf){2, 32 + 31 * a, 2, 2};
    layout->preferred_height = 30;
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

    MCcall(mcu_init_textblock(panel->node, &textblock));
    row->key_textblock = textblock;
    textblock->background_color = (render_color){0.10f, 0.10f, 0.22f, 1.f};
    textblock->clip_text_to_bounds = true;

    layout = textblock->node->layout;
    layout->preferred_width = 210;
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
    layout->padding = (mc_paddingf){1, 1, 1, 1};

    MCcall(mcu_init_textblock(panel->node, &textblock));
    row->value_textblock = textblock;
    textblock->background_color = (render_color){0.10f, 0.10f, 0.22f, 1.f};
    textblock->clip_text_to_bounds = true;

    layout = textblock->node->layout;
    layout->preferred_width = 310;
    layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
    layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
    layout->padding = (mc_paddingf){1, 1, 1, 1};
  }

  // Exit Button
  MCcall(mcu_alloc_button(cv->node, &button));

  layout = button->node->layout;
  layout->preferred_width = 16;
  layout->preferred_height = 16;
  layout->padding = (mc_paddingf){4, 4, 4, 4};
  layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT;
  layout->vertical_alignment = VERTICAL_ALIGNMENT_TOP;

  button->background_color = COLOR_MIDNIGHT_EXPRESS;
  MCcall(mc_set_str(&button->str, "X"));
  button->tag = cv;
  button->left_click = (void *)&_mc_mo_cv_cancel_clicked;

  return 0;
}

int mc_mo_toggle_context_viewer_visibility(mc_node *node)
{
  mc_mo_context_viewer_data *cv = (mc_mo_context_viewer_data *)node->data;

  node->layout->visible = !node->layout->visible;

  MCcall(_mc_mo_refresh_context_viewer_display((mc_mo_context_viewer_data *)node->data));

  return 0;
}

// TODO remove @parent
int init_mo_context_viewer(mc_mo_process_stack *process_stack, mc_node **p_context_viewer)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // TODO -- get rid of node type
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_MODULE_ROOT, "context-viewer", &node));
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;

  MCcall(mca_init_node_layout(&node->layout));
  node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTRED;
  node->layout->vertical_alignment = VERTICAL_ALIGNMENT_CENTRED;
  node->layout->max_width = 540;
  node->layout->max_height = 480;

  node->layout->visible = false;
  node->layout->z_layer_index = 5U;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_mo_cv_render_headless;
  node->layout->render_present = (void *)&_mc_mo_cv_render_present;
  node->layout->handle_input_event = (void *)&_mc_mo_cv_handle_input;

  MCcall(_mc_mo_cv_init_data(node, process_stack));
  MCcall(_mc_mo_cv_init_ui(node));

  MCcall(mca_attach_node_to_hierarchy(app_info->global_node, node));
  *p_context_viewer = node;

  return 0;
}