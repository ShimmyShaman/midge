/* find_function_info.c */
/* Dependencies: */
#define NULL (void *)0
typedef struct function_info {
  const char *const name;
} function_info;
typedef struct node {
  const char *const name;
  node *parent;
  unsigned int function_count;
  function_info **functions;
} node;
/* End-Dependencies */

function_info *find_function_info(node *nodespace, char const *const function_name)
{
  printf("ffi-nodespace.name:%s\n", nodespace->name);
  for (int i = 0; i < nodespace->function_count; ++i) {

    // printf("ffi-3\n");
    // printf("dope\n");
    function_info *finfo = nodespace->functions[i];

    // printf("ffi-4a\n");
    printf("findfunc-cmp: '%s'<>'%s'\n", finfo->name, function_name);
    // printf("ffi-4b\n");
    if (strcmp(finfo->name, function_name))
      continue;
    // printf("dwde\n");

    // printf("ffi-5\n");

    // Matches
    printf("find_function_info:set with '%s'\n", finfo->name);
    return finfo;
  }
  // printf("dopu\n");

  // printf("ffi-2\n");

  if (nodespace->parent) {
    // Search in the parent nodespace
    return find_function_info(nodespace->parent, function_name);
  }
  printf("find_function_info: '%s' could not be found!\n", function_name);
  return NULL;
}