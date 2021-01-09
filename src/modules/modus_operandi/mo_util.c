/* mo_util.c */

#include <stdio.h>

#include "core/midge_app.h"

#include "modules/collections/hash_table.h"
#include "modules/modus_operandi/mo_util.h"

int mc_mo_get_specific_context_cstr(hash_table_t *context, const char *name, const char **result)
{
  mc_str *str;

  str = (mc_str *)hash_table_get(name, context);
  if (str) {
    *result = str->text;
    return 0;
  }

  *result = NULL;
  return 0;
}

int mc_mo_get_context_cstr(mc_mo_process_stack *process_stack, const char *name, bool search_stack, const char **result)
{
  mc_str *str;
  hash_table_t *ctx = &process_stack->context_maps[process_stack->index];

  str = (mc_str *)hash_table_get(name, ctx);

  if (str) {
    *result = str->text;
    return 0;
  }
  if (!search_stack) {
    *result = NULL;
    return 0;
  }

  --ctx;
  for (; ctx >= process_stack->context_maps; --ctx) {
    str = (mc_str *)hash_table_get(name, ctx);
    if (str) {
      *result = str->text;
      return 0;
    }
  }

  // Search in the active project context
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  if (!app_info->projects.active) {
    MCerror(5828, "TODO Set an active project to appinfo");
  }

  ctx = (hash_table_t *)hash_table_get(app_info->projects.active->name, &process_stack->project_contexts);
  if (!ctx) {
    printf("TODO 5982, Why is there no context for project:'%s'?\n", app_info->projects.active->name);
  }

  str = (mc_str *)hash_table_get(name, ctx);
  if (str) {
    *result = str->text;
    return 0;
  }

  // Search in the global context
  str = (mc_str *)hash_table_get(name, &process_stack->global_context);
  if (str) {
    *result = str->text;
    return 0;
  }

  // No Result
  *result = NULL;
  return 0;
}

int mc_mo_set_top_context_cstr(mc_mo_process_stack *process_stack, const char *name, const char *value)
{
  MCcall(mc_mo_set_specific_context_cstr(&process_stack->context_maps[process_stack->index], name, value));

  return 0;
}

int mc_mo_set_specific_context_cstr(hash_table_t *context, const char *name, const char *value)
{
  mc_str *str;

  // printf("setting '%s' to %p\n", name, context);

  str = (mc_str *)hash_table_get(name, context);
  if (!str) {
    MCcall(init_mc_str(&str));

    hash_table_set(name, str, context);
  }

  MCcall(set_mc_str(str, value));

  return 0;
}