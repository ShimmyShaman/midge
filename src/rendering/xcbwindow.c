/* window.c */

/* -----------------------------------------------------
This source code is public domain ( CC0 )
The code is provided as-is without limitations, requirements and responsibilities.
Creators and contributors to this source code are provided as a token of appreciation
and no one associated with this source code can be held responsible for any possible
damages or losses of any kind.

Original file creator:  Niko Kauppi (Code maintenance)
Contributors:
hodasemi (XCB validation)
----------------------------------------------------- */

// #include "BUILD_OPTIONS.h"
// #include "Platform.h"

// #include "Window.h"
// #include "Shared.h"
// #include "Renderer.h"

// #include <assert.h>
// #include <iostream>

// #if VK_USE_PLATFORM_XCB_KHR

#include "rendering/xcbwindow.h"
#include <stdio.h>
#include <vulkan/vulkan_xcb.h>

int mxcb_init_window(mxcb_window_info *p_wnfo, int surfaceSizeX, int surfaceSizeY)
{
  // create connection to X11 server
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

  // 	uint32_t							_surface_size_x = 512; 	uint32_t
  // _surface_size_y					= 512; 	std::string
  // _window_name;

  // create window
  VkRect2D dimensions;
  dimensions.offset.x = 0;
  dimensions.offset.y = 0;
  dimensions.extent.width = surfaceSizeX;
  dimensions.extent.height = surfaceSizeY;

  assert(dimensions.extent.width > 0);
  assert(dimensions.extent.height > 0);

  uint32_t value_mask, value_list[32];

  p_wnfo->window = xcb_generate_id(p_wnfo->connection);

  value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  value_list[0] = p_wnfo->screen->black_pixel;
  value_list[1] = XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS |
                  XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_FOCUS_CHANGE;

  // uint32_t mask = XCB_KB_AUTO_REPEAT_MODE;
  // uint32_t values[] = {XCB_AUTO_REPEAT_MODE_ON};
  // xcb_change_keyboard_control(p_wnfo->connection, mask, values);

  xcb_create_window(p_wnfo->connection, XCB_COPY_FROM_PARENT, p_wnfo->window, p_wnfo->screen->root, dimensions.offset.x,
                    dimensions.offset.y, dimensions.extent.width, dimensions.extent.height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, p_wnfo->screen->root_visual, value_mask, value_list);

  /* Magic code that will send notification when window is destroyed */
  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(p_wnfo->connection, 1, 12, "WM_PROTOCOLS");
  xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(p_wnfo->connection, cookie, 0);

  xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(p_wnfo->connection, 0, 16, "WM_DELETE_WINDOW");
  p_wnfo->atom_window_reply = xcb_intern_atom_reply(p_wnfo->connection, cookie2, 0);

  xcb_change_property(p_wnfo->connection, XCB_PROP_MODE_REPLACE, p_wnfo->window, (*reply).atom, 4, 32, 1,
                      &(*p_wnfo->atom_window_reply).atom);
  free(reply);

  xcb_map_window(p_wnfo->connection, p_wnfo->window);

  // Force the x/y coordinates to 100,100 results are identical in consecutive
  // runs
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

  return 0;
}

// int initOSSurface(mxcb_window_info *p_wnfo, VkInstance vulkanInstance, VkSurfaceKHR *surface)
// {
//   VkXcbSurfaceCreateInfoKHR create_info;
//   create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
//   create_info.connection = p_wnfo->xcb_connection;
//   create_info.window = p_wnfo->xcb_window;

//   if (vkCreateXcbSurfaceKHR(vulkanInstance, &create_info, NULL, surface) != VK_SUCCESS)
//   {
//     printf("error54252");
//     return -1;
//   }
//   return 0;
// }

int mxcb_update_window(mxcb_window_info *p_wnfo, window_input_buffer *input_buffer)
{
  while (true) {
    xcb_generic_event_t *event = xcb_poll_for_event(p_wnfo->connection);

    // if there is no event, event will be NULL
    // need to check for event == NULL to prevent segfault
    if (!event)
      return 0;

    // printf("xcb_full_sequence:%u\n", event->full_sequence);
    // printf("xcb_pad:%u,%u,%u,%u,%u,%u,%u\n", event->pad[0], event->pad[1], event->pad[2], event->pad[3],
    // event->pad[4],
    //        event->pad[5], event->pad[6]);
    // printf("xcb_sequence:%u\n", event->sequence);

    switch (event->response_type & ~0x80) {
    case XCB_CLIENT_MESSAGE:
      if (((xcb_client_message_event_t *)event)->data.data32[0] == p_wnfo->atom_window_reply->atom) {
        p_wnfo->shouldExit = 1;
      }
      break;
    case XCB_EXPOSE: {
      // Nothing?
    } break;
    case XCB_FOCUS_IN: {
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count >= MAX_QUEUED_KEY_EVENTS)
        break;
      input_buffer->events[input_buffer->event_count++].type = INPUT_EVENT_FOCUS_IN;
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_FOCUS_OUT: {
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count >= MAX_QUEUED_KEY_EVENTS)
        break;
      input_buffer->events[input_buffer->event_count++].type = INPUT_EVENT_FOCUS_OUT;
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_KEY_PRESS: {
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count >= MAX_QUEUED_KEY_EVENTS)
        break;
      input_buffer->events[input_buffer->event_count].type = INPUT_EVENT_KEY_PRESS;
      input_buffer->events[input_buffer->event_count++].detail.keyboard.key = (mc_key_code)event->pad0;
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_KEY_RELEASE: {
      pthread_mutex_lock(&input_buffer->mutex);

      // xcb_key_press_event_t *kpet = (xcb_key_press_event_t *)event;
      // printf("KeyRelease:\n"
      //        " -- response_type:%u  detail:%i  sequence:%u  time:%u\n"
      //        " -- root:%u  event:%u  child:%u  root_x:%i  root_y:%i\n"
      //        " -- event_x:%i  event_y:%i  state:%u  same_screen:%u\n"
      //        " -- pad0:%u\n",
      //        kpet->response_type, kpet->detail, kpet->sequence, kpet->time, kpet->root, event, kpet->child,
      //        kpet->root_x, kpet->root_y, kpet->event_x, kpet->event_y, kpet->state, kpet->same_screen, kpet->pad0);

      if (input_buffer->event_count >= MAX_QUEUED_KEY_EVENTS)
        break;
      input_buffer->events[input_buffer->event_count].type = INPUT_EVENT_KEY_RELEASE;
      input_buffer->events[input_buffer->event_count++].detail.keyboard.key = (mc_key_code)event->pad0;
      pthread_mutex_unlock(&input_buffer->mutex);
    } break;
    case XCB_BUTTON_PRESS: {
      xcb_button_press_event_t *press = (xcb_button_press_event_t *)event;
      pthread_mutex_lock(&input_buffer->mutex);
      if (input_buffer->event_count >= MAX_QUEUED_KEY_EVENTS)
        break;
      input_buffer->events[input_buffer->event_count].type = INPUT_EVENT_MOUSE_PRESS;
      input_buffer->events[input_buffer->event_count].detail.mouse.button = (mc_mouse_button_code)press->detail;
      input_buffer->events[input_buffer->event_count].detail.mouse.x = press->event_x;
      input_buffer->events[input_buffer->event_count++].detail.mouse.y = press->event_y;
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
  xcb_destroy_window(p_wnfo->connection, p_wnfo->window);
  xcb_disconnect(p_wnfo->connection);
  p_wnfo->window = 0;
  p_wnfo->connection = NULL;
}

// void deInitOSSurface(VkInstance vulkanInstance, VkSurfaceKHR *surface)
// {
//   vkDestroySurfaceKHR(vulkanInstance, *surface, NULL);
//   surface = NULL;
// }

// #endif