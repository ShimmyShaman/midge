/* render_thread.h */

#ifndef MC_RENDER_THREAD_H
#define MC_RENDER_THREAD_H

#include "m_threads.h"
#include "midge_common.h"
#include "platform/mc_xcb.h"
#include "render/render_common.h"

#define MRT_SEQUENCE_COPY_BUFFER_SIZE 8192

typedef struct mrt_sequence_copy_buffer {
  VkDescriptorBufferInfo vpc_desc_buffer_info;
  u_char data[MRT_SEQUENCE_COPY_BUFFER_SIZE];
  u_int32_t index;
} mrt_sequence_copy_buffer;

typedef struct vert_data_scale_offset {
  struct {
    float x, y;
  } offset;
  struct {
    float x, y;
  } scale;
} vert_data_scale_offset;

typedef struct frag_ubo_tint_texcoordbounds {
  struct {
    float r, g, b, a;
  } tint;
  struct {
    float s0, s1, t0, t1;
  } tex_coord_bounds;
} frag_ubo_tint_texcoordbounds;

typedef struct coloured_rect_draw_data {
  struct {
    // Vertex Fields
    vec2 offset;
    vec2 scale;
  } vert;

  struct {
    // Fragment Fields
    vec4 tint_color;
  } frag;

} coloured_rect_draw_data;

extern "C" {
void *midge_render_thread(void *vargp);
}

#endif // RENDER_THREAD_H