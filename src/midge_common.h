/* midge_common.h */

#ifndef MIDGE_COMMON_H
#define MIDGE_COMMON_H

#include <stdarg.h>
#include <stdio.h>

#include "m_threads.h"

#define APPLICATION_SET_WIDTH 1540
#define APPLICATION_SET_HEIGHT 900

#ifndef max
#define max(x, y) x < y ? y : x
#endif
#ifndef min
#define min(x, y) x > y ? y : x
#endif

#define MCcall(function)                                           \
  {                                                                \
    int mc_res = function;                                         \
    if (mc_res) {                                                  \
      printf("--" #function "line:%i:ERR:%i\n", __LINE__, mc_res); \
      return mc_res;                                               \
    }                                                              \
  }

#define MCvacall(function)                                            \
  {                                                                   \
    int mc_res = function;                                            \
    if (mc_res) {                                                     \
      printf("-- line:%d varg-function-call:%i\n", __LINE__, mc_res); \
      return mc_res;                                                  \
    }                                                                 \
  }

#define MCerror(error_code, error_message, ...)                          \
  printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
  return error_code;

#define COLOR_TRANSPARENT \
  (render_color) { 0.0f, 0.0f, 0.0f, 0.0f }
#define COLOR_CORNFLOWER_BLUE \
  (render_color) { 0.19f, 0.34f, 0.83f, 1.f }
#define COLOR_GREEN \
  (render_color) { 0.0f, 0.5f, 0.0f, 1.f }
#define COLOR_LIME \
  (render_color) { 0.0f, 1.f, 0.0f, 1.f }
#define COLOR_FUNCTION_GREEN \
  (render_color) { 40.f / 255.f, 235.f / 255.f, 40.f / 255.f, 1.f }
#define COLOR_POWDER_BLUE \
  (render_color) { 176.f / 255.f, 224.f / 255.f, 230.f / 255.f, 1.f }
#define COLOR_LIGHT_SKY_BLUE \
  (render_color) { 135.f / 255.f, 206.f / 255.f, 255.f / 250.f, 1.f }
#define COLOR_TEAL \
  (render_color) { 0.0f, 0.52f, 0.52f, 1.f }
#define COLOR_PURPLE \
  (render_color) { 160.f / 255.f, 32.f / 255.f, 240.f / 255.f, 1.f }
#define COLOR_BURLY_WOOD \
  (render_color) { 0.87f, 0.72f, 0.52f, 1.f }
#define COLOR_DARK_SLATE_GRAY \
  (render_color) { 0.18f, 0.18f, 0.31f, 1.f }
#define COLOR_NEARLY_BLACK \
  (render_color) { 0.13f, 0.13f, 0.13f, 1.f }
#define COLOR_GHOST_WHITE \
  (render_color) { 0.97f, 0.97f, 1.f, 1.f }
#define COLOR_BLACK \
  (render_color) { 0.f, 0.f, 0.f, 1.f }
#define COLOR_YELLOW \
  (render_color) { 1.f, 1.f, 0.f, 1.f }
#define COLOR_LIGHT_YELLOW \
  (render_color) { 1.f, 1.f, 102.f / 255.f, 1.f }

#define MAX_QUEUED_KEY_EVENTS 128
typedef enum window_input_event_type {
  INPUT_EVENT_NONE = 0,
  INPUT_EVENT_FOCUS_IN,
  INPUT_EVENT_FOCUS_OUT,
  INPUT_EVENT_KEY_PRESS,
  INPUT_EVENT_KEY_RELEASE,
  INPUT_EVENT_MOUSE_PRESS,
  INPUT_EVENT_MOUSE_RELEASE,
} window_input_event_type;

typedef enum key_event_code {
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
  KEY_CODE_NUM_X_EQUALS = 125,
  KEY_CODE_NUM_X_OPEN_BRACKET = 187,
  KEY_CODE_NUM_X_CLOSE_BRACKET = 188,
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
} key_event_code;

typedef enum mouse_event_code {
  MOUSE_BUTTON_NONE = 0,
  MOUSE_BUTTON_LEFT = 1,
  MOUSE_BUTTON_MIDDLE = 2,
  MOUSE_BUTTON_RIGHT = 3,
  MOUSE_BUTTON_SCROLL_UP = 4,
  MOUSE_BUTTON_SCROLL_DOWN = 5,
  MOUSE_BUTTON_PREVIOUS = 8,
  MOUSE_BUTTON_NEXT = 9,
} mouse_event_code;

typedef union window_input_event_detail {
  struct {
    key_event_code key;
  } keyboard;
  struct {
    mouse_event_code button;
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

typedef struct frame_time {
  long frame_secs, frame_nsecs;
  long app_secs, app_nsecs;
} frame_time;

int get_key_input_code_char(bool shift, key_event_code code, char *c);

typedef struct c_str {
  uint alloc;
  uint len;
  char *text;
} c_str;
int init_c_str(c_str **ptr);
int release_c_str(c_str *ptr, bool free_char_string_also);
int append_to_c_str(c_str *cstr, const char *text);
int append_to_c_strn(c_str *cstr, const char *text, int n);
int append_to_c_strf(c_str *cstr, const char *format, ...);

int get_key_input_code_char(bool shift, key_event_code code, char *c)
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

int init_c_str(c_str **ptr)
{
  (*ptr) = (c_str *)malloc(sizeof(c_str));
  (*ptr)->alloc = 2;
  (*ptr)->len = 0;
  (*ptr)->text = (char *)malloc(sizeof(char) * (*ptr)->alloc);
  (*ptr)->text[0] = '\0';

  return 0;
}

int set_c_str(c_str *cstr, const char *src)
{
  cstr->len = 0;
  cstr->text[0] = '\0';
  append_to_c_str(cstr, src);

  return 0;
}

int release_c_str(c_str *ptr, bool free_char_string_also)
{
  if (ptr->alloc > 0 && free_char_string_also && ptr->text) {
    free(ptr->text);
  }

  free(ptr);

  return 0;
}

int append_char_to_c_str(c_str *cstr, char c)
{
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  MCcall(append_to_c_str(cstr, buf));
  return 0;
}

int append_to_c_str(c_str *cstr, const char *text)
{
  int len = strlen(text);
  if (cstr->len + len + 1 >= cstr->alloc) {
    unsigned int new_allocated_size = cstr->alloc + len + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  // printf("atcs-a cstrtext:'%s' len:%u end:'%c'\n", cstr->text, cstr->len, cstr->text[cstr->len]);
  // printf("atcs-b text:'%s'\n", text);
  // memcpy(cstr->text + cstr->len, text, sizeof(char) * len);
  strcpy(cstr->text + cstr->len, text);
  // printf("atcs-c cstrtext:'%s' len:%u end:'%c'\n", cstr->text + cstr->len - 2, cstr->len, cstr->text[cstr->len]);
  cstr->len += len;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int append_to_c_strn(c_str *cstr, const char *text, int n)
{
  if (cstr->len + n + 1 >= cstr->alloc) {
    unsigned int new_allocated_size = cstr->alloc + n + 16 + (cstr->alloc) / 10;
    // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
    // printf("atc-5\n");
    free(cstr->text);
    // printf("atc-6\n");
    cstr->text = newptr;
    // printf("atc-7\n");
    cstr->alloc = new_allocated_size;
    // printf("atc-8\n");
  }

  strncpy(cstr->text + cstr->len, text, n);
  cstr->len += n;
  cstr->text[cstr->len] = '\0';

  return 0;
}

int append_to_c_strf(c_str *cstr, const char *format, ...)
{
  // printf("atcs-0\n");
  int chunk_size = 4;
  int i = 0;

  va_list valist;
  va_start(valist, format);

  // printf("atcs-1\n");
  while (1) {
    if (cstr->len + chunk_size + 1 >= cstr->alloc) {
      unsigned int new_allocated_size = chunk_size + cstr->alloc + 16 + (chunk_size + cstr->alloc) / 10;
      // printf("atc-3 : len:%u new_allocated_size:%u\n", cstr->len, new_allocated_size);
      char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
      // printf("atc-4\n");
      memcpy(newptr, cstr->text, sizeof(char) * cstr->alloc);
      // printf("atc-5\n");
      free(cstr->text);
      // printf("atc-6\n");
      cstr->text = newptr;
      // printf("atc-7\n");
      cstr->alloc = new_allocated_size;
      // printf("atc-8\n");
    }
    // printf("atcs-2\n");
    // sleep(1);

    // printf("'%c' chunk_size=%i cstr->len=%u\n", format[i], chunk_size, cstr->len);
    for (int a = 0; a < chunk_size; ++a) {
      cstr->text[cstr->len++] = format[i];
      // cstr->text[cstr->len] = '\0';
      // printf("cstr:'%s'\n", cstr->text);

      if (format[i] == '\0') {
        // printf("atcs-3\n");
        --cstr->len;
        va_end(valist);
        return 0;
      }

      if (format[i] == '%') {
        if (format[i + 1] == '%') {
          // printf("atcs-4\n");
          // Use as an escape character
          ++i;
        }
        else {
          // printf("atcs-5 i:%i i:'%c'\n", i, format[i]);
          --cstr->len;
          // Search to replace the format
          ++i;
          switch (format[i]) {
          case 'i': {
            int value = va_arg(valist, int);

            char buf[18];
            sprintf(buf, "%i", value);
            append_to_c_str(cstr, buf);
          } break;
          case 'l': {
            switch (format[i + 1]) {
            case 'i': {
              ++i;
              long int value = va_arg(valist, long int);

              char buf[24];
              sprintf(buf, "%li", value);
              append_to_c_str(cstr, buf);
            } break;
            default:
              MCerror(99, "TODO:l'%c'", format[i + 1]);
            }
          } break;
          case 'p': {
            void *value = va_arg(valist, void *);

            char buf[18];
            sprintf(buf, "%p", value);
            append_to_c_str(cstr, buf);
          } break;
          case 's': {
            char *value = va_arg(valist, char *);
            // printf("atcs-7a cstrtext:'%s' len:%u end:'%c'\n", cstr->text, cstr->len, cstr->text[cstr->len]);
            // printf("atcs-7a value:'%s'\n", value);
            // cstr->text[i] = '\0';
            // --cstr->len;
            append_to_c_str(cstr, value);
            // printf("atcs-7b cstrtext:'%s'\n", cstr->text);
          } break;
          case 'u': {
            unsigned int value = va_arg(valist, unsigned int);

            char buf[18];
            sprintf(buf, "%u", value);
            append_to_c_str(cstr, buf);
          } break;
          default: {
            MCerror(99, "TODO:%c", format[i]);
          }
          }
        }
        ++i;

        // Chunk size is unreliable -- reset
        break;
      }

      ++i;
      // printf("atcs-8 i:%i i:'%c'\n", i, format[i]);
    }

    // printf("atcs-6\n");
    chunk_size = (chunk_size * 5) / 3;
  }
}
#endif // MIDGE_COMMON_H