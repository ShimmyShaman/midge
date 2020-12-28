/* source_editor.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "control/mc_controller.h"
#include "core/midge_app.h"
#include "env/environment_definitions.h"
#include "mc_str.h"

#include "modules/mc_io/mc_file.h"
#include "modules/render_utilities/render_util.h"
#include "modules/source_editor/source_editor.h"

#define MC_SE_LINE_STRIDE 21

int _mc_se_render_tab_list_headless(render_thread_info *render_thread, mc_node *node)
{
  // Data
  mc_source_editor *se = (mc_source_editor *)node->data;

  unsigned int left = (unsigned int)node->layout->__bounds.x;
  unsigned int top = (unsigned int)node->layout->__bounds.y;

  image_render_details *irq;
  mcr_obtain_image_render_request(render_thread, &irq);
  irq->render_target = NODE_RENDER_TARGET_IMAGE;
  irq->clear_color = COLOR_TRANSPARENT;
  // printf("global_data->screen : %u, %u\n", global_data->screen.width,
  // global_data->screen.height);
  irq->image_width = se->tab_index.width;
  irq->image_height = se->tab_index.height;
  irq->data.target_image.image = se->tab_index.image;
  irq->data.target_image.screen_offset_coordinates.x = left;
  irq->data.target_image.screen_offset_coordinates.y = top;

  float tw, th;
  char buf[64];

  // Focused source
  mcf_obtain_filename_with_extension(se->source_files.focus->sf->filepath, buf, 64);
  mcr_determine_text_display_dimensions(NULL, buf, &tw, &th);

  mcr_issue_render_command_colored_quad(irq, left, top, tw + 8 + 5, se->tab_index.height, COLOR_BLACKCURRANT);

  mcr_issue_render_command_text(irq, left + 4, top + 4, buf, NULL, COLOR_SILVER);

  mcr_submit_image_render_request(render_thread, irq);

  return 0;
}

int _mc_se_rerender_lines_headless(render_thread_info *render_thread, mc_node *node)
{
  // Data
  mc_source_editor *se = (mc_source_editor *)node->data;
  mc_source_editor_file *esf = se->source_files.focus;
  mc_source_editor_line_group *lg = &se->line_images;
  mc_source_editor_line *rln;

  // For each visible line
  unsigned int top = (int)(node->layout->__bounds.y + se->border.size + se->content_padding.top + se->tab_index.height);
  unsigned int bottom =
      (int)(node->layout->__bounds.y + node->layout->__bounds.height - se->border.size - se->content_padding.bottom);
  unsigned int y;

  char *str;
  int li = (esf->scroll_offset) / MC_SE_LINE_STRIDE, ri = 0;

  for (y = top + esf->scroll_offset % MC_SE_LINE_STRIDE; y < bottom; y += MC_SE_LINE_STRIDE) {
    if (li >= esf->lines.count)
      break;

    rln = &lg->lines[ri];

    rln->active = true;
    rln->draw_offset_y = y;

    str = esf->lines.items[li].text;
    {
      image_render_details *irq;
      MCcall(mcr_obtain_image_render_request(render_thread, &irq));
      irq->render_target = NODE_RENDER_TARGET_IMAGE;
      irq->clear_color = COLOR_TRANSPARENT;
      // printf("global_data->screen : %u, %u\n", global_data->screen.width,
      // global_data->screen.height);
      irq->image_width = rln->width;
      irq->image_height = rln->height;
      irq->data.target_image.image = rln->image;
      irq->data.target_image.screen_offset_coordinates.x = 0;
      irq->data.target_image.screen_offset_coordinates.y = 0;

      MCcall(mcr_issue_render_command_text(irq, 0, 0, str, NULL, COLOR_LIGHT_SKY_BLUE));

      MCcall(mcr_submit_image_render_request(render_thread, irq));
    }

    ++li;
    ++ri;
  }
  for (; ri < lg->lines_size; ++ri) {
    lg->lines[ri].active = false;
  }

  return 0;
}

int _mc_se_render_headless(render_thread_info *render_thread, mc_node *node)
{
  // Data
  mc_source_editor *se = (mc_source_editor *)node->data;

  // // Children
  // for (int a = 0; a < node->children->count; ++a) {
  //   mc_node *child = node->children->items[a];
  //   if (child->layout && child->layout->visible && child->layout->render_headless &&
  //       child->layout->__requires_rerender) {
  //     // TODO fptr casting
  //     void (*render_node_headless)(render_thread_info *, mc_node *) =
  //         (void (*)(render_thread_info *, mc_node *))child->layout->render_headless;
  //     render_node_headless(render_thread, child);
  //   }
  // }
  MCcall(_mc_se_rerender_lines_headless(render_thread, node));

  if (se->tab_index.requires_rerender) {
    MCcall(_mc_se_render_tab_list_headless(render_thread, node));
  }

  return 0;
}

void _mc_se_render_present(image_render_details *image_render_queue, mc_node *node)
{
  // Data
  mc_source_editor *se = (mc_source_editor *)node->data;

  mcr_issue_render_command_colored_quad(
      image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,
      (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, se->background_color);

  mc_source_editor_line *sel;
  for (sel = se->line_images.lines; sel->active && sel < se->line_images.lines + se->line_images.lines_size; ++sel) {
    mcr_issue_render_command_textured_quad(
        image_render_queue, (unsigned int)(node->layout->__bounds.x + se->border.size + se->content_padding.left),
        sel->draw_offset_y, sel->width, sel->height, sel->image);
  }

  mcr_issue_render_command_textured_quad(image_render_queue, (unsigned int)(node->layout->__bounds.x + se->border.size),
                                         (unsigned int)(node->layout->__bounds.y + se->border.size),
                                         se->tab_index.width, se->tab_index.height, se->tab_index.image);

  mc_render_border(image_render_queue, node, se->border.size, se->border.color);
}

void _mc_se_handle_input(mc_node *node, mci_input_event *input_event)
{
  // Data
  mc_source_editor *se = (mc_source_editor *)node->data;

  // printf("_mco_handle_input\n");
  input_event->handled = true;
  if (input_event->type == INPUT_EVENT_MOUSE_PRESS || input_event->type == INPUT_EVENT_MOUSE_RELEASE) {
    input_event->handled = true;
    mca_focus_node(node);
  }
}

int _mc_load_focused_editing_source(mc_source_editor *se)
{
  mc_source_editor_file *esf = se->source_files.focus;

  char *file_text;
  MCcall(read_file_text(esf->sf->filepath, &file_text));

  // Overwrite lines with current source file text
  int li = 0, a;
  char *c = file_text, *s;
  mc_str *ls;
  while (*c != '\0') {
    // Set the str
    if (li >= esf->lines.size) {
      // Resize the array
      reallocate_array((void **)&esf->lines.items, &esf->lines.size, esf->lines.size * 2, sizeof(mc_str), true);
      // for (int k = li; k < esf->lines.size; ++k) {
      //   if (esf->lines.items[k].alloc || esf->lines.items[k].len || esf->lines.items[k].text) {
      //     MCerror(2484, "reallocate_array with memsetzero failed %i ", k);
      //   }
      // }
    }
    ls = &esf->lines.items[li];
    ls->len = 0U;
    // puts("aaa");

    s = c;
    while (*c != '\0' && *c != '\n') {
      ++c;
      // printf("%c-", *c);
    }
    // printf("\n");
    // printf("li:%i esflisz:%u ls:%p\n", li, esf->lines.size, ls);
    // printf("ls %u \n", ls->alloc);
    // printf("ls %u \n", ls->len);
    // printf("ls %s \n", ls->text);
    MCcall(append_to_mc_strn(ls, s, c - s));
    // puts("bbb");

    if (*c == '\n')
      ++c;

    ++li;
  }
  // puts("ccc");

  // Wipe any remainder lines from previous source file & set line count
  for (ls = esf->lines.items + li; ls < esf->lines.items + esf->lines.count; ++ls) {
    ls->len = 0U;
  }
  esf->lines.count = li;

  free(file_text);
  return 0;
}

int _mc_se_open_filepath(mc_source_editor *se, const char *filepath)
{
  mc_app_itp_data *app_itp;
  mc_obtain_app_itp_data(&app_itp);

  int a;
  mc_source_editor_file *esf, *xesf;
  mc_source_file_info *sf;
  char fp[256];

  // Obtain the completed esf for the filepath
  MCcall(mcf_obtain_full_path(filepath, fp, 256));

  xesf = se->source_files.items + se->source_files.size;
  for (esf = se->source_files.items; esf < xesf; ++esf) {
    if (!esf->sf)
      break;
    if (!strcmp(esf->sf->filepath, fp))
      break;
  }
  if (esf == xesf) {
    MCerror(9528, "TODO resize array and set sf");
  }
  if (!esf->sf) {
    // Find and set the source file
    for (a = 0; a < app_itp->source_files.count; ++a) {
      sf = app_itp->source_files.items[a];
      if (!strcmp(sf->filepath, fp)) {
        esf->sf = sf;
        esf->scroll_offset = 0;
        break;
      }
    }
    if (!esf->sf) {
      MCerror(4728, "couldn't find source file for path TODO");
    }
    ++se->source_files.used;
  }

  // Open
  se->source_files.focus = esf;
  MCcall(_mc_load_focused_editing_source(se));

  // Update Tabs
  se->tab_index.requires_rerender = true;
  MCcall(mca_set_node_requires_rerender(se->node));

  return 0;
}

int _mc_se_handle_source_file_open_request(void *handler_state, void *event_args)
{
  // Data
  mc_source_editor *se = (mc_source_editor *)handler_state;
  const char *filepath = (const char *)event_args;

  MCcall(_mc_se_open_filepath(se, filepath));

  se->node->layout->visible = true;
  MCcall(mca_focus_node(se->node));
  return 0;
}

int _mc_se_handle_source_entity_focus_request(void *handler_state, void *event_args)
{
  // Data
  mc_source_editor *se = (mc_source_editor *)handler_state;
  void **evargs = (void **)event_args;
  const char *entity_name = (const char *)evargs[0];
  mc_source_entity_focus_options options = *(mc_source_entity_focus_options *)evargs[1];

  mc_source_file_info *sf;
  struct_info *si;

  // Find the entity
  MCcall(find_struct_info(entity_name, &si));
  if (si) {
    sf = si->source_file;
  }
  else {
    MCerror(8428, "TODO");
  }
  if (!sf) {
    MCerror(4191, "TODO");
  }

  // Get the source file
  MCcall(_mc_se_open_filepath(se, sf->filepath));
  printf("SE_FOCUS:'struct %s {'\n", si->name);

  se->node->layout->visible = true;
  MCcall(mca_focus_node(se->node));
  return 0;
}

int _mc_se_load_resources(mc_node *node)
{
  // Data
  mc_source_editor *se = (mc_source_editor *)node->data;

  se->tab_index.height = 27;
  se->tab_index.width = 1200;
  se->tab_index.requires_rerender = true;

  // Lines Images
  MCcall(create_hash_table(64, &se->line_images.cache));
  se->line_images.lines_size = 64;
  se->line_images.lines = (mc_source_editor_line *)calloc(se->line_images.lines_size, sizeof(mc_source_editor_line));
  for (int a = 0; a < se->line_images.lines_size; ++a) {
    se->line_images.lines[a].width = 1200;               // TODO -- dynamic size?
    se->line_images.lines[a].height = MC_SE_LINE_STRIDE; // TODO -- dynamic size?
    MCcall(mcr_create_texture_resource(se->line_images.lines[a].width, se->line_images.lines[a].height,
                                       MVK_IMAGE_USAGE_RENDER_TARGET_2D, &se->line_images.lines[a].image));
  }

  // Tab Index
  se->tab_index.image = NULL;
  MCcall(mcr_create_texture_resource(se->tab_index.width, se->tab_index.height, MVK_IMAGE_USAGE_RENDER_TARGET_2D,
                                     &se->tab_index.image));

  // TODO -- mca_attach_node_to_hierarchy_pending_resource_acquisition ??
  while (!se->tab_index.image) {
    // puts("wait");
    usleep(100);
  }
  return 0;
}

int _mc_se_init_data(mc_node *node)
{
  mc_source_editor *se = (mc_source_editor *)malloc(sizeof(mc_source_editor));
  node->data = (void *)se;
  se->node = node;

  se->source_files.size = 16U;
  se->source_files.items = (mc_source_editor_file *)calloc(se->source_files.size, sizeof(mc_source_editor_file));
  se->source_files.used = 0U;
  se->source_files.focus = NULL;

  se->background_color = COLOR_NEARLY_BLACK;

  se->border.size = 4;
  se->border.color = COLOR_GRAY;

  se->content_padding.left = 4U;
  se->content_padding.top = 8U;
  se->content_padding.bottom = 4U;

  return 0;
}

int mc_se_init_source_editor()
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);
  //   instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);

  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_ABSTRACT, "source-editor", &node));
  MCcall(mca_init_node_layout(&node->layout));
  node->children = (mc_node_list *)malloc(sizeof(mc_node_list));
  node->children->count = 0;
  node->children->alloc = 0;
  // node->layout->preferred_width = 400;
  // node->layout->preferred_height = 500;

  node->layout->padding = (mc_paddingf){280, 80, 80, 80};
  // node->layout->horizontal_alignment = HORIZONTAL_ALIGNMENT_LEFT;
  // node->layout->vertical_alignment = VERTICAL_ALIGNMENT_BOTTOM;

  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = (void *)&_mc_se_render_headless;
  node->layout->render_present = (void *)&_mc_se_render_present;
  node->layout->handle_input_event = (void *)&_mc_se_handle_input;

  // TODO
  node->layout->visible = false;

  // Source Editor Data
  MCcall(_mc_se_init_data(node));

  // Event Registers
  MCcall(mca_register_event_handler(MC_APP_EVENT_SOURCE_FILE_OPEN_REQ, &_mc_se_handle_source_file_open_request,
                                    node->data));
  MCcall(mca_register_event_handler(MC_APP_EVENT_SOURCE_ENTITY_FOCUS_REQ, &_mc_se_handle_source_entity_focus_request,
                                    node->data));

  // Graphical resources
  MCcall(_mc_se_load_resources(node));

  // Attach to midge hierarchy & return
  MCcall(mca_attach_node_to_hierarchy(app_info->global_node, node));
  return 0;
}