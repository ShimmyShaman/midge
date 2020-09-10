
/* render_thread.h */

#ifndef MC_RENDER_COMMON_H
#define MC_RENDER_COMMON_H

#include "midge_common.h"
#include "platform/mc_xcb.h"

typedef struct render_color {
  float r, g, b, a;
} render_color;

#define COLOR_TRANSPARENT \
  /*(render_color)*/ { 0.0f, 0.0f, 0.0f, 0.0f }
#define COLOR_CORNFLOWER_BLUE \
  /*(render_color)*/ { 0.19f, 0.34f, 0.83f, 1.f }
#define COLOR_DARK_GREEN \
  /*(render_color)*/ { 0.0f, 0.25f, 0.0f, 1.f }
#define COLOR_GREEN \
  /*(render_color)*/ { 0.0f, 0.5f, 0.0f, 1.f }
#define COLOR_LIME \
  /*(render_color)*/ { 0.0f, 1.f, 0.0f, 1.f }
#define COLOR_FUNCTION_GREEN \
  /*(render_color)*/ { 40.f / 255.f, 235.f / 255.f, 40.f / 255.f, 1.f }
#define COLOR_FUNCTION_RED \
  /*(render_color)*/ { 215.f / 255.f, 195.f / 255.f, 40.f / 255.f, 1.f }
#define COLOR_POWDER_BLUE \
  /*(render_color)*/ { 176.f / 255.f, 224.f / 255.f, 230.f / 255.f, 1.f }
#define COLOR_LIGHT_SKY_BLUE \
  /*(render_color)*/ { 135.f / 255.f, 206.f / 255.f, 255.f / 250.f, 1.f }
#define COLOR_NODE_ORANGE \
  /*(render_color)*/ { 216.f / 255.f, 134.f / 255.f, 51.f / 250.f, 1.f }
#define COLOR_TEAL \
  /*(render_color)*/ { 0.0f, 0.52f, 0.52f, 1.f }
#define COLOR_PURPLE \
  /*(render_color)*/ { 160.f / 255.f, 32.f / 255.f, 240.f / 255.f, 1.f }
#define COLOR_BURLY_WOOD \
  /*(render_color)*/ { 0.87f, 0.72f, 0.52f, 1.f }
#define COLOR_DARK_SLATE_GRAY \
  /*(render_color)*/ { 0.18f, 0.18f, 0.31f, 1.f }
#define COLOR_NEARLY_BLACK \
  /*(render_color)*/ { 0.13f, 0.13f, 0.13f, 1.f }
#define COLOR_GHOST_WHITE \
  /*(render_color)*/ { 0.97f, 0.97f, 1.f, 1.f }
#define COLOR_BLACK \
  /*(render_color)*/ { 0.f, 0.f, 0.f, 1.f }
#define COLOR_GRAY \
  /*(render_color)*/ { 0.5f, 0.5f, 0.5f, 1.f }
#define COLOR_DIM_GRAY \
  /*(render_color)*/ { 0.3f, 0.3f, 0.3f, 1.f }
#define COLOR_DARK_GRAY \
  /*(render_color)*/ { 0.2f, 0.2f, 0.2f, 1.f }
#define COLOR_YELLOW \
  /*(render_color)*/ { 1.f, 1.f, 0.f, 1.f }
#define COLOR_RED \
  /*(render_color)*/ { 1.f, 0.f, 0.f, 1.f }
#define COLOR_LIGHT_YELLOW \
  /*(render_color)*/ { 1.f, 1.f, 102.f / 255.f, 1.f }

typedef struct mc_rectf {
  float x, y;
  float width, height;
} mc_rect;

extern "C" {
}

#endif // MC_RENDER_COMMON_H