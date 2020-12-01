#ifndef MC_FILE_H
#define MC_FILE_H

#include "mc_str.h"

int mcf_concat_filepath(mc_str *path_prefix, const char *appendage);
int mcf_directory_exists(const char *path, bool *exists);

#endif // MC_FILE_H