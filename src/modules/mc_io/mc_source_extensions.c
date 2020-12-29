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

int mcs_insert_struct_declaration(mc_source_file_info *source_file, const char *name)
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
    si->fields = (field_info_list *)malloc(sizeof(field_info_list));
    si->fields->alloc = si->fields->count = 0;

    MCcall(mc_register_struct_info_to_app(si));
    MCcall(mc_insert_segment_judiciously_in_source_file(source_file, MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION, si));
    MCcall(mc_save_source_file_from_updated_info(source_file));
  }

  return 0;
}