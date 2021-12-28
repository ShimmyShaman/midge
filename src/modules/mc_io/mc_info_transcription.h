#ifndef INFO_TRANSCRIPTION_H
#define INFO_TRANSCRIPTION_H

#include "core/core_definitions.h"
#include "mc_str.h"

int mc_generate_header_source(mc_source_file_info *source_file, mc_str *str);

int mc_generate_c_source(mc_source_file_info *source_file, mc_str *str);

/* Transcribe a function info into a source file where it is the sole defined function with all declarations 
 * it requires.
 * @use_midge_c_instead: Set this flag to true if function source is being directly utilized by the midge interpreter.
 * This should be most use-cases of this function.
 */
int mc_transcribe_specific_function_source(mc_str *str, function_info *function, bool use_midge_c_instead);

#endif // INFO_TRANSCRIPTION_H
