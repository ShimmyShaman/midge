/* obj_loader.h */

#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "midge_common.h"
#include "render/mc_vulkan.h"

// typedef struct mcr_model {
//   unsigned int vertex_buffer;
//   unsigned int index_buffer;
//   unsigned int texture;
// } mcr_model;

typedef enum _wvf_obj_cmd_type {
  _wvf_obj_cmd_NULL = 0,
  _wvf_obj_cmd_EMPTY,
  _wvf_obj_cmd_V,
  _wvf_obj_cmd_VN,
  _wvf_obj_cmd_VT,
  _wvf_obj_cmd_F,
  _wvf_obj_cmd_G,
  _wvf_obj_cmd_O,
  _wvf_obj_cmd_USEMTL,
  _wvf_obj_cmd_MTLLIB
} _wvf_obj_cmd_type;

typedef struct _wvf_obj_vertex_index_list {
  int v_idx, vt_idx, vn_idx;
} _wvf_obj_vertex_index_list;

#define _WVF_OBJ_IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))
#define _WVF_OBJ_CPTR_SKIP_SPACE(x)                         \
  while ((*(x) == ' ') || (*(x) == '\t') || (*(x) == '\r')) \
    ++(x);
#define _WVF_OBJ_IS_NEW_LINE(x) (((x) == '\r') || ((x) == '\n') || ((x) == '\0'))

typedef struct _wvf_obj_cmd {
  _wvf_obj_cmd_type type;
  int begin;
} _wvf_obj_cmd;

typedef struct _wvf_obj_parsed_obj_info {
  _wvf_obj_cmd *cmds;
  unsigned int cmd_count;

  unsigned int f_cmd_begin;
  unsigned int v_cmd_begin;
  unsigned int vt_cmd_begin;
  unsigned int vn_cmd_begin;

  unsigned int num_faces;

  unsigned int triangle_count;
} _wvf_obj_parsed_obj_info;

extern "C" {
void mcr_load_wavefront_obj(const char *obj_path, mcr_vertex_buffer **vertices, mcr_index_buffer **indices);
void mcr_load_wavefront_obj_model(const char *obj_path, const char *diffuse_path, mcr_model **loaded_model);
}

#endif // OBJ_LOADER_H