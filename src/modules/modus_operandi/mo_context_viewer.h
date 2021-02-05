#ifndef MO_CONTEXT_VIEWER_H
#define MO_CONTEXT_VIEWER_H

#include "core/core_definitions.h"

#include "modules/modus_operandi/mo_util.h"

int init_mo_context_viewer(mc_mo_process_stack *process_stack, mc_node **p_context_viewer);
int mc_mo_toggle_context_viewer_visibility(mc_node *context_viewer_node);

#endif // MO_CONTEXT_VIEWER_H