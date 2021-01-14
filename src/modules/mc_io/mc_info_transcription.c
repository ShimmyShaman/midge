/* info_transcription.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"
#include "core/midge_app.h"
#include "mc_str.h"
#include "midge_error_handling.h"

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_info_transcription.h"

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
      MCcall(append_to_mc_str(str, fi->std.type_name));

      // Declarators
      MCcall(_mc_transcribe_field_declarator_info_list(str, &fi->declarators));

      // Rest
      MCcall(append_to_mc_str(str, ";\n"));
    } break;
    case FIELD_KIND_NESTED_UNION:
    case FIELD_KIND_NESTED_STRUCT: {
      MCcall(_mc_transcribe_sub_type_info(str, fi, indent));

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
  MCcall(_mc_transcribe_field_declarator_info_list(str, &sub_type->declarators));

  MCcall(append_to_mc_str(str, ";\n"));
  return 0;
}

int _mc_transcribe_structure_info(mc_str *str, struct_info *structure)
{
  MCcall(append_to_mc_str(str, "typedef struct "));
  MCcall(append_to_mc_str(str, structure->name));
  MCcall(append_to_mc_str(str, " {\n"));

  MCcall(_mc_transcribe_field_info_list(str, &structure->fields, 1));

  MCcall(append_to_mc_str(str, "} "));
  MCcall(append_to_mc_str(str, structure->name));
  MCcall(append_to_mc_str(str, ";\n"));

  return 0;
}

int _mc_transcribe_function_header(mc_str *str, function_info *function)
{
  int a, b;
  parameter_info *p;

  MCcall(append_to_mc_str(str, function->return_type.name));
  MCcall(append_char_to_mc_str(str, ' '));
  for (a = 0; a < function->return_type.deref_count; ++a)
    MCcall(append_char_to_mc_str(str, '*'));

  MCcall(append_to_mc_str(str, function->name));
  MCcall(append_char_to_mc_str(str, '('));

  // printf("parameter_count:%u\n", function->parameter_count);
  for (int a = 0; a < function->parameters.count; ++a) {
    p = function->parameters.items[a];

    // printf("--parameter:%p %s\n", p, p ? p->name : "(null)");

    if (a) {
      MCcall(append_to_mc_str(str, ", "));
    }

    switch (p->parameter_type) {
    case PARAMETER_KIND_STANDARD: {
      MCcall(append_to_mc_str(str, p->type_name));
      MCcall(append_char_to_mc_str(str, ' '));
      for (b = 0; b < p->type_deref_count; ++b)
        MCcall(append_char_to_mc_str(str, '*'));
      MCcall(append_to_mc_str(str, p->name));
    } break;
    default:
      MCerror(9650, "TODO parameter_type=%i", p->parameter_type);
    }
  }
  MCcall(append_to_mc_str(str, ")"));

  return 0;
}

int _mc_transcribe_function_declaration(mc_str *str, function_info *function)
{
  MCcall(_mc_transcribe_function_header(str, function));
  MCcall(append_to_mc_str(str, ";\n"));

  return 0;
}

int _mc_transcribe_function_info(mc_str *str, function_info *function)
{
  printf("_mc_transcribe_function_info: function:%p (%s) code:%p\n", function, function ? function->name : "(null)",
         function ? function->code : NULL);

  MCcall(_mc_transcribe_function_header(str, function));

  MCcall(append_char_to_mc_str(str, ' '));
  MCcall(append_to_mc_str(str, function->code));
  MCcall(append_char_to_mc_str(str, '\n'));

  return 0;
}

int _mc_transcribe_include_directive_info(mc_str *str, mc_include_directive_info *idi)
{
  // printf("idi :%p\n", idi);
  // printf("idi :%s\n", idi->is_system_search ? "true" : "false");
  // printf("idi :%p\n", idi->filepath);
  // printf("idi :'%s'\n", idi->filepath);

  MCcall(append_to_mc_str(str, "#include "));
  MCcall(append_char_to_mc_str(str, idi->is_system_search ? '<' : '"'));
  MCcall(append_to_mc_str(str, idi->filepath));
  MCcall(append_char_to_mc_str(str, idi->is_system_search ? '>' : '"'));
  MCcall(append_char_to_mc_str(str, '\n'));
  // puts("idiend");
  return 0;
}

int _mc_transcribe_segment_list(mc_str *str, mc_source_file_code_segment_list *segments)
{
  for (int a = 0; a < segments->count; ++a) {
    mc_source_file_code_segment *seg = segments->items[a];
    switch (seg->type) {
    case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE:
      MCcall(_mc_transcribe_include_directive_info(str, seg->include));
      break;
    case MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR:
      MCcall(append_char_to_mc_str(str, '\n'));
      break;
    case MC_SOURCE_SEGMENT_FUNCTION_DEFINITION:
      MCcall(_mc_transcribe_function_info(str, seg->function));
      break;
    case MC_SOURCE_SEGMENT_FUNCTION_DECLARATION:
      MCcall(_mc_transcribe_function_declaration(str, seg->function));
      break;
    case MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION:
      MCcall(_mc_transcribe_structure_info(str, seg->structure));
      break;
    default:
      MCerror(6458, "Unsupported Segment Type : %i", seg->type);
    }
  }

  return 0;
}

int _mc_generate_header_source(mc_source_file_info *source_file, mc_str *str)
{
  int a, phase = 0;

  // puts("_mc_generate_header_source");
  // puts(source_file->filepath);
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

  MCcall(_mc_transcribe_segment_list(str, &source_file->segments));

  // End the include guard
  MCcall(append_char_to_mc_str(str, '\n'));
  MCcall(append_to_mc_str(str, "#endif // "));
  MCcall(append_uppercase_to_mc_str(str, filename));
  MCcall(append_to_mc_str(str, "_H"));

  // puts("END_mc_generate_header_source");
  return 0;
}

int _mc_generate_c_source(mc_source_file_info *source_file, mc_str *str)
{
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
  MCcall(append_to_mc_strf(str, "/* %s.c */\n", filename));
  MCcall(append_char_to_mc_str(str, '\n'));

  MCcall(_mc_transcribe_segment_list(str, &source_file->segments));

  return 0;
}

int mc_redefine_function(function_info *function)
{
  puts("aaa");
  printf("function:%p\n", function);
  printf("function->source:%p\n", function->source);
  printf("function->source->segments:%i\n", function->source->segments.count);
  int a;
  mc_str *str;

  MCcall(init_mc_str(&str));

  mc_source_file_code_segment_list *sl = &function->source->segments;
  for (a = 0; a < sl->count; ++a) {
    mc_source_file_code_segment *seg = sl->items[a];
    switch (seg->type) {
    case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE:
      MCcall(_mc_transcribe_include_directive_info(str, seg->include));
      break;
    case MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR:
      MCcall(append_char_to_mc_str(str, '\n'));
      break;
    case MC_SOURCE_SEGMENT_FUNCTION_DEFINITION: {
      if (!strcmp(seg->function->name, function->name)) {
        MCcall(_mc_transcribe_function_info(str, function));
      }
    } break;
    case MC_SOURCE_SEGMENT_FUNCTION_DECLARATION:
    case MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION:
      // Do Nothing
      break;
    default:
      MCerror(5624, "Unsupported Segment Type : %i", seg->type);
    }
  }

  printf("redeffunc-check:\n%s||\n", str->text);

  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  char tempfn[128];
  sprintf(tempfn, "%s::%s", function->source->filepath, function->name);
  MCcall(tcci_add_string(app_info->itp_data->interpreter, tempfn, str->text));

  release_mc_str(str, true);

  return 0;
}

int mc_redefine_structure(struct_info *structure)
{
  // Can only handle .h & .c files atm
  int n = strlen(structure->source_file->filepath);
  if (structure->source_file->filepath[n - 2] != '.') {
    MCerror(9814, "TODO -- Filetype error? '%s'", structure->source_file->filepath);
  }

  // Generate the source file text & persist it to disk
  mc_str *str;
  MCcall(init_mc_str(&str));
  if (structure->source_file->filepath[n - 1] == 'h') {
    MCcall(_mc_generate_header_source(structure->source_file, str));
  }
  else {
    MCerror(9815, "TODO -- Filetype error? '%s'", structure->source_file->filepath);
  }

  // Save to file
  MCcall(save_text_to_file(structure->source_file->filepath, str->text));
  release_mc_str(str, true);

  // TODO -- header dependencies

  // mc_source_file_code_segment_list *sl = &structure->source_file->segments;
  // for (a = 0; a < sl->count; ++a) {
  //   mc_source_file_code_segment *seg = sl->items[a];
  //   switch (seg->type) {
  //   case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE:
  //     MCcall(_mc_transcribe_include_directive_info(str, seg->include));
  //     break;
  //   case MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR:
  //     MCcall(append_char_to_mc_str(str, '\n'));
  //     break;
  //   case MC_SOURCE_SEGMENT_FUNCTION_DEFINITION:
  //   case MC_SOURCE_SEGMENT_FUNCTION_DECLARATION:
  //     // Do Nothing
  //     break;
  //   case MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION: {
  //     if (!strcmp(seg->structure->name, structure->name)) {
  //       MCcall(_mc_transcribe_structure_info(str, structure));
  //     }
  //   } break;
  //   default:
  //     MCerror(5624, "Unsupported Segment Type : %i", seg->type);
  //   }
  // }

  // MCerror(8582, "Progress");

  return 0;
}

// int mc_save_source_file_from_updated_info(mc_source_file_info *source_file)
// {
//   // Can only handle .h & .c files atm
//   int n = strlen(source_file->filepath);
//   if (source_file->filepath[n - 2] != '.') {
//     MCerror(9814, "TODO -- Filetype error? '%s'", source_file->filepath);
//   }

//   // Generate the source file text & persist it to disk
//   mc_str *str;
//   MCcall(init_mc_str(&str));
//   if (source_file->filepath[n - 1] == 'h') {
//     MCcall(_mc_generate_header_source(source_file, str));
//   }
//   else if (source_file->filepath[n - 1] == 'c') {
//     MCcall(_mc_generate_c_source(source_file, str));
//   }
//   else {
//     MCerror(9815, "TODO -- Filetype error? '%s'", source_file->filepath);
//   }

//   // Reinterpret the file
//   MCcall(mcs_interpret_file(source_file->filepath));

//   return 0;
// }