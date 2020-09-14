
#include "control/mc_controller.h"
#include "core/core_definitions.h"

void mcc_initialize_input_state()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mci_input_state *input_state = (mci_input_state *)malloc(sizeof(mci_input_state));
  global_data->input_state = input_state;

  global_data->input_state_requires_update = false;

  input_state->alt_function = KEY_STATE_UP;
  input_state->ctrl_function = KEY_STATE_UP;
  input_state->shift_function = KEY_STATE_UP;
}

// Only call this method directly to 'simulate' a mouse left click
void mcc_input_mouse_left_click(int screen_x, int screen_y)
{
  mc_node_list *node_hit_list;
  mui_get_ui_elements_at_point(screen_x, screen_y, &node_hit_list);

  bool handled = false;
  for (int a = 0; a < node_hit_list->count && !handled; ++a)
    mui_handle_mouse_left_click(node_hit_list->items[a], screen_x, screen_y, &handled);
}

// Only call this method directly to 'simulate' a mouse right click
void mcc_input_mouse_right_click(int screen_x, int screen_y)
{
  // Get all elements under the mouse cursor
  mc_node_list *node_hit_list;
  mui_get_ui_elements_at_point(screen_x, screen_y, &node_hit_list);

  bool handled = false;
  for (int a = 0; a < node_hit_list->count && !handled; ++a)
    mui_handle_mouse_right_click(node_hit_list->items[a], screen_x, screen_y, &handled);
}

// Handles all input from the X11/xcb? platform
void mcc_handle_xcb_input()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mci_input_state *input_state = global_data->input_state;

  // if (global_data->render_thread->input_buffer.event_count > 0) {
  //   input_event->handled = false;
  //   // printf("input_recorded\n");

  for (int i = 0; i < global_data->render_thread->input_buffer.event_count; ++i) {
    // New Input Event
    window_input_event *xcb_input = &global_data->render_thread->input_buffer.events[i];

    switch (xcb_input->type) {
    case INPUT_EVENT_MOUSE_PRESS: {
      // Set input event for controls to handle
      // input_event->type = xcb_input->type;
      // input_event->detail = xcb_input->detail;

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
      //   // printf("%p\n", child->data.vinput_stateual.input_handler);
      //   // if (child->data.vinput_stateual.input_handler) {
      //   //   printf("%p\n", (*child->data.vinput_stateual.input_handler));
      //   // }
      //   // Check input_state vinput_stateual and has input handler and mouse event input_state within bounds
      //   if (child->type != NODE_TYPE_VISUAL || !child->data.vinput_stateual.vinput_stateible ||
      //   !child->data.vinput_stateual.input_handler
      //   ||
      //       !(*child->data.vinput_stateual.input_handler))
      //     continue;
      //   // printf("A >%s\n", child->name);
      //   if (input_event->detail.mouse.x < child->data.vinput_stateual.bounds.x ||
      //       input_event->detail.mouse.y < child->data.vinput_stateual.bounds.y ||
      //       input_event->detail.mouse.x >= child->data.vinput_stateual.bounds.x +
      //       child->data.vinput_stateual.bounds.width || input_event->detail.mouse.y >=
      //       child->data.vinput_stateual.bounds.y + child->data.vinput_stateual.bounds.height)
      //     continue;
      //   // printf("B >%s\n", child->name);

      //   void *vargs[3];
      //   vargs[0] = &elapsed;
      //   vargs[1] = &child;
      //   vargs[2] = &input_event;
      //   // printf("calling input delegate for %s\n", child->name);
      //   // printf("loop](*child->data.vinput_stateual.input_handler):%p\n",
      //   (*child->data.vinput_stateual.input_handler)); MCcall((*child->data.vinput_stateual.input_handler)(3,
      //   vargs));
      // }

      // if (!input_event.handled) {
      //   printf("unhandled_mouse_event:%i::%i\n", xcb_input->type,
      //          xcb_input->detail.mouse.button);
      // }
    } break;
    // case INPUT_EVENT_FOCUS_IN:
    // case INPUT_EVENT_FOCUS_OUT: {
    //   input_event->altDown = false;
    //   // printf("alt input_state %s\n", input_event->altDown ? "DOWN" : "UP");
    // } break;
    case INPUT_EVENT_KEY_RELEASE:
    case INPUT_EVENT_KEY_PRESS: {
      switch (xcb_input->detail.keyboard.key) {
      case KEY_CODE_LEFT_ALT:
      case KEY_CODE_RIGHT_ALT:
        if (xcb_input->type == INPUT_EVENT_KEY_PRESS) {
          input_state->alt_function = (int)KEY_STATE_DOWN | KEY_STATE_PRESSED;
        }
        else if (xcb_input->type == INPUT_EVENT_KEY_RELEASE) {
          input_state->alt_function = KEY_STATE_UP | KEY_STATE_RELEASED;
        }
        break;
      case KEY_CODE_LEFT_CTRL:
      case KEY_CODE_RIGHT_CTRL:
        if (xcb_input->type == INPUT_EVENT_KEY_PRESS) {
          input_state->ctrl_function = KEY_STATE_DOWN | KEY_STATE_PRESSED;
        }
        else if (xcb_input->type == INPUT_EVENT_KEY_RELEASE) {
          input_state->ctrl_function = KEY_STATE_UP | KEY_STATE_RELEASED;
        }
        break;
      case KEY_CODE_LEFT_SHIFT:
      case KEY_CODE_RIGHT_SHIFT:
        if (xcb_input->type == INPUT_EVENT_KEY_PRESS) {
          input_state->shift_function = KEY_STATE_DOWN | KEY_STATE_PRESSED;
        }
        else if (xcb_input->type == INPUT_EVENT_KEY_RELEASE) {
          input_state->shift_function = KEY_STATE_UP | KEY_STATE_RELEASED;
        }
        break;

      default: {
        // Set input event for controls to handle
        // input_event->type = xcb_input->type;
        // input_event->detail = xcb_input->detail;

        if ((input_state->ctrl_function & KEY_STATE_DOWN) && (input_state->ctrl_function & KEY_STATE_DOWN) &&
            xcb_input->detail.keyboard.key == KEY_CODE_W) {
          global_data->exit_requested = true;
          continue;
        }

        if ((input_state->ctrl_function & KEY_STATE_DOWN) && (input_state->ctrl_function & KEY_STATE_DOWN) &&
            xcb_input->detail.keyboard.key == KEY_CODE_N) {

          mch_create_new_visual_project("PushTheButton");
          continue;
        }

        // Global Node Hierarchy for (int i = 0; !input_event->handled && i < global_data->global_node->children.count;
        //                            ++i)
        // {
        //   node *child = (node *)global_data->global_node->children.items[i];
        //   if (child->type != NODE_TYPE_VISUAL)
        //     continue;
        //   // printf("checking input delegate exinput_statets\n");
        //   if (!child->data.vinput_stateual.input_handler || !*child->data.vinput_stateual.input_handler)
        //     continue;

        //   void *vargs[3];
        //   vargs[0] = &elapsed;
        //   vargs[1] = &child;
        //   vargs[2] = &input_event;
        //   // printf("calling input delegate\n");
        //   // printf("loop](*child->data.vinput_stateual.input_handler):%p\n",
        //   (*child->data.vinput_stateual.input_handler));
        //   MCcall((*child->data.vinput_stateual.input_handler)(3, vargs));
        // }

        // if (!input_event->handled) {
        //   printf("unhandled_keyboard_event:%i::%i\n", xcb_input->type, xcb_input->detail.keyboard.key);
        // }
        break;
      }
      }
    } break;
    default:
      break;
    }
  }

  // Reset render thread input buffer
  global_data->render_thread->input_buffer.event_count = 0;
}

void mcc_update_xcb_input()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mci_input_state *input_state = global_data->input_state;

  // Update functions
  input_state->alt_function &= ~KEY_STATE_PRESSED;
  input_state->alt_function &= ~KEY_STATE_RELEASED;
  input_state->ctrl_function &= ~KEY_STATE_PRESSED;
  input_state->ctrl_function &= ~KEY_STATE_RELEASED;
  input_state->shift_function &= ~KEY_STATE_PRESSED;
  input_state->shift_function &= ~KEY_STATE_RELEASED;
  global_data->input_state_requires_update = false;

  // Handle new input
  if (global_data->render_thread->input_buffer.event_count > 0) {
    mcc_handle_xcb_input();

    global_data->input_state_requires_update = true;
  }
}
