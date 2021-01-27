#ifndef INFO_TRANSCRIPTION_H
#define INFO_TRANSCRIPTION_H

#include "core/core_definitions.h"
#include "mc_str.h"

int mc_generate_header_source(mc_source_file_info *source_file, mc_str *str);

int mc_generate_c_source(mc_source_file_info *source_file, mc_str *str);

int mc_transcribe_specific_function_source(mc_str *str, function_info *function);

#endif // INFO_TRANSCRIPTION_H
