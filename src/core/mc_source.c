
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <sys/stat.h>
// #include <unistd.h>

#include "tinycc/libtccinterp.h"

#include "midge_common.h"
#include "midge_error_handling.h"

#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
// #include "core/mc_code_transcription.h"

int register_sub_type_syntax_to_field_info(mc_syntax_node *subtype_syntax, field_info *field);

int attach_function_info_to_owner(function_info *func_info)
{
  mc_global_data *data;
  obtain_midge_global_root(&data);

  append_to_collection((void ***)&data->functions.items, &data->functions.alloc, &data->functions.count,
                       (void *)func_info);

  return 0;
}

int attach_struct_info_to_owner(struct_info *structure_info)
{
  mc_global_data *data;
  obtain_midge_global_root(&data);

  append_to_collection((void ***)&data->structs.items, &data->structs.alloc, &data->structs.count,
                       (void *)structure_info);

  return 0;
}

int attach_enumeration_info_to_owner(enumeration_info *enum_info)
{
  mc_global_data *data;
  obtain_midge_global_root(&data);

  append_to_collection((void ***)&data->enumerations.items, &data->enumerations.alloc, &data->enumerations.count,
                       (void *)enum_info);

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

    // Name
    copy_syntax_node_to_text(parameter_syntax_node->parameter.name, (char **)&parameter->name);

    // Type
    copy_syntax_node_to_text(parameter_syntax_node->parameter.type_identifier, (char **)&parameter->type_name);
    if (parameter_syntax_node->parameter.type_dereference) {
      parameter->type_deref_count = parameter_syntax_node->parameter.type_dereference->dereference_sequence.count;
    }
    else {
      parameter->type_deref_count = 0;
    }

  } break;
  case PARAMETER_KIND_FUNCTION_POINTER: {
    register_midge_error_tag("initialize_parameter_info_from_syntax_node-FUNCTION_POINTER");
    parameter->parameter_type = PARAMETER_KIND_FUNCTION_POINTER;

    // Name
    copy_syntax_node_to_text(parameter_syntax_node->parameter.function_pointer->fptr_declarator.name, &parameter->name);

    // Type
    copy_syntax_node_to_text(parameter_syntax_node->parameter.type_identifier, (char **)&parameter->return_type);
    if (parameter_syntax_node->parameter.type_dereference) {
      parameter->return_deref_count = parameter_syntax_node->parameter.type_dereference->dereference_sequence.count;
    }
    else {
      parameter->return_deref_count = 0;
    }

    // print_syntax_node(parameter_syntax_node, 0);
  } break;
  case PARAMETER_KIND_VARIABLE_ARGS: {
    register_midge_error_tag("initialize_parameter_info_from_syntax_node-VARIABLE_ARGS");
    parameter->parameter_type = PARAMETER_KIND_VARIABLE_ARGS;

    parameter->type_name = NULL;
    parameter->type_version = 0;
    parameter->type_deref_count = 0;
    parameter->name = NULL;

  } break;
  default:
    MCerror(125, "NotSupported:%i", parameter_syntax_node->parameter.type);
  }

  *initialized_parameter = parameter;
  return 0;
}

int mcs_summarize_field_declarator_list(mc_syntax_node_list *syntax_declarators,
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
    if (declarator_syntax->field_declarator.function_pointer) {
      // Function pointer
      // print_syntax_node(declarator_syntax->field_declarator.function_pointer, 0);
      copy_syntax_node_to_text(declarator_syntax->field_declarator.function_pointer->fptr_declarator.name,
                               &declarator->function_pointer.identifier);
      if (declarator_syntax->field_declarator.function_pointer->fptr_declarator.fp_dereference) {
        declarator->function_pointer.fp_deref_count = declarator_syntax->field_declarator.function_pointer
                                                          ->fptr_declarator.fp_dereference->dereference_sequence.count;
      }
      else {
        declarator->function_pointer.fp_deref_count = 0;
      }

      // declarator_syntax->field_declarator.function_pointer->function_pointer.parameters
      // TODO -- parameters
    }
    else {
      copy_syntax_node_to_text(declarator_syntax->field_declarator.name, &declarator->name);
    }
    if (declarator_syntax->field_declarator.type_dereference) {
      declarator->deref_count = declarator_syntax->field_declarator.type_dereference->dereference_sequence.count;
    }
    else {
      declarator->deref_count = 0;
    }

    declarator->is_array = (declarator_syntax->field_declarator.array_size ? true : false);

    append_to_collection((void ***)&(*field_declarators_list)->items, &(*field_declarators_list)->alloc,
                         &(*field_declarators_list)->count, declarator);
  }
  // case FIELD_KIND_FUNCTION_POINTER: {
  //   field->field_type = FIELD_KIND_FUNCTION_POINTER;

  //   mc_syntax_node *fps = field_syntax->field.function_pointer;
  //   copy_syntax_node_to_text(fps->function_pointer_declaration.identifier, &field->function_pointer.identifier);
  //   if (!fps->function_pointer_declaration.type_dereference)
  //     field->function_pointer.deref_count = 0;
  //   else
  //     field->function_pointer.deref_count =
  //         fps->function_pointer_declaration.type_dereference->dereference_sequence.count;

  //   // TODO -- more details
  // } break;

  return 0;
}

int mcs_summarize_type_field_list(mc_syntax_node_list *field_syntax_list, field_info_list **field_list)
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

    register_midge_error_tag("mcs_summarize_type_field_list-2a");
    switch (field_syntax->type) {
    case MC_SYNTAX_FIELD_DECLARATION: {
      switch (field_syntax->field.type) {
      case FIELD_KIND_STANDARD: {
        field->field_type = FIELD_KIND_STANDARD;
        copy_syntax_node_to_text(field_syntax->field.type_identifier, &field->field.type_name);

        mcs_summarize_field_declarator_list(field_syntax->field.declarators, &field->field.declarators);
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

        mcs_summarize_field_declarator_list(field_syntax->nested_type.declarators, &field->sub_type.declarators);
      }
      else {
        field->sub_type.is_anonymous = !field->sub_type.type_name;
      }
    } break;
    default: {
      MCerror(317, "NotSupported:%s", get_mc_syntax_token_type_name(field_syntax->type));
    }
    }

    register_midge_error_tag("mcs_summarize_type_field_list-2d");

    append_to_collection((void ***)&(*field_list)->items, &(*field_list)->alloc, &(*field_list)->count, field);
    register_midge_error_tag("mcs_summarize_type_field_list-2e");
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
      printf("sub-type:'%s'\n", field->sub_type.type_name);
    }
    else {
      field->sub_type.type_name = NULL;
    }

    if (!subtype_syntax->union_decl.fields) {
      MCerror(298, "Unexpected?");
    }

    mcs_summarize_type_field_list(subtype_syntax->union_decl.fields, &field->sub_type.fields);
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

    mcs_summarize_type_field_list(subtype_syntax->structure.fields, &field->sub_type.fields);
  }
  else {
    MCerror(283, "NotSupported:%s", get_mc_syntax_token_type_name(subtype_syntax->type));
  }

  return 0;
}

int mcs_register_function_declaration(mc_syntax_node *function_ast, function_info **p_func_info)
{
  // Ensure a function info entry exists for the declared function
  function_info *func_info;
  find_function_info(function_ast->function.name->text, &func_info);
  if (func_info) {
    // TODO -- argument equity check
    return 0;
  }

  func_info = (function_info *)calloc(1, sizeof(function_info));
  attach_function_info_to_owner(func_info);

  func_info->name = strdup(function_ast->function.name->text);

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

  return 0;
}

int mcs_register_function_definition(mc_syntax_node *function_ast, function_info **p_func_info)
{
  function_info *func_info;
  mcs_register_function_declaration(function_ast, &func_info);

  MCerror(241, "TODO");

  //   function_info *func_info;
  //   find_function_info(function_ast->function.name->text, &func_info);

  //   bool is_declaration_only = (mc_token_type)function_ast->function.code_block->type == MC_TOKEN_SEMI_COLON;

  //   register_midge_error_tag("update_or_register_function_info_from_syntax-1");
  //   if (!func_info) {
  //     func_info = (function_info *)malloc(sizeof(function_info));

  //     if (is_declaration_only) {
  //       // Attach to loose declarations
  //       mc_global_data *root_data;
  //       obtain_midge_global_root(&root_data);

  //       append_to_collection((void ***)&root_data->function_declarations.items,
  //       &root_data->function_declarations.alloc,
  //                            &root_data->function_declarations.count, func_info);
  //     }
  //     else {
  //       // Only attach definitions, not declarations
  //       if (!owner) {
  //         MCerror(162, "owner can only be NULL for a function declaration");
  //       }

  //       attach_function_info_to_owner(owner, func_info);
  //     }

  //     func_info->type_id = (struct_id *)malloc(sizeof(struct_id));
  //     allocate_and_copy_cstr(func_info->type_id->identifier, "function_info");
  //     func_info->type_id->version = 1U;

  //     // Name & Version
  //     allocate_and_copy_cstr(func_info->name, function_ast->function.name->text);
  //     func_info->latest_iteration = is_declaration_only ? 0U : 1U;

  //     // Declare the functions pointer with cling
  //     // printf("--attempting:'%s'\n", func_info->name);
  //     // char buf[512];
  //     // MCerror(8211, "progress");
  //     // func_info->ptr_declaration = mcc_set_global_symbol(func_info->name, NULL);
  //     // sprintf(buf, "int (*%s)(int, void **);", func_info->name);
  //     // clint_declare(buf);
  //     // sprintf(buf, "%s = (int (*)(int, void **))0;", func_info->name);
  //     // clint_process(buf);
  //     // printf("--declared:'%s()'\n", func_info->name);

  //     // Assign the functions pointer
  //     // sprintf(buf, "*((void **)%p) = (void *)&%s;", &func_info->ptr_declaration, func_info->name);
  //     // clint_process(buf);
  //     // printf("func_info->ptr_declaration:%p\n", func_info->ptr_declaration);
  //   }
  //   else {
  //     if (!is_declaration_only && func_info->latest_iteration < 1) {
  //       // Function has only been declared, not defined
  //       // Remove from loose declarations
  //       mc_global_data *root_data;
  //       obtain_midge_global_root(&root_data);
  //       remove_ptr_from_collection((void ***)&root_data->function_declarations.items,
  //                                  &root_data->function_declarations.count, true, func_info);

  //       // Attach to owner
  //       func_info->latest_iteration = 1U;
  //       attach_function_info_to_owner(owner, func_info);
  //     }

  //     // TODO -- this was causing a segmentation fault or something - TODO
  //     // if (func_info->return_type.name) {
  //     //   free(func_info->return_type.name);
  //     // }

  //     // Free parameters -- allow them to be changed
  //     // register_midge_error_tag("update_or_register_function_info_from_syntax-1a");
  //     // if (func_info->parameter_count) {
  //     //   for (int a = 0; a < func_info->parameter_count; ++a) {
  //     //     if (func_info->parameters[a]) {
  //     //       release_parameter_info(func_info->parameters[a]);
  //     //       func_info->parameters[a] = NULL;
  //     //     }
  //     //   }
  //     // }
  //     func_info->parameter_count = 0;
  //     register_midge_error_tag("update_or_register_function_info_from_syntax-1b");
  //   }
  //   register_midge_error_tag("update_or_register_function_info_from_syntax-2");

  //   // Return-type & Parameters
  //   copy_syntax_node_to_text(function_ast->function.return_type_identifier, &func_info->return_type.name);
  //   if (function_ast->function.return_type_dereference) {
  //     func_info->return_type.deref_count =
  //     function_ast->function.return_type_dereference->dereference_sequence.count;
  //   }
  //   else {
  //     func_info->return_type.deref_count = 0;
  //   }

  //   register_midge_error_tag("update_or_register_function_info_from_syntax-3");
  //   func_info->parameter_count = function_ast->function.parameters->count;
  //   func_info->parameters = (parameter_info **)malloc(sizeof(parameter_info *) * func_info->parameter_count);
  //   for (int p = 0; p < func_info->parameter_count; ++p) {
  //     parameter_info *parameter;
  //     initialize_parameter_info_from_syntax_node(function_ast->function.parameters->items[p], &parameter);
  //     func_info->parameters[p] = parameter;
  //   }

  //   // TODO
  //   func_info->variable_parameter_begin_index = -1;
  //   func_info->struct_usage_count = 0;
  //   func_info->struct_usage = NULL;

  //   // Set
  //   if (p_func_info)
  //     *p_func_info = func_info;

  return 0;
}

int mcs_register_struct_declaration(mc_syntax_node *struct_ast)
{
  if (!struct_ast->structure.type_name) {
    MCerror(8461, "root-level anonymous external structures not yet supported");
  }

  struct_info *structure_info;
  find_struct_info(struct_ast->structure.type_name->text, &structure_info);
  if (structure_info) {
    MCerror(1013, "TODO?");
  }

  // register_midge_error_tag("update_or_register_struct_info_from_syntax-1");
  if (!structure_info) {
    structure_info = (struct_info *)malloc(sizeof(struct_info));

    attach_struct_info_to_owner(structure_info);

    structure_info->type_id = (struct_id *)malloc(sizeof(struct_id));
    allocate_and_copy_cstr(structure_info->type_id->identifier, "struct_info");
    structure_info->type_id->version = 1U;

    // Name & Version
    if (struct_ast->structure.type_name) {
      allocate_and_copy_cstr(structure_info->name, struct_ast->structure.type_name->text);
    }
    else {
      structure_info->name = NULL;
    }
    structure_info->latest_iteration = 0U;
    structure_info->source = NULL;
  }
  else {
    free(structure_info->mc_declared_name);

    if (structure_info->is_defined) {
      // Free the field summaries
      printf("releasing '%s'\n", structure_info->name);
      release_field_info_list(structure_info->fields);
    }
  }
  register_midge_error_tag("update_or_register_struct_info_from_syntax-2");

  structure_info->is_union = struct_ast->type == MC_SYNTAX_UNION;
  structure_info->mc_declared_name = NULL;

  // Set the values parsed
  if (struct_ast->structure.fields) {
    structure_info->is_defined = true;

    mcs_summarize_type_field_list(struct_ast->structure.fields, &structure_info->fields);
  }
  else {
    structure_info->is_defined = false;
  }
  register_midge_error_tag("update_or_register_struct_info_from_syntax-4");

  // Set
  // *p_struct_info = structure_info;a

  return 0;
}

int mcs_process_ast_root_children(mc_syntax_node_list *children)
{
  mc_syntax_node *child;
  for (int a = 0; a < children->count; ++a) {
    child = children->items[a];
    switch (child->type) {
    case MC_SYNTAX_EXTERN_C_BLOCK: {
      MCerror(1115, "TODO");
      // for (int b = 0; b < child->extern_block.declarations->count; ++b) {
      //   mc_syntax_node *declaration = child->extern_block.declarations->items[b];
      //   switch (declaration->type) {
      //   case MC_SYNTAX_FUNCTION: {
      //     if ((mc_token_type)declaration->function.code_block->type != MC_TOKEN_SEMI_COLON) {
      //       MCerror(565, "Full Function definition in an extern c block ? ? ?");
      //     }
      //     // Function Declaration only
      //     update_or_register_function_info_from_syntax(NULL, declaration, NULL);
      //   } break;
      //   default:
      //     MCerror(572, "TODO : %s", get_mc_syntax_token_type_name(declaration->type));
      //   }
      // }
    } break;
    case MC_SYNTAX_FUNCTION: {
      if ((mc_token_type)child->function.code_block->type == MC_TOKEN_SEMI_COLON) {
        function_info *info;
        // Function Declaration only
        mcs_register_function_declaration(child, &info);
        printf("--fdecl:'%s'\n", child->function.name->text);
      }
      else {
        MCerror(1124, "TODO");
        // Assume to be function definition
        function_info *info;
        mcs_register_function_definition(child, &info);
        // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
        // info->source->source_file = source_file;
        printf("--defined:'%s'\n", child->function.name->text);
      }
    } break;
    case MC_SYNTAX_TYPE_ALIAS: {
      char buf[1024];
      switch (child->type_alias.type_descriptor->type) {
      case MC_SYNTAX_UNION:
      case MC_SYNTAX_STRUCTURE: {
        print_syntax_node(child->type_alias.type_descriptor, 0);
        mcs_register_struct_declaration(child->type_alias.type_descriptor);
        // MCerror(1151, "TODO");
        // struct_info *info;
        // instantiate_definition(definitions_owner, NULL, child->type_alias.type_descriptor, NULL, (void **)&info);
        // info->source->source_file = source_file;
        // // printf("--defined: struct '%s'\n", child->type_alias.type_descriptor->structure.type_name->text);
        // // sprintf(buf,
        // //         "#ifndef %s\n"
        // //         // "#undef %s\n"
        // //         "#define %s struct %s\n"
        // //         "#endif\n",
        // //         info->name, info->name, info->mc_declared_name);
        // // clint_process(buf);
      } break;
      case MC_SYNTAX_ENUM: {
        MCerror(1152, "TODO");
        // register_external_enum_declaration(definitions_owner, child->type_alias.type_descriptor);
        // enumeration_info *info;
        // instantiate_definition(definitions_owner, NULL, child->type_alias.type_descriptor, NULL, (void **)&info);
        // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-0");
        // info->source->source_file = source_file;
        // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-1");
        // // printf("--defined: enum '%s'\n", child->type_alias.type_descriptor->enumeration.name->text);
        // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-2");
        // // sprintf(buf,
        // //         "#ifndef %s\n"
        // //         // "#undef %s\n"
        // //         "#define %s enum %s\n"
        // //         "#endif\n",
        // //         info->name, info->name, info->mc_declared_name);
        // // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-3");
        // // clint_process(buf);
        // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-4");
      } break;
      default:
        print_syntax_node(child->type_alias.type_descriptor, 0);
        MCerror(1185, "Unhandled type_alias-descriptor-syntax-type:%s",
                get_mc_syntax_token_type_name(child->type_alias.type_descriptor->type));
        break;
      }
    } break;
    case MC_SYNTAX_STRUCTURE: {
      MCerror(1191, "TODO");
      // struct_info *info;
      // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
      // info->source->source_file = source_file;
      // // printf("--declared: struct '%s'\n", child->structure.type_name->text);
    } break;
    case MC_SYNTAX_ENUM: {
      MCerror(1198, "TODO");
      // enumeration_info *info;
      // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
      // info->source->source_file = source_file;
      // // printf("--declared: enum '%s'\n", child->enumeration.name->text);
    } break;
    case MC_TOKEN_PP_DIRECTIVE_DEFINE: {
      // Ignore for now
      // MCerror(1205, "TODO");

      // preprocess_define_info *info;
      // instantiate_define_statement(definitions_owner, child, &info);

      // // switch (info->statement_type) {
      // // case PREPROCESSOR_DEFINE_REMOVAL: {

      // // } break;

      // // default:
      // //   MCerror(887, "TODO :%i", info->statement_type);
      // // }
    } break;
    // TODO
    case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_IFNDEF: {
      // Assume all ifndefs (for the moment TODO ) to be true
      mcs_process_ast_root_children(child->preprocess_ifndef.groupopt);
    } break;
    case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_INCLUDE:
      MCerror(1256, "TODO");
      break;
    default: {
      switch ((mc_token_type)child->type) {
      case MC_TOKEN_PP_KEYWORD_IFNDEF:
      case MC_TOKEN_PP_KEYWORD_ENDIF:
        MCerror(5313, "TODO");
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
    } break;
    }
  }
  return 0;
}

int mcs_process_file_ast(mc_syntax_node *file_ast, char **generated_code)
{
  c_str *genc;

  // Init generated code
  init_c_str(&genc);

  mcs_process_ast_root_children(file_ast->children);

  // TODO -- transcribe file

  *generated_code = genc->text;
  release_c_str(genc, false);
  return 0;
}

int mcs_interpret_file(TCCInterpState *tis, const char *filepath)
{
  int res;
  char *code;
  mc_syntax_node *file_ast;
  mc_global_data *gdata;

  // Read the file
  read_file_text(filepath, &code);

  // Parse the file into an AST
  parse_file_to_syntax_tree(code, &file_ast);
  free(code);

  // Obtain function/struct/enum information & dependencies
  // Generate code (from the AST) with midge insertions integrated (stack call / error tracking)
  mcs_process_file_ast(file_ast, &code);

  // Send the code to the interpreter
  obtain_midge_global_root(&gdata);
  tcci_add_string(gdata->interpreter, filepath, code);

  // Cleanup
  free(code);
  release_syntax_node(file_ast);
  free(file_ast);

  return 0;
}

// #################################################################################################################################

// // int attach_preprocess_define_info_to_owner(mc_node *owner, preprocess_define_info *define_info)
// // {
// //   switch (owner->type) {
// //   case NODE_TYPE_GLOBAL_ROOT: {
// //     mc_global_data *data = (mc_global_data *)owner->data;
// //     append_to_collection((void ***)&data->preprocess_defines.items, &data->preprocess_defines.alloc,
// //                          &data->preprocess_defines.count, (void *)define_info);
// //     break;
// //   }
// //   default:
// //     MCerror(9, "TODO:%i", owner->type);
// //   }

// //   return 0;
// // }

// int append_syntax_node_to_file_context(mc_syntax_node *child, c_str *file_context)
// {
//   char *code;
//   copy_syntax_node_to_text(child, &code);
//   append_to_c_str(file_context, code);
//   append_char_to_c_str(file_context, '\n');
//   free(code);
// }

// int mc_init_source_file_info(mc_node *owner, char *filepath, mc_source_file_info **source_file)
// {
//   mc_source_file_info *sfi = (mc_source_file_info *)malloc(sizeof(mc_source_file_info));
//   allocate_and_copy_cstr(sfi->filepath, filepath);
//   sfi->definitions.alloc = 0;
//   sfi->definitions.count = 0;

//   switch (owner->type) {
//   case NODE_TYPE_GLOBAL_ROOT: {
//     mc_global_data *data = (mc_global_data *)owner->data;
//     append_to_collection((void ***)&data->source_files.items, &data->source_files.alloc, &data->source_files.count,
//                          (void *)sfi);
//     break;
//   }
//   default:
//     MCerror(52, "TODO:%i", owner->type);
//   }

//   // // Cache files
//   // _mcl_determine_cached_file_name(filepath, &sfi->cached_file_name);

//   if (source_file)
//     *source_file = sfi;

//   return 0;
// }

// int update_or_register_struct_info_from_syntax(mc_node *owner, mc_syntax_node *struct_ast, struct_info
// **p_struct_info)
// {
//   if (!struct_ast->structure.type_name) {
//     MCerror(8461, "root-level anonymous structures not yet supported");
//   }

//   struct_info *structure_info;
//   // printf("ursif0:'%p'\n", struct_ast);
//   // printf("ursif1:'%p'\n", struct_ast->structure.type_name);
//   // printf("ursif2:'%p'\n", struct_ast->structure.type_name->text);
//   // printf("ursif3:'%s'\n", struct_ast->structure.type_name->text);
//   if (struct_ast->structure.type_name) {
//     find_struct_info(struct_ast->structure.type_name->text, &structure_info);
//   }

//   register_midge_error_tag("update_or_register_struct_info_from_syntax-1");
//   if (!structure_info) {
//     structure_info = (struct_info *)malloc(sizeof(struct_info));

//     attach_struct_info_to_owner(owner, structure_info);

//     structure_info->type_id = (struct_id *)malloc(sizeof(struct_id));
//     allocate_and_copy_cstr(structure_info->type_id->identifier, "struct_info");
//     structure_info->type_id->version = 1U;

//     // Name & Version
//     if (struct_ast->structure.type_name) {
//       allocate_and_copy_cstr(structure_info->name, struct_ast->structure.type_name->text);
//     }
//     else {
//       structure_info->name = NULL;
//     }
//     structure_info->latest_iteration = 1U;
//     structure_info->source = NULL;
//   }
//   else {
//     free(structure_info->mc_declared_name);

//     if (structure_info->is_defined) {
//       // Free the field summaries
//       printf("releasing '%s'\n", structure_info->name);
//       release_field_info_list(structure_info->fields);
//     }
//   }
//   register_midge_error_tag("update_or_register_struct_info_from_syntax-2");

//   structure_info->is_union = struct_ast->type == MC_SYNTAX_UNION;
//   c_str *mc_func_name;
//   init_c_str(&mc_func_name);
//   append_to_c_strf(mc_func_name, "%s_mc_v%u", structure_info->name, structure_info->latest_iteration);
//   structure_info->mc_declared_name = mc_func_name->text;
//   release_c_str(mc_func_name, false);

//   // Set the values parsed
//   if (struct_ast->structure.fields) {
//     structure_info->is_defined = true;

//     mcs_summarize_type_field_list(struct_ast->structure.fields, &structure_info->fields);
//   }
//   else {
//     structure_info->is_defined = false;
//   }
//   register_midge_error_tag("update_or_register_struct_info_from_syntax-4");

//   // Set
//   *p_struct_info = structure_info;

//   return 0;
// }

// int update_or_register_enum_info_from_syntax(mc_node *owner, mc_syntax_node *enum_ast, enumeration_info
// **p_enum_info)
// {
//   enumeration_info *enum_info;
//   find_enumeration_info(enum_ast->enumeration.name->text, &enum_info);

//   register_midge_error_tag("update_or_register_enum_info_from_syntax-1");
//   char buf[256];
//   if (!enum_info) {
//     enum_info = (enumeration_info *)malloc(sizeof(enumeration_info));

//     attach_enumeration_info_to_owner(owner, enum_info);

//     enum_info->type_id = (struct_id *)malloc(sizeof(struct_id));
//     allocate_and_copy_cstr(enum_info->type_id->identifier, "enum_info");
//     enum_info->type_id->version = 1U;

//     // Name & Version
//     allocate_and_copy_cstr(enum_info->name, enum_ast->enumeration.name->text);
//     enum_info->latest_iteration = 1U;

//     enum_info->members.alloc = 0;
//     enum_info->members.count = 0;
//   }
//   else {
//     // Empty

//     // Clear the current values
//     for (int i = 0; i < enum_info->members.count; ++i) {
//       // sprintf(buf,
//       //         "#ifdef %s\n"
//       //         "#undef %s\n"
//       //         "#endif\n",
//       //         enum_info->members.items[i]->identity, enum_info->members.items[i]->identity);
//       // clint_process(buf);

//       if (enum_info->members.items[i]) {
//         if (enum_info->members.items[i]->identity) {
//           free(enum_info->members.items[i]->identity);
//         }
//         if (enum_info->members.items[i]->value) {
//           free(enum_info->members.items[i]->value);
//         }
//         free(enum_info->members.items[i]);
//       }
//     }

//     ++enum_info->latest_iteration;
//   }
//   register_midge_error_tag("update_or_register_enum_info_from_syntax-2");

//   char enum_name[32];
//   sprintf(enum_name, "%s_mc_v%u", enum_info->name, enum_info->latest_iteration);
//   allocate_and_copy_cstr(enum_info->mc_declared_name, enum_name);

//   // Set the values parsed
//   enum_info->members.count = 0;
//   int latest_value = -1;
//   for (int i = 0; i < enum_ast->enumeration.members->count; ++i) {
//     enum_member_info *member = (enum_member_info *)malloc(sizeof(enum_member_info));

//     copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.identifier, &member->identity);
//     if (enum_ast->enumeration.members->items[i]->enum_member.value_expression) {
//       copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.value_expression,
//       &member->value);

//       sprintf(buf, "*(int *)(%p) = %s;", &latest_value, member->value); // TODO -- semi-colon
//       // clint_process(buf); TODO
//       sprintf(buf, "#define %s (%s)%i\n", member->identity, enum_info->mc_declared_name, latest_value);
//     }
//     else {
//       member->value = NULL;
//       ++latest_value;
//       sprintf(buf, "#define %s (%s)%i\n", member->identity, enum_info->mc_declared_name, latest_value);
//     }

//     // printf("%s", buf);
//     // clint_process(buf); TODO

//     append_to_collection((void ***)&enum_info->members.items, &enum_info->members.alloc, &enum_info->members.count,
//                          member);
//   }

//   // for (int b = 0; b < child->enumeration.members->count; ++b) {
//   //   mc_syntax_node *enum_member = child->enumeration.members->items[b];
//   //   sprintf(buf,
//   //           "#ifdef %s\n"
//   //           "#undef %s\n"
//   //           "#endif\n"
//   //           "#define %s %s\n",
//   //           enum_member->enum_member.identifier->text, enum_member->enum_member.identifier->text,
//   //           enum_member->enum_member.identifier->text, enum_member->enum_member.value->text);
//   //   printf("%s", buf);

//   //   return 0;

//   // Set
//   *p_enum_info = enum_info;

//   return 0;
// }

// int instantiate_function_definition_from_ast(mc_node *definition_owner, source_definition *source, c_str
// *file_context,
//                                              mc_syntax_node *ast, void **definition_info)
// {
//   // Register Function
//   function_info *func_info;
//   update_or_register_function_info_from_syntax(definition_owner, ast, &func_info);

//   // Instantiate Function
//   char *mc_transcription;
//   mct_function_transcription_options options = {};
//   options.report_invocations_to_error_stack = false;
//   options.report_simple_args_to_error_stack = false;
//   options.check_mc_functions_not_null = false;
//   options.tag_on_function_entry = false;
//   options.tag_on_function_exit = false;
//   options.report_variable_values = NULL;
//   mct_transcribe_function_to_mc(func_info, ast, &options, &mc_transcription);

//   if (!strcmp(func_info->name, "_mce_render_function_editor_present")) {
//     // print_syntax_node(ast, 0);
//     printf("mc_transcription:\n%s||\n", mc_transcription);
//   }
//   // if (!strcmp(func_info->name, "mcs_parse_through_supernumerary_tokens")) {
//   //   // print_syntax_node(ast, 0);
//   //   // printf("callit-fptr-addr:%p\n", func_info->ptr_declaration);
//   //   printf("mc_transcription:\n%s||\n", mc_transcription);
//   // }

//   int result = 0;
//   MCerror(8386, "progress");
//   // int result = clint_declare(mc_transcription);
//   if (result) {
//     printf("\n\nmc_transcription:\n%.50s||\n", mc_transcription);
//     // printf("\n\nmc_transcription:\n%s||\n", mc_transcription);
//     MCerror(615, "Failed to declare function");
//   }
//   free(mc_transcription);
//   // printf("idfc-5\n");

//   // TODO extract this method and common call it where its repeated across midge
//   char buf[512];
//   sprintf(buf, "%s = &%s_mc_v%u;", func_info->name, func_info->name, func_info->latest_iteration);
//   // printf("idfc-6\n");
//   // clint_process(buf); TODO

//   // printf("idfc-7 %s_v%u\n", func_info->name, func_info->latest_iteration);
//   // sprintf(buf,
//   //         "{void *vargs[1];void *vargs0 = NULL;vargs[0] = &vargs0;%s(1, vargs);"
//   //         "printf(\"addr of fptr:%%p\\n\", &%s);}",
//   //         func_info->name, func_info->name);
//   // clint_process(buf);
//   // printf("idfc-8\n");

//   if (definition_info) {
//     *definition_info = func_info;
//   }

//   return 0;
// }

// int instantiate_struct_definition_from_ast(mc_node *definition_owner, source_definition *source, mc_syntax_node
// *ast,
//                                            void **definition_info)
// {
//   // Register Struct
//   struct_info *structure_info;
//   update_or_register_struct_info_from_syntax(definition_owner, ast, &structure_info);

//   // Instantiate Struct
//   char *mc_transcription;
//   transcribe_struct_to_mc(structure_info, ast, &mc_transcription);

//   // if (!strcmp(structure_info->name, "render_color"))
//   //   printf("struct:\n%s||\n", mc_transcription);
//   // int result = clint_declare(mc_transcription);
//   int result = 0;
//   if (result) {
//     printf("\n\nmc_transcription:\n%s||\n", mc_transcription);
//     MCerror(7667, "Failed to declare structure");
//   }
//   free(mc_transcription);

//   if (definition_info) {
//     *definition_info = structure_info;
//   }

//   register_midge_error_tag("instantiate_struct_definition_from_ast(~)");
//   return 0;
// }

// int instantiate_enum_definition_from_ast(mc_node *definition_owner, source_definition *source, mc_syntax_node *ast,
//                                          void **definition_info)
// {
//   // Register enum
//   enumeration_info *enum_info;
//   update_or_register_enum_info_from_syntax(definition_owner, ast, &enum_info);

//   char buf[256];
//   sprintf(buf, "enum %s { };", enum_info->mc_declared_name);
//   int result; // = clint_declare(buf);
//   MCerror(8359, "progress");
//   if (result) {
//     printf("\nmc_declaration:\n%s||\n", buf);
//     MCerror(691, "Failed to declare enumeration");
//   }

//   if (definition_info) {
//     *definition_info = enum_info;
//   }

//   register_midge_error_tag("instantiate_enum_definition_from_ast(~)");
//   return 0;
// }

// int instantiate_define_statement(mc_node *definition_owner, mc_syntax_node *ast, preprocess_define_info **info)
// {
//   switch (ast->preprocess_define.statement_type) {
//   case PREPROCESSOR_DEFINE_REMOVAL: {
//     *info = (preprocess_define_info *)malloc(sizeof(preprocess_define_info));
//     (*info)->statement_type = ast->preprocess_define.statement_type;
//     copy_syntax_node_to_text(ast->preprocess_define.identifier, &(*info)->identifier);

//     (*info)->replacement = NULL;
//   } break;
//   case PREPROCESSOR_DEFINE_FUNCTION_LIKE: {
//     // Do nothing...
//     // char *statement_text;
//     // copy_syntax_node_to_text(ast, &statement_text);
//     // printf("\nfunctionlike:\n%s||\n", statement_text);
//     // free(statement_text);
//   } break;
//   case PREPROCESSOR_DEFINE_REPLACEMENT: {
//     *info = (preprocess_define_info *)malloc(sizeof(preprocess_define_info));
//     (*info)->statement_type = ast->preprocess_define.statement_type;
//     copy_syntax_node_to_text(ast->preprocess_define.identifier, &(*info)->identifier);

//     c_str *str;
//     init_c_str(&str);
//     for (int i = 0; i < ast->preprocess_define.replacement_list->count; ++i) {
//       char *node_text;
//       copy_syntax_node_to_text(ast->preprocess_define.replacement_list->items[i], &node_text);
//       append_to_c_str(str, node_text);
//       free(node_text);
//     }

//     (*info)->replacement = str->text;
//     release_c_str(str, false);

//     // printf("define:\n'%s'\n'%s'\n", (*info)->identifier, (*info)->replacement);
//   } break;
//   default:
//     MCerror(830, "TODO :%i", ast->preprocess_define.statement_type);
//   }

//   // char *statement_text;
//   // copy_syntax_node_to_text(ast, &statement_text);
//   // // printf("\ndefine_declaration:\n%s||\n", statement_text);
//   // int result; //= clint_declare(statement_text);
//   // MCerror(9582, "progress");
//   // if (result) {
//   //   printf("\ndefine_declaration:\n%s||\n", statement_text);
//   //   MCerror(691, "Failed to declare define statement");
//   // }
//   // free(statement_text);

//   return 0;
// }

// /*
//   From code definition: constructs source definition & parses to syntax, registers with hierarchy, and declares the
//   definition for immediate use.
//   @definition_owner the node in the hierarchy to attach this definition to.
//   @code may be NULL only if ast is not, if so it will be generated from the syntax parse.
//   @ast may be NULL only if code is not, if so it will be parsed from the code.
//   @source may be NULL, if so it will be created.
//   @definition_info is OUT. May be NULL, if not dereference will be set with
//   p-to-function_info/struct_info/enum_info etc.
// */
// int instantiate_definition(mc_node *definition_owner, c_str *file_context, char *code, mc_syntax_node *ast,
//                            source_definition *source, void **definition_info)
// {
//   register_midge_error_tag("instantiate_definition()");
//   // Compile Code to Syntax
//   if (!ast) {
//     parse_definition_to_syntax_tree(code, &ast);
//   }
//   else if (!code) {
//     copy_syntax_node_to_text(ast, &code);
//   }

//   // TODO -- check type hasn't changed with definition
//   if (!source) {
//     source = (source_definition *)malloc(sizeof(source_definition));
//     source->source_file = NULL;
//   }
//   source->code = code;

//   void *p_definition_info;

//   switch (ast->type) {
//   case MC_SYNTAX_FUNCTION: {
//     source->type = SOURCE_DEFINITION_FUNCTION;
//     instantiate_function_definition_from_ast(definition_owner, source, file_context, ast, &p_definition_info);

//     function_info *func_info = (function_info *)p_definition_info;
//     func_info->source = source;
//   } break;
//   case MC_SYNTAX_UNION:
//   case MC_SYNTAX_STRUCTURE: {
//     source->type = SOURCE_DEFINITION_STRUCTURE;
//     instantiate_struct_definition_from_ast(definition_owner, source, ast, &p_definition_info);

//     struct_info *structure_info = (struct_info *)p_definition_info;
//     structure_info->source = source;
//   } break;
//   case MC_SYNTAX_ENUM: {
//     source->type = SOURCE_DEFINITION_ENUMERATION;
//     instantiate_enum_definition_from_ast(definition_owner, source, ast, &p_definition_info);

//     enumeration_info *enum_info = (enumeration_info *)p_definition_info;
//     enum_info->source = source;
//   } break;
//   default: {
//     MCerror(325, "instantiate_definition:%i NotYetSupported", ast->type);
//   }
//   }

//   source->data.p_data = p_definition_info;
//   if (definition_info)
//     *definition_info = p_definition_info;

//   register_midge_error_tag("instantiate_definition(~)");
//   return 0;
// }

// int instantiate_ast_children(mc_node *definitions_owner, mc_source_file_info *source_file, c_str *file_context,
//                              mc_syntax_node_list *syntax_node_list)
// {
//   for (int a = 0; a < syntax_node_list->count; ++a) {
//     mc_syntax_node *child = syntax_node_list->items[a];
//     const char *type_name = get_mc_syntax_token_type_name(child->type);
//     // printf("instantiate_definition[%i]:%s\n", a, type_name);
//     switch (child->type) {
//     case MC_SYNTAX_EXTERN_C_BLOCK: {
//       for (int b = 0; b < child->extern_block.declarations->count; ++b) {
//         mc_syntax_node *declaration = child->extern_block.declarations->items[b];
//         switch (declaration->type) {
//         case MC_SYNTAX_FUNCTION: {
//           if ((mc_token_type)declaration->function.code_block->type != MC_TOKEN_SEMI_COLON) {
//             MCerror(565, "Full Function definition in an extern c block ? ? ?");
//           }
//           // Function Declaration only
//           update_or_register_function_info_from_syntax(NULL, declaration, NULL);
//         } break;
//         default:
//           MCerror(572, "TODO : %s", get_mc_syntax_token_type_name(declaration->type));
//         }
//       }
//     } break;
//     case MC_SYNTAX_FUNCTION: {

//       if ((mc_token_type)child->function.code_block->type == MC_TOKEN_SEMI_COLON) {
//         // Function Declaration only
//         // puts(file_context->text);
//         append_syntax_node_to_file_context(child, file_context);

//         update_or_register_function_info_from_syntax(NULL, child, NULL);
//         // printf("--fdecl:'%s'\n", child->function.name->text);
//       }
//       else {
//         // Assume to be function definition
//         function_info *info;
//         instantiate_definition(definitions_owner, file_context, NULL, child, NULL, (void **)&info);
//         info->source->source_file = source_file;

//         append_to_collection((void ***)&info->source->source_file->definitions.items,
//                              &info->source->source_file->definitions.alloc,
//                              &info->source->source_file->definitions.count, info->source);
//         // printf("--defined:'%s'\n", child->function.name->text);
//       }
//     } break;
//     case MC_SYNTAX_TYPE_ALIAS: {
//       MCerror(9821, "TODO");
//       char buf[1024];
//       switch (child->type_alias.type_descriptor->type) {
//       case MC_SYNTAX_UNION:
//       case MC_SYNTAX_STRUCTURE: {
//         struct_info *info;
//         instantiate_definition(definitions_owner, file_context, NULL, child->type_alias.type_descriptor, NULL,
//                                (void **)&info);
//         info->source->source_file = source_file;
//         append_to_collection((void ***)&info->source->source_file->definitions.items,
//                              &info->source->source_file->definitions.alloc,
//                              &info->source->source_file->definitions.count, info->source);
//         // printf("--defined: struct '%s'\n", child->type_alias.type_descriptor->structure.type_name->text);
//         // sprintf(buf,
//         //         "#ifndef %s\n"
//         //         // "#undef %s\n"
//         //         "#define %s struct %s\n"
//         //         "#endif\n",
//         //         info->name, info->name, info->mc_declared_name);
//         // clint_process(buf);
//       } break;
//       case MC_SYNTAX_ENUM: {
//         enumeration_info *info;
//         instantiate_definition(definitions_owner, file_context, NULL, child->type_alias.type_descriptor, NULL,
//                                (void **)&info);
//         register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-0");
//         info->source->source_file = source_file;
//         append_to_collection((void ***)&info->source->source_file->definitions.items,
//                              &info->source->source_file->definitions.alloc,
//                              &info->source->source_file->definitions.count, info->source);
//         register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-1");
//         // printf("--defined: enum '%s'\n", child->type_alias.type_descriptor->enumeration.name->text);
//         register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-2");
//         // sprintf(buf,
//         //         "#ifndef %s\n"
//         //         // "#undef %s\n"
//         //         "#define %s enum %s\n"
//         //         "#endif\n",
//         //         info->name, info->name, info->mc_declared_name);
//         // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-3");
//         // clint_process(buf);
//         register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-4");
//       } break;
//       default:
//         print_syntax_node(child->type_alias.type_descriptor, 0);
//         MCerror(668, "Unhandled type_alias-descriptor-syntax-type:%s",
//                 get_mc_syntax_token_type_name(child->type_alias.type_descriptor->type));
//         break;
//       }
//     } break;
//     case MC_SYNTAX_STRUCTURE: {
//       MCerror(9691, "TODO");
//       struct_info *info;
//       instantiate_definition(definitions_owner, file_context, NULL, child, NULL, (void **)&info);
//       info->source->source_file = source_file;
//       append_to_collection((void ***)&info->source->source_file->definitions.items,
//                            &info->source->source_file->definitions.alloc,
//                            &info->source->source_file->definitions.count, info->source);
//       // printf("--declared: struct '%s'\n", child->structure.type_name->text);
//     } break;
//     case MC_SYNTAX_ENUM: {
//       MCerror(9781, "TODO");
//       enumeration_info *info;
//       instantiate_definition(definitions_owner, file_context, NULL, child, NULL, (void **)&info);
//       info->source->source_file = source_file;
//       append_to_collection((void ***)&info->source->source_file->definitions.items,
//                            &info->source->source_file->definitions.alloc,
//                            &info->source->source_file->definitions.count, info->source);
//       // printf("--declared: enum '%s'\n", child->enumeration.name->text);
//     } break;
//     case MC_TOKEN_PP_DIRECTIVE_DEFINE: {
//       append_syntax_node_to_file_context(child, file_context);

//       preprocess_define_info *info;
//       instantiate_define_statement(definitions_owner, child, &info);

//       // switch (info->statement_type) {
//       // case PREPROCESSOR_DEFINE_REMOVAL: {

//       // } break;

//       // default:
//       //   MCerror(887, "TODO :%i", info->statement_type);
//       // }
//     } break;
//     // TODO
//     case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_IFNDEF: {
//       // char *identifier;
//       // copy_syntax_node_to_text(child->preprocess_ifndef.identifier, &identifier);
//       // char buf[1024];
//       // int is_defined;
//       // sprintf(buf,
//       //         "#ifndef %s\n"
//       //         "*((int *)%p) = 222;\n"
//       //         "#else\n"
//       //         "*((int *)%p) = 111;\n"
//       //         "#endif\n",
//       //         identifier, &is_defined, &is_defined);
//       // MCerror(8383, "progress");
//       // // clint_process(buf);
//       // if (is_defined == 222) {
//       // Assume it is not defined
//       instantiate_ast_children(definitions_owner, source_file, file_context, child->preprocess_ifndef.groupopt);
//       // }
//       // else if (is_defined == 111) {
//       //   // Do Nothing
//       //   // printf("'%s' was already defined\n", identifier);
//       // }
//       // else {
//       //   MCerror(950, "All did not go to plan");
//       // }
//       // free(identifier);
//     } break;
//     case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_INCLUDE: {
//       MCerror(1028, "TODO");
//     } break;
//     case MC_TOKEN_PP_KEYWORD_ENDIF:
//       break;
//     default: {
//       switch ((mc_token_type)child->type) {
//       case MC_TOKEN_SPACE_SEQUENCE:
//       case MC_TOKEN_NEW_LINE:
//       case MC_TOKEN_LINE_COMMENT:
//       case MC_TOKEN_MULTI_LINE_COMMENT: {
//         break;
//       }
//       default: {
//         print_syntax_node(child, 0);
//         MCerror(576, "Unhandled root-syntax-type:%s", get_mc_syntax_token_type_name(child->type));
//       }
//       }
//     }
//     }
//   }

//   return 0;
// }

// int instantiate_definitions_from_cached_file(mc_node *definitions_owner, char *filepath, char *cached_file_name,
//                                              mc_source_file_info **source_file)
// {

//   MCerror(1030, "TODO %s", cached_file_name);
//   // Parse all definitions
//   // mc_source_file_info *lv_source_file;
//   // mc_init_source_file_info(definitions_owner, filepath, &lv_source_file);
//   // if (source_file) {
//   //   *source_file = lv_source_file;
//   // }
//   char *cache_text;
//   read_file_text(filepath, &cache_text);

//   return 0;
// }

// int mcl_determine_cached_file_name(const char *input, char **output)
// {
//   c_str *str;
//   init_c_str(&str);
//   set_c_str(str, "bin/cached/");

//   int fni = 0;
//   fni += 11;
//   for (int k = 0; k < 256; ++k) {
//     if (input[k] == '/') {
//       append_char_to_c_str(str, '_');
//     }
//     else {
//       if (input[k] == '\0') {
//         break;
//       }
//       append_char_to_c_str(str, input[k]);
//     }
//   }

//   *output = str->text;
//   release_c_str(str, false);

//   return 0;
// }

// int attempt_instantiate_all_definitions_from_cached_file(mc_node *definitions_owner, char *filepath,
//                                                          mc_source_file_info **source_file, bool *used_cached_file)
// {
//   char *cached_file_name;
//   mcl_determine_cached_file_name(filepath, &cached_file_name);

//   // Compare modified times and process new source or use cache
//   *used_cached_file = false;
//   if (access(cached_file_name, F_OK) != -1) {

//     struct stat src_attrib;
//     stat(filepath, &src_attrib);

//     struct stat cch_attrib;
//     stat(cached_file_name, &cch_attrib);

//     *used_cached_file = (src_attrib.st_mtime < cch_attrib.st_mtime);
//   }
//   // if (!strcmp(filepath, "src/m_threads.h")) {
//   //   *used_cached_file = true;
//   // }
//   // printf("'%s'\n", filepath);
//   if (!*used_cached_file) {
//     free(cached_file_name);
//     return 0;
//   }

//   instantiate_definitions_from_cached_file(definitions_owner, filepath, cached_file_name, source_file);

//   return 0;
// }

// int instantiate_all_definitions_from_file(mc_node *definitions_owner, char *filepath, mc_source_file_info
// **source_file)
// {
//   printf("instantiate file:'%s'\n", filepath);
//   // bool used_cache_file;
//   // attempt_instantiate_all_definitions_from_cached_file(definitions_owner, filepath, source_file,
//   &used_cache_file);
//   // if (used_cache_file) {
//   //   return 0;
//   // }

//   char *file_text;
//   read_file_text(filepath, &file_text);

//   mc_syntax_node *syntax_node;
//   parse_file_to_syntax_tree(file_text, &syntax_node);

//   // Parse all definitions
//   mc_source_file_info *lv_source_file;
//   mc_init_source_file_info(definitions_owner, filepath, &lv_source_file);
//   if (source_file) {
//     *source_file = lv_source_file;
//   }

//   c_str *file_context;
//   init_c_str(&file_context);
//   instantiate_ast_children(definitions_owner, lv_source_file, file_context, syntax_node->children);

//   release_c_str(file_context, true);
//   // int *p = 0;
//   // printf("about\n");
//   // printf("%i\n", *p);
//   // printf("end\n");

//   register_midge_error_tag("instantiate_all_definitions_from_file(~)");
//   return 0;
// }

// int register_external_struct_declaration(mc_node *owner, mc_syntax_node *struct_ast)
// {
//   if (!struct_ast->structure.type_name) {
//     MCerror(8461, "root-level anonymous external structures not yet supported");
//   }

//   struct_info *structure_info;
//   find_struct_info(struct_ast->structure.type_name->text, &structure_info);
//   if (structure_info) {
//     MCerror(1013, "TODO?");
//   }

//   // register_midge_error_tag("update_or_register_struct_info_from_syntax-1");
//   if (!structure_info) {
//     structure_info = (struct_info *)malloc(sizeof(struct_info));

//     attach_struct_info_to_owner(owner, structure_info);

//     structure_info->type_id = (struct_id *)malloc(sizeof(struct_id));
//     allocate_and_copy_cstr(structure_info->type_id->identifier, "struct_info");
//     structure_info->type_id->version = 1U;

//     // Name & Version
//     if (struct_ast->structure.type_name) {
//       allocate_and_copy_cstr(structure_info->name, struct_ast->structure.type_name->text);
//     }
//     else {
//       structure_info->name = NULL;
//     }
//     structure_info->latest_iteration = 0U;
//     structure_info->source = NULL;
//   }
//   else {
//     free(structure_info->mc_declared_name);

//     if (structure_info->is_defined) {
//       // Free the field summaries
//       printf("releasing '%s'\n", structure_info->name);
//       release_field_info_list(structure_info->fields);
//     }
//   }
//   register_midge_error_tag("update_or_register_struct_info_from_syntax-2");

//   structure_info->is_union = struct_ast->type == MC_SYNTAX_UNION;
//   structure_info->mc_declared_name = NULL;

//   // Set the values parsed
//   if (struct_ast->structure.fields) {
//     structure_info->is_defined = true;

//     mcs_summarize_type_field_list(struct_ast->structure.fields, &structure_info->fields);
//   }
//   else {
//     structure_info->is_defined = false;
//   }
//   register_midge_error_tag("update_or_register_struct_info_from_syntax-4");

//   // Set
//   // *p_struct_info = structure_info;a

//   return 0;
// }

// int register_external_enum_declaration(mc_node *owner, mc_syntax_node *enum_ast)
// {
//   enumeration_info *enum_info;
//   find_enumeration_info(enum_ast->enumeration.name->text, &enum_info);

//   // printf("reed-0\n");
//   if (enum_info) {
//     MCerror(1013, "TODO?");
//   }

//   char buf[256];
//   if (!enum_info) {
//     // printf("reed-1\n");
//     enum_info = (enumeration_info *)malloc(sizeof(enumeration_info));

//     // printf("reed-2\n");
//     attach_enumeration_info_to_owner(owner, enum_info);
//     // printf("reed-3\n");

//     enum_info->type_id = (struct_id *)malloc(sizeof(struct_id));
//     allocate_and_copy_cstr(enum_info->type_id->identifier, "enum_info");
//     enum_info->type_id->version = 1U;
//     // printf("reed-4\n");

//     // Name & Version
//     allocate_and_copy_cstr(enum_info->name, enum_ast->enumeration.name->text);
//     enum_info->latest_iteration = 0U;

//     enum_info->members.alloc = 0;
//     enum_info->members.count = 0;

//     enum_info->mc_declared_name = NULL;
//     // printf("reed-5\n");
//   }
//   else {
//     // Empty
//     // printf("reed-6\n");

//     // Clear the current values
//     for (int i = 0; i < enum_info->members.count; ++i) {
//       sprintf(buf,
//               "#ifdef %s\n"
//               "#undef %s\n"
//               "#endif\n",
//               enum_info->members.items[i]->identity, enum_info->members.items[i]->identity);
//       // clint_process(buf);
//       MCerror(8521, "progress");

//       if (enum_info->members.items[i]) {
//         if (enum_info->members.items[i]->identity) {
//           free(enum_info->members.items[i]->identity);
//         }
//         if (enum_info->members.items[i]->value) {
//           free(enum_info->members.items[i]->value);
//         }
//         free(enum_info->members.items[i]);
//       }
//     }
//     // printf("reed-7\n");

//     ++enum_info->latest_iteration;
//   }
//   // printf("reed-8\n");

//   // print_syntax_node(enum_ast, 0);
//   // Set the values parsed
//   enum_info->members.count = 0;
//   int latest_value = -1;
//   // printf("reed-8a\n");
//   for (int i = 0; i < enum_ast->enumeration.members->count; ++i) {
//     // printf("reed-8b\n");
//     enum_member_info *member = (enum_member_info *)malloc(sizeof(enum_member_info));
//     // printf("reed-8c\n");

//     copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.identifier, &member->identity);
//     // printf("reed-8d\n");
//     if (enum_ast->enumeration.members->items[i]->enum_member.value_expression) {
//       copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.value_expression,
//       &member->value);
//       // printf("reed-8e\n");
//     }
//     else {
//       member->value = NULL;
//       // cprintf(member->value, "%i", ++latest_value);
//     }

//     // printf("reed-8f\n");
//     append_to_collection((void ***)&enum_info->members.items, &enum_info->members.alloc, &enum_info->members.count,
//                          member);
//     // printf("reed-8g\n");
//   }
//   // printf("reed-9\n");

//   return 0;
// }

// int register_external_declarations_from_syntax_children(mc_node *definitions_owner, mc_source_file_info
// *source_file,
//                                                         mc_syntax_node_list *syntax_node_list)
// {
//   for (int a = 0; a < syntax_node_list->count; ++a) {
//     mc_syntax_node *child = syntax_node_list->items[a];
//     const char *type_name = get_mc_syntax_token_type_name(child->type);
//     // printf("instantiate_definition[%i]:%s\n", a, type_name);
//     switch (child->type) {
//     case MC_SYNTAX_EXTERN_C_BLOCK: {
//       MCerror(1115, "TODO");
//       // for (int b = 0; b < child->extern_block.declarations->count; ++b) {
//       //   mc_syntax_node *declaration = child->extern_block.declarations->items[b];
//       //   switch (declaration->type) {
//       //   case MC_SYNTAX_FUNCTION: {
//       //     if ((mc_token_type)declaration->function.code_block->type != MC_TOKEN_SEMI_COLON) {
//       //       MCerror(565, "Full Function definition in an extern c block ? ? ?");
//       //     }
//       //     // Function Declaration only
//       //     update_or_register_function_info_from_syntax(NULL, declaration, NULL);
//       //   } break;
//       //   default:
//       //     MCerror(572, "TODO : %s", get_mc_syntax_token_type_name(declaration->type));
//       //   }
//       // }
//     } break;
//     case MC_SYNTAX_FUNCTION: {
//       MCerror(1132, "TODO");
//       // if ((mc_token_type)child->function.code_block->type == MC_TOKEN_SEMI_COLON) {
//       //   // Function Declaration only
//       //   update_or_register_function_info_from_syntax(NULL, child, NULL);
//       //   printf("--fdecl:'%s'\n", child->function.name->text);
//       // }
//       // else {
//       //   // Assume to be function definition
//       //   function_info *info;
//       //   instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
//       //   info->source->source_file = source_file;
//       //   printf("--defined:'%s'\n", child->function.name->text);
//       // }
//     } break;
//     case MC_SYNTAX_TYPE_ALIAS: {
//       char buf[1024];
//       switch (child->type_alias.type_descriptor->type) {
//       case MC_SYNTAX_UNION:
//       case MC_SYNTAX_STRUCTURE: {
//         register_external_struct_declaration(definitions_owner, child->type_alias.type_descriptor);
//         // MCerror(1151, "TODO");
//         // struct_info *info;
//         // instantiate_definition(definitions_owner, NULL, child->type_alias.type_descriptor, NULL, (void
//         **)&info);
//         // info->source->source_file = source_file;
//         // // printf("--defined: struct '%s'\n", child->type_alias.type_descriptor->structure.type_name->text);
//         // // sprintf(buf,
//         // //         "#ifndef %s\n"
//         // //         // "#undef %s\n"
//         // //         "#define %s struct %s\n"
//         // //         "#endif\n",
//         // //         info->name, info->name, info->mc_declared_name);
//         // // clint_process(buf);
//       } break;
//       case MC_SYNTAX_ENUM: {
//         register_external_enum_declaration(definitions_owner, child->type_alias.type_descriptor);
//         // enumeration_info *info;
//         // instantiate_definition(definitions_owner, NULL, child->type_alias.type_descriptor, NULL, (void
//         **)&info);
//         // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-0");
//         // info->source->source_file = source_file;
//         // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-1");
//         // // printf("--defined: enum '%s'\n", child->type_alias.type_descriptor->enumeration.name->text);
//         // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-2");
//         // // sprintf(buf,
//         // //         "#ifndef %s\n"
//         // //         // "#undef %s\n"
//         // //         "#define %s enum %s\n"
//         // //         "#endif\n",
//         // //         info->name, info->name, info->mc_declared_name);
//         // // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-3");
//         // // clint_process(buf);
//         // register_midge_error_tag("instantiate_all_definitions_from_file-TA-E-4");
//       } break;
//       default:
//         print_syntax_node(child->type_alias.type_descriptor, 0);
//         MCerror(1185, "Unhandled type_alias-descriptor-syntax-type:%s",
//                 get_mc_syntax_token_type_name(child->type_alias.type_descriptor->type));
//         break;
//       }
//     } break;
//     case MC_SYNTAX_STRUCTURE: {
//       MCerror(1191, "TODO");
//       // struct_info *info;
//       // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
//       // info->source->source_file = source_file;
//       // // printf("--declared: struct '%s'\n", child->structure.type_name->text);
//     } break;
//     case MC_SYNTAX_ENUM: {
//       MCerror(1198, "TODO");
//       // enumeration_info *info;
//       // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
//       // info->source->source_file = source_file;
//       // // printf("--declared: enum '%s'\n", child->enumeration.name->text);
//     } break;
//     case MC_TOKEN_PP_DIRECTIVE_DEFINE: {
//       MCerror(1205, "TODO");
//       // preprocess_define_info *info;
//       // instantiate_define_statement(definitions_owner, child, &info);

//       // // switch (info->statement_type) {
//       // // case PREPROCESSOR_DEFINE_REMOVAL: {

//       // // } break;

//       // // default:
//       // //   MCerror(887, "TODO :%i", info->statement_type);
//       // // }
//     } break;
//     // TODO
//     case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_IFNDEF: {
//       MCerror(1220, "TODO");
//       // char *identifier;
//       // copy_syntax_node_to_text(child->preprocess_ifndef.identifier, &identifier);
//       // char buf[1024];
//       // int is_defined;
//       // sprintf(buf,
//       //         "#ifndef %s\n"
//       //         "*((int *)%p) = 222;\n"
//       //         "#else\n"
//       //         "*((int *)%p) = 111;\n"
//       //         "#endif\n",
//       //         identifier, &is_defined, &is_defined);
//       // clint_process(buf);
//       // if (is_defined == 222) {
//       //   instantiate_ast_children(definitions_owner, source_file, child->preprocess_ifndef.groupopt);
//       // }
//       // else if (is_defined == 111) {
//       //   // Do Nothing
//       //   printf("'%s' was already defined\n", identifier);
//       // }
//       // else {
//       //   MCerror(950, "All did not go to plan");
//       // }
//       // free(identifier);
//     } break;
//     case MC_SYNTAX_PREPROCESSOR_DIRECTIVE_INCLUDE:
//       MCerror(1256, "TODO");
//     case MC_TOKEN_PP_KEYWORD_ENDIF:
//       MCerror(1259, "TODO");
//       break;
//     default: {
//       switch ((mc_token_type)child->type) {
//       case MC_TOKEN_SPACE_SEQUENCE:
//       case MC_TOKEN_NEW_LINE:
//       case MC_TOKEN_LINE_COMMENT:
//       case MC_TOKEN_MULTI_LINE_COMMENT: {
//         break;
//       }
//       default: {
//         print_syntax_node(child, 0);
//         MCerror(576, "Unhandled root-syntax-type:%s", get_mc_syntax_token_type_name(child->type));
//       }
//       }
//     }
//     }
//   }

//   return 0;
// }

// int register_external_definitions_from_file(mc_node *definitions_owner, char *filepath,
//                                             mc_source_file_info **source_file)
// {
//   char *file_text;
//   read_file_text(filepath, &file_text);

//   mc_syntax_node *syntax_node;
//   parse_file_to_syntax_tree(file_text, &syntax_node);

//   // Parse all definitions
//   mc_source_file_info *lv_source_file;
//   mc_init_source_file_info(definitions_owner, filepath, &lv_source_file);
//   if (source_file) {
//     *source_file = lv_source_file;
//   }

//   register_external_declarations_from_syntax_children(definitions_owner, lv_source_file, syntax_node->children);

//   // int *p = 0;
//   // printf("about\n");
//   // printf("%i\n", *p);
//   // printf("end\n");
//   register_midge_error_tag("instantiate_all_definitions_from_file(~)");
//   return 0;
// }