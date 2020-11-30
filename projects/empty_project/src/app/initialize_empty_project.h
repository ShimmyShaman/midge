#ifndef INITIALIZE_EMPTY_PROJECT_H
#define INITIALIZE_EMPTY_PROJECT_H

// TODO -- pathing
#include "../projects/empty_project/src/mc/midge.h"

#include "cglm/include/cglm/types.h"

typedef struct empty_project_data {
  mc_node *node;

} empty_project_data;

int initialize_empty_project();

#endif // INITIALIZE_EMPTY_PROJECT_H
