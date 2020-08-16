#include "core/midge_core.h"

void update_or_register_function_info_from_syntax(node *owner, mc_syntax_node *function_ast,
                                                  mc_function_info_v1 **p_func_info)
{
  mc_function_info_v1 *func_info;
  {
    // void *vargs[3];
    // vargs[0] = &func_info;
    // vargs[1] = &command_hub->global_node; // TODO atm
    // vargs[2] = &function_ast->function.name;
    // find_function_info(3, vargs);
    find_function_info(&func_info, command_hub->global_node, function_ast->function.name);
  }

  register_midge_error_tag("update_or_register_function_info_from_syntax-1");
  if (!func_info) {
    func_info = (mc_function_info_v1 *)malloc(sizeof(mc_function_info_v1));
    append_to_collection((void ***)&owner->functions, &owner->functions_alloc, &owner->function_count,
                         (void *)func_info);

    func_info->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
    allocate_and_copy_cstr(func_info->struct_id->identifier, "function_info");
    func_info->struct_id->version = 1U;

    // Name & Version
    allocate_and_copy_cstr(func_info->name, function_ast->function.name->text);
    func_info->latest_iteration = 1U;

    // Declare the functions pointer with cling
    char buf[512];
    sprintf(buf, "int (*%s)(int, void **);", func_info->name);
    clint_declare(buf);

    sprintf(buf, "*((void **)%p) = (void *)&%s;", &func_info->ptr_declaration, func_info->name);
    clint_process(buf);
    // printf("func_info->ptr_declaration:%p\n", func_info->ptr_declaration);
  }
  else {
    // Empty
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
  func_info->parameters = (mc_parameter_info_v1 **)malloc(sizeof(mc_parameter_info_v1 *) * func_info->parameter_count);
  for (int p = 0; p < func_info->parameter_count; ++p) {
    mc_parameter_info_v1 *parameter;
    initialize_parameter_info_from_syntax_node(function_ast->function.parameters->items[p], &parameter);
    func_info->parameters[p] = parameter;
  }

  // TODO
  func_info->variable_parameter_begin_index = -1;
  func_info->struct_usage_count = 0;
  func_info->struct_usage = NULL;

  // Set
  *p_func_info = func_info;
}

void update_or_register_enum_info_from_syntax(node *owner, mc_syntax_node *enum_ast, mc_function_info_v1 **p_enum_info)
{
  enumeration_info *enum_info;
  {
    find_enum_info(command_hub->global_node, enum_ast->enumeration.name, &enum_info);
  }

  register_midge_error_tag("update_or_register_enum_info_from_syntax-1");
  if (!enum_info) {
    enum_info = (enumeration_info *)malloc(sizeof(enumeration_info));
    append_to_collection((void ***)&owner->enums, &owner->enums_alloc, &owner->enum_count, (void *)enum_info);

    enum_info->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
    allocate_and_copy_cstr(enum_info->struct_id->identifier, "enum_info");
    enum_info->struct_id->version = 1U;

    // Name & Version
    allocate_and_copy_cstr(enum_info->name, enum_ast->enumeration.name->text);
    enum_info->latest_iteration = 1U;
  }
  else {
    // Empty

    // Clear the current values
    for (int i = 0; i < enum_info->members.count; ++i) {
      if (enum_info->members[i]) {
        if (enum_info->members[i].identity) {
          free(enum_info->members[i].identity);
        }
        free(enum_info->members[i]);
      }
    }

    ++enum_info->latest_iteration;
  }
  register_midge_error_tag("update_or_register_enum_info_from_syntax-2");

  // Set the values parsed
  enum_info->members.count = 0;
  for (int i = 0; i < enum_ast->enumeration.members->count; ++i) {
    mc_enum_member_v1 *enum_member = (mc_enum_member_v1 *)malloc(sizeof(mc_enum_member_v1));
    allocate_and_copy_cstr(enum_member->identity, enum_ast->enumeration.members->items[i]->enum_member.identifier);
    if (enum_ast->enumeration.members->items[i]->enum_member.value) {
      copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.value, enum_member->value);
    }
    else {
      enum_member->value = NULL;
    }

    append_to_collection((void ***)&enum_info->members.items, &enum_info->members.alloc, &enum_info->members.count,
                         enum_member);
  }

  // Set
  *p_enum_info = enum_info;
}

void instantiate_function_definition_from_parsed_code(node *definition_owner, char *code, mc_syntax_node *enum_ast,
                                                      void **definition_info)
{
  // Register Enumeration
  mc_enumeration_info_v1 *enum_info;
  update_or_register_enum_info_from_syntax(definition_owner, enum_ast, &enum_info);

  enum_info->source = (mc_source_definition_v1 *)malloc(sizeof(mc_source_definition_v1));
  enum_info->source->type = SOURCE_DEFINITION_FUNCTION;
  enum_info->source->source_file = NULL;
  enum_info->source->code = code;
  enum_info->source->data.enum_info = enum_info;

  // Instantiate Function
  char *mc_transcription;
  transcribe_enumeration_to_mc(enum_info, enum_ast, &mc_transcription);

  clint_declare(mc_transcription);

  if (definition_info) {
    *definition_info = enum_info;
  }
}

void instantiate_enum_definition_from_parsed_code(node *definition_owner, mc_source_definition_v1 *source,
                                                  mc_syntax_node *enum_ast, void **definition_info)
{
  // Register Enumeration
  mc_enumeration_info_v1 *enum_info;
  update_or_register_enum_info_from_syntax(definition_owner, enum_ast, &enum_info);

  // Instantiate Function
  char *mc_transcription;
  transcribe_enumeration_to_mc(enum_info, enum_ast, &mc_transcription);

  clint_declare(mc_transcription);

  if (definition_info) {
    *definition_info = enum_info;
  }
}

void instantiate_function_definition_from_ast(node *definition_owner, mc_source_definition_v1 *source,
                                              mc_syntax_node *ast, void **definition_info)
{
  // Register Function
  mc_function_info_v1 *func_info;
  update_or_register_function_info_from_syntax(definition_owner, ast, &func_info);

  // Instantiate Function
  char *mc_transcription;
  transcribe_function_to_mc(func_info, ast, &mc_transcription);

  clint_declare(mc_transcription);
  // printf("idfc-5\n");
  char buf[512];
  sprintf(buf, "%s = &%s_v%u;", func_info->name, func_info->name, func_info->latest_iteration);
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
void instantiate_definition(node *definition_owner, char *code, mc_syntax_node *ast, mc_source_definition_v1 *source,
                            void **definition_info)
{
  // Compile Code to Syntax
  if (!ast) {
    parse_definition_to_syntax_tree(code, &ast);
  }
  else if (!code) {
    copy_syntax_node_to_text(ast, code);
  }

  // TODO -- check type hasn't changed with definition
  if (!source) {
    source = (mc_source_definition_v1 *)malloc(sizeof(mc_source_definition_v1));
    source->source_file = NULL;
  }
  source->code = code;

  switch (ast->type) {
  case MC_SYNTAX_FUNCTION:
    source->type = SOURCE_DEFINITION_FUNCTION;
    instantiate_function_definition_from_ast(definition_owner, source, ast, definition_info);
    break;
  case MC_SYNTAX_ENUM: {
    source->type = SOURCE_DEFINITION_ENUMERATION;
    instantiate_enum_definition_from_ast(definition_owner, source, ast, definition_info);
  } break;
  default: {
    printf("only functions supported atm\n");
    return;
  }
  }

  source->data.p_data = *definition_info;
}