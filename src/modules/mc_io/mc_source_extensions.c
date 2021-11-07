/* mc_source_extensions.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "tinycc/libtccinterp.h"

#include "core/mc_source.h"
#include "core/midge_app.h"
#include "mc_error_handling.h"

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_info_transcription.h"

#include "modules/mc_io/mc_source_extensions.h"

int find_source_entity_info(source_entity_info *dest, const char *name)
{
  // puts("find_source_entity_info");
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
      // puts("--function");
      return 0;
    }
  }

  for (int i = 0; i < app_itp_data->function_declarations.count; ++i) {
    fi = app_itp_data->function_declarations.items[i];
    if (!strcmp(name, fi->name)) {
      dest->type = MC_SOURCE_SEGMENT_FUNCTION_DECLARATION;
      dest->fu_info = fi;
      // puts("--funcdecl");
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
      // puts("--enum");
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
  printf("obtsrcf:'%s'\n", path);
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
      MCerror(5883, "PROGRESS");
      // sf = (mc_source_file_info *)malloc(sizeof(mc_source_file_info));
      // sf->filepath = strdup(path);
      // sf->segments.capacity = sf->segments.count = 0U;

      // // Register & persist
      // MCcall(append_to_collection((void ***)&app_itp_data->source_files.items, &app_itp_data->source_files.alloc,
      //                             &app_itp_data->source_files.count, sf));
      // MCcall(mc_save_source_file_from_updated_info(sf));
    }
  }

  if (source_file) {
    *source_file = sf;
  }
  return 0;
}

int mc_save_source_file_info_to_disk(mc_source_file_info *source_file)
{
  // Can only handle .h & .c files atm
  int n = strlen(source_file->filepath);
  if (source_file->filepath[n - 2] != '.') {
    MCerror(9814, "TODO -- Filetype error? '%s'", source_file->filepath);
  }

  // Generate the source file text & persist it to disk
  mc_str *str;
  MCcall(mc_alloc_str(&str));
  if (source_file->filepath[n - 1] == 'h') {
    MCcall(mc_generate_header_source(source_file, str));
  }
  else if (source_file->filepath[n - 1] == 'c') {
    MCcall(mc_generate_c_source(source_file, str));
  }
  else {
    MCerror(9815, "TODO -- Filetype error? '%s'", source_file->filepath);
  }

  MCcall(save_text_to_file(source_file->filepath, str->text));
  clock_gettime(CLOCK_REALTIME, &source_file->recent_disk_sync);

  mc_release_str(str, true);

  // Reinterpret the file
  // MCcall(mcs_interpret_file(source_file->filepath));

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
  case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE:
    *priority = 1;
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

int mcs_insert_segment_judiciously_in_source_file(mc_source_file_info *source_file,
                                                  mc_source_file_code_segment_type type, void *data)
{
  mc_source_file_code_segment *nlseg = (mc_source_file_code_segment *)malloc(sizeof(mc_source_file_code_segment));
  nlseg->type = MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR;
  nlseg->data = NULL;
  mc_source_file_code_segment *ins_seg = (mc_source_file_code_segment *)malloc(sizeof(mc_source_file_code_segment));
  ins_seg->type = type;
  ins_seg->data = data;

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
  bool inserted = false;
  MCcall(_mc_insert_segment_get_priority(type, &priority));

  for (a = source_file->segments.count - 1; a >= 0; --a) {
    seg = source_file->segments.items[a];

    MCcall(_mc_insert_segment_get_priority(seg->type, &sp));
    // printf("seg:%i p:%i\n", seg->type, sp);
    if (sp > 0 && sp < priority) {
      // Heres a good place
      // printf("inserted segment at index %i\n", a);

      MCcall(insert_in_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                                  &source_file->segments.count, a + 1, ins_seg));
      MCcall(insert_in_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                                  &source_file->segments.count, a + 2, nlseg));
      inserted = true;
      break;
    }
  }
  if (!inserted) {
    // printf("inserted segment at index 0\n");
    MCcall(insert_in_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                                &source_file->segments.count, 0, ins_seg));
    MCcall(insert_in_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                                &source_file->segments.count, 1, nlseg));
  }

  MCcall(mc_save_source_file_info_to_disk(source_file));

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
    MCcall(mcs_insert_segment_judiciously_in_source_file(source_file, MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION, si));

    MCcall(mc_redefine_structure(si));
  }

  return 0;
}

int mcs_ensure_header_include_for_type(mc_source_file_info *sf, const char *type_name)
{
  // printf("mcs_ensure_header_include_for_type:'%s'\n", type_name);

  source_entity_info sei;
  find_source_entity_info(&sei, type_name);

  if (!sei.type) {
    // Can't do anything more, just yet...
    // TODO -- std::types
    return 0;
  }

  int a, n, m;
  char ext[8], *c, cwd[256];
  mc_source_file_info *type_sf;
  mc_source_file_code_segment *seg;
  switch (sei.type) {
  case MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION: {
    // Obtain the file it was declared with
    type_sf = sei.st_info->source_file;
    printf("Looking for:%s\n", type_sf->filepath);

    // Ensure it is a header - for now
    MCcall(mcf_obtain_file_extension(type_sf->filepath, ext, 8));
    if (strcmp(ext, "h")) {
      MCerror(7529, "'%s' is not a header file for struct declaration '%s'", type_sf->filepath, type_name);
    }
    n = strlen(type_sf->filepath);

    // Search for it
    bool included_already = false;
    for (a = 0; a < sf->segments.count; ++a) {
      seg = sf->segments.items[a];
      if (seg->type == MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE) {
        m = strlen(seg->include->filepath);
        c = type_sf->filepath + (n - m);

        // Ends-with
        if (!strcmp(c, seg->include->filepath)) {
          printf("'%s' already included\n", seg->include->filepath);
          included_already = true;
          break;
        }
      }
    }
    if (included_already) {
      break;
    }

    // Add an include segment
    // -- Subtract the cwd
    if (!getcwd(cwd, 256)) {
      MCerror(7912, "CWD FAILED TODO");
    }
    // printf("cwd:'%s'\n", cwd);
    m = strlen(cwd);
    if (strncmp(cwd, type_sf->filepath, m)) {
      MCerror(4728, "TODO '%s' isn't part of '%s'", cwd, type_sf->filepath);
    }
    c = type_sf->filepath + m;
    if (*c == '/' || *c == '\\')
      ++c;

    // Include Paths
    // -- Find the longest matching fit within the include paths
    // TODO ???
    // if(!strncmp("src/modules/", c, 12))
    //   c+= 12;
    // else
    if (!strncmp("src/", c, 4))
      c += 4;

    mc_include_directive_info *idi = (mc_include_directive_info *)malloc(sizeof(mc_include_directive_info));
    idi->filepath = strdup(c);
    idi->is_system_search = false; // TODO
    MCcall(mcs_insert_segment_judiciously_in_source_file(sf, MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE, idi));
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
  MCcall(mc_redefine_structure(si));
  return 0;
}

int mcs_append_field_to_struct_and_remap(struct_info *si, const char *type_name, unsigned int type_deref_count,
                                         const char *field_name, void **data, void **p_field)
{
  // TODO -- ideally the struct_info would have this information
  char nme[64], inc[128], buf[512];
  size_t before_size, after_size;
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  sprintf(inc, "#include \"%s\"", si->source_file->filepath);
  const char *su_includes[] = {
      "#include <stdio.h>",
      inc,
  };

  sprintf(nme, "mcs_append_field_to_struct_and_remap_%u", app_info->uid_counter++);

  sprintf(buf,
          "  printf(\"aaa=%%lu\\n\", sizeof(tetris_data));\n  *((size_t *)vargs) = sizeof(%s);\n  puts(\"bbb\");\n  "
          "return NULL;",
          si->name);
  MCcall(tcci_execute_single_use_code(app_info->itp_data->interpreter, nme, 2, su_includes, buf, &before_size, NULL));

  // printf("structure %s had a size before of %lu\n", si->name, before_size);

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
  // WARNING If its ever changed from appending to insertion need to update the remapping also
  MCcall(append_to_collection((void ***)&si->fields.items, &si->fields.alloc, &si->fields.count, f));

  MCcall(mcs_ensure_header_include_for_type(si->source_file, type_name));

  // puts("e");
  MCcall(mc_redefine_structure(si));

  sprintf(nme, "mcs_append_field_to_struct_and_remap_%u", app_info->uid_counter++);

  sprintf(buf,
          "  printf(\"aaa=%%lu\\n\", sizeof(tetris_data));\n  *((size_t *)vargs) = sizeof(%s);\n  puts(\"bbb\");\n  "
          "return NULL;",
          si->name);
  MCcall(tcci_execute_single_use_code(app_info->itp_data->interpreter, nme, 2, su_includes, buf, &after_size, NULL));

  // printf("structure %s had a size after of %lu\n", si->name, after_size);

  // Remap the data
  void *new_data = (void *)malloc(after_size);
  memcpy(new_data, *data, before_size);

  // printf("replacing data at %p with data at %p\n", *data, new_data);
  void *t = *data;
  *data = new_data;
  free(t);

  *p_field = (void *)((unsigned char *)*data + before_size);

  return 0;
}

int mcs_construct_function_definition(mc_source_file_info *source_file, const char *name, const char *return_type_name,
                                      unsigned int return_type_deref, int parameter_count, const char **parameters,
                                      const char *code)
{
  // puts("mcs_construct_function_definition");
  source_entity_info sei;
  function_info *fi;
  parameter_info *pp;

  MCcall(find_source_entity_info(&sei, name));
  if (sei.type) {
    MCerror(4592, "Another symbol already possesses this name");
  }

  // printf("name'%s'\n", name);
  // printf("return_type_name'%s'\n", return_type_name);

  fi = (function_info *)calloc(1, sizeof(function_info));
  fi->name = strdup(name);
  fi->is_defined = true;
  fi->source = source_file;
  fi->return_type.name = strdup(return_type_name);
  fi->return_type.deref_count = return_type_deref;
  fi->code = NULL;

  fi->parameters.alloc = 0U;
  fi->parameters.count = 0U;
  if (parameter_count) {
    pp = calloc(parameter_count, sizeof(parameter_info));
  }
  for (int a = 0; a < parameter_count; ++a) {
    // TODO -- non-standard parameters (fptrs etc)
    const char *c = parameters[a], *s;

    parameter_info *p = &pp[a];
    MCcall(append_to_collection((void ***)&fi->parameters.items, &fi->parameters.alloc, &fi->parameters.count, p));

    p->parameter_type = PARAMETER_KIND_STANDARD;

    // Type
    // printf("0:'%s'\n", s);
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
    // printf("1:'%s'\n", c);
    p->type_deref_count = 0U;
    while (*c == '*') {
      if (*c == '\0') {
        MCerror(8447, "TODO");
      }
      ++c;
      ++p->type_deref_count;
    }

    // printf("2:'%s'\n", c);
    s = c;
    while (*c != '\0') {
      ++c;
    }
    p->name = strndup(s, c - s);
    // printf("3:'%s'\n", p->name);
  }

  fi->nb_dependents = 0;
  fi->nb_dependencies = 0;

  fi->code = strdup(code);

  MCcall(mc_register_function_info_to_app(fi));
  // printf("constfd:%i\n", source_file->segments.count);
  MCcall(mcs_insert_segment_judiciously_in_source_file(source_file, MC_SOURCE_SEGMENT_FUNCTION_DEFINITION, fi));
  // printf("constfd:%i\n", source_file->segments.count);
  MCcall(mc_redefine_function(fi));

  return 0;
}

int mcs_attach_code_to_function(function_info *fi, const char *code)
{
  int fic_len, clen;
  clen = strlen(code);
  char *nc;
  if (!strcmp(fi->return_type.name, "void") && !fi->return_type.deref_count) {
    // Just attach to the end before the curly bracket
    const char *c = fi->code, *s = NULL;
    while (*c != '\0') {
      ++c;
    }
    fic_len = c - fi->code - 1;

    --c;
    while (*c != '}') {
      while (*c == ' ' || *c == '\t')
        --c;
    }
    --c;

    // while (*c == ' ' || *c == '\t')
    //   --c;

    nc = (char *)malloc(sizeof(char) * (fic_len + clen + 1));
    strncpy(nc, fi->code, c - fi->code);
    strcpy(nc + (c - fi->code), code);
    strcpy(nc + (c - fi->code) + clen, c);
  }
  else {

    // Find the final return and insert code before that
    // Move to the end -- obtain the position of the last return
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

    nc = (char *)malloc(sizeof(char) * (fic_len + clen + 1));
    strncpy(nc, fi->code, c - fi->code);
    strcpy(nc + (c - fi->code), code);
    strcpy(nc + (c - fi->code) + clen, c);
  }

  free(fi->code);
  fi->code = nc;

  // printf("check:\n%s||\n", nc);

  // Save the source file info and define the function
  MCcall(mc_save_source_file_info_to_disk(fi->source));
  MCcall(mc_redefine_function(fi));

  return 0;
}

int mcs_add_include_to_source_file(mc_source_file_info *source_file, const char *include_stanza)
{
  // Ensure it isn't already included
  mc_source_file_code_segment *seg;
  for (int a = 0; a < source_file->segments.count; ++a) {
    seg = source_file->segments.items[a];

    if (seg->type != MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE)
      continue;
    if (seg->include->is_system_search != include_stanza[0] == '<')
      continue;
    if (strncmp(seg->include->filepath, include_stanza + 1, strlen(seg->include->filepath)))
      continue;

    return 0;
  }

  mc_include_directive_info *idi = (mc_include_directive_info *)malloc(sizeof(mc_include_directive_info));
  idi->filepath = strndup(include_stanza + 1, strlen(include_stanza) - 2);
  idi->is_system_search = (include_stanza[0] == '<');

  MCcall(mcs_insert_segment_judiciously_in_source_file(source_file, MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE, idi));

  return 0;
}

int mc_redefine_function(function_info *function)
{
  // printf("function:%p\n", function);
  // printf("function->source:%p\n", function->source);
  // printf("function->source->segments:%i\n", function->source->segments.count);
  int a, b, c;
  mc_str *str;
  MCcall(mc_alloc_str(&str));

  MCcall(mc_transcribe_specific_function_source(str, function, true));

  // printf("mc_redefine_function-gen:\n%s||\n", str->text);

  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  char tempfn[128];
  sprintf(tempfn, "%s::%s", function->source->filepath, function->name);
  MCcall(tcci_add_string(app_info->itp_data->interpreter, tempfn, str->text));

  mc_release_str(str, true);

  return 0;
}

int mc_redefine_function_provisionally(function_info *function, const char *code)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  // printf("function:%p\n", function);
  // printf("function->source:%p\n", function->source);
  // printf("function->source->segments:%i\n", function->source->segments.count);
  mc_str *str;
  MCcall(mc_alloc_str(&str));

  // Swap
  char *old_code = function->code;
  function->code = (char *)code;

  // Call
  MCcall(mc_transcribe_specific_function_source(str, function, true));

  char tempfn[128];
  sprintf(tempfn, "%s::%s", function->source->filepath, function->name);
  int res = tcci_add_string(app_info->itp_data->interpreter, tempfn, str->text);
  if (res) {
    // Re-set old code
    function->code = old_code;
  }
  else {
    // Release old code
    free(old_code);

    // Duplicate New Code to function
    function->code = strdup(code);
  }

  mc_release_str(str, true);
  return res;
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
  MCcall(mc_alloc_str(&str));
  if (structure->source_file->filepath[n - 1] == 'h') {
    MCcall(mc_generate_header_source(structure->source_file, str));
  }
  else {
    MCerror(9815, "TODO -- Filetype error? '%s'", structure->source_file->filepath);
  }

  // Save to file
  MCcall(save_text_to_file(structure->source_file->filepath, str->text));
  mc_release_str(str, true);

  // TODO -- header dependencies

  // mc_source_file_code_segment_list *sl = &structure->source_file->segments;
  // for (a = 0; a < sl->count; ++a) {
  //   mc_source_file_code_segment *seg = sl->items[a];
  //   switch (seg->type) {
  //   case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE:
  //     MCcall(_mc_transcribe_include_directive_info(str, seg->include));
  //     break;
  //   case MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR:
  //     MCcall(mc_append_char_to_str(str, '\n'));
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
