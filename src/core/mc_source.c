#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "midge_common.h"

int register_sub_type_syntax_to_field_info(mc_syntax_node *subtype_syntax, field_info *field);

int attach_function_info_to_owner(node *owner, function_info *func_info)
{
  switch (owner->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *data = (global_root_data *)owner->data;
    append_to_collection((void ***)&data->functions.items, &data->functions.alloc, &data->functions.count,
                         (void *)func_info);
    break;
  }
  default:
    MCerror(9, "TODO:%i", owner->type);
  }

  return 0;
}

int attach_struct_info_to_owner(node *owner, struct_info *structure_info)
{
  switch (owner->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *data = (global_root_data *)owner->data;
    append_to_collection((void ***)&data->structs.items, &data->structs.alloc, &data->structs.count,
                         (void *)structure_info);
    break;
  }
  default:
    MCerror(9, "TODO:%i", owner->type);
  }

  return 0;
}

int attach_enumeration_info_to_owner(node *owner, enumeration_info *enum_info)
{
  switch (owner->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *data = (global_root_data *)owner->data;
    append_to_collection((void ***)&data->enumerations.items, &data->enumerations.alloc, &data->enumerations.count,
                         (void *)enum_info);
    break;
  }
  default:
    MCerror(9, "TODO:%i", owner->type);
  }

  return 0;
}

// int attach_preprocess_define_info_to_owner(node *owner, preprocess_define_info *define_info)
// {
//   switch (owner->type) {
//   case NODE_TYPE_GLOBAL_ROOT: {
//     global_root_data *data = (global_root_data *)owner->data;
//     append_to_collection((void ***)&data->preprocess_defines.items, &data->preprocess_defines.alloc,
//                          &data->preprocess_defines.count, (void *)define_info);
//     break;
//   }
//   default:
//     MCerror(9, "TODO:%i", owner->type);
//   }

//   return 0;
// }

int initialize_source_file_info(node *owner, char *filepath, source_file_info **source_file)
{
  source_file_info *sfi = (source_file_info *)malloc(sizeof(source_file_info));
  allocate_and_copy_cstr(sfi->filepath, filepath);
  sfi->definitions.alloc = 0;
  sfi->definitions.count = 0;

  switch (owner->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *data = (global_root_data *)owner->data;
    append_to_collection((void ***)&data->source_files.items, &data->source_files.alloc, &data->source_files.count,
                         (void *)sfi);
    break;
  }
  default:
    MCerror(52, "TODO:%i", owner->type);
  }

  if (source_file)
    *source_file = sfi;

  return 0;
}

int initialize_parameter_info_from_syntax_node(mc_syntax_node *parameter_syntax_node,
                                               parameter_info **initialized_parameter)
{
  parameter_info *parameter = (parameter_info *)calloc(sizeof(parameter_info), 1);
  parameter->type_id = (struct_id *)malloc(sizeof(struct_id));
  allocate_and_copy_cstr(parameter->type_id->identifier, "parameter_info");
  parameter->type_id->version = 1U;

  switch (parameter_syntax_node->parameter.type) {
  case PARAMETER_KIND_STANDARD: {
    register_midge_error_tag("initialize_parameter_info_from_syntax_node-STANDARD");
    parameter->parameter_type = PARAMETER_KIND_STANDARD;

    // Type
    // print_syntax_node(parameter_syntax_node->parameter.type_identifier, 0);
    copy_syntax_node_to_text(parameter_syntax_node->parameter.type_identifier, (char **)&parameter->type_name);
    if (parameter_syntax_node->parameter.type_dereference) {
      parameter->type_deref_count = parameter_syntax_node->parameter.type_dereference->dereference_sequence.count;
    }
    else {
      parameter->type_deref_count = 0;
    }
    // printf("parameter->type_deref_count:%i\n", parameter->type_deref_count);

    // -- TODO -- mc-type?

    // Name
    copy_syntax_node_to_text(parameter_syntax_node->parameter.name, (char **)&parameter->name);
  } break;
  case PARAMETER_KIND_FUNCTION_POINTER: {
    register_midge_error_tag("initialize_parameter_info_from_syntax_node-FUNCTION_POINTER");
    parameter->parameter_type = PARAMETER_KIND_FUNCTION_POINTER;

    copy_syntax_node_to_text(parameter_syntax_node, &parameter->full_function_pointer_declaration);

    mc_syntax_node *fpsn = parameter_syntax_node->parameter.function_pointer;
    copy_syntax_node_to_text(fpsn->function_pointer_declaration.identifier, (char **)&parameter->name);

    if (fpsn->function_pointer_declaration.type_dereference) {
      parameter->type_deref_count = fpsn->function_pointer_declaration.type_dereference->dereference_sequence.count;
    }
    else {
      parameter->type_deref_count = 0;
    }

    // void *ptr = 0;
    // int (*fptr)(int, void **) = (int (*)(int, void **))ptr;
    parameter->function_type = NULL;
  } break;
  case PARAMETER_KIND_VARIABLE_ARGS: {
    register_midge_error_tag("initialize_parameter_info_from_syntax_node-VARIABLE_ARGS");
    parameter->parameter_type = PARAMETER_KIND_VARIABLE_ARGS;

    parameter->type_name = NULL;
    parameter->declared_type = NULL;
    parameter->type_version = 0;
    parameter->function_type = NULL;
    parameter->full_function_pointer_declaration = NULL;
    parameter->type_deref_count = 0;
    parameter->name = NULL;

  } break;
  default:
    MCerror(125, "NotSupported:%i", parameter_syntax_node->parameter.type);
  }

  *initialized_parameter = parameter;
  return 0;
}

int update_or_register_function_info_from_syntax(node *owner, mc_syntax_node *function_ast, function_info **p_func_info)
{
  function_info *func_info;
  find_function_info(function_ast->function.name->text, &func_info);

  bool is_declaration_only = (mc_token_type)function_ast->function.code_block->type == MC_TOKEN_SEMI_COLON;

  register_midge_error_tag("update_or_register_function_info_from_syntax-1");
  if (!func_info) {
    func_info = (function_info *)malloc(sizeof(function_info));

    if (is_declaration_only) {
      // Attach to loose declarations
      global_root_data *root_data;
      obtain_midge_global_root(&root_data);

      append_to_collection((void ***)&root_data->function_declarations.items, &root_data->function_declarations.alloc,
                           &root_data->function_declarations.count, func_info);
    }
    else {
      // Only attach definitions, not declarations
      if (!owner) {
        MCerror(162, "owner can only be NULL for a function declaration");
      }

      attach_function_info_to_owner(owner, func_info);
    }

    func_info->type_id = (struct_id *)malloc(sizeof(struct_id));
    allocate_and_copy_cstr(func_info->type_id->identifier, "function_info");
    func_info->type_id->version = 1U;

    // Name & Version
    allocate_and_copy_cstr(func_info->name, function_ast->function.name->text);
    func_info->latest_iteration = is_declaration_only ? 0U : 1U;

    // Declare the functions pointer with cling
    // printf("--attempting:'%s'\n", func_info->name);
    char buf[512];
    sprintf(buf, "int (*%s)(int, void **);", func_info->name);
    clint_declare(buf);
    sprintf(buf, "%s = (int (*)(int, void **))0;", func_info->name);
    clint_process(buf);
    printf("--declared:'%s()'\n", func_info->name);

    // Assign the functions pointer
    sprintf(buf, "*((void **)%p) = (void *)&%s;", &func_info->ptr_declaration, func_info->name);
    clint_process(buf);
    // printf("func_info->ptr_declaration:%p\n", func_info->ptr_declaration);
  }
  else {
    if (!is_declaration_only && func_info->latest_iteration < 1) {
      // Function has only been declared, not defined
      // Remove from loose declarations
      global_root_data *root_data;
      obtain_midge_global_root(&root_data);
      remove_ptr_from_collection((void ***)&root_data->function_declarations.items,
                                 &root_data->function_declarations.count, true, func_info);

      // Attach to owner
      func_info->latest_iteration = 1U;
      attach_function_info_to_owner(owner, func_info);
    }

    // TODO -- this was causing a segmentation fault or something - TODO
    // if (func_info->return_type.name) {
    //   free(func_info->return_type.name);
    // }

    // Free parameters -- allow them to be changed
    // register_midge_error_tag("update_or_register_function_info_from_syntax-1a");
    // if (func_info->parameter_count) {
    //   for (int a = 0; a < func_info->parameter_count; ++a) {
    //     if (func_info->parameters[a]) {
    //       release_parameter_info(func_info->parameters[a]);
    //       func_info->parameters[a] = NULL;
    //     }
    //   }
    // }
    func_info->parameter_count = 0;
    register_midge_error_tag("update_or_register_function_info_from_syntax-1b");
  }
  register_midge_error_tag("update_or_register_function_info_from_syntax-2");

  // Return-type & Parameters
  copy_syntax_node_to_text(function_ast->function.return_type_identifier, &func_info->return_type.name);
  if (function_ast->function.return_type_dereference) {
    func_info->return_type.deref_count = function_ast->function.return_type_dereference->dereference_sequence.count;
  }
  else {
    func_info->return_type.deref_count = 0;
  }

  register_midge_error_tag("update_or_register_function_info_from_syntax-3");
  func_info->parameter_count = function_ast->function.parameters->count;
  func_info->parameters = (parameter_info **)malloc(sizeof(parameter_info *) * func_info->parameter_count);
  for (int p = 0; p < func_info->parameter_count; ++p) {
    parameter_info *parameter;
    initialize_parameter_info_from_syntax_node(function_ast->function.parameters->items[p], &parameter);
    func_info->parameters[p] = parameter;
  }

  // TODO
  func_info->variable_parameter_begin_index = -1;
  func_info->struct_usage_count = 0;
  func_info->struct_usage = NULL;

  // Set
  if (p_func_info)
    *p_func_info = func_info;

  return 0;
}

int summarize_field_declarator_list(mc_syntax_node_list *syntax_declarators,
                                    field_declarator_info_list **field_declarators_list)
{
  if (!syntax_declarators) {
    *field_declarators_list = NULL;
    return 0;
  }

  *field_declarators_list = (field_declarator_info_list *)malloc(sizeof(field_declarator_info_list));
  (*field_declarators_list)->alloc = 0;
  (*field_declarators_list)->count = 0;

  for (int d = 0; d < syntax_declarators->count; ++d) {
    mc_syntax_node *declarator_syntax = syntax_declarators->items[d];

    field_declarator_info *declarator = (field_declarator_info *)malloc(sizeof(field_declarator_info));
    copy_syntax_node_to_text(declarator_syntax->field_declarator.name, &declarator->name);
    if (declarator_syntax->field_declarator.type_dereference) {
      declarator->deref_count = declarator_syntax->field_declarator.type_dereference->dereference_sequence.count;
    }
    else {
      declarator->deref_count = 0;
    }

    append_to_collection((void ***)&(*field_declarators_list)->items, &(*field_declarators_list)->alloc,
                         &(*field_declarators_list)->count, declarator);
  }

  return 0;
}

int summarize_type_field_list(mc_syntax_node_list *field_syntax_list, field_info_list **field_list)
{
  (*field_list) = (field_info_list *)malloc(sizeof(field_info_list));
  (*field_list)->alloc = 0;
  (*field_list)->count = 0;

  for (int i = 0; i < field_syntax_list->count; ++i) {
    mc_syntax_node *field_syntax = field_syntax_list->items[i];

    field_info *field = (field_info *)malloc(sizeof(field_info));
    field->type_id = (struct_id *)malloc(sizeof(struct_id));
    allocate_and_copy_cstr(field->type_id->identifier, "field_info");
    field->type_id->version = 1U;

    register_midge_error_tag("summarize_type_field_list-2a");
    switch (field_syntax->type) {
    case MC_SYNTAX_FIELD_DECLARATION: {
      switch (field_syntax->field.type) {
      case FIELD_KIND_STANDARD: {
        field->field_type = FIELD_KIND_STANDARD;
        copy_syntax_node_to_text(field_syntax->field.type_identifier, &field->field.type_name);

        summarize_field_declarator_list(field_syntax->field.declarators, &field->field.declarators);
      } break;
      case FIELD_KIND_FUNCTION_POINTER: {
        field->field_type = FIELD_KIND_FUNCTION_POINTER;

        mc_syntax_node *fps = field_syntax->field.function_pointer;
        copy_syntax_node_to_text(fps->function_pointer_declaration.identifier, &field->function_pointer.identifier);
        if (!fps->function_pointer_declaration.type_dereference)
          field->function_pointer.deref_count = 0;
        else
          field->function_pointer.deref_count =
              fps->function_pointer_declaration.type_dereference->dereference_sequence.count;

        // TODO -- more details
      } break;
      default: {
        MCerror(302, "NotSupported:%i", field_syntax->field.type);
      }
      }
    } break;
    case MC_SYNTAX_NESTED_TYPE_DECLARATION: {
      if (field_syntax->nested_type.declaration->type == MC_SYNTAX_UNION) {
        field->field_type = FIELD_KIND_NESTED_UNION;
      }
      else if (field_syntax->nested_type.declaration->type == MC_SYNTAX_STRUCTURE) {
        field->field_type = FIELD_KIND_NESTED_STRUCT;
      }
      else {
        MCerror(328, "Not Supported");
      }

      // allocate_and_copy_cstr(field->name, field_syntax->nested_type.name->text);
      register_sub_type_syntax_to_field_info(field_syntax->nested_type.declaration, field);

      if (field_syntax->nested_type.declarators) {
        field->sub_type.is_anonymous = false;

        if (field_syntax->nested_type.declarators) {
          summarize_field_declarator_list(field_syntax->nested_type.declarators, &field->sub_type.declarators);
        }
      }
      else {
        field->sub_type.is_anonymous = true;
      }
    } break;
    default: {
      MCerror(317, "NotSupported:%s", get_mc_syntax_token_type_name(field_syntax->type));
    }
    }

    register_midge_error_tag("summarize_type_field_list-2d");

    append_to_collection((void ***)&(*field_list)->items, &(*field_list)->alloc, &(*field_list)->count, field);
    register_midge_error_tag("summarize_type_field_list-2e");
  }

  return 0;
}

int register_sub_type_syntax_to_field_info(mc_syntax_node *subtype_syntax, field_info *field)
{
  // struct {
  //   bool is_union;
  //   char *type_name;
  //   struct {
  //     unsigned int alloc, count;
  //     field_info **items;
  //   } fields;
  //   struct {
  //     unsigned int alloc, count;
  //     field_declarator_info **items;
  //   } declarators;
  // } sub_type;

  // print_syntax_node(subtype_syntax, 0);
  if (subtype_syntax->type == MC_SYNTAX_UNION) {
    field->sub_type.is_union = true;

    if (subtype_syntax->union_decl.type_name) {
      copy_syntax_node_to_text(subtype_syntax->union_decl.type_name, &field->sub_type.type_name);
    }
    else {
      field->sub_type.type_name = NULL;
    }

    if (!subtype_syntax->union_decl.fields) {
      MCerror(298, "Unexpected?");
    }

    summarize_type_field_list(subtype_syntax->union_decl.fields, &field->sub_type.fields);
  }
  else if (subtype_syntax->type == MC_SYNTAX_STRUCTURE) {
    field->sub_type.is_union = false;

    if (subtype_syntax->structure.type_name) {
      copy_syntax_node_to_text(subtype_syntax->structure.type_name, &field->sub_type.type_name);
    }
    else {
      field->sub_type.type_name = NULL;
    }

    if (!subtype_syntax->structure.fields) {
      MCerror(396, "Unexpected?");
    }

    summarize_type_field_list(subtype_syntax->structure.fields, &field->sub_type.fields);
  }
  else {
    MCerror(283, "NotSupported:%s", get_mc_syntax_token_type_name(subtype_syntax->type));
  }

  return 0;
}

int update_or_register_struct_info_from_syntax(node *owner, mc_syntax_node *struct_ast, struct_info **p_struct_info)
{
  struct_info *structure_info;
  find_struct_info(struct_ast->structure.type_name->text, &structure_info);

  register_midge_error_tag("update_or_register_struct_info_from_syntax-1");
  if (!structure_info) {
    structure_info = (struct_info *)malloc(sizeof(struct_info));

    attach_struct_info_to_owner(owner, structure_info);

    structure_info->type_id = (struct_id *)malloc(sizeof(struct_id));
    allocate_and_copy_cstr(structure_info->type_id->identifier, "struct_info");
    structure_info->type_id->version = 1U;

    // Name & Version
    allocate_and_copy_cstr(structure_info->name, struct_ast->structure.type_name->text);
    structure_info->latest_iteration = 1U;
    structure_info->source = NULL;
  }
  else {
    free(structure_info->mc_declared_name);

    if (structure_info->is_defined) {
      // Free the field summaries
      release_field_info_list(structure_info->fields);
    }
  }
  register_midge_error_tag("update_or_register_struct_info_from_syntax-2");

  cprintf(structure_info->mc_declared_name, "%s_mc_v%u", structure_info->name, structure_info->latest_iteration);

  // Set the values parsed
  if (struct_ast->structure.fields) {
    structure_info->is_defined = true;

    summarize_type_field_list(struct_ast->structure.fields, &structure_info->fields);
  }
  else {
    structure_info->is_defined = false;
  }
  register_midge_error_tag("update_or_register_struct_info_from_syntax-4");

  // Set
  *p_struct_info = structure_info;

  return 0;
}

int update_or_register_enum_info_from_syntax(node *owner, mc_syntax_node *enum_ast, enumeration_info **p_enum_info)
{
  enumeration_info *enum_info;
  find_enumeration_info(enum_ast->enumeration.name->text, &enum_info);

  register_midge_error_tag("update_or_register_enum_info_from_syntax-1");
  char buf[256];
  if (!enum_info) {
    enum_info = (enumeration_info *)malloc(sizeof(enumeration_info));

    attach_enumeration_info_to_owner(owner, enum_info);

    enum_info->type_id = (struct_id *)malloc(sizeof(struct_id));
    allocate_and_copy_cstr(enum_info->type_id->identifier, "enum_info");
    enum_info->type_id->version = 1U;

    // Name & Version
    allocate_and_copy_cstr(enum_info->name, enum_ast->enumeration.name->text);
    enum_info->latest_iteration = 1U;

    enum_info->members.alloc = 0;
    enum_info->members.count = 0;
  }
  else {
    // Empty

    // Clear the current values
    for (int i = 0; i < enum_info->members.count; ++i) {
      sprintf(buf,
              "#ifdef %s\n"
              "#undef %s\n"
              "#endif\n",
              enum_info->members.items[i]->identity, enum_info->members.items[i]->identity);
      clint_process(buf);

      if (enum_info->members.items[i]) {
        if (enum_info->members.items[i]->identity) {
          free(enum_info->members.items[i]->identity);
        }
        if (enum_info->members.items[i]->value) {
          free(enum_info->members.items[i]->value);
        }
        free(enum_info->members.items[i]);
      }
    }

    ++enum_info->latest_iteration;
  }
  register_midge_error_tag("update_or_register_enum_info_from_syntax-2");

  cprintf(enum_info->mc_declared_name, "%s_mc_v%u", enum_info->name, enum_info->latest_iteration);

  // Set the values parsed
  enum_info->members.count = 0;
  int latest_value = -1;
  for (int i = 0; i < enum_ast->enumeration.members->count; ++i) {
    enum_member_info *member = (enum_member_info *)malloc(sizeof(enum_member_info));

    copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.identifier, &member->identity);
    if (enum_ast->enumeration.members->items[i]->enum_member.value_expression) {
      copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.value_expression, &member->value);

      sprintf(buf, "*(int *)(%p) = %s;", &latest_value, member->value); // TODO -- semi-colon
      clint_process(buf);
      sprintf(buf, "#define %s (%s)%i\n", member->identity, enum_info->mc_declared_name, latest_value);
    }
    else {
      member->value = NULL;
      ++latest_value;
      sprintf(buf, "#define %s (%s)%i\n", member->identity, enum_info->mc_declared_name, latest_value);
    }

    // printf("%s", buf);
    clint_process(buf);

    append_to_collection((void ***)&enum_info->members.items, &enum_info->members.alloc, &enum_info->members.count,
                         member);
  }

  // for (int b = 0; b < child->enumeration.members->count; ++b) {
  //   mc_syntax_node *enum_member = child->enumeration.members->items[b];
  //   sprintf(buf,
  //           "#ifdef %s\n"
  //           "#undef %s\n"
  //           "#endif\n"
  //           "#define %s %s\n",
  //           enum_member->enum_member.identifier->text, enum_member->enum_member.identifier->text,
  //           enum_member->enum_member.identifier->text, enum_member->enum_member.value->text);
  //   printf("%s", buf);

  //   return 0;

  // Set
  *p_enum_info = enum_info;

  return 0;
}

int instantiate_function_definition_from_ast(node *definition_owner, source_definition *source, mc_syntax_node *ast,
                                             void **definition_info)
{
  // Register Function
  function_info *func_info;
  update_or_register_function_info_from_syntax(definition_owner, ast, &func_info);

  // Instantiate Function
  char *mc_transcription;
  transcribe_function_to_mc(func_info, ast, &mc_transcription);

  if (!strcmp(func_info->name, "transcribe_function_to_mc")) {
    // print_syntax_node(ast, 0);
    printf("mc_transcription:\n%s||\n", mc_transcription);
  }
  if (!strcmp(func_info->name, "append_to_c_strf")) {
    // print_syntax_node(ast, 0);
    printf("mc_transcription:\n%s||\n", mc_transcription);
  }

  clint_declare(mc_transcription);
  free(mc_transcription);
  // printf("idfc-5\n");
  char buf[512];
  sprintf(buf, "%s = &%s_mc_v%u;", func_info->name, func_info->name, func_info->latest_iteration);
  // printf("idfc-6\n");
  clint_process(buf);

  // printf("idfc-7 %s_v%u\n", func_info->name, func_info->latest_iteration);
  // sprintf(buf,
  //         "{void *vargs[1];void *vargs0 = NULL;vargs[0] = &vargs0;%s(1, vargs);"
  //         "printf(\"addr of fptr:%%p\\n\", &%s);}",
  //         func_info->name, func_info->name);
  // clint_process(buf);
  // printf("idfc-8\n");

  if (definition_info) {
    *definition_info = func_info;
  }

  return 0;
}

int instantiate_struct_definition_from_ast(node *definition_owner, source_definition *source, mc_syntax_node *ast,
                                           void **definition_info)
{
  // Register Struct
  struct_info *structure_info;
  update_or_register_struct_info_from_syntax(definition_owner, ast, &structure_info);

  // Instantiate Struct
  char *mc_transcription;
  transcribe_struct_to_mc(structure_info, ast, &mc_transcription);

  // if (!strcmp(structure_info->name, "mc_syntax_node"))
  //   printf("struct:\n%s||\n", mc_transcription);
  clint_declare(mc_transcription);
  free(mc_transcription);

  if (definition_info) {
    *definition_info = structure_info;
  }

  register_midge_error_tag("instantiate_struct_definition_from_ast(~)");
  return 0;
}

int instantiate_enum_definition_from_ast(node *definition_owner, source_definition *source, mc_syntax_node *ast,
                                         void **definition_info)
{
  // Register enum
  enumeration_info *enum_info;
  update_or_register_enum_info_from_syntax(definition_owner, ast, &enum_info);

  char buf[256];
  sprintf(buf, "enum %s { };", enum_info->mc_declared_name);
  clint_declare(buf);

  if (definition_info) {
    *definition_info = enum_info;
  }

  register_midge_error_tag("instantiate_enum_definition_from_ast(~)");
  return 0;
}

int instantiate_define_statement(node *definition_owner, mc_syntax_node *ast, preprocess_define_info **info)
{
  switch (ast->preprocess_define.statement_type) {
  case PREPROCESSOR_DEFINE_REMOVAL: {
    *info = (preprocess_define_info *)malloc(sizeof(preprocess_define_info));
    (*info)->statement_type = ast->preprocess_define.statement_type;
    copy_syntax_node_to_text(ast->preprocess_define.identifier, &(*info)->identifier);

    (*info)->replacement = NULL;
  } break;
  case PREPROCESSOR_DEFINE_FUNCTION_LIKE: {
    // Do nothing...
  } break;
  case PREPROCESSOR_DEFINE_REPLACEMENT: {
    *info = (preprocess_define_info *)malloc(sizeof(preprocess_define_info));
    (*info)->statement_type = ast->preprocess_define.statement_type;
    copy_syntax_node_to_text(ast->preprocess_define.identifier, &(*info)->identifier);

    c_str *str;
    init_c_str(&str);
    for (int i = 0; i < ast->preprocess_define.replacement_list->count; ++i) {
      char *node_text;
      copy_syntax_node_to_text(ast->preprocess_define.replacement_list->items[i], &node_text);
      append_to_c_str(str, node_text);
      free(node_text);
    }

    (*info)->replacement = str->text;
    release_c_str(str, false);

    // printf("define:\n'%s'\n'%s'\n", (*info)->identifier, (*info)->replacement);
  } break;
  default:
    MCerror(830, "TODO :%i", ast->preprocess_define.statement_type);
  }

  return 0;
}

/*
  From code definition: constructs source definition & parses to syntax, registers with heirarchy, and declares the
  definition for immediate use.
  @definition_owner the node in the heirarchy to attach this definition to.
  @code may be NULL only if ast is not, if so it will be generated from the syntax parse.
  @ast may be NULL only if code is not, if so it will be parsed from the code.
  @source may be NULL, if so it will be created.
  @definition_info is OUT. May be NULL, if not dereference will be set with p-to-function_info/struct_info/enum_info
  etc.
*/
int instantiate_definition(node *definition_owner, char *code, mc_syntax_node *ast, source_definition *source,
                           void **definition_info)
{
  register_midge_error_tag("instantiate_definition()");
  // Compile Code to Syntax
  if (!ast) {
    parse_definition_to_syntax_tree(code, &ast);
  }
  else if (!code) {
    copy_syntax_node_to_text(ast, &code);
  }

  // TODO -- check type hasn't changed with definition
  if (!source) {
    source = (source_definition *)malloc(sizeof(source_definition));
    source->source_file = NULL;
  }
  source->code = code;

  void *p_definition_info;

  switch (ast->type) {
  case MC_SYNTAX_FUNCTION: {
    source->type = SOURCE_DEFINITION_FUNCTION;
    instantiate_function_definition_from_ast(definition_owner, source, ast, &p_definition_info);

    function_info *func_info = (function_info *)p_definition_info;
    func_info->source = source;
  } break;
  case MC_SYNTAX_STRUCTURE: {
    source->type = SOURCE_DEFINITION_STRUCT;
    instantiate_struct_definition_from_ast(definition_owner, source, ast, &p_definition_info);

    struct_info *structure_info = (struct_info *)p_definition_info;
    structure_info->source = source;
  } break;
  case MC_SYNTAX_ENUM: {
    source->type = SOURCE_DEFINITION_ENUMERATION;
    instantiate_enum_definition_from_ast(definition_owner, source, ast, &p_definition_info);

    enumeration_info *enum_info = (enumeration_info *)p_definition_info;
    enum_info->source = source;
  } break;
  default: {
    MCerror(325, "instantiate_definition:%i NotYetSupported", ast->type);
  }
  }

  source->data.p_data = p_definition_info;
  if (definition_info)
    *definition_info = p_definition_info;

  register_midge_error_tag("instantiate_definition(~)");
  return 0;
}

int instantiate_all_definitions_from_file(node *definitions_owner, char *filepath, source_file_info **source_file)
{
  char *file_text;
  read_file_text(filepath, &file_text);

  mc_syntax_node *syntax_node;
  parse_file_to_syntax_tree(file_text, &syntax_node);

  // Parse all definitions
  source_file_info *lv_source_file;
  initialize_source_file_info(definitions_owner, filepath, &lv_source_file);
  if (source_file) {
    *source_file = lv_source_file;
  }

  for (int a = 0; a < syntax_node->children->count; ++a) {
    mc_syntax_node *child = syntax_node->children->items[a];
    switch (child->type) {
    case MC_SYNTAX_EXTERN_C_BLOCK: {
      for (int b = 0; b < child->extern_block.declarations->count; ++b) {
        mc_syntax_node *declaration = child->extern_block.declarations->items[b];
        switch (declaration->type) {
        case MC_SYNTAX_FUNCTION: {
          if ((mc_token_type)declaration->function.code_block->type != MC_TOKEN_SEMI_COLON) {
            MCerror(565, "Full Function definition in an extern c block ? ? ?");
          }
          // Function Declaration only
          update_or_register_function_info_from_syntax(NULL, declaration, NULL);
        } break;
        default:
          MCerror(572, "TODO : %s", get_mc_syntax_token_type_name(declaration->type));
        }
      }
    } break;
    case MC_SYNTAX_FUNCTION: {
      if ((mc_token_type)child->function.code_block->type == MC_TOKEN_SEMI_COLON) {
        // Function Declaration only
        update_or_register_function_info_from_syntax(NULL, child, NULL);
        printf("--fdecl:'%s'\n", child->function.name->text);
      }
      else {
        // Assume to be function definition
        function_info *info;
        instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
        info->source->source_file = lv_source_file;
        printf("--defined:'%s'\n", child->function.name->text);
      }
    } break;
    case MC_SYNTAX_TYPE_ALIAS: {
      char buf[1024];
      switch (child->type_alias.type_descriptor->type) {
      case MC_SYNTAX_STRUCTURE: {
        struct_info *info;
        instantiate_definition(definitions_owner, NULL, child->type_alias.type_descriptor, NULL, (void **)&info);
        info->source->source_file = lv_source_file;
        // printf("--defined: struct '%s'\n", child->type_alias.type_descriptor->structure.type_name->text);
        // sprintf(buf,
        //         "#ifndef %s\n"
        //         // "#undef %s\n"
        //         "#define %s struct %s\n"
        //         "#endif\n",
        //         info->name, info->name, info->mc_declared_name);
        // clint_process(buf);
      } break;
      case MC_SYNTAX_ENUM: {
        enumeration_info *info;
        instantiate_definition(definitions_owner, NULL, child->type_alias.type_descriptor, NULL, (void **)&info);
        register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-0");
        info->source->source_file = lv_source_file;
        register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-1");
        // printf("--defined: enum '%s'\n", child->type_alias.type_descriptor->enumeration.name->text);
        register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-2");
        // sprintf(buf,
        //         "#ifndef %s\n"
        //         // "#undef %s\n"
        //         "#define %s enum %s\n"
        //         "#endif\n",
        //         info->name, info->name, info->mc_declared_name);
        // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-3");
        // clint_process(buf);
        register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-4");
      } break;
      default:
        print_syntax_node(child->type_alias.type_descriptor, 0);
        MCerror(668, "Unhandled type_alias-descriptor-syntax-type:%s",
                get_mc_syntax_token_type_name(child->type_alias.type_descriptor->type));
        break;
      }
    } break;
    case MC_SYNTAX_STRUCTURE: {
      struct_info *info;
      instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
      info->source->source_file = lv_source_file;
      // printf("--declared: struct '%s'\n", child->structure.type_name->text);
    } break;
    case MC_SYNTAX_ENUM: {
      enumeration_info *info;
      instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
      info->source->source_file = lv_source_file;
      // printf("--declared: enum '%s'\n", child->enumeration.name->text);
    } break;
    case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_DEFINE: {
      preprocess_define_info *info;
      instantiate_define_statement(definitions_owner, child, &info);

      // switch (info->statement_type) {
      // case PREPROCESSOR_DEFINE_REMOVAL: {

      // } break;

      // default:
      //   MCerror(887, "TODO :%i", info->statement_type);
      // }
    } break;
    // TODO
    case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_IFNDEF:
    case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_INCLUDE:
    case MC_TOKEN_PREPROCESSOR_KEYWORD_ENDIF:
      break;
    default: {
      switch ((mc_token_type)child->type) {
      case MC_TOKEN_SPACE_SEQUENCE:
      case MC_TOKEN_NEW_LINE:
      case MC_TOKEN_LINE_COMMENT:
      case MC_TOKEN_MULTI_LINE_COMMENT: {
        break;
      }
      default: {
        print_syntax_node(child, 0);
        MCerror(576, "Unhandled root-syntax-type:%s", get_mc_syntax_token_type_name(child->type));
      }
      }
    }
    }
  }

  // int *p = 0;
  // printf("about\n");
  // printf("%i\n", *p);
  printf("end\n");
  register_midge_error_tag("instantiate_all_definitions_from_file(~)");
  return 0;
}