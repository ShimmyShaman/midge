/* window.h */

#ifndef XCB_WINDOW_H
#define XCB_WINDOW_H

#include "midge_common.h"
#include "rendering/mvk_core.h"

typedef struct mxcb_window_info {
  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_intern_atom_reply_t *atom_window_reply;
  bool shouldExit;
} mxcb_window_info;

int mxcb_init_window(mxcb_window_info *mcxbWindowInfo, int surfaceSizeX, int surfaceSizeY);

int mxcb_update_window(mxcb_window_info *p_wnfo, window_input_buffer *input_buffer);

void mxcb_destroy_window(mxcb_window_info *mcxbWindowInfo);

#endif // XCB_WINDOW_H