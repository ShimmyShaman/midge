#ifndef MO_UTIL_H
#define MO_UTIL_H

#include "modules/modus_operandi/mo_types.h"

int mc_mo_get_context_value(mc_mo_process_stack *process_stack, const char *name, bool search_stack, void **result);

#endif // MO_UTIL_H
