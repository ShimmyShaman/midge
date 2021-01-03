/* mc_source_extensions.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "core/mc_source.h"
#include "core/midge_app.h"
#include "midge_error_handling.h"

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_info_transcription.h"

#include "modules/mc_io/mc_source_extensions.h"

int find_source_entity_info(source_entity_info *dest, const char *name)
{
  mc_app_itp_data *app_itp_data;
  mc_obtain_app_itp_data(&app_itp_data);

  function_info *fi;
  for (int i = 0; i < app_itp_data->functions.count; ++i) {
    fi = app_itp_data->functions.items[i];
    if (!strcmp(name, fi->name)) {

      if (fi->is_defined)
        dest->type = MC_SOURCE_SEGMENT_FUNCTION_DEFINITION;
      else
        dest->type = MC_SOURCE_SEGMENT_FUNCTION_DECLARATION;
      dest->fu_info = fi;
      return 0;
    }
  }

  for (int i = 0; i < app_itp_data->function_declarations.count; ++i) {
    fi = app_itp_data->function_declarations.items[i];
    if (!strcmp(name, fi->name)) {
      dest->type = MC_SOURCE_SEGMENT_FUNCTION_DECLARATION;
      dest->fu_info = fi;
      return 0;
    }
  }

  struct_info *si;
  for (int i = 0; i < app_itp_data->structs.count; ++i) {
    si = app_itp_data->structs.items[i];
    if (!strcmp(name, si->name)) {
      if (si->is_defined)
        dest->type = MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION;
      else
        dest->type = MC_SOURCE_SEGMENT_STRUCTURE_DECLARATION;
      dest->st_info = si;
      // printf("struct type '%s'\n", name);
      return 0;
    }
  }

  enumeration_info *ei;
  for (int i = 0; i < app_itp_data->enumerations.count; ++i) {
    ei = app_itp_data->enumerations.items[i];
    if (!strcmp(name, ei->name)) {
      if (ei->is_defined)
        dest->type = MC_SOURCE_SEGMENT_ENUMERATION_DEFINITION;
      else
        dest->type = MC_SOURCE_SEGMENT_ENUMERATION_DECLARATION;
      dest->en_info = ei;
      return 0;
    }
  }

  // Nothing Found
  // printf("no type found for '%s'\n", name);
  memset(dest, 0, sizeof(source_entity_info));

  return 0;
}

int mcs_obtain_source_file_info(const char *path, bool create_if_not_exists, mc_source_file_info **source_file)
{
  char full_path[256];
  mcf_obtain_full_path(path, full_path, 256);

  mc_app_itp_data *app_itp_data;
  mc_obtain_app_itp_data(&app_itp_data);

  // Determine if the header already exists
  mc_source_file_info *sf;
  for (int a = 0; a < app_itp_data->source_files.count; ++a) {
    sf = app_itp_data->source_files.items[a];

    if (strcmp(sf->filepath, full_path)) {
      sf = NULL;
      continue;
    }

    // Found it
    break;
  }
  if (!sf) {
    if (!create_if_not_exists) {
      *source_file = NULL;
      return 0;
    }

    // Create it
    if (access(full_path, F_OK) != -1) {
      // Tentatively, load it in
      printf("[warning] source-file '%s' exists on disk but has not yet been interpreted. Interpreting now...\n",
             full_path);
      MCcall(mcs_interpret_source_file(full_path, &sf));
    }
    else {
      sf = (mc_source_file_info *)malloc(sizeof(mc_source_file_info));
      sf->filepath = strdup(path);
      sf->segments.capacity = sf->segments.count = 0U;

      // Register & persist
      MCcall(append_to_collection((void ***)&app_itp_data->source_files.items, &app_itp_data->source_files.alloc,
                                  &app_itp_data->source_files.count, sf));
      MCcall(mc_save_source_file_from_updated_info(sf));
    }
  }

  if (source_file) {
    *source_file = sf;
  }
  return 0;
}

int _mc_insert_segment_get_priority(mc_source_file_code_segment_type type, int *priority)
{
  switch (type) {
  case MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION:
    *priority = 5;
    return 0;
  case MC_SOURCE_SEGMENT_FUNCTION_DECLARATION:
    *priority = 8;
    return 0;
  case MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR:
  case MC_SOURCE_SEGMENT_SINGLE_LINE_COMMENT:
  case MC_SOURCE_SEGMENT_MULTI_LINE_COMMENT:
    *priority = -1;
    return 0;
  case MC_SOURCE_SEGMENT_FUNCTION_DEFINITION:
    *priority = 12;
    return 0;
  default: {
    MCerror(7824, "TODO :%i", type);
  }
  }
  return 0;
}

int mc_insert_segment_judiciously_in_source_file(mc_source_file_info *source_file,
                                                 mc_source_file_code_segment_type type, void *data)
{
  mc_source_file_code_segment *nlseg = (mc_source_file_code_segment *)malloc(sizeof(mc_source_file_code_segment));
  nlseg->type = MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR;
  nlseg->data = NULL;
  mc_source_file_code_segment *segment = (mc_source_file_code_segment *)malloc(sizeof(mc_source_file_code_segment));
  segment->type = type;
  segment->data = data;

  // Find the best place for it according to order:
  // {.h}
  // -- includes
  // -- defines
  // -- enums
  // -- struct declarations
  // -- structs
  // -- function declarations
  // -- function definitions;

  // char buf[8];
  // MCcall(mcf_obtain_file_extension(source_file->filepath, buf, 8));
  mc_source_file_code_segment *seg;
  int priority, sp, a;
  MCcall(_mc_insert_segment_get_priority(type, &sp));

  for (a = source_file->segments.count - 1; a >= 0; --a) {
    seg = source_file->segments.items[a];

    MCcall(_mc_insert_segment_get_priority(seg->type, &sp));
    if (sp > 0 && sp < priority) {
      // Heres a good place
      MCcall(insert_in_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                                  &source_file->segments.count, a + 1, nlseg));
      MCcall(insert_in_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                                  &source_file->segments.count, a + 2, segment));
      a = 800;
      break;
    }
  }
  if (a != 800) {
    MCcall(insert_in_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                                &source_file->segments.count, 0, segment));
    MCcall(insert_in_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                                &source_file->segments.count, 1, nlseg));
  }

  return 0;
}

int mcs_construct_struct_declaration(mc_source_file_info *source_file, const char *name)
{
  struct_info *si = NULL;
  mc_source_file_code_segment *seg = NULL;

  MCcall(find_struct_info(name, &si));
  if (si) {
    MCerror(7490, "TODO");
    //   // for (int a = 0; a < source_file->segments.count; ++a) {
    //   //   d = source_file->segments.items[a];
    //   //   if (d->type == MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION && !strcmp(d->structure->name, struct_name)) {
    //   //     seg = d;
    //   //     break;
    //   //   }
    //   // }
  }

  {
    // Construct an empty structure with the given name and register it globally and to the source file
    struct_info *si = (struct_info *)malloc(sizeof(struct_info));
    si->is_defined = true;
    si->is_union = false;
    si->name = strdup(name);
    si->source_file = source_file;
    si->fields.alloc = 0U;
    si->fields.count = 0U;

    MCcall(mc_register_struct_info_to_app(si));
    MCcall(mc_insert_segment_judiciously_in_source_file(source_file, MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION, si));
    MCcall(mc_save_source_file_from_updated_info(source_file));
  }

  return 0;
}

int mcs_ensure_header_include_for_type(mc_source_file_info *sf, const char *type_name)
{
  printf("mcs_ensure_header_include_for_type:'%s'\n", type_name);

  source_entity_info sei;
  find_source_entity_info(&sei, type_name);

  if (!sei.type) {
    // Can't do anything more, just yet...
    // TODO -- std::types
    return 0;
  }

  switch (sei.type) {
  case MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION: {
    MCerror(2748, "PROGRESS");
  } break;
  default: {
    MCerror(9587, "TODO %i", sei.type);
  }
  }

  return 0;
}

int mcs_append_field_to_struct(struct_info *si, const char *type_name, unsigned int type_deref_count,
                               const char *field_name)
{
  // puts("a");
  field_info *f = (field_info *)malloc(sizeof(field_info *));
  f->field_type = FIELD_KIND_STANDARD;
  f->std.type_name = strdup(type_name);
  f->declarators.count = 0U;
  f->declarators.alloc = 0U;

  // puts("b");
  field_declarator_info *fdecl = (field_declarator_info *)malloc(sizeof(field_declarator_info));
  fdecl->array.dimension_count = 0;
  fdecl->deref_count = type_deref_count;
  fdecl->name = strdup(field_name);
  // puts("c");
  MCcall(append_to_collection((void ***)&f->declarators.items, &f->declarators.alloc, &f->declarators.count, fdecl));

  // puts("d");
  MCcall(append_to_collection((void ***)&si->fields.items, &si->fields.alloc, &si->fields.count, f));

  MCcall(mcs_ensure_header_include_for_type(si->source_file, type_name));

  // puts("e");
  MCcall(mc_save_source_file_from_updated_info(si->source_file));
  return 0;
}

int mcs_construct_function_definition(mc_source_file_info *source_file, const char *name, const char *return_type_name,
                                      unsigned int return_type_deref, int parameter_count, const char **parameters,
                                      const char *code)
{
  source_entity_info sei;
  function_info *fi;

  MCcall(find_source_entity_info(&sei, name));
  if (sei.type) {
    MCerror(4592, "Another symbol already possesses this name");
  }

  fi = (function_info *)calloc(1, sizeof(function_info));
  fi->name = strdup(name);
  fi->is_defined = true;
  fi->source = source_file;
  fi->return_type.name = strdup(return_type_name);
  fi->return_type.deref_count = return_type_deref;

  fi->parameters.alloc = 0U;
  fi->parameters.count = 0U;
  parameter_info *pp = calloc(parameter_count, sizeof(parameter_info));
  for (int a = 0; a < parameter_count; ++a) {
    // TODO -- non-standard parameters (fptrs etc)
    const char *c = parameters[a], *s;

    parameter_info *p = &pp[a];
    MCcall(append_to_collection((void ***)&fi->parameters.items, &fi->parameters.alloc, &fi->parameters.count, p));

    p->parameter_type = PARAMETER_KIND_STANDARD;

    // Type
    s = c;
    while (*c != ' ') {
      if (*c == '\0') {
        MCerror(8427, "TODO");
      }
      ++c;
    }
    p->type_name = strndup(s, c - s);
    ++c;

    // Deref
    p->type_deref_count = 0U;
    while (*c == '*') {
      if (*c == '\0') {
        MCerror(8447, "TODO");
      }
      ++c;
      ++p->type_deref_count;
    }

    s = c;
    while (*c != '\0') {
      ++c;
    }
    p->name = strndup(s, c - s);
  }

  fi->nb_dependents = 0;
  fi->nb_dependencies = 0;

  fi->code = strdup(code);

  MCcall(mc_register_function_info_to_app(fi));
  MCcall(mc_insert_segment_judiciously_in_source_file(source_file, MC_SOURCE_SEGMENT_FUNCTION_DEFINITION, fi));
  MCcall(mc_save_source_file_from_updated_info(source_file));

  return 0;
}

int mcs_attach_code_to_function(function_info *fi, const char *code)
{
  // Find the final return and insert code before that
  if (!strcmp(fi->return_type.name, "void") && !fi->return_type.deref_count) {
    MCerror(9482, "TODO");
  }

  // Move to the end -- obtain the position of the last return
  int fic_len, clen;
  const char *c = fi->code, *s = NULL;
  while (*c != '\0') {
    if (*c == 'r' && !strncmp(c, "return ", 7))
      s = c;
    ++c;
  }
  fic_len = c - fi->code;

  if (!s) {
    MCerror(7592, "TODO");
  }
  c = s;

  --c;
  if (*c != ';' && *c != '}') {
    while (*c == ' ' || *c == '\t')
      --c;
    ++c;
  }

  clen = strlen(code);
  char *nc = (char *)malloc(sizeof(char) * (fic_len + clen + 1));
  strncpy(nc, fi->code, c - fi->code);
  strcpy(nc + (c - fi->code), code);
  strcpy(nc + (c - fi->code) + clen, c);

  free(fi->code);
  fi->code = nc;

  printf("check:\n%s||\n", nc);

  MCcall(mc_save_source_file_from_updated_info(fi->source));

  return 0;
}