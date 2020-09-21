
#include "control/mc_controller.h"
#include "core/core_definitions.h"

void mcc_initialize_input_state()
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mci_input_state *input_state = (mci_input_state *)malloc(sizeof(mci_input_state));
  global_data->input_state = input_state;

  global_data->input_state_requires_update = false;

  input_state->alt_function = BUTTON_STATE_UP;
  input_state->ctrl_function = BUTTON_STATE_UP;
  input_state->shift_function = BUTTON_STATE_UP;
}

// typedef enum mci_mouse_event_type {
//   MOUSE_EVENT_NONE = 0,
//   MOUSE_EVENT_LEFT_DOWN,
//   MOUSE_EVENT_LEFT_DOWN,
//   MOUSE_EVENT_LEFT_DOWN,
//   MOUSE_EVENT_LEFT_DOWN,
//   MOUSE_EVENT_LEFT_DOWN,
//   MOUSE_EVENT_LEFT_DOWN,
//   MOUSE_EVENT_LEFT_DOWN,
//   MOUSE_EVENT_LEFT_DOWN,
// } mci_mouse_event_type;

typedef struct mci_input_event {
  window_input_event_type type;
  int button_code;

  mci_input_state *input_state;

  bool handled;
} mci_input_event;

void mcc_issue_mouse_event(window_input_event_type event_type, int button_code)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  mci_input_event input_event;
  input_event.type = event_type;
  input_event.button_code = button_code;
  input_event.input_state = global_data->input_state;
  input_event.handled = false;

  mc_node_list *node_hit_list;
  mui_get_interactive_nodes_at_point(global_data->input_state->mouse.x, global_data->input_state->mouse.y,
                                     &node_hit_list);

  for (int a = 0; a < node_hit_list->count && !input_event.handled; ++a) {
    mc_node *node = node_hit_list->items[a];
    if (node->layout && node->layout->handle_input_event) {
      // TODO fptr casting
      void (*handle_input_event)(mc_node *, mci_input_event *) =
          (void (*)(mc_node *, mci_input_event *))node->layout->handle_input_event; // TODO add type of mouse event
      handle_input_event(node, &input_event);
    }
  }
}

void _mcc_set_button_state(bool is_down, bool is_event, int *output)
{
  if (is_down) {
    if (is_event)
      *output = (int)BUTTON_STATE_DOWN | BUTTON_STATE_PRESSED;
    else
      *output = (int)BUTTON_STATE_DOWN;
  }
  else {
    if (is_event)
      *output = (int)BUTTON_STATE_UP | BUTTON_STATE_RELEASED;
    else
      *output = (int)BUTTON_STATE_UP;
  }
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

  for (int xi_index = 0; xi_index < global_data->render_thread->input_buffer.event_count; ++xi_index) {
    // New Input Event
    window_input_event *xcb_input = &global_data->render_thread->input_buffer.events[xi_index];

    window_input_event_type event_type;
    switch (xcb_input->type) {
    case INPUT_EVENT_MOUSE_PRESS: {
      input_state->mouse.x = xcb_input->detail.mouse.x;
      input_state->mouse.y = xcb_input->detail.mouse.y;

      // Set input event for controls to handle
      switch (xcb_input->detail.mouse.button) {
      case MOUSE_BUTTON_LEFT: {
        _mcc_set_button_state(true, true, &input_state->mouse.left);
      } break;
      case MOUSE_BUTTON_RIGHT: {
        _mcc_set_button_state(true, true, &input_state->mouse.right);
      } break;
      default:
        // TODO
        break;
      }

      mcc_issue_input_event(xcb_input->type, xcb_input->detail.mouse.button);
    } break;
    case INPUT_EVENT_MOUSE_RELEASE: {
      input_state->mouse.x = xcb_input->detail.mouse.x;
      input_state->mouse.y = xcb_input->detail.mouse.y;

      // Set input event for controls to handle
      switch (xcb_input->detail.mouse.button) {
      case MOUSE_BUTTON_LEFT: {
        _mcc_set_button_state(false, true, &input_state->mouse.left);
      } break;
      case MOUSE_BUTTON_RIGHT: {
        _mcc_set_button_state(false, true, &input_state->mouse.right);
      } break;
      default:
        // TODO
        break;
      }

      mcc_issue_input_event(xcb_input->type, xcb_input->detail.mouse.button);
    } break;
    case INPUT_EVENT_FOCUS_IN:
    case INPUT_EVENT_FOCUS_OUT: {
      _mcc_set_button_state(false, (input_state->alt_function & BUTTON_STATE_DOWN), &input_state->alt_function);
      // printf("alt input_state %s\n", input_event->altDown ? "DOWN" : "UP");
    } break;
    case INPUT_EVENT_KEY_RELEASE:
    case INPUT_EVENT_KEY_PRESS: {
      switch (xcb_input->detail.keyboard.key) {
      case KEY_CODE_LEFT_ALT:
      case KEY_CODE_RIGHT_ALT:
        if (xcb_input->type == INPUT_EVENT_KEY_PRESS) {
          input_state->alt_function = (int)BUTTON_STATE_DOWN | BUTTON_STATE_PRESSED;
        }
        else if (xcb_input->type == INPUT_EVENT_KEY_RELEASE) {
          input_state->alt_function = BUTTON_STATE_UP | BUTTON_STATE_RELEASED;
        }
        break;
      case KEY_CODE_LEFT_CTRL:
      case KEY_CODE_RIGHT_CTRL:
        if (xcb_input->type == INPUT_EVENT_KEY_PRESS) {
          input_state->ctrl_function = BUTTON_STATE_DOWN | BUTTON_STATE_PRESSED;
        }
        else if (xcb_input->type == INPUT_EVENT_KEY_RELEASE) {
          input_state->ctrl_function = BUTTON_STATE_UP | BUTTON_STATE_RELEASED;
        }
        break;
      case KEY_CODE_LEFT_SHIFT:
      case KEY_CODE_RIGHT_SHIFT:
        if (xcb_input->type == INPUT_EVENT_KEY_PRESS) {
          input_state->shift_function = BUTTON_STATE_DOWN | BUTTON_STATE_PRESSED;
        }
        else if (xcb_input->type == INPUT_EVENT_KEY_RELEASE) {
          input_state->shift_function = BUTTON_STATE_UP | BUTTON_STATE_RELEASED;
        }
        break;

      default: {
        // Set input event for controls to handle
        // input_event->type = xcb_input->type;
        // input_event->detail = xcb_input->detail;

        if ((input_state->ctrl_function & BUTTON_STATE_DOWN) && (input_state->shift_function & BUTTON_STATE_DOWN) &&
            xcb_input->detail.keyboard.key == KEY_CODE_W) {
          global_data->exit_requested = true;
          continue;
        }

        if ((input_state->ctrl_function & BUTTON_STATE_DOWN) && (input_state->shift_function & BUTTON_STATE_DOWN) &&
            xcb_input->detail.keyboard.key == KEY_CODE_N) {

          // Lets only have one project at a time for the time being -- TODO
          bool visual_app_exists = false;
          for (int a = 0; a < global_data->global_node->children->count; ++a) {
            if (global_data->global_node->children->items[a]->type == NODE_TYPE_VISUAL_PROJECT) {
              visual_app_exists = true;
              break;
            }
          }

          if (!visual_app_exists)
            mca_create_new_visual_project("PushTheButton");
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
  input_state->alt_function &= ~BUTTON_STATE_PRESSED;
  input_state->alt_function &= ~BUTTON_STATE_RELEASED;
  input_state->ctrl_function &= ~BUTTON_STATE_PRESSED;
  input_state->ctrl_function &= ~BUTTON_STATE_RELEASED;
  input_state->shift_function &= ~BUTTON_STATE_PRESSED;
  input_state->shift_function &= ~BUTTON_STATE_RELEASED;
  global_data->input_state_requires_update = false;

  // Handle new input
  if (global_data->render_thread->input_buffer.event_count > 0) {
    mcc_handle_xcb_input();

    global_data->input_state_requires_update = true;
  }
}
