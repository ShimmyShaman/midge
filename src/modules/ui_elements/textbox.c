/* textbox.c */

#include <stdio.h>
#include <stdlib.h>

#include "control/mc_controller.h"
#include "env/environment_definitions.h"

#include "modules/ui_elements/textbox.h"

void _mcu_render_textbox_present(image_render_details *image_render_queue, mc_node *node)
{
  mcu_textbox *textbox = (mcu_textbox *)node->data;

  // Background
  mcr_issue_render_command_colored_quad(image_render_queue, (unsigned int)node->layout->__bounds.x,
                                        (unsigned int)node->layout->__bounds.y,
                                        (unsigned int)node->layout->__bounds.width,
                                        (unsigned int)node->layout->__bounds.height, textbox->background_color);
  if (textbox->cursor.visible) {
    mcr_issue_render_command_colored_quad(
        image_render_queue,
        (unsigned int)(node->layout->__bounds.x + textbox->content_padding.left +
                       textbox->font_horizontal_stride * ((float)textbox->cursor.col - 0.05f)),
        (unsigned int)(node->layout->__bounds.y + textbox->content_padding.top - 0), 2, 18 - 2, COLOR_GHOST_WHITE);
  }

  // Text
  // printf("rendertextbox- %u %u %s %u\n", (unsigned int)node->layout->__bounds.x,
  //        (unsigned int)node->layout->__bounds.y, textbox->str->text, textbox->font->resource_uid);
  mcr_issue_render_command_text(image_render_queue,
                                (unsigned int)(node->layout->__bounds.x + textbox->content_padding.left),
                                (unsigned int)(node->layout->__bounds.y + textbox->content_padding.top),
                                textbox->contents->text, textbox->font, textbox->font_color);

  // TODO -- a better way to determine if the node is focused...
  // mc_node *focused_node;
  // mca_obtain_focused_node(&focused_node);
}

void _mcu_set_textbox_cursor_position(mcu_textbox *textbox, int line, int column)
{
  // Determine how the cursor will move
  if (column <= 0) {
    textbox->cursor.col = 0;
  }
  else {
    if (column > textbox->contents->len) {
      textbox->cursor.col = textbox->contents->len;
    }
    else {
      textbox->cursor.col = column;
    }
  }

  // printf("Cursor placed at {%i,%i}\n", textbox->cursor.line, textbox->cursor.col);
  textbox->cursor.visible = true;
  mca_set_node_requires_rerender(textbox->node);
}

void _mcu_textbox_handle_input_event(mc_node *node, mci_input_event *input_event)
{
  // printf("_mcu_textbox_handle_input_event\n");
  mcu_textbox *textbox = (mcu_textbox *)node->data;

  switch (input_event->type) {
  case INPUT_EVENT_MOUSE_PRESS: {
    if (input_event->button_code == MOUSE_BUTTON_LEFT) {

      // Find the column index
      int click_relative_x =
          input_event->input_state->mouse.x -
          (int)(node->layout->__bounds.x + textbox->content_padding.left - textbox->font_horizontal_stride * 0.5f);
      printf("click_relative_x:%i\n", click_relative_x);
      if (click_relative_x < 0 && click_relative_x > -2)
        click_relative_x = 0;
      if (click_relative_x >= 0) {
        _mcu_set_textbox_cursor_position(textbox, 0, (int)((float)click_relative_x / textbox->font_horizontal_stride));
      }
    }
    // printf("_mcu_textbox_handle_input_event-1\n");
    // if ((mc_mouse_button_code)input_event->button_code == MOUSE_BUTTON_LEFT) {
    // }
    // if (textbox->left_click && (mc_mouse_button_code)input_event->textbox_code == MOUSE_BUTTON_LEFT) {
    //   // printf("_mcu_textbox_handle_input_event-2\n");
    //   // Fire left-click
    //   // TODO fptr casting
    //   void (*left_click)(mcu_textbox *, mc_point) = (void (*)(mcu_textbox *, mc_point))textbox->left_click;
    //   left_click(textbox, (mc_point){input_event->input_state->mouse.x, input_event->input_state->mouse.y});
    // }
    mca_focus_node(node);
  } break;
  case INPUT_EVENT_KEY_RELEASE: {
    switch (input_event->button_code) {
    case KEY_CODE_BACKSPACE: {
      if (input_event->input_state->ctrl_function & BUTTON_STATE_DOWN) {
        restrict_mc_str(textbox->contents, 0);
      }
      else if (textbox->cursor.col > 0) {
        remove_from_mc_str(textbox->contents, textbox->cursor.col - 1, 1);
        --textbox->cursor.col;
      }
    } break;
    case KEY_CODE_DELETE: {
      if (input_event->input_state->ctrl_function & BUTTON_STATE_DOWN) {
        restrict_mc_str(textbox->contents, textbox->cursor.col);
      }
      else {
        remove_from_mc_str(textbox->contents, textbox->cursor.col, 1);
      }
    } break;
    case KEY_CODE_ENTER:
    case KEY_CODE_RETURN: {
      if (textbox->submit) {
        void (*submit)(mci_input_event *, mcu_textbox *) = (void (*)(mci_input_event *, mcu_textbox *))textbox->submit;
        submit(input_event, textbox);
      }
    } break;
    default: {
      char c[2];
      int res = get_key_input_code_char((input_event->input_state->shift_function & BUTTON_STATE_DOWN),
                                        input_event->button_code, &c[0]);
      c[1] = '\0';
      if (!res) {
        insert_into_mc_str(textbox->contents, c, textbox->cursor.col);
        ++textbox->cursor.col;
      }
    } break;
    }

    // Rerender in all cases
    mca_set_node_requires_rerender(textbox->node);
  } break;
  }

  input_event->handled = true;
}

int _mcu_init_textbox_data(mc_node *node)
{
  mcu_textbox *textbox = (mcu_textbox *)malloc(sizeof(mcu_textbox)); // TODO -- malloc check
  textbox->node = node;
  node->data = textbox;

  textbox->tag = NULL;
  textbox->background_color = COLOR_NEARLY_BLACK;
  textbox->content_padding.left = 1;
  textbox->content_padding.top = 1;

  MCcall(init_mc_str(&textbox->contents));
  textbox->font = NULL;
  textbox->font_color = COLOR_LIGHT_YELLOW;
  textbox->font_horizontal_stride = 9.2794f;

  textbox->cursor.col = 0;
  textbox->cursor.visible = false;
  textbox->submit = NULL;

  return 0;
}

int mcu_init_textbox(mc_node *parent, mcu_textbox **p_textbox)
{
  // Node
  mc_node *node;
  MCcall(mca_init_mc_node(NODE_TYPE_DOESNT_MATTER, "unnamed-textbox", &node));

  // Layout
  MCcall(mca_init_node_layout(&node->layout));
  node->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;
  node->layout->update_layout = (void *)&mca_update_typical_node_layout;
  node->layout->render_headless = NULL;
  node->layout->render_present = (void *)&_mcu_render_textbox_present;
  node->layout->handle_input_event = (void *)&_mcu_textbox_handle_input_event;

  // Default Settings
  node->layout->preferred_width = 80;
  node->layout->preferred_height = 24;

  // Control
  MCcall(_mcu_init_textbox_data(node));

  // Set to out pointer
  *p_textbox = (mcu_textbox *)node->data;

  MCcall(mca_attach_node_to_hierarchy(parent, node));

  return 0;
}

// void mca_init_textbox_context_menu_options()
// {
//   // // TODO this void * casting business
//   // void *arg = (void *)&mca_visual_project_create_add_textbox;
//   // mca_global_context_menu_add_option_to_node_context(NODE_TYPE_textbox, "change text", arg);
// }