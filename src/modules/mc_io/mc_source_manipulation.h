#ifndef MC_SOURCE_MANIPULATION_H
#define MC_SOURCE_MANIPULATION_H

#include "core/core_definitions.h"

int mcs_obtain_source_file_info(const char *path, bool create_if_not_exists, mc_source_file_info **source_file);

#endif // MC_SOURCE_MANIPULATION_H
