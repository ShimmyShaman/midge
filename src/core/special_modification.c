/* special_modification.c */

#include "core/midge_core.h"

// [_mc_iteration=2]
void special_modification(special_data * data) {
// This stuff
  printf("special_modification() : number=%i\n", data->num);
  data->num += 3;
}