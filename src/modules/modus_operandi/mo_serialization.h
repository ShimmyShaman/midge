#ifndef MO_SERIALIZATION_H
#define MO_SERIALIZATION_H

#include "modules/modus_operandi/mo_types.h"

int mc_mo_serialize_process(mo_operational_process *process, const char **serialization);
int mc_mo_parse_serialized_process(mc_mo_process_stack *pstack, const char *serialization,
                                  mo_operational_process **p_process);

#endif // MO_SERIALIZATION_H