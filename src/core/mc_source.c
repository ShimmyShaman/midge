#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "midge_common.h"

int attach_function_info_to_owner(node *owner, function_info *func_info)
{
  switch (owner->type) {
  case NODE_TYPE_GLOBAL_ROOT: {
    global_root_data *data = (global_root_data *)owner->data;
    MCcall(append_to_collection((void ***)&data->functions.items, &data->functions.alloc, &data->functions.count,
                                (void *)func_info));
    break;
  }
  default:
    MCerror(9, "TODO:%i", owner->type);
  }

  return 0;
}

int initialize_parameter_info_from_syntax_node(mc_syntax_node *parameter_syntax_node,
                                               parameter_info **initialized_parameter)
{
  parameter_info *parameter = (parameter_info *)malloc(sizeof(parameter_info));
  parameter->type_id = (struct_id *)malloc(sizeof(mc_struct_id));
  allocate_and_copy_cstr(parameter->type_id->identifier, "parameter_info");
  parameter->type_id->version = 1U;

  if (parameter_syntax_node->parameter.is_function_pointer_declaration) {
    parameter->is_function_pointer = true;

    MCcall(copy_syntax_node_to_text(parameter_syntax_node, &parameter->full_function_pointer_declaration));

    mc_syntax_node *fpsn = parameter_syntax_node->parameter.function_pointer_declaration;
    MCcall(copy_syntax_node_to_text(fpsn->function_pointer_declaration.identifier, (char **)&parameter->name));

    if (fpsn->function_pointer_declaration.type_dereference) {
      parameter->type_deref_count = fpsn->function_pointer_declaration.type_dereference->dereference_sequence.count;
    }
    else {
      parameter->type_deref_count = 0;
    }

    // void *ptr = 0;
    // int (*fptr)(int, void **) = (int (*)(int, void **))ptr;
    parameter->function_type = NULL;
  }
  else {
    parameter->is_function_pointer = false;

    // Type
    MCcall(copy_syntax_node_to_text(parameter_syntax_node->parameter.type_identifier, (char **)&parameter->type_name));
    if (parameter_syntax_node->parameter.type_dereference) {
      parameter->type_deref_count = parameter_syntax_node->parameter.type_dereference->dereference_sequence.count;
    }
    else {
      parameter->type_deref_count = 0;
    }
    // printf("parameter->type_deref_count:%i\n", parameter->type_deref_count);
    register_midge_error_tag("parse_and_process_mc_file_syntax-3b");

    // -- TODO -- mc-type?

    // Name
    MCcall(copy_syntax_node_to_text(parameter_syntax_node->parameter.name, (char **)&parameter->name));
  }

  *initialized_parameter = parameter;
  return 0;
}

int update_or_register_function_info_from_syntax(node *owner, mc_syntax_node *function_ast, function_info **p_func_info)
{
  function_info *func_info;
  MCcall(find_function_info(owner, function_ast->function.name->text, &func_info));

  register_midge_error_tag("update_or_register_function_info_from_syntax-1");
  if (!func_info) {
    func_info = (function_info *)malloc(sizeof(function_info));

    MCcall(attach_function_info_to_owner(owner, func_info));

    func_info->type_id = (struct_id *)malloc(sizeof(struct_id));
    allocate_and_copy_cstr(func_info->type_id->identifier, "function_info");
    func_info->type_id->version = 1U;

    // Name & Version
    allocate_and_copy_cstr(func_info->name, function_ast->function.name->text);
    func_info->latest_iteration = 1U;

    // Declare the functions pointer with cling
    char buf[512];
    sprintf(buf, "int (*%s)(int, void **);", func_info->name);
    MCcall(clint_declare(buf));

    sprintf(buf, "*((void **)%p) = (void *)&%s;", &func_info->ptr_declaration, func_info->name);
    MCcall(clint_process(buf));
    // printf("func_info->ptr_declaration:%p\n", func_info->ptr_declaration);
  }
  else {
    // Empty
  }
  register_midge_error_tag("update_or_register_function_info_from_syntax-2");

  // Return-type & Parameters
  MCcall(copy_syntax_node_to_text(function_ast->function.return_type_identifier, &func_info->return_type.name));
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
    MCcall(initialize_parameter_info_from_syntax_node(function_ast->function.parameters->items[p], &parameter));
    func_info->parameters[p] = parameter;
  }

  // TODO
  func_info->variable_parameter_begin_index = -1;
  func_info->struct_usage_count = 0;
  func_info->struct_usage = NULL;

  // Set
  *p_func_info = func_info;
}

int update_or_register_enum_info_from_syntax(node *owner, mc_syntax_node *enum_ast, function_info **p_enum_info)
{
  enumeration_info *enum_info;
  MCcall(find_enum_info(owner, enum_ast->enumeration.name, &enum_info));

  register_midge_error_tag("update_or_register_enum_info_from_syntax-1");
  if (!enum_info) {
    enum_info = (enumeration_info *)malloc(sizeof(enumeration_info));

    MCcall(attach_enumeration_info_to_owner(owner, enum_info));

    enum_info->type_id = (struct_id *)malloc(sizeof(struct_id));
    allocate_and_copy_cstr(enum_info->type_id->identifier, "enum_info");
    enum_info->type_id->version = 1U;

    // Name & Version
    allocate_and_copy_cstr(enum_info->name, enum_ast->enumeration.name->text);
    enum_info->latest_iteration = 1U;
  }
  else {
    // Empty

    // Clear the current values
    for (int i = 0; i < enum_info->members.count; ++i) {
      if (enum_info->members.items[i]) {
        if (enum_info->members.items[i]->identity) {
          free(enum_info->members.items[i]->identity);
        }
        free(enum_info->members.items[i]);
      }
    }

    ++enum_info->latest_iteration;
  }
  register_midge_error_tag("update_or_register_enum_info_from_syntax-2");

  // Set the values parsed
  enum_info->members.count = 0;
  for (int i = 0; i < enum_ast->enumeration.members->count; ++i) {
    enum_member *enum_member = (enum_member *)malloc(sizeof(enum_member));
    MCcall(
        allocate_and_copy_cstr(enum_member->identity, enum_ast->enumeration.members->items[i]->enum_member.identifier));
    if (enum_ast->enumeration.members->items[i]->enum_member.value) {
      MCcall(copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.value, enum_member->value));
    }
    else {
      enum_member->value = NULL;
    }

    MCcall(append_to_collection((void ***)&enum_info->members.items, &enum_info->members.alloc,
                                &enum_info->members.count, enum_member));
  }

  // Set
  *p_enum_info = enum_info;
}

int instantiate_function_definition_from_parsed_code(node *definition_owner, char *code, mc_syntax_node *enum_ast,
                                                     void **definition_info)
{
  // Register Enumeration
  enumeration_info *enum_info;
  MCcall(update_or_register_enum_info_from_syntax(definition_owner, enum_ast, &enum_info));

  enum_info->source = (source_definition *)malloc(sizeof(source_definition));
  enum_info->source->type = SOURCE_DEFINITION_FUNCTION;
  enum_info->source->source_file = NULL;
  enum_info->source->code = code;
  enum_info->source->data.enum_info = enum_info;

  // Instantiate Function
  char *mc_transcription;
  MCcall(transcribe_enumeration_to_mc(enum_info, enum_ast, &mc_transcription));

  MCcall(clint_declare(mc_transcription));

  if (definition_info) {
    *definition_info = enum_info;
  }
}

int instantiate_enum_definition_from_parsed_code(node *definition_owner, source_definition *source,
                                                 mc_syntax_node *enum_ast, void **definition_info)
{
  // Register Enumeration
  enumeration_info *enum_info;
  MCcall(update_or_register_enum_info_from_syntax(definition_owner, enum_ast, &enum_info));

  // Instantiate Function
  char *mc_transcription;
  MCcall(transcribe_enumeration_to_mc(enum_info, enum_ast, &mc_transcription));

  MCcall(clint_declare(mc_transcription));

  if (definition_info) {
    *definition_info = enum_info;
  }
}

int instantiate_function_definition_from_ast(node *definition_owner, source_definition *source, mc_syntax_node *ast,
                                             void **definition_info)
{
  // Register Function
  function_info *func_info;
  MCcall(update_or_register_function_info_from_syntax(definition_owner, ast, &func_info));

  // Instantiate Function
  char *mc_transcription;
  MCcall(transcribe_function_to_mc(func_info, ast, &mc_transcription));

  MCcall(clint_declare(mc_transcription));
  // printf("idfc-5\n");
  char buf[512];
  sprintf(buf, "%s = &%s_v%u;", func_info->name, func_info->name, func_info->latest_iteration);
  // printf("idfc-6\n");
  MCcall(clint_process(buf));

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
}

/*
  From code definition: constructs source definition & parses to syntax, registers with heirarchy, and declares the
  definition for immediate use.
  @definition_owner the node in the heirarchy to attach this definition to.
  @code may be NULL only if ast is not, if so it will be generated from the syntax parse.
  @ast may be NULL only if code is not, if so it will be parsed from the code.
  @source may be NULL, if so it will be created.
  @definition_info is OUT. Will be set with function_info/struct_info/enum_info etc.
*/
int instantiate_definition(node *definition_owner, char *code, mc_syntax_node *ast, source_definition *source,
                           void **definition_info)
{
  // Compile Code to Syntax
  if (!ast) {
    MCcall(parse_definition_to_syntax_tree(code, &ast));
  }
  else if (!code) {
    MCcall(copy_syntax_node_to_text(ast, code));
  }

  // TODO -- check type hasn't changed with definition
  if (!source) {
    source = (source_definition *)malloc(sizeof(source_definition));
    source->source_file = NULL;
  }
  source->code = code;

  switch (ast->type) {
  case MC_SYNTAX_FUNCTION:
    source->type = SOURCE_DEFINITION_FUNCTION;
    MCcall(instantiate_function_definition_from_ast(definition_owner, source, ast, definition_info));
    break;
  case MC_SYNTAX_ENUM: {
    source->type = SOURCE_DEFINITION_ENUMERATION;
    MCcall(instantiate_enum_definition_from_ast(definition_owner, source, ast, definition_info));
  } break;
  default: {
    printf("only functions supported atm\n");
    return;
  }
  }

  source->data.p_data = *definition_info;
}

int instantiate_all_definitions_from_file(node *definitions_owner, char *filepath, source_file_info **source_file)
{
  char *file_text;
  MCcall(read_file_text(filepath, &file_text));

  mc_syntax_node *syntax_node;
  MCcall(parse_mc_file_to_syntax_tree(file_text, &syntax_node));

  // Parse all definitions
  *source_file = (source_file_info *)malloc(sizeof(source_file_info));
  MCcall(append_to_collection((void ***)&definitions_owner->source_files.items, &definitions_owner->source_files.alloc,
                              &definitions_owner->source_files.count, *source_file));
  allocate_and_copy_cstr((*source_file)->filepath, filepath);
  (*source_file)->definitions.alloc = 0;
  (*source_file)->definitions.count = 0;

  for (int a = 0; a < syntax_node->children->count; ++a) {
    mc_syntax_node *child = syntax_node->children->items[a];
    switch (child->type) {
    case MC_SYNTAX_PREPROCESSOR_DIRECTIVE: {
      break;
    }
    case MC_SYNTAX_FUNCTION: {
      function_info *finfo;
      MCcall(instantiate_definition(definitions_owner, NULL, child, NULL, &finfo));
      finfo->source->source_file = *source_file;
    } break;
    case MC_SYNTAX_STRUCTURE: {
      struct_info *finfo;
      MCcall(instantiate_definition(definitions_owner, NULL, child, NULL, &finfo));
      finfo->source->source_file = *source_file;
    } break;
    case MC_SYNTAX_ENUM: {
      enumeration_info *finfo;
      MCcall(instantiate_definition(definitions_owner, NULL, child, NULL, &finfo));
      finfo->source->source_file = *source_file;
    } break;
    default: {
      switch ((mc_token_type)child->type) {
      case MC_TOKEN_SPACE_SEQUENCE:
      case MC_TOKEN_NEW_LINE:
      case MC_TOKEN_LINE_COMMENT:
      case MC_TOKEN_MULTI_LINE_COMMENT: {
        break;
      }
      default: {
        MCerror(6030, "Unhandled root-syntax-type:%i", child->type);
      }
      }
    }
    }
  }
  return 0;
}