/* info_transcription.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"
#include "mc_str.h"
#include "midge_error_handling.h"

#include "modules/info_transcription/info_transcription.h"
#include "modules/mc_io/mc_file.h"

int _mc_transcribe_enum_info(mc_str *str, enumeration_info *enumeration)
{
  MCerror(7313, "TODO");
  return 0;
}

int _mc_transcribe_structure_info(mc_str *str, struct_info *structure)
{
  MCcall(append_to_mc_str(str, "typedef struct "));
  MCcall(append_to_mc_str(str, structure->name));
  MCcall(append_to_mc_str(str, " {\n"));

  field_info *fi;
  for (int f = 0; f < structure->fields->count; ++f) {
    fi = structure->fields->items[f];

    MCcall(append_to_mc_str(str, "  "));
    switch (fi->field_type) {
    case FIELD_KIND_STANDARD: {
      // Type
      MCcall(append_to_mc_str(str, fi->field.type_name));

      // Declarators
      field_declarator_info *di;
      for (int d = 0; d < fi->field.declarators->count; ++d) {
        di = fi->field.declarators->items[d];

        if (d) {
          MCcall(append_char_to_mc_str(str, ','));
        }
        MCcall(append_char_to_mc_str(str, ' '));

        for (int p = 0; p < di->deref_count; ++p)
          MCcall(append_char_to_mc_str(str, '*'));

        MCcall(append_to_mc_str(str, di->name));
      }

      // Rest
      MCcall(append_to_mc_str(str, ";\n"));
    } break;
    default:
      MCerror(9536, "TODO :%i", fi->field_type);
    }
  }

  MCcall(append_to_mc_str(str, "} "));
  MCcall(append_to_mc_str(str, structure->name));
  MCcall(append_to_mc_str(str, ";\n"));

  return 0;
}

int _mc_transcribe_function_info(mc_str *str, function_info *function)
{
  MCerror(7315, "TODO");
  return 0;
}

int _mc_generate_header_source(mc_source_file_info *source_file, mc_str *str)
{
  puts("_mc_generate_header_source");
  puts(source_file->filepath);
  // DEBUG
  // PROTECT CERTAIN SOURCE UNTIL THIS METHOD IS SAFE
  for (int i = 0;; ++i) {
    if (source_file->filepath[i] == '\0')
      break;
    if (!strncmp(source_file->filepath + i, "midge/src/", 10)) {
      MCerror(7004, "INVALID FUNCTION CALL");
    }
  }
  // DEBUG

  // Extract the name
  char filename[64];
  MCcall(mcf_obtain_filename(source_file->filepath, filename, 64));

  // Generate the preamble
  MCcall(append_to_mc_strf(str, "/* %s.h */\n", filename));
  MCcall(append_to_mc_str(str, "\n"));

  // Generate the include guard
  MCcall(append_to_mc_str(str, "#ifndef "));
  MCcall(append_uppercase_to_mc_str(str, filename));
  MCcall(append_to_mc_str(str, "_H\n"));
  MCcall(append_to_mc_str(str, "#define "));
  MCcall(append_uppercase_to_mc_str(str, filename));
  MCcall(append_to_mc_str(str, "_H\n"));

  // Write out in order enums > structs > functions
  int a, phase = 0;
  source_definition *sd;
  while (phase < 4) {

    for (a = 0; a < source_file->definitions.count; ++a) {
      sd = source_file->definitions.items[a];

      switch (sd->type) {
      case SOURCE_DEFINITION_ENUMERATION: {
        if (phase == 1) {
          // Write it
          MCcall(append_char_to_mc_str(str, '\n'));
          MCcall(_mc_transcribe_enum_info(str, sd->data.enum_info));
        }
      } break;
      case SOURCE_DEFINITION_STRUCTURE: {
        if (phase == 2) {
          // Write it
          MCcall(append_char_to_mc_str(str, '\n'));
          MCcall(_mc_transcribe_structure_info(str, sd->data.structure_info));
        }
      } break;
      case SOURCE_DEFINITION_FUNCTION: {
        if (phase == 3) {
          // Write it
          MCcall(append_char_to_mc_str(str, '\n'));
          MCcall(_mc_transcribe_function_info(str, sd->data.func_info));
        }
      } break;
      default:
        MCerror(4939, "Unsupported:%i", sd->type);
      }
    }
    ++phase;
  }

  // End the include guard
  MCcall(append_char_to_mc_str(str, '\n'));
  MCcall(append_to_mc_str(str, "#endif // "));
  MCcall(append_uppercase_to_mc_str(str, filename));
  MCcall(append_to_mc_str(str, "_H"));

  // Save to file
  MCcall(save_text_to_file(source_file->filepath, str->text));
  release_mc_str(str, true);

  return 0;
}

int _mc_generate_c_source(mc_source_file_info *source_file, mc_str *str)
{
  MCerror(1185, "TODO");
  return 0;
}

int mc_save_source_file_from_updated_info(mc_source_file_info *source_file)
{
  // Can only handle .h & .c files atm
  int n = strlen(source_file->filepath);
  if (source_file->filepath[n - 2] != '.') {
    MCerror(9814, "TODO -- Filetype error? '%s'", source_file->filepath);
  }

  // Generate the source file text & persist it to disk
  mc_str *str;
  MCcall(init_mc_str(&str));
  if (source_file->filepath[n - 1] == 'h') {
    MCcall(_mc_generate_header_source(source_file, str));
  }
  else if (source_file->filepath[n - 1] == 'c') {
    MCcall(_mc_generate_c_source(source_file, str));
  }
  else {
    MCerror(9815, "TODO -- Filetype error? '%s'", source_file->filepath);
  }

  // Reinterpret the file
  MCcall(mcs_interpret_file(source_file->filepath));

  return 0;
}