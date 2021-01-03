
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <sys/stat.h>
#include <unistd.h>

#include "midge_error_handling.h"

#include "core/c_parser_lexer.h"
#include "core/mc_code_transcription.h"

#include "mc_source.h"

int register_sub_type_syntax_to_field_info(mc_syntax_node *subtype_syntax, field_info *field);

int mc_register_function_info_to_app(function_info *func_info)
{
  mc_app_itp_data *app_itp_data;
  mc_obtain_app_itp_data(&app_itp_data);

  append_to_collection((void ***)&app_itp_data->functions.items, &app_itp_data->functions.alloc,
                       &app_itp_data->functions.count, (void *)func_info);

  return 0;
}

int mc_register_struct_info_to_app(struct_info *structure_info)
{
  mc_app_itp_data *app_itp_data;
  mc_obtain_app_itp_data(&app_itp_data);

  append_to_collection((void ***)&app_itp_data->structs.items, &app_itp_data->structs.alloc,
                       &app_itp_data->structs.count, (void *)structure_info);

  return 0;
}

int mc_register_enumeration_info_to_app(enumeration_info *enum_info)
{
  mc_app_itp_data *app_itp_data;
  mc_obtain_app_itp_data(&app_itp_data);

  append_to_collection((void ***)&app_itp_data->enumerations.items, &app_itp_data->enumerations.alloc,
                       &app_itp_data->enumerations.count, (void *)enum_info);

  return 0;
}

int mc_append_segment_to_source_file(mc_source_file_info *source_file, mc_source_file_code_segment_type type,
                                     void *data)
{
  mc_source_file_code_segment *segment = (mc_source_file_code_segment *)malloc(sizeof(mc_source_file_code_segment));
  segment->type = type;
  segment->data = data;

  MCcall(append_to_collection((void ***)&source_file->segments.items, &source_file->segments.capacity,
                              &source_file->segments.count, segment));

  return 0;
}

int _mc_find_segment_in_source_file(mc_source_file_info *source_file, mc_source_file_code_segment_type type, void *data,
                                    mc_source_file_code_segment **result)
{
  mc_source_file_code_segment *seg;
  for (int a = 0; a < source_file->segments.count; ++a) {
    seg = source_file->segments.items[a];

    if (seg->type == type && seg->data == data) {
      *result = seg;
      return 0;
    }
  }

  *result = NULL;
  return 0;
}

int initialize_parameter_info_from_syntax_node(mc_syntax_node *parameter_syntax_node, parameter_info *param_to_init)
{
  param_to_init->type_id = (struct_id *)malloc(sizeof(struct_id));
  param_to_init->type_id->identifier = strdup("parameter_info");
  param_to_init->type_id->version = 1U;

  switch (parameter_syntax_node->parameter.type) {
  case PARAMETER_KIND_STANDARD: {
    register_midge_error_tag("initialize_parameter_info_from_syntax_node-STANDARD");
    param_to_init->parameter_type = PARAMETER_KIND_STANDARD;

    // Name
    mcs_copy_syntax_node_to_text(parameter_syntax_node->parameter.name, (char **)&param_to_init->name);

    // Type
    mcs_copy_syntax_node_to_text(parameter_syntax_node->parameter.type_identifier, (char **)&param_to_init->type_name);
    if (parameter_syntax_node->parameter.type_dereference) {
      param_to_init->type_deref_count = parameter_syntax_node->parameter.type_dereference->dereference_sequence.count;
    }
    else {
      param_to_init->type_deref_count = 0;
    }

  } break;
  case PARAMETER_KIND_FUNCTION_POINTER: {
    register_midge_error_tag("initialize_parameter_info_from_syntax_node-FUNCTION_POINTER");
    param_to_init->parameter_type = PARAMETER_KIND_FUNCTION_POINTER;

    // Name
    mcs_copy_syntax_node_to_text(parameter_syntax_node->parameter.function_pointer->fptr_declarator.name,
                                 &param_to_init->name);

    // Type
    mcs_copy_syntax_node_to_text(parameter_syntax_node->parameter.type_identifier,
                                 (char **)&param_to_init->fptr.return_type);
    if (parameter_syntax_node->parameter.type_dereference) {
      param_to_init->fptr.return_deref_count =
          parameter_syntax_node->parameter.type_dereference->dereference_sequence.count;
    }
    else {
      param_to_init->fptr.return_deref_count = 0;
    }

    // print_syntax_node(parameter_syntax_node, 0);
  } break;
  case PARAMETER_KIND_VARIABLE_ARGS: {
    register_midge_error_tag("initialize_parameter_info_from_syntax_node-VARIABLE_ARGS");
    param_to_init->parameter_type = PARAMETER_KIND_VARIABLE_ARGS;

    param_to_init->type_name = NULL;
    param_to_init->type_deref_count = 0;
    param_to_init->name = NULL;

  } break;
  default:
    MCerror(125, "NotSupported:%i", parameter_syntax_node->parameter.type);
  }

  return 0;
}

int mcs_summarize_field_declarator_list(mc_syntax_node_list *syntax_declarators,
                                        field_declarator_info_list *field_declarators_list)
{
  if (!syntax_declarators) {
    field_declarators_list->count = 0;
    return 0;
  }

  field_declarators_list->count = 0;

  for (int d = 0; d < syntax_declarators->count; ++d) {
    mc_syntax_node *declarator_syntax = syntax_declarators->items[d];

    field_declarator_info *declarator = (field_declarator_info *)malloc(sizeof(field_declarator_info));
    if (declarator_syntax->field_declarator.function_pointer) {
      // Function pointer
      // print_syntax_node(declarator_syntax->field_declarator.function_pointer, 0);
      mcs_copy_syntax_node_to_text(declarator_syntax->field_declarator.function_pointer->fptr_declarator.name,
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
      mcs_copy_syntax_node_to_text(declarator_syntax->field_declarator.name, &declarator->name);
    }
    if (declarator_syntax->field_declarator.type_dereference) {
      declarator->deref_count = declarator_syntax->field_declarator.type_dereference->dereference_sequence.count;
    }
    else {
      declarator->deref_count = 0;
    }

    mc_syntax_node_list *array_dimensions = declarator_syntax->field_declarator.array_dimensions;
    declarator->array.dimension_count = array_dimensions->count;
    if (declarator->array.dimension_count) {
      declarator->array.dimensions = (char **)malloc(sizeof(char *) * array_dimensions->count);

      for (int a = 0; a < array_dimensions->count; ++a)
        MCcall(mcs_copy_syntax_node_to_text(array_dimensions->items[a], &declarator->array.dimensions[a]));
    }

    append_to_collection((void ***)&field_declarators_list->items, &field_declarators_list->alloc,
                         &field_declarators_list->count, declarator);
  }
  // case FIELD_KIND_FUNCTION_POINTER: {
  //   field->field_type = FIELD_KIND_FUNCTION_POINTER;

  //   mc_syntax_node *fps = field_syntax->field.function_pointer;
  //   mcs_copy_syntax_node_to_text(fps->function_pointer_declaration.identifier, &field->function_pointer.identifier);
  //   if (!fps->function_pointer_declaration.type_dereference)
  //     field->function_pointer.deref_count = 0;
  //   else
  //     field->function_pointer.deref_count =
  //         fps->function_pointer_declaration.type_dereference->dereference_sequence.count;

  //   // TODO -- more details
  // } break;

  return 0;
}

int mcs_summarize_type_field_list(mc_syntax_node_list *field_syntax_list, field_info_list *field_list)
{
  field_list->count = 0;

  for (int i = 0; i < field_syntax_list->count; ++i) {
    mc_syntax_node *field_syntax = field_syntax_list->items[i];

    field_info *field = (field_info *)malloc(sizeof(field_info));
    field->declarators.alloc = 0U;
    field->declarators.count = 0U;

    register_midge_error_tag("mcs_summarize_type_field_list-2");
    switch (field_syntax->type) {
    case MC_SYNTAX_FIELD_DECLARATION: {
      switch (field_syntax->field.type) {
      case FIELD_KIND_STANDARD: {
        field->field_type = FIELD_KIND_STANDARD;
        register_midge_error_tag("mcs_summarize_type_field_list-2b");
        MCcall(mcs_copy_syntax_node_to_text(field_syntax->field.type_identifier, &field->std.type_name));

        MCcall(mcs_summarize_field_declarator_list(field_syntax->field.declarators, &field->declarators));
      } break;
      default: {
        MCerror(302, "NotSupported:%i", field_syntax->field.type);
      }
      }
    } break;
    case MC_SYNTAX_NESTED_TYPE_DECLARATION: {
      register_midge_error_tag("mcs_summarize_type_field_list-2d");
      if (field_syntax->nested_type.declaration->type == MC_SYNTAX_UNION_DECL) {
        field->field_type = FIELD_KIND_NESTED_UNION;
      }
      else if (field_syntax->nested_type.declaration->type == MC_SYNTAX_STRUCT_DECL) {
        field->field_type = FIELD_KIND_NESTED_STRUCT;
      }
      else {
        MCerror(328, "Not Supported");
      }
      register_midge_error_tag("mcs_summarize_type_field_list-2e");

      // allocate_and_copy_cstr(field->name, field_syntax->nested_type.name->text);
      // printf("field_syntax:%p\n", field_syntax->nested_type.declaration);
      field->sub_type.fields = (field_info_list *)malloc(sizeof(field_info_list));
      field->sub_type.fields->alloc = 0U;
      field->sub_type.fields->count = 0U;
      MCcall(register_sub_type_syntax_to_field_info(field_syntax->nested_type.declaration, field));

      register_midge_error_tag("mcs_summarize_type_field_list-2f");
      if (field_syntax->nested_type.declarators) {
        field->sub_type.is_anonymous = false;

        MCcall(mcs_summarize_field_declarator_list(field_syntax->nested_type.declarators, &field->declarators));
      }
      else {
        field->sub_type.is_anonymous = !field->sub_type.type_name;
      }
    } break;
    default: {
      MCerror(317, "NotSupported:%s", get_mc_syntax_token_type_name(field_syntax->type));
    }
    }
    register_midge_error_tag("mcs_summarize_type_field_list-3");

    append_to_collection((void ***)&field_list->items, &field_list->alloc, &field_list->count, field);
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
  if (subtype_syntax->type == MC_SYNTAX_UNION_DECL) {
    field->sub_type.is_union = true;

    if (subtype_syntax->union_decl.type_name) {
      mcs_copy_syntax_node_to_text(subtype_syntax->union_decl.type_name, &field->sub_type.type_name);
      printf("sub-type:'%s'\n", field->sub_type.type_name);
    }
    else {
      field->sub_type.type_name = NULL;
    }

    if (!subtype_syntax->union_decl.fields) {
      MCerror(298, "Unexpected?");
    }

    MCcall(mcs_summarize_type_field_list(subtype_syntax->union_decl.fields, field->sub_type.fields));
  }
  else if (subtype_syntax->type == MC_SYNTAX_STRUCT_DECL) {
    field->sub_type.is_union = false;

    if (subtype_syntax->struct_decl.type_name) {
      mcs_copy_syntax_node_to_text(subtype_syntax->struct_decl.type_name, &field->sub_type.type_name);
    }
    else {
      field->sub_type.type_name = NULL;
    }

    if (!subtype_syntax->struct_decl.fields) {
      MCerror(396, "Unexpected?");
    }

    MCcall(mcs_summarize_type_field_list(subtype_syntax->struct_decl.fields, field->sub_type.fields));
  }
  else {
    MCerror(283, "NotSupported:%s", get_mc_syntax_token_type_name(subtype_syntax->type));
  }

  return 0;
}

int mcs_construct_dependency(function_info *dependent_function, const char *identity)
{
  struct_info *si;
  find_struct_info(identity, &si);
  if (si) {
  }

  enumeration_info *ei;
  find_enumeration_info(identity, &ei);
  if (ei) {
  }

  function_info *fi;
  find_function_info(identity, &fi);
  if (fi) {
  }

  return 0;
}

int mcs_determine_code_dependencies(function_info *dependent_function, mc_syntax_node *code_block) { return 0; }

int mcs_register_function_declaration(mc_source_file_info *source_file, mc_syntax_node *function_ast)
{
  int p;

  // Ensure a function info entry exists for the declared function
  function_info *fi;
  find_function_info(function_ast->function.name->text, &fi);

  if (!fi) {
    fi = (function_info *)calloc(1, sizeof(function_info));
    MCcall(mc_register_function_info_to_app(fi));

    fi->name = strdup(function_ast->function.name->text);
    fi->is_defined = false;

    // Return-type & Parameters
    MCcall(mcs_copy_syntax_node_to_text(function_ast->function.return_type_identifier, &fi->return_type.name));
    if (function_ast->function.return_type_dereference) {
      fi->return_type.deref_count = function_ast->function.return_type_dereference->dereference_sequence.count;
    }
    else {
      fi->return_type.deref_count = 0;
    }

    // fi->parameter_count = function_ast->function.parameters->count;
    // fi->parameters = (parameter_info **)malloc(sizeof(parameter_info *) * fi->parameter_count);
    for (p = 0; p < function_ast->function.parameters->count; ++p) {
      parameter_info *pi = (parameter_info *)malloc(sizeof(parameter_info));

      MCcall(initialize_parameter_info_from_syntax_node(function_ast->function.parameters->items[p], pi));

      MCcall(append_to_collection((void ***)&fi->parameters.items, &fi->parameters.alloc, &fi->parameters.count, pi));
    }
  }
  else {
    // TODO -- check equality of ast signature with previous declaration
  }

  // puts("bbb");
  if (function_ast->function.code_block) {
    if (fi->is_defined) {
      puts("Redefinition - TODO check");

      // TODO
      // Just make sure the parameter counts are even for now
      if (fi->parameters.count != function_ast->function.parameters->count) {
        MCerror(6873, "TODO");
      }
    }

    // Source
    if (fi->source) {
      if (fi->source != source_file) {
        MCerror(9484, "Source File Change for '%s'", fi->name);
      }
    }

    // Attach to the source file
    fi->source = source_file;
    MCcall(mc_append_segment_to_source_file(source_file, MC_SOURCE_SEGMENT_FUNCTION_DEFINITION, fi));

    fi->is_defined = true;
    MCcall(mcs_copy_syntax_node_to_text(function_ast->function.code_block, &fi->code));

    // puts("1");
    // Reset dependencies
    // TODO schedule dependencies
    fi->nb_dependencies = 0;
    // TODO
    // Move through the function code and determine the dependencies of the function(functions/structs/enums it uses)
    // mcs_determine_code_dependencies(fi, function_ast->function.code_block);

    // mc_syntax_node *param;
    // for (p = 0; p < function_ast->function.parameters->count; ++p) {
    //   param = function_ast->function.parameters->items[p];

    //   if (param->parameter.type_identifier &&
    //       param->parameter.type_identifier->type_identifier.identifier->type == MC_TOKEN_IDENTIFIER) {
    //     // TODO
    //     // mcs_construct_dependency(fi, param->parameter.type_identifier->type_identifier.identifier->text);
    //   }
    // }
  }
  else {
    MCcall(mc_append_segment_to_source_file(source_file, MC_SOURCE_SEGMENT_FUNCTION_DECLARATION, fi));
  }

  return 0;
}

int mcs_register_struct_declaration(mc_source_file_info *source_file, mc_syntax_node *struct_ast)
{
  if (!struct_ast->struct_decl.type_name) {
    MCerror(8461, "root-level anonymous external structures not yet supported");
  }

  struct_info *si;
  int a, b;
  bool is_definition;
  field_info *f;
  mc_syntax_node *sf;

  // Obtain the struct info
  find_struct_info(struct_ast->struct_decl.type_name->text, &si);

  if (!si) {
    si = (struct_info *)malloc(sizeof(struct_info));
    si->name = strdup(struct_ast->struct_decl.type_name->text);
    si->fields.alloc = 0U;
    si->fields.count = 0U;

    si->source_file = NULL;
    si->is_defined = false;

    mc_register_struct_info_to_app(si);
  }

  is_definition = false;
  if (struct_ast->type == MC_SYNTAX_STRUCT_DECL) {
    // Struct
    si->is_union = false;

    if (struct_ast->struct_decl.fields) {
      if (si->is_defined) {
        bool redef = false;
        if (si->fields.count == struct_ast->struct_decl.fields->count)
          redef = true;

        for (a = 0; !redef && a < si->fields.count; ++a) {
          f = si->fields.items[a];
          sf = struct_ast->struct_decl.fields->items[a];

          if (f->field_type == FIELD_KIND_NESTED_STRUCT && sf->type != MC_SYNTAX_STRUCT_DECL)
            redef = true;
          if (f->field_type == FIELD_KIND_NESTED_UNION && sf->type != MC_SYNTAX_UNION_DECL)
            redef = true;
          if (f->field_type == FIELD_KIND_STANDARD) {
            if (sf->type != MC_SYNTAX_FIELD_DECLARATION)
              redef = true;
            // if(!redef && sf->field.type_identifier)
            redef = true;
          }
        }
        if (redef) {
          // TODO -- Actual Redefinition -- For now just check that fields are not different at all
          printf("WARNING redefinition of struct '%s' TODO handle\n", si->name);
          printf("WARNING redefinition of struct '%s' TODO handle\n", si->name);
          printf("WARNING redefinition of struct '%s' TODO handle\n", si->name);
        }
      }

      // Define
      MCcall(mcs_summarize_type_field_list(struct_ast->struct_decl.fields, &si->fields));
      si->is_defined = is_definition = true;
    }
  }
  else if (struct_ast->type == MC_SYNTAX_UNION_DECL) {
    // Union
    si->is_union = true;

    if (struct_ast->struct_decl.fields) {
      if (si->is_defined) {
        MCerror(7428, "Redefinition - TODO check");
      }

      // Define
      MCcall(mcs_summarize_type_field_list(struct_ast->union_decl.fields, &si->fields));
      si->is_defined = is_definition = true;
    }
  }
  else {
    MCerror(8483, "Not struct or union?");
  }

  if (is_definition) {
    // Fill the Source Definition
    if (si->source_file && si->source_file != source_file) {
      MCerror(8384, "Source File Change for '%s' : from %p:'%s' to %p:'%s'", si->name, si->source_file,
              si->source_file->filepath, source_file, source_file->filepath);
    }

    si->source_file = source_file;
    MCcall(mc_append_segment_to_source_file(source_file, MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION, si));

    // // TODO -- can't the original code just be posted to it -- instead of having to generate more...
    // MCcall(mcs_copy_syntax_node_to_text(struct_ast, &si->source->code));
  }
  else {
    MCcall(mc_append_segment_to_source_file(source_file, MC_SOURCE_SEGMENT_STRUCTURE_DECLARATION, si));
  }

  return 0;
}

int mcs_process_ast_root_children(mc_source_file_info *source_file, mc_syntax_node_list *children)
{
  mc_syntax_node *child;
  for (int a = 0; a < children->count; ++a) {
    child = children->items[a];
    switch (child->type) {
    case MC_SYNTAX_EXTERN_C_BLOCK: {
      MCerror(1115, "TODO - Extern C Block Handling");
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
      // Function Declaration only
      MCcall(mcs_register_function_declaration(source_file, child));
    } break;
    case MC_SYNTAX_TYPE_ALIAS: {
      char buf[1024];
      switch (child->type_alias.type_descriptor->type) {
      case MC_SYNTAX_UNION_DECL:
      case MC_SYNTAX_STRUCT_DECL: {
        // print_syntax_node(child->type_alias.type_descriptor, 0);
        MCcall(mcs_register_struct_declaration(source_file, child->type_alias.type_descriptor));
        // MCerror(1151, "TODO");
        // struct_info *info;
        // instantiate_definition(definitions_owner, NULL, child->type_alias.type_descriptor, NULL, (void **)&info);
        // info->source->source_file = source_file;
        // // printf("--defined: struct '%s'\n", child->type_alias.type_descriptor->struct_decl.type_name->text);
        // // sprintf(buf,
        // //         "#ifndef %s\n"
        // //         // "#undef %s\n"
        // //         "#define %s struct %s\n"
        // //         "#endif\n",
        // //         info->name, info->name, info->mc_declared_name);
        // // clint_process(buf);
      } break;
      case MC_SYNTAX_ENUM_DECL: {
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
        MCerror(3547, "Unhandled type_alias-descriptor-syntax-type:%s",
                get_mc_syntax_token_type_name(child->type_alias.type_descriptor->type));
        break;
      }
    } break;
    case MC_SYNTAX_UNION_DECL:
    case MC_SYNTAX_STRUCT_DECL: {
      struct_info *info;
      MCcall(mcs_register_struct_declaration(source_file, child));
      // struct_info *info;
      // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
      // info->source->source_file = source_file;
      // // printf("--declared: struct '%s'\n", child->struct_decl.type_name->text);
    } break;
    case MC_SYNTAX_ENUM_DECL: {
      MCerror(1198, "TODO");
      // enumeration_info *info;
      // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
      // info->source->source_file = source_file;
      // // printf("--declared: enum '%s'\n", child->enumeration.name->text);
    } break;
    case MC_SYNTAX_GLOBAL_VARIABLE_DECLARATION:
    case MC_SYNTAX_PP_DIRECTIVE_DEFINE:
    case MC_SYNTAX_PP_DIRECTIVE_UNDEFINE: {
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
    case MC_SYNTAX_PP_DIRECTIVE_IFDEF: {
      // Assume all ifndefs (for the moment TODO ) to be true
      MCcall(mcs_process_ast_root_children(source_file, child->preprocess_ifdef.groupopt));
    } break;
    case MC_SYNTAX_PP_DIRECTIVE_IFNDEF: {
      // Assume all ifndefs (for the moment TODO ) to be true
      MCcall(mcs_process_ast_root_children(source_file, child->preprocess_ifndef.groupopt));
    } break;
    case MC_SYNTAX_PP_DIRECTIVE_INCLUDE: {
      mc_include_directive_info *incd = (mc_include_directive_info *)malloc(sizeof(mc_include_directive_info));
      incd->is_system_search = child->include_directive.is_system_header_search;
      MCcall(mcs_copy_syntax_node_to_text(child->include_directive.filepath, &incd->filepath));

      MCcall(mc_append_segment_to_source_file(source_file, MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE, incd));
    } break;
    default: {
      switch ((mc_token_type)child->type) {
      case MC_TOKEN_PP_KEYWORD_IFDEF:
      case MC_TOKEN_PP_KEYWORD_IFNDEF:
      case MC_TOKEN_PP_KEYWORD_ENDIF:
        MCerror(5313, "TODO");
      case MC_TOKEN_NEW_LINE: {
        if (a && children->items[a - 1]->type == MC_TOKEN_NEW_LINE) {
          MCcall(mc_append_segment_to_source_file(source_file, MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR, NULL));
        }
      } break;
      case MC_TOKEN_SPACE_SEQUENCE:
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

/* interpret file to the global interpreter and process for midge.
 */
int mcs_interpret_source_file(const char *filepath, mc_source_file_info **source_file)
{
  int res, a;
  char *code;
  mc_syntax_node *file_ast;
  mc_app_itp_data *app_itp_data;

  MCcall(mc_obtain_app_itp_data(&app_itp_data));

  printf("interpreting...'%s'\n", filepath);

  // Read the file
  MCcall(read_file_text(filepath, &code));

  // Parse the file into an AST
  MCcall(parse_file_to_syntax_tree(code, &file_ast));
  free(code);

  // Ensure the full path is written
  char fullpath[256];
  if (filepath[0] != '/') {
    // Given filepath is relative -- convert to absolute
    getcwd(fullpath, 256);
    char c = fullpath[strlen(fullpath) - 1];
    if (c != '\\' && c != '/') {
      strcat(fullpath, "/");
    }
    strcat(fullpath, filepath);
  }
  else {
    strcpy(fullpath, filepath);
  }

  // Search for already existing file info
  mc_source_file_info *sf = NULL, *isf;
  int nb_before_segs = 0;
  mc_source_file_code_segment **segs = NULL, *seg, *seg2;
  for (a = 0; a < app_itp_data->source_files.count; ++a) {
    isf = app_itp_data->source_files.items[a];

    if (!strcmp(fullpath, isf->filepath)) {
      // Use this
      sf = isf;
      break;
    }
  }

  if (sf) {
    nb_before_segs = sf->segments.count;
    if (nb_before_segs) {
      segs = (mc_source_file_code_segment **)malloc(sizeof(mc_source_file_code_segment *) * nb_before_segs);
      memcpy(segs, sf->segments.items, sizeof(mc_source_file_code_segment *) * nb_before_segs);
    }

    sf->segments.count = 0U;
  }
  else {
    // Set
    sf = malloc(sizeof(mc_source_file_info));
    sf->segments.capacity = sf->segments.count = 0U;
    sf->filepath = strdup(fullpath);
    MCcall(append_to_collection((void ***)&app_itp_data->source_files.items, &app_itp_data->source_files.alloc,
                                &app_itp_data->source_files.count, sf));
  }

  // Obtain function/struct/enum information & dependencies
  MCcall(mcs_process_ast_root_children(sf, file_ast->children));

  if (nb_before_segs) {
    for (a = 0; a < nb_before_segs; ++a) {
      seg = segs[a];

      switch (seg->type) {
      case MC_SOURCE_SEGMENT_INCLUDE_DIRECTIVE: {
        if (seg->include) {
          if (seg->include->filepath)
            free(seg->include->filepath);
          free(seg->include);
        }
        free(seg);
      } break;
      case MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION: {
        MCcall(_mc_find_segment_in_source_file(sf, MC_SOURCE_SEGMENT_STRUCTURE_DEFINITION, seg->data, &seg2));
        if (!seg2) {
          MCerror(6721, "Unsupported - redefining a structure out of a file");
        }

        // Just delete the segment
        free(seg);
      } break;
      case MC_SOURCE_SEGMENT_NEWLINE_SEPERATOR:
      case MC_SOURCE_SEGMENT_FUNCTION_DECLARATION:
      case MC_SOURCE_SEGMENT_FUNCTION_DEFINITION:
      case MC_SOURCE_SEGMENT_STRUCTURE_DECLARATION: {
        // Just delete the segment
        // TODO -- this probably isn't right
        free(seg);
      } break;
      default:
        MCerror(8707, "Unsupported type : %i", seg->type);
      }
    }
  }

  // // DEBUG -- FILE GENERATION TEST
  // MCcall(mcdebug_generate_files_from_sf_info(sf));
  // // DEBUG

  // Generate code (from the AST) with midge insertions integrated (stack call / error tracking)
  {
    mct_function_transcription_options options = {};
    options.report_function_entry_exit_to_stack = true;
    options.report_simple_args_to_error_stack = true;
    options.tag_on_function_entry = false;
    options.tag_on_function_exit = false;
    options.report_variable_values = NULL;

    // printf("transcribing '%s'...\n", filepath);
    MCcall(mct_transcribe_file_ast(file_ast, &options, &code));
  }

  // if (!strcmp("src/modules/modus_operandi/init_modus_operandi.c", filepath)) {
  //   // usleep(10000);
  //   // printf("\ngen-code:\n%s||\n", code);
  //   save_text_to_file("src/temp/todelete.h", code);
  //   // MCerror(7704, "TODO");
  // }

  // Send the code to the interpreter
  {
    int mc_res = tcci_add_string(app_itp_data->interpreter, filepath, code);
    if (mc_res) {
      printf("--"
             "tcci_add_string(app_itp_data->interpreter, filepath, code)"
             "line:%i:ERR:%i\n",
             __LINE__ - 5, mc_res);
      save_text_to_file("src/temp/todelete.h", code);
      return mc_res;
    }
  }

  // Cleanup
  free(code);
  MCcall(release_syntax_node(file_ast));
  free(file_ast);

  if (source_file)
    *source_file = sf;

  return 0;
}

int mcs_interpret_file(const char *filepath)
{
  MCcall(mcs_interpret_source_file(filepath, NULL));

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

// int append_syntax_node_to_file_context(mc_syntax_node *child, mc_str *file_context)
// {
//   char *code;
//   mcs_copy_syntax_node_to_text(child, &code);
//   append_to_mc_str(file_context, code);
//   append_char_to_mc_str(file_context, '\n');
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
//   if (!struct_ast->struct_decl.type_name) {
//     MCerror(8461, "root-level anonymous structures not yet supported");
//   }

//   struct_info *structure_info;
//   // printf("ursif0:'%p'\n", struct_ast);
//   // printf("ursif1:'%p'\n", struct_ast->struct_decl.type_name);
//   // printf("ursif2:'%p'\n", struct_ast->struct_decl.type_name->text);
//   // printf("ursif3:'%s'\n", struct_ast->struct_decl.type_name->text);
//   if (struct_ast->struct_decl.type_name) {
//     find_struct_info(struct_ast->struct_decl.type_name->text, &structure_info);
//   }

//   register_midge_error_tag("update_or_register_struct_info_from_syntax-1");
//   if (!structure_info) {
//     structure_info = (struct_info *)malloc(sizeof(struct_info));

//     mc_register_struct_info_to_app(owner, structure_info);

//     structure_info->type_id = (struct_id *)malloc(sizeof(struct_id));
//     allocate_and_copy_cstr(structure_info->type_id->identifier, "struct_info");
//     structure_info->type_id->version = 1U;

//     // Name & Version
//     if (struct_ast->struct_decl.type_name) {
//       allocate_and_copy_cstr(structure_info->name, struct_ast->struct_decl.type_name->text);
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

//   structure_info->is_union = struct_ast->type == MC_SYNTAX_UNION_DECL;
//   mc_str *mc_func_name;
//   init_mc_str(&mc_func_name);
//   append_to_mc_strf(mc_func_name, "%s_mc_v%u", structure_info->name, structure_info->latest_iteration);
//   structure_info->mc_declared_name = mc_func_name->text;
//   release_mc_str(mc_func_name, false);

//   // Set the values parsed
//   if (struct_ast->struct_decl.fields) {
//     structure_info->is_defined = true;

//     mcs_summarize_type_field_list(struct_ast->struct_decl.fields, &structure_info->fields);
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

//     mc_register_enumeration_info_to_app(owner, enum_info);

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

//     mcs_copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.identifier,
//     &member->identity); if (enum_ast->enumeration.members->items[i]->enum_member.value_expression) {
//       mcs_copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.value_expression,
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

// int instantiate_function_definition_from_ast(mc_node *definition_owner, mc_source_definition *source, mc_str
// *file_context,
//                                              mc_syntax_node *ast, void **definition_info)
// {
//   // Register Function
//   function_info *func_info;
//   update_or_register_function_info_from_syntax(definition_owner, ast, &func_info);

//   // Instantiate Function
//   char *mc_transcription;
//   mct_function_transcription_options options = {};
//   options.report_function_entry_exit_to_stack = false;
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

// int register_external_struct_declaration(mc_node *owner, mc_syntax_node *struct_ast)
// {
//   if (!struct_ast->struct_decl.type_name) {
//     MCerror(8461, "root-level anonymous external structures not yet supported");
//   }

//   struct_info *structure_info;
//   find_struct_info(struct_ast->struct_decl.type_name->text, &structure_info);
//   if (structure_info) {
//     MCerror(1013, "TODO?");
//   }

//   // register_midge_error_tag("update_or_register_struct_info_from_syntax-1");
//   if (!structure_info) {
//     structure_info = (struct_info *)malloc(sizeof(struct_info));

//     mc_register_struct_info_to_app(owner, structure_info);

//     structure_info->type_id = (struct_id *)malloc(sizeof(struct_id));
//     allocate_and_copy_cstr(structure_info->type_id->identifier, "struct_info");
//     structure_info->type_id->version = 1U;

//     // Name & Version
//     if (struct_ast->struct_decl.type_name) {
//       allocate_and_copy_cstr(structure_info->name, struct_ast->struct_decl.type_name->text);
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

//   structure_info->is_union = struct_ast->type == MC_SYNTAX_UNION_DECL;
//   structure_info->mc_declared_name = NULL;

//   // Set the values parsed
//   if (struct_ast->struct_decl.fields) {
//     structure_info->is_defined = true;

//     mcs_summarize_type_field_list(struct_ast->struct_decl.fields, &structure_info->fields);
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
//     mc_register_enumeration_info_to_app(owner, enum_info);
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

//     mcs_copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.identifier,
//     &member->identity);
//     // printf("reed-8d\n");
//     if (enum_ast->enumeration.members->items[i]->enum_member.value_expression) {
//       mcs_copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.value_expression,
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
//       case MC_SYNTAX_UNION_DECL:
//       case MC_SYNTAX_STRUCT_DECL: {
//         register_external_struct_declaration(definitions_owner, child->type_alias.type_descriptor);
//         // MCerror(1151, "TODO");
//         // struct_info *info;
//         // instantiate_definition(definitions_owner, NULL, child->type_alias.type_descriptor, NULL, (void
//         **)&info);
//         // info->source->source_file = source_file;
//         // // printf("--defined: struct '%s'\n", child->type_alias.type_descriptor->struct_decl.type_name->text);
//         // // sprintf(buf,
//         // //         "#ifndef %s\n"
//         // //         // "#undef %s\n"
//         // //         "#define %s struct %s\n"
//         // //         "#endif\n",
//         // //         info->name, info->name, info->mc_declared_name);
//         // // clint_process(buf);
//       } break;
//       case MC_SYNTAX_ENUM_DECL: {
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
//     case MC_SYNTAX_STRUCT_DECL: {
//       MCerror(1191, "TODO");
//       // struct_info *info;
//       // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
//       // info->source->source_file = source_file;
//       // // printf("--declared: struct '%s'\n", child->struct_decl.type_name->text);
//     } break;
//     case MC_SYNTAX_ENUM_DECL: {
//       MCerror(1198, "TODO");
//       // enumeration_info *info;
//       // instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
//       // info->source->source_file = source_file;
//       // // printf("--declared: enum '%s'\n", child->enumeration.name->text);
//     } break;
//     case MC_SYNTAX_PP_DIRECTIVE_DEFINE: {
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
//     case MC_SYNTAX_PP_DIRECTIVE_IFNDEF: {
//       MCerror(1220, "TODO");
//       // char *identifier;
//       // mcs_copy_syntax_node_to_text(child->preprocess_ifndef.identifier, &identifier);
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
//     case MC_SYNTAX_PP_DIRECTIVE_INCLUDE:
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