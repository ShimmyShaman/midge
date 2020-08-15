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
    find_function_info(func_info, command_hub->global_node, function_ast->function.name);
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
    char *cling_declaration;
    cprintf(cling_declaration, "int (*%s)(int, void **);", func_info->name);
    clint_declare(cling_declaration);
    free(cling_declaration);

    char *addr_cmd;
    cprintf(addr_cmd, "*((void **)%p) = *(void **)&%s;", &func_info->ptr_declaration, func_info->name);
    clint_process(addr_cmd);
    free(addr_cmd);
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

/*
  From code definition: parses to syntax, registers with heirarchy, and declares the definition for immediate use.
*/
void instantiate_definition_from_code(node *definition_owner, char *code, void **definition_info)
{
  // Compile Code to Syntax
  mc_syntax_node *ast;
  parse_definition_to_syntax_tree(code, &ast);

  if (ast->type != MC_SYNTAX_FUNCTION) {
    printf("only functions supported atm\n");
    return;
  }

  // Register Function
  mc_function_info_v1 *func_info;
  update_or_register_function_info_from_syntax(definition_owner, ast, &func_info);

  func_info->source = (mc_source_definition_v1 *)malloc(sizeof(mc_source_definition_v1));
  func_info->source->type = SOURCE_DEFINITION_FUNCTION;
  func_info->source->source_file = NULL;
  func_info->source->code = code;
  func_info->source->func_info = func_info;

  // Instantiate Function
  char *mc_transcription;
  transcribe_function_to_mc(func_info, ast, &mc_transcription);

  clint_declare(mc_transcription);

  if (*definition_info) {
    *definition_info = func_info;
  }
}