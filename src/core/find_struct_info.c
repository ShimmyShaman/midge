#include "core/midge_core.h"

mc_struct_info_v1 *find_struct_info(mc_node_v1 *nodespace, char *struct_name)
{
  for (int i = 0; i < nodespace->struct_count; ++i) {

    // printf("fsi-3\n");
    mc_struct_info_v1 *sinfo = nodespace->structs[i];

    // printf("fsi-4a\n");
    // printf("fsi-4b\n");
    if (strcmp(sinfo->name, struct_name)) {
      printf("findstruct-cmp: '%s'!='%s'\n", sinfo->name, struct_name);
      continue;
    }
    printf("findstruct-cmp: '%s'=='%s'\n", sinfo->name, struct_name);

    // printf("fsi-5\n");

    // Matches
    // printf("find_struct_info:set with '%s'\n", sinfo->name);
    return sinfo;
  }

  // printf("fsi-2\n");

  if (nodespace->parent) {
    // Search in the parent nodespace
    mc_struct_info_v1 *sinfo = find_struct_info(nodespace->parent, struct_name);
    return sinfo;
  }
  printf("find_struct_info: '%s' could not be found!\n", struct_name);
  return NULL;
}