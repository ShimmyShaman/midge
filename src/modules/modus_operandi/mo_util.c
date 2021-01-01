/* mo_util.c */

#include <stdio.h>

#include "core/midge_app.h"

#include "modules/collections/hash_table.h"
#include "modules/modus_operandi/mo_util.h"

int mc_mo_get_context_value(mc_mo_process_stack *process_stack, const char *name, bool search_stack, void **result)
{
  hash_table_t *ctx = &process_stack->context_maps[process_stack->index];

  *result = hash_table_get(name, ctx);

  if (*result || !search_stack) {
    return 0;
  }

  --ctx;
  for (; ctx >= process_stack->context_maps; --ctx) {
    *result = hash_table_get(name, ctx);
    if (*result)
      return 0;
  }

  // Search in the active project context
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  if (!app_info->projects.active) {
    puts("TODO 5828 Set an active project to appinfo");
  }

  ctx = (hash_table_t *)hash_table_get(app_info->projects.active->name, &process_stack->project_contexts);
  if (!ctx) {
    printf("TODO 5982, Why is there no context for project:'%s'?\n", app_info->projects.active->name);
  }

  *result = hash_table_get(name, ctx);
  if (*result)
    return 0;

  // Search in the global context
  *result = hash_table_get(name, &process_stack->global_context);
  return 0;
}
