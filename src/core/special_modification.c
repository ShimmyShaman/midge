/* special_modification.c */

#include "core/midge_core.h"

// [_mc_iteration=2]
void special_modification(mc_special_data_v1 * data) {
// This stuff
  printf("special_modification() : number=%i\n", data->num);
  data->num += 3;
}