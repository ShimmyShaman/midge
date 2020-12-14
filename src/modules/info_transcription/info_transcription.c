/* info_transcription.c */

#include <stdio.h>
#include <stdlib.h>

#include "core/core_definitions.h"
#include "mc_str.h"

#include "modules/info_transcription/info_transcription.h"
#include "modules/mc_io/mc_file.h"

int _mc_generate_header_source(mc_source_file_info *source_file, mc_str *code)
{
  // DEBUG
  // PROTECT CERTAIN SOURCE UNTIL THIS METHOD IS SAFE
  for (int i = 0;; ++i) {
    if (source_file->filepath[i] == '\0')
      break;
    if (strncmp(source_file->filepath + i, "midge/src/", 10)) {
      MCerror(7004, "INVALID FUNCTION CALL");
    }
  }
  // DEBUG

  // Extract the name
  char filename[64];
  MCcall(mcf_obtain_filename(source_file->filepath, filename, 64));

  // Generate the preamble
  MCcall(append_to_mc_strf(code, "/* %s.h */\n", filename));
  MCcall(append_to_mc_str(code, "\n"));

  // Generate the include guard
  MCcall(append_to_mc_str(code, "#ifndef "));
  MCcall(append_uppercase_to_mc_str(code, filename));
  MCcall(append_to_mc_str(code, "_H"));
  MCcall(append_to_mc_str(code, "\n"));
  MCcall(append_to_mc_str(code, "#define "));
  MCcall(append_uppercase_to_mc_str(code, filename));
  MCcall(append_to_mc_str(code, "_H"));
  MCcall(append_to_mc_str(code, "\n"));

  // Write out in order enums > structs > functions
  int a, phase = 0;
  source_definition *sd;
  while (phase < 3) {
    for (a = 0; a < source_file->definitions.count; ++a) {
      sd = source_file->definitions.items[a];

      switch (sd->type) {
      case SOURCE_DEFINITION_ENUMERATION: {
        if (phase <= 1) {
          // Write it
        }
      } break;
      case SOURCE_DEFINITION_STRUCTURE: {
        if (phase <= 2) {
          // Write it
        }
      } break;
      case SOURCE_DEFINITION_FUNCTION: {
        if (phase <= 3) {
          // Write it
        }
      } break;
      default:
        MCerror(4939, "Unsupported:%i", sd->type);
      }
    }
  }
  MCcall(append_to_mc_str(code, "\n"));

  // End the include guard
  MCcall(append_to_mc_str(code, "#endif // "));
  MCcall(append_uppercase_to_mc_str(code, filename));
  MCcall(append_to_mc_str(code, "_H"));

  return 0;
}

int _mc_generate_c_source(mc_source_file_info *source_file, mc_str *code)
{
  MCerror(1185, "TODO");
  return 0;
}

int mc_save_source_file_from_updated_info(mc_source_file_info *source_file)
{
  int n = strlen(source_file->filepath);
  if (source_file->filepath[n - 2] != '.') {
    MCerror(9814, "TODO -- Filetype error? '%s'", source_file->filepath);
  }

  mc_str *code;
  MCcall(init_mc_str(&code));
  if (source_file->filepath[n - 1] == 'h') {
    MCcall(_mc_generate_header_source(source_file, code));
  }
  else if (source_file->filepath[n - 1] == 'c') {
    MCcall(_mc_generate_c_source(source_file, code));
  }
  else {
    MCerror(9815, "TODO -- Filetype error? '%s'", source_file->filepath);
  }

  printf("TODO -- '%s'", source_file->filepath);

  return 0;
}