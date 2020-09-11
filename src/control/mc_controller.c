#include "control/mc_controller.h"

void mcc_input_mouse_left_click(int screen_x, int screen_y) { printf("left_mouse_click %i, %i\n", screen_x, screen_y); }

void mcc_input_mouse_right_click(int screen_x, int screen_y)
{
  node *node_hit = NULL;
  mui_get_ui_elements_at_point(screen_x, screen_y);
  if (!node_hit) {
    return;
  }
}

void mcc_handle_xcb_input()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // if (global_data->render_thread->input_buffer.event_count > 0) {
  //   input_event->handled = false;
  //   // printf("input_recorded\n");

  for (int i = 0; i < global_data->render_thread->input_buffer.event_count; ++i) {
    // New Input Event
    window_input_event *xcb_input = &global_data->render_thread->input_buffer.events[i];

    switch (xcb_input->type) {
    case INPUT_EVENT_MOUSE_PRESS: {
      // Set input event for controls to handle
      // input_event->type = global_data->render_thread->input_buffer.events[i].type;
      // input_event->detail = global_data->render_thread->input_buffer.events[i].detail;

      switch (xcb_input->detail.mouse.button) {
      case MOUSE_BUTTON_LEFT:
        mcc_input_mouse_left_click(xcb_input->detail.mouse.x, xcb_input->detail.mouse.y);
        break;
      case MOUSE_BUTTON_RIGHT:
        mcc_input_mouse_right_click(xcb_input->detail.mouse.x, xcb_input->detail.mouse.y);
        break;
      default:
        break;
      }

      // Send input through the Global Node Hierarchy
      // for (int i = 0; !input_event->handled && i < global_data->global_node->children.count; ++i) {
      //   node *child = (node *)global_data->global_node->children.items[i];

      //   // printf("INPUT_EVENT_MOUSE_PRESS>%s\n", child->name);
      //   // printf("%p\n", child->data.visual.input_handler);
      //   // if (child->data.visual.input_handler) {
      //   //   printf("%p\n", (*child->data.visual.input_handler));
      //   // }
      //   // Check is visual and has input handler and mouse event is within bounds
      //   if (child->type != NODE_TYPE_VISUAL || !child->data.visual.visible || !child->data.visual.input_handler
      //   ||
      //       !(*child->data.visual.input_handler))
      //     continue;
      //   // printf("A >%s\n", child->name);
      //   if (input_event->detail.mouse.x < child->data.visual.bounds.x ||
      //       input_event->detail.mouse.y < child->data.visual.bounds.y ||
      //       input_event->detail.mouse.x >= child->data.visual.bounds.x + child->data.visual.bounds.width ||
      //       input_event->detail.mouse.y >= child->data.visual.bounds.y + child->data.visual.bounds.height)
      //     continue;
      //   // printf("B >%s\n", child->name);

      //   void *vargs[3];
      //   vargs[0] = &elapsed;
      //   vargs[1] = &child;
      //   vargs[2] = &input_event;
      //   // printf("calling input delegate for %s\n", child->name);
      //   // printf("loop](*child->data.visual.input_handler):%p\n", (*child->data.visual.input_handler));
      //   MCcall((*child->data.visual.input_handler)(3, vargs));
      // }

      // if (!input_event.handled) {
      //   printf("unhandled_mouse_event:%i::%i\n", global_data->render_thread->input_buffer.events[i].type,
      //          global_data->render_thread->input_buffer.events[i].detail.mouse.button);
      // }
    } break;
      // case INPUT_EVENT_FOCUS_IN:
      // case INPUT_EVENT_FOCUS_OUT: {
      //   input_event->altDown = false;
      //   // printf("alt is %s\n", input_event->altDown ? "DOWN" : "UP");
      // } break;
      // case INPUT_EVENT_KEY_RELEASE:
      // case INPUT_EVENT_KEY_PRESS: {
      //   switch (global_data->render_thread->input_buffer.events[i].detail.keyboard.key) {
      //   case KEY_CODE_LEFT_ALT:
      //   case KEY_CODE_RIGHT_ALT:
      //     input_event->altDown = global_data->render_thread->input_buffer.events[i].type ==
      //     INPUT_EVENT_KEY_PRESS;
      //     // printf("alt is %s\n", input_event->altDown ? "DOWN" : "UP");
      //     break;
      //   case KEY_CODE_LEFT_SHIFT:
      //   case KEY_CODE_RIGHT_SHIFT:
      //     input_event->shiftDown = global_data->render_thread->input_buffer.events[i].type ==
      //     INPUT_EVENT_KEY_PRESS; break;
      //   case KEY_CODE_LEFT_CTRL:
      //   case KEY_CODE_RIGHT_CTRL:
      //     input_event->ctrlDown = global_data->render_thread->input_buffer.events[i].type ==
      //     INPUT_EVENT_KEY_PRESS; break;

      // default: {
      // // Set input event for controls to handle
      // input_event->type = global_data->render_thread->input_buffer.events[i].type;
      // input_event->detail = global_data->render_thread->input_buffer.events[i].detail;

      // if (input_event->detail.keyboard.key == KEY_CODE_W && input_event->ctrlDown && input_event->shiftDown) {
      //   exit_loop = true;
      //   continue;
      // }

      // Global Node Hierarchy
      // for (int i = 0; !input_event->handled && i < global_data->global_node->children.count; ++i) {
      //   node *child = (node *)global_data->global_node->children.items[i];
      //   if (child->type != NODE_TYPE_VISUAL)
      //     continue;
      //   // printf("checking input delegate exists\n");
      //   if (!child->data.visual.input_handler || !*child->data.visual.input_handler)
      //     continue;

      //   void *vargs[3];
      //   vargs[0] = &elapsed;
      //   vargs[1] = &child;
      //   vargs[2] = &input_event;
      //   // printf("calling input delegate\n");
      //   // printf("loop](*child->data.visual.input_handler):%p\n", (*child->data.visual.input_handler));
      //   MCcall((*child->data.visual.input_handler)(3, vargs));
      // }

      //   if (!input_event->handled) {
      //     printf("unhandled_keyboard_event:%i::%i\n", global_data->render_thread->input_buffer.events[i].type,
      //            global_data->render_thread->input_buffer.events[i].detail.keyboard.key);
      //   }
      //   break;
      // }
      // }
    // }
    default:
      break;
    }
  }
  global_data->render_thread->input_buffer.event_count = 0;
}