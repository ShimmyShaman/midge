/* mc_xcb.c */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <vulkan/vulkan.h>

#include "platform/mc_xcb.h"

// Using code modified from:  Niko Kauppi
//   -- And his contributers:
//      hodasemi (XCB validation)

int get_key_input_code_char(bool shift, mc_key_code code, char *c)
{
  switch (code) {
  case KEY_CODE_D1:
    *c = shift ? '!' : '1';
    return 0;
  case KEY_CODE_D2:
    *c = shift ? '@' : '2';
    return 0;
  case KEY_CODE_D3:
    *c = shift ? '#' : '3';
    return 0;
  case KEY_CODE_D4:
    *c = shift ? '$' : '4';
    return 0;
  case KEY_CODE_D5:
    *c = shift ? '%' : '5';
    return 0;
  case KEY_CODE_D6:
    *c = shift ? '^' : '6';
    return 0;
  case KEY_CODE_D7:
    *c = shift ? '&' : '7';
    return 0;
  case KEY_CODE_D8:
    *c = shift ? '*' : '8';
    return 0;
  case KEY_CODE_D9:
    *c = shift ? '(' : '9';
    return 0;
  case KEY_CODE_D0:
    *c = shift ? ')' : '0';
    return 0;
  case KEY_CODE_D_SUBTRACT:
    *c = shift ? '_' : '-';
    return 0;
  case KEY_CODE_D_EQUALS:
    *c = shift ? '+' : '=';
    return 0;
  case KEY_CODE_Q:
    *c = shift ? 'Q' : 'q';
    return 0;
  case KEY_CODE_W:
    *c = shift ? 'W' : 'w';
    return 0;
  case KEY_CODE_E:
    *c = shift ? 'E' : 'e';
    return 0;
  case KEY_CODE_R:
    *c = shift ? 'R' : 'r';
    return 0;
  case KEY_CODE_T:
    *c = shift ? 'T' : 't';
    return 0;
  case KEY_CODE_Y:
    *c = shift ? 'Y' : 'y';
    return 0;
  case KEY_CODE_U:
    *c = shift ? 'U' : 'u';
    return 0;
  case KEY_CODE_I:
    *c = shift ? 'I' : 'i';
    return 0;
  case KEY_CODE_O:
    *c = shift ? 'O' : 'o';
    return 0;
  case KEY_CODE_P:
    *c = shift ? 'P' : 'p';
    return 0;
  case KEY_CODE_SQUARE_OPEN_BRACKET:
    *c = shift ? '{' : '[';
    return 0;
  case KEY_CODE_SQUARE_CLOSE_BRACKET:
    *c = shift ? '}' : ']';
    return 0;
  case KEY_CODE_A:
    *c = shift ? 'A' : 'a';
    return 0;
  case KEY_CODE_S:
    *c = shift ? 'S' : 's';
    return 0;
  case KEY_CODE_D:
    *c = shift ? 'D' : 'd';
    return 0;
  case KEY_CODE_F:
    *c = shift ? 'F' : 'f';
    return 0;
  case KEY_CODE_G:
    *c = shift ? 'G' : 'g';
    return 0;
  case KEY_CODE_H:
    *c = shift ? 'H' : 'h';
    return 0;
  case KEY_CODE_J:
    *c = shift ? 'J' : 'j';
    return 0;
  case KEY_CODE_K:
    *c = shift ? 'K' : 'k';
    return 0;
  case KEY_CODE_L:
    *c = shift ? 'L' : 'l';
    return 0;
  case KEY_CODE_SEMI_COLON:
    *c = shift ? ':' : ';';
    return 0;
  case KEY_CODE_QUOTE:
    *c = shift ? '"' : '\'';
    return 0;
  case KEY_CODE_BACK_SLASH:
    *c = shift ? '|' : '\\';
    return 0;
  case KEY_CODE_Z:
    *c = shift ? 'Z' : 'z';
    return 0;
  case KEY_CODE_X:
    *c = shift ? 'X' : 'x';
    return 0;
  case KEY_CODE_C:
    *c = shift ? 'C' : 'c';
    return 0;
  case KEY_CODE_V:
    *c = shift ? 'V' : 'v';
    return 0;
  case KEY_CODE_B:
    *c = shift ? 'B' : 'b';
    return 0;
  case KEY_CODE_N:
    *c = shift ? 'N' : 'n';
    return 0;
  case KEY_CODE_M:
    *c = shift ? 'M' : 'm';
    return 0;
  case KEY_CODE_COMMA:
    *c = shift ? '<' : ',';
    return 0;
  case KEY_CODE_DECIMAL:
    *c = shift ? '>' : '.';
    return 0;
  case KEY_CODE_FORWARD_SLASH:
    *c = shift ? '?' : '/';
    return 0;
  case KEY_CODE_SPACE:
    *c = shift ? ' ' : ' ';
    return 0;
  case KEY_CODE_NUM_STAR:
    *c = '*';
    return 0;
  case KEY_CODE_NUM_SUBTRACT:
    *c = '-';
    return 0;
  case KEY_CODE_NUM_PLUS:
    *c = '+';
    return 0;
  case KEY_CODE_NUM_FORWARD_SLASH:
    *c = '/';
    return 0;
  case KEY_CODE_NUM0:
    *c = '0';
    return 0;
  case KEY_CODE_NUM1:
    *c = '1';
    return 0;
  case KEY_CODE_NUM2:
    *c = '2';
    return 0;
  case KEY_CODE_NUM3:
    *c = '3';
    return 0;
  case KEY_CODE_NUM4:
    *c = '4';
    return 0;
  case KEY_CODE_NUM5:
    *c = '5';
    return 0;
  case KEY_CODE_NUM6:
    *c = '6';
    return 0;
  case KEY_CODE_NUM7:
    *c = '7';
    return 0;
  case KEY_CODE_NUM8:
    *c = '8';
    return 0;
  case KEY_CODE_NUM9:
    *c = '9';
    return 0;
  case KEY_CODE_NUM_DECIMAL:
    *c = '.';
    return 0;
  case KEY_CODE_NUM_X_EQUALS:
    *c = '=';
    return 0;
  case KEY_CODE_NUM_X_OPEN_BRACKET:
    *c = '(';
    return 0;
  case KEY_CODE_NUM_X_CLOSE_BRACKET:
    *c = ')';
    return 0;

  default:
    return -1;
  }
}

int mxcb_init_window(mxcb_window_info *p_wnfo, int surfaceSizeX, int surfaceSizeY)
{
  // Create connection to X11 server
  const xcb_setup_t *setup = 0;

  xcb_screen_iterator_t iter;
  int screen = 0;

  p_wnfo->connection = xcb_connect(0, &screen);
  if (!p_wnfo->connection) {
    printf("Cannot find a compatible Vulkan ICD.\n");
    return -1;
  }

  setup = xcb_get_setup(p_wnfo->connection);
  iter = xcb_setup_roots_iterator(setup);
  while (screen-- > 0) {
    xcb_screen_next(&iter);
  }
  p_wnfo->screen = iter.data;

  // uint32_t _surface_size_x = 512;
  // uint32_t _surface_size_y = 512;
  // std::string _window_name;

  // Create window
  VkRect2D dimensions;
  dimensions.offset.x = 0;
  dimensions.offset.y = 0;
  dimensions.extent.width = surfaceSizeX;
  dimensions.extent.height = surfaceSizeY;

  MCassert(dimensions.extent.width > 0, "");
  MCassert(dimensions.extent.height > 0, "");

  uint32_t value_mask, value_list[32];

  p_wnfo->window = xcb_generate_id(p_wnfo->connection);

  value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  value_list[0] = p_wnfo->screen->black_pixel;
  value_list[1] = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS |
                  XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_EXPOSURE |
                  XCB_EVENT_MASK_FOCUS_CHANGE;

  // uint32_t mask = XCB_KB_AUTO_REPEAT_MODE;
  // uint32_t values[] = {XCB_AUTO_REPEAT_MODE_ON};
  // xcb_change_keyboard_control(p_wnfo->connection, mask, values);

  xcb_create_window(p_wnfo->connection, XCB_COPY_FROM_PARENT, p_wnfo->window, p_wnfo->screen->root, dimensions.offset.x,
                    dimensions.offset.y, dimensions.extent.width, dimensions.extent.height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, p_wnfo->screen->root_visual, value_mask, value_list);

  int xce = xcb_connection_has_error(p_wnfo->connection);
  if (xce) {
    printf("XCB_CONNECTION_ERROR:A:%i\n", xce);
    return xce;
  }

  /* Magic code that will send notification when window is destroyed */
  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(p_wnfo->connection, 1, 12, "WM_PROTOCOLS");
  xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(p_wnfo->connection, cookie, 0);

  xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(p_wnfo->connection, 0, 16, "WM_DELETE_WINDOW");
  p_wnfo->atom_window_reply = xcb_intern_atom_reply(p_wnfo->connection, cookie2, 0);
  // if (p_wnfo->atom_window_reply)
  //   printf("The _NET_WM_NAME atom has ID %u\n", p_wnfo->atom_window_reply->atom);
  // else
  //   printf("The _NET_WM_NAME atom has NO ID\n");

  xcb_change_property(p_wnfo->connection, XCB_PROP_MODE_REPLACE, p_wnfo->window, (*reply).atom, 4, 32, 1,
                      &(*p_wnfo->atom_window_reply).atom);
  free(reply);

  xcb_map_window(p_wnfo->connection, p_wnfo->window);

  // Force the x/y coordinates to 100,100 results are identical in consecutive runs
  const uint32_t coords[] = {100, 100};
  xcb_configure_window(p_wnfo->connection, p_wnfo->window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
  xcb_flush(p_wnfo->connection);

  /*
    xcb_generic_event_t *e;
    while( ( e = xcb_wait_for_event( p_wnfo->xcb_connection ) ) ) {
        if( ( e->response_type & ~0x80 ) == XCB_EXPOSE )
                break;
    }
    */

  xce = xcb_connection_has_error(p_wnfo->connection);
  if (xce) {
    printf("XCB_CONNECTION_ERROR:B:%i\n", xce);
    return xce;
  }

  return 0;
}

int mxcb_update_window(mxcb_window_info *p_wnfo, window_input_buffer *input_buffer)
{
  xcb_generic_event_t *event;
  while (true) {
    // printf("mxcb_update_window:mxcb_window_info *=%p", p_wnfo);
    // int xce = xcb_connection_has_error(p_wnfo->connection);
    // if (xce) {
    //   printf("XCB_CONNECTION_ERROR:U:%i\n", xce);
    //   return xce;
    // }
    event = xcb_poll_for_event(p_wnfo->connection);

    // if there is no event, event will be NULL
    // need to check for event == NULL to prevent segfault
    if (!event)
      return 0;

  // XCB Auto-Repeat falloff
  xcb_auto_repeat_loc:
    // printf("xcb_full_sequence:%u\n", event->full_sequence);
    // printf("xcb_pad:%u,%u,%u,%u,%u,%u,%u\n", event->pad[0], event->pad[1], event->pad[2], event->pad[3],
    // event->pad[4],
    //        event->pad[5], event->pad[6]);
    // printf("xcb_sequence:%u\n", event->sequence);
    // printf("xcb_response_type:%u\n", event->response_type);

    switch (event->response_type & ~0x80) {
    case XCB_CLIENT_MESSAGE:
      // for (int i = 0; i < 10; ++i)
      //   printf("ERROR XCB_CLIENT_MESSAGE!!!!!\n");
      if (((xcb_client_message_event_t *)event)->data.data32[0] == p_wnfo->atom_window_reply->atom) {
        p_wnfo->input_requests_exit = 1;
      }
      break;
    case XCB_EXPOSE: {
      // Nothing?
    } break;
    case XCB_FOCUS_IN: {
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count < MAX_QUEUED_KEY_EVENTS) {
        input_buffer->events[input_buffer->event_count++].type = INPUT_EVENT_FOCUS_IN;
      }
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_FOCUS_OUT: {
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count < MAX_QUEUED_KEY_EVENTS) {
        input_buffer->events[input_buffer->event_count++].type = INPUT_EVENT_FOCUS_OUT;
      }
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_KEY_PRESS: {
      pthread_mutex_lock(&input_buffer->mutex);
      // xcb_button_press_event_t *rev = (xcb_button_press_event_t *)event;
      // printf("XCB_KEY_PRESS->time:%u\n", rev->time);

      if (input_buffer->event_count < MAX_QUEUED_KEY_EVENTS) {
        input_buffer->events[input_buffer->event_count].type = INPUT_EVENT_KEY_PRESS;
        input_buffer->events[input_buffer->event_count++].detail.keyboard.key = (mc_key_code)event->pad0;
      }
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_KEY_RELEASE: {
      // Yes, it's pretty bad. I don't know what else to do.
      // Immediate polling after sometimes doesn't return the consecutive event despite time being listed the same.
      usleep(1);

      xcb_button_release_event_t *rev = (xcb_button_release_event_t *)event;
      xcb_button_press_event_t *nev = (xcb_button_press_event_t *)xcb_poll_for_event(p_wnfo->connection);

      if (nev && (nev->response_type & ~0x80) == XCB_KEY_PRESS && nev->time == rev->time && nev->pad0 == rev->pad0) {
        // Ignore auto-repeat events
        // puts("ignored auto-repeat");
        continue;
      }

      // if (nev) {
      //   printf("nev exists %u<>%u\n", nev->time, rev->time);
      // }
      // printf("XCB_KEY_RELEASE->time:%u\n", rev->time);

      pthread_mutex_lock(&input_buffer->mutex);

      if (input_buffer->event_count < MAX_QUEUED_KEY_EVENTS) {
        input_buffer->events[input_buffer->event_count].type = INPUT_EVENT_KEY_RELEASE;
        input_buffer->events[input_buffer->event_count++].detail.keyboard.key = (mc_key_code)event->pad0;
      }
      pthread_mutex_unlock(&input_buffer->mutex);

      if (nev) {
        event = (xcb_generic_event_t *)nev;
        goto xcb_auto_repeat_loc;
      }
    } break;
    case XCB_BUTTON_PRESS: {
      xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count < MAX_QUEUED_KEY_EVENTS) {
        input_buffer->events[input_buffer->event_count].type = INPUT_EVENT_MOUSE_PRESS;
        input_buffer->events[input_buffer->event_count].detail.mouse.button = (mc_mouse_button_code)press->detail;
        input_buffer->events[input_buffer->event_count].detail.mouse.x = press->event_x;
        input_buffer->events[input_buffer->event_count++].detail.mouse.y = press->event_y;
      }
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_BUTTON_RELEASE: {
      xcb_button_release_event_t *release = (xcb_button_release_event_t *)event;
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count < MAX_QUEUED_KEY_EVENTS) {
        input_buffer->events[input_buffer->event_count].type = INPUT_EVENT_MOUSE_RELEASE;
        input_buffer->events[input_buffer->event_count].detail.mouse.button = (mc_mouse_button_code)release->detail;
        input_buffer->events[input_buffer->event_count].detail.mouse.x = release->event_x;
        input_buffer->events[input_buffer->event_count++].detail.mouse.y = release->event_y;
      }
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_MOTION_NOTIFY: {
      xcb_motion_notify_event_t *motion = (xcb_motion_notify_event_t *)event;
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count < MAX_QUEUED_KEY_EVENTS) {
        input_buffer->events[input_buffer->event_count].type = INPUT_EVENT_MOUSE_MOVE;
        input_buffer->events[input_buffer->event_count].detail.mouse.button = MOUSE_BUTTON_NONE;
        input_buffer->events[input_buffer->event_count].detail.mouse.x = motion->event_x;
        input_buffer->events[input_buffer->event_count++].detail.mouse.y = motion->event_y;
      }
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    default:
      printf("unhandled xcb_response_type:%u\n", event->response_type);
      break;
    }
    free(event);
  }

  return 0;
}

void mxcb_destroy_window(mxcb_window_info *p_wnfo)
{
  // free(p_wnfo->atom_window_reply);

  xcb_destroy_window(p_wnfo->connection, p_wnfo->window);
  xcb_disconnect(p_wnfo->connection);
  p_wnfo->window = 0;
  p_wnfo->connection = NULL;
}