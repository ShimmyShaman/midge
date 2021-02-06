#ifndef MO_UTIL_H
#define MO_UTIL_H

#include "modules/modus_operandi/mo_types.h"

int mc_mo_get_specific_context_cstr(hash_table_t *context, const char *key, const char **result);
int mc_mo_get_context_cstr(mc_mo_process_stack *process_stack, const char *key, bool search_stack,
                           const char **result);
int mc_mo_set_specific_context_cstr(hash_table_t *context, const char *key, const char *value);
int mc_mo_set_top_context_cstr(mc_mo_process_stack *process_stack, const char *key, const char *value);

int mc_mo_get_specific_context_ptr(hash_table_t *context, const char *key, void **result);
int mc_mo_get_context_ptr(mc_mo_process_stack *process_stack, const char *key, bool search_stack, void **result);
int mc_mo_set_specific_context_ptr(hash_table_t *context, const char *key, void *value);
int mc_mo_set_top_context_ptr(mc_mo_process_stack *process_stack, const char *key, void *value);

#endif // MO_UTIL_H