#ifndef MO_UTIL_H
#define MO_UTIL_H

#include "modules/modus_operandi/mo_types.h"

int mc_mo_get_specific_context_cstr(hash_table_t *context, const char *name, const char **result);
int mc_mo_get_context_cstr(mc_mo_process_stack *process_stack, const char *name, bool search_stack,
                           const char **result);
int mc_mo_set_specific_context_cstr(hash_table_t *context, const char *name, const char *value);
int mc_mo_set_top_context_cstr(mc_mo_process_stack *process_stack, const char *name, const char *value);

#endif // MO_UTIL_H
