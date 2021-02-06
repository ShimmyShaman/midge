/* mo_util.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/midge_app.h"

#include "modules/collections/hash_table.h"
#include "modules/modus_operandi/mo_util.h"

int mc_mo_get_specific_context_cstr(hash_table_t *context, const char *key, const char **result)
{
  mc_mo_context_data *data = (mc_mo_context_data *)hash_table_get(key, context);

  if (!data) {
    *result = NULL;
    return 0;
  }

  if (data->value_type != MC_MO_CONTEXT_DATA_VALUE_MC_STR) {
    MCerror(8821, "TODO : %i", data->value_type);
  }

  *result = data->str.text;
  return 0;
}

int _mc_mo_get_context_from_stack(mc_mo_process_stack *process_stack, const char *key, bool search_stack,
                                  mc_mo_context_data **p_data)
{
  mc_mo_context_data *data;
  hash_table_t *ctx = &process_stack->context_maps[process_stack->index];

  // printf("mc_mo_get_context_cstr:ctx0=%p\n", ctx);
  data = (mc_mo_context_data *)hash_table_get(key, ctx);
  if (data || !search_stack) {
    *p_data = data;
    return 0;
  }

  --ctx;
  for (; ctx >= process_stack->context_maps; --ctx) {
    // printf("mc_mo_get_context_cstr:ctx1=%p\n", ctx);
    data = (mc_mo_context_data *)hash_table_get(key, ctx);
    if (data) {
      *p_data = data;
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

  data = (mc_mo_context_data *)hash_table_get(key, ctx);
  if (data) {
    *p_data = data;
    return 0;
  }

  // Search in the global context
  *p_data = (mc_mo_context_data *)hash_table_get(key, &process_stack->global_context);
  return 0;
}

int mc_mo_get_context_cstr(mc_mo_process_stack *process_stack, const char *key, bool search_stack, const char **result)
{
  // printf("mc_mo_get_context_cstr:'%s'\n", key);
  mc_mo_context_data *data;
  MCcall(_mc_mo_get_context_from_stack(process_stack, key, search_stack, &data));
  if (!data) {
    *result = NULL;
    return 0;
  }

  if (data->value_type != MC_MO_CONTEXT_DATA_VALUE_MC_STR) {
    MCerror(8822, "TODO : %i", data->value_type);
  }

  *result = data->str.text;
  return 0;
}

int mc_mo_set_specific_context_cstr(hash_table_t *context, const char *key, const char *value)
{
  mc_mo_context_data *data = (mc_mo_context_data *)hash_table_get(key, context);
  if (!data) {
    data = (mc_mo_context_data *)calloc(sizeof(mc_mo_context_data), 1);
    data->key = strdup(key);

    hash_table_set(key, data, context);
  }

  printf(".set-context:'%s':MC_STR='%s'\n", key, value);
  data->value_type = MC_MO_CONTEXT_DATA_VALUE_MC_STR;
  printf("str: %u %u\n", data->str.alloc, data->str.len);
  MCcall(set_mc_str(&data->str, value));

  return 0;
}

int mc_mo_set_top_context_cstr(mc_mo_process_stack *process_stack, const char *key, const char *value)
{
  MCcall(mc_mo_set_specific_context_cstr(&process_stack->context_maps[process_stack->index], key, value));

  return 0;
}

int mc_mo_get_specific_context_ptr(hash_table_t *context, const char *key, void **result)
{
  mc_mo_context_data *data = (mc_mo_context_data *)hash_table_get(key, context);

  if (!data) {
    *result = NULL;
    return 0;
  }

  if (data->value_type != MC_MO_CONTEXT_DATA_VALUE_PTR) {
    MCerror(7219, "TODO : %i", data->value_type);
  }

  *result = data->value;

  return 0;
}

int mc_mo_get_context_ptr(mc_mo_process_stack *process_stack, const char *key, bool search_stack, void **result)
{
  mc_mo_context_data *data;
  MCcall(_mc_mo_get_context_from_stack(process_stack, key, search_stack, &data));
  if (!data) {
    *result = NULL;
    return 0;
  }

  if (data->value_type != MC_MO_CONTEXT_DATA_VALUE_PTR) {
    MCerror(8823, "TODO : %i", data->value_type);
  }

  *result = data->value;
  return 0;
}

int mc_mo_set_specific_context_ptr(hash_table_t *context, const char *key, void *value)
{
  mc_mo_context_data *data = (mc_mo_context_data *)hash_table_get(key, context);
  if (!data) {
    data = (mc_mo_context_data *)calloc(sizeof(mc_mo_context_data), 1);
    data->key = strdup(key);

    hash_table_set(key, data, context);
  }

  printf(".set-context:'%s':VALUE_PTR=%p\n", key, value);
  data->value_type = MC_MO_CONTEXT_DATA_VALUE_PTR;
  data->value = value;

  return 0;
}

int mc_mo_set_top_context_ptr(mc_mo_process_stack *process_stack, const char *key, void *value)
{
  MCcall(mc_mo_set_specific_context_ptr(&process_stack->context_maps[process_stack->index], key, value));

  return 0;
}