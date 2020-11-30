#ifndef MAIN_INIT_H
#define MAIN_INIT_H

// TODO -- pathing
#include "../projects/empty_project/src/mc/midge.h"

#include "cglm/include/cglm/types.h"

typedef struct cube_template_root_data {
  mc_node *node;

  struct {
    unsigned int width, height;
    mcr_texture_image *image;
  } render_target;

  struct {
    mat4 world;
    // mcr_model *model;
  } cube;
} cube_template_root_data;

int initialize_empty_project();

#endif // MAIN_INIT_H
