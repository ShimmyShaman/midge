/* obj_loader.h */

#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "midge_common.h"
#include "render/mc_vulkan.h"

typedef struct mcr_model {
    unsigned int vertex_buffer_uid;
    unsigned int index_buffer_uid;
}mcr_model;

extern "C" {
void mcr_load_wavefront_obj_model(const char *obj_path, mcr_model **loaded_model);
}

#endif // OBJ_LOADER_H