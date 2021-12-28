#ifndef MC_XCB_H
#define MC_XCB_H

#include "xcb/xcb.h"

// #include "m_threads.h"
#include <stdbool.h>

#define APPLICATION_SET_WIDTH 1760
#define APPLICATION_SET_HEIGHT 840

#ifndef max
#define max(x, y) ((x) < (y) ? (y) : (x))
#endif
#ifndef min
#define min(x, y) ((x) > (y) ? (y) : (x))
#endif

#define MAX_QUEUED_KEY_EVENTS 128
typedef enum window_input_event_type {
  INPUT_EVENT_NONE = 0,
  INPUT_EVENT_FOCUS_IN,
  INPUT_EVENT_FOCUS_OUT,
  INPUT_EVENT_KEY_PRESS,
  INPUT_EVENT_KEY_RELEASE,
  INPUT_EVENT_MOUSE_PRESS,
  INPUT_EVENT_MOUSE_RELEASE,
  INPUT_EVENT_MOUSE_MOVE,
  INPUT_EVENT_PROGRAMMATIC_INVOCATION,
} window_input_event_type;

typedef enum mc_key_code {
  KEY_CODE_NONE = 0,

  // From XCB codes I THINK... Poor way of doing it
  KEY_CODE_ESCAPE = 9,
  KEY_CODE_D1 = 10,
  KEY_CODE_D2 = 11,
  KEY_CODE_D3 = 12,
  KEY_CODE_D4 = 13,
  KEY_CODE_D5 = 14,
  KEY_CODE_D6 = 15,
  KEY_CODE_D7 = 16,
  KEY_CODE_D8 = 17,
  KEY_CODE_D9 = 18,
  KEY_CODE_D0 = 19,
  KEY_CODE_D_SUBTRACT = 20,
  KEY_CODE_D_EQUALS = 21,
  KEY_CODE_BACKSPACE = 22,
  KEY_CODE_TAB = 23,
  KEY_CODE_Q = 24,
  KEY_CODE_W = 25,
  KEY_CODE_E = 26,
  KEY_CODE_R = 27,
  KEY_CODE_T = 28,
  KEY_CODE_Y = 29,
  KEY_CODE_U = 30,
  KEY_CODE_I = 31,
  KEY_CODE_O = 32,
  KEY_CODE_P = 33,
  KEY_CODE_SQUARE_OPEN_BRACKET = 34,
  KEY_CODE_SQUARE_CLOSE_BRACKET = 35,
  KEY_CODE_ENTER = 36,
  KEY_CODE_LEFT_CTRL = 37,
  KEY_CODE_A = 38,
  KEY_CODE_S = 39,
  KEY_CODE_D = 40,
  KEY_CODE_F = 41,
  KEY_CODE_G = 42,
  KEY_CODE_H = 43,
  KEY_CODE_J = 44,
  KEY_CODE_K = 45,
  KEY_CODE_L = 46,
  KEY_CODE_SEMI_COLON = 47,
  KEY_CODE_QUOTE = 48,
  KEY_CODE_LEFT_SHIFT = 50,
  KEY_CODE_BACK_SLASH = 51,
  KEY_CODE_Z = 52,
  KEY_CODE_X = 53,
  KEY_CODE_C = 54,
  KEY_CODE_V = 55,
  KEY_CODE_B = 56,
  KEY_CODE_N = 57,
  KEY_CODE_M = 58,
  KEY_CODE_COMMA = 59,
  KEY_CODE_DECIMAL = 60,
  KEY_CODE_FORWARD_SLASH = 61,
  KEY_CODE_RIGHT_SHIFT = 62,
  KEY_CODE_LEFT_ALT = 64,
  KEY_CODE_SPACE = 65,
  KEY_CODE_NUM_STAR = 63,
  KEY_CODE_F1 = 67,
  KEY_CODE_F2 = 68,
  KEY_CODE_F3 = 69,
  KEY_CODE_F4 = 70,
  KEY_CODE_F5 = 71,
  KEY_CODE_F6 = 72,
  KEY_CODE_F7 = 73,
  KEY_CODE_F8 = 74,
  KEY_CODE_F9 = 75,
  KEY_CODE_F10 = 76,
  KEY_CODE_F11 = 95,
  KEY_CODE_F12 = 96,
  KEY_CODE_NUM_SUBTRACT = 82,
  KEY_CODE_NUM_PLUS = 86,
  KEY_CODE_NUM_FORWARD_SLASH = 106,
  KEY_CODE_NUM0 = 90,
  KEY_CODE_NUM1 = 87,
  KEY_CODE_NUM2 = 88,
  KEY_CODE_NUM3 = 89,
  KEY_CODE_NUM4 = 83,
  KEY_CODE_NUM5 = 84,
  KEY_CODE_NUM6 = 85,
  KEY_CODE_NUM7 = 79,
  KEY_CODE_NUM8 = 80,
  KEY_CODE_NUM9 = 81,
  KEY_CODE_NUM_DECIMAL = 91,
  KEY_CODE_RETURN = 104,
  KEY_CODE_RIGHT_CTRL = 105,
  KEY_CODE_RIGHT_ALT = 108,
  KEY_CODE_HOME = 110,
  KEY_CODE_END = 115,
  KEY_CODE_ARROW_UP = 111,
  KEY_CODE_ARROW_DOWN = 116,
  KEY_CODE_ARROW_LEFT = 113,
  KEY_CODE_ARROW_RIGHT = 114,
  KEY_CODE_DELETE = 119,
  KEY_CODE_NUM_X_EQUALS = 125,
  KEY_CODE_NUM_X_OPEN_BRACKET = 187,
  KEY_CODE_NUM_X_CLOSE_BRACKET = 188,
} mc_key_code;

typedef enum mc_mouse_button_code {
  MOUSE_BUTTON_NONE = 0,
  MOUSE_BUTTON_LEFT = 1,
  MOUSE_BUTTON_MIDDLE = 2,
  MOUSE_BUTTON_RIGHT = 3,
  MOUSE_BUTTON_SCROLL_UP = 4,
  MOUSE_BUTTON_SCROLL_DOWN = 5,
  MOUSE_BUTTON_PREVIOUS = 8,
  MOUSE_BUTTON_NEXT = 9,
} mc_mouse_button_code;

typedef union window_input_event_detail {
  struct {
    mc_key_code key;
  } keyboard;
  struct {
    mc_mouse_button_code button;
    uint x, y;
  } mouse;
} window_input_event_detail;

typedef struct window_input_event {
  window_input_event_type type;
  window_input_event_detail detail;
} window_input_event;

typedef struct window_input_buffer {
  pthread_mutex_t mutex;
  window_input_event events[MAX_QUEUED_KEY_EVENTS];
  uint event_count;
} window_input_buffer;

typedef struct mxcb_window_info {
  xcb_connection_t *connection;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_intern_atom_reply_t *atom_window_reply;
  bool input_requests_exit;

  uint32_t width, height;
  struct {
    void *state;
    int (*delegate)(struct mxcb_window_info *wnfo, uint32_t prev_width, uint32_t prev_height, void *callback_state);
  } window_resized_callback;
} mxcb_window_info;

// TODO -- naming / correct file placement
int get_key_input_code_char(bool shift, mc_key_code code, char *c);

int mxcb_init_window(mxcb_window_info *mcxbWindowInfo, int surfaceSizeX, int surfaceSizeY);

int mxcb_update_window(mxcb_window_info *p_wnfo, window_input_buffer *input_buffer);

void mxcb_destroy_window(mxcb_window_info *mcxbWindowInfo);
#endif // MC_XCB_H
