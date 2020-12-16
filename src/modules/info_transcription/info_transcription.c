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

int _mc_transcribe_sub_type_info(mc_str *str, field_info *sub_type, int indent);

int _mc_transcribe_enum_info(mc_str *str, enumeration_info *enumeration)
{
  MCerror(7313, "TODO");
  return 0;
}

int _mc_transcribe_field_declarator_info_list(mc_str *str, field_declarator_info_list *list)
{
  field_declarator_info *di;
  for (int d = 0; d < list->count; ++d) {
    di = list->items[d];

    if (d) {
      MCcall(append_char_to_mc_str(str, ','));
    }
    MCcall(append_char_to_mc_str(str, ' '));

    for (int p = 0; p < di->deref_count; ++p)
      MCcall(append_char_to_mc_str(str, '*'));

    MCcall(append_to_mc_str(str, di->name));
  }
  return 0;
}

int _mc_transcribe_field_info_list(mc_str *str, field_info_list *list, int indent)
{
  field_info *fi;
  for (int f = 0; f < list->count; ++f) {
    fi = list->items[f];

    for (int i = 0; i < indent; ++i)
      MCcall(append_to_mc_str(str, "  "));
    switch (fi->field_type) {
    case FIELD_KIND_STANDARD: {
      // Type
      MCcall(append_to_mc_str(str, fi->field.type_name));

      // Declarators
      MCcall(_mc_transcribe_field_declarator_info_list(str, fi->field.declarators));

      // Rest
      MCcall(append_to_mc_str(str, ";\n"));
    } break;
    case FIELD_KIND_NESTED_UNION:
    case FIELD_KIND_NESTED_STRUCT: {
      MCcall(_mc_transcribe_sub_type_info(str, fi, 1));

      // // Rest
      // for (int i = 0; i < indent - 1; ++i)
      //   MCcall(append_to_mc_str(str, "  "));
      // MCcall(append_to_mc_str(str, "} "));

      // MCcall(_mc_transcribe_field_declarator_info_list(str, fi->sub_type.declarators));

      // // Rest
      // MCcall(append_to_mc_str(str, ";\n"));
    } break;
    default:
      MCerror(9536, "TODO :%i", fi->field_type);
    }
  }

  return 0;
}

int _mc_transcribe_sub_type_info(mc_str *str, field_info *sub_type, int indent)
{
  // Type
  if (sub_type->field_type == FIELD_KIND_NESTED_UNION) {
    MCcall(append_to_mc_str(str, "union "));
  }
  else if (sub_type->field_type == FIELD_KIND_NESTED_STRUCT) {
    MCcall(append_to_mc_str(str, "struct "));
  }
  else {
    MCerror(9131, "Not Supported :%i", sub_type->field_type);
  }

  if (sub_type->sub_type.type_name) {
    MCcall(append_to_mc_str(str, sub_type->sub_type.type_name));
    MCcall(append_char_to_mc_str(str, ' '));
  }

  MCcall(append_to_mc_str(str, " {\n"));

  MCcall(_mc_transcribe_field_info_list(str, sub_type->sub_type.fields, indent + 1));

  for (int i = 0; i < indent; ++i)
    MCcall(append_to_mc_str(str, "  "));
  MCcall(append_to_mc_str(str, "}"));
  MCcall(_mc_transcribe_field_declarator_info_list(str, sub_type->sub_type.declarators));

  MCcall(append_to_mc_str(str, ";\n"));
  return 0;
}

int _mc_transcribe_structure_info(mc_str *str, struct_info *structure)
{
  MCcall(append_to_mc_str(str, "typedef struct "));
  MCcall(append_to_mc_str(str, structure->name));
  MCcall(append_to_mc_str(str, " {\n"));

  MCcall(_mc_transcribe_field_info_list(str, structure->fields, 1));

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

int _mc_transcribe_include_directive_info(mc_str *str, mc_include_directive_info *idi)
{
  MCcall(append_to_mc_str(str, "#include "));
  MCcall(append_char_to_mc_str(str, idi->is_system_search ? '<' : '"'));
  MCcall(append_to_mc_str(str, idi->filepath));
  MCcall(append_char_to_mc_str(str, idi->is_system_search ? '>' : '"'));
  MCcall(append_char_to_mc_str(str, '\n'));

  return 0;
}

int _mc_generate_header_source(mc_source_file_info *source_file, mc_str *str)
{
  int a, phase = 0;

  puts("_mc_generate_header_source");
  puts(source_file->filepath);
  // DEBUG
  // PROTECT CERTAIN SOURCE UNTIL THIS METHOD IS SAFE
  for (int a = 0;; ++a) {
    if (source_file->filepath[a] == '\0')
      break;
    if (!strncmp(source_file->filepath + a, "midge/src/", 10)) {
      MCerror(7004, "INVALID FUNCTION CALL");
    }
  }
  // DEBUG

  // Extract the name
  char filename[64];
  MCcall(mcf_obtain_filename(source_file->filepath, filename, 64));

  // Generate the preamble
  MCcall(append_to_mc_strf(str, "/* %s.h */\n", filename));
  MCcall(append_char_to_mc_str(str, '\n'));

  // Generate the include guard
  MCcall(append_to_mc_str(str, "#ifndef "));
  MCcall(append_uppercase_to_mc_str(str, filename));
  MCcall(append_to_mc_str(str, "_H\n"));
  MCcall(append_to_mc_str(str, "#define "));
  MCcall(append_uppercase_to_mc_str(str, filename));
  MCcall(append_to_mc_str(str, "_H\n"));

  mc_source_file_code_segment *seg;
  for (a = 0; a < source_file->segments.count; ++a) {
    seg = source_file->segments.items[a];

    switch (seg->type) {
    case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE:
      MCcall(_mc_transcribe_include_directive_info(str, seg->include));
      break;
    case MC_SOURCE_SEGMENT_FUNCTION_DEFINITION:
      MCcall(_mc_transcribe_function_info(str, seg->function));
      break;
    case MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION:
      MCcall(_mc_transcribe_structure_info(str, seg->structure));
      break;
    default:
      MCerror(9715, "Unsupported Segment Type : %i", seg->type);
    }
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