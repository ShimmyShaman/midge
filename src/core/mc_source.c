#include "core/c_parser_lexer.h"
#include "core/core_definitions.h"
#include "midge_common.h"

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

  switch (parameter_syntax_node->parameter.parameter_kind) {
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
    MCerror(125, "NotSupported:%i", parameter_syntax_node->parameter.parameter_kind);
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
    printf("--declared:'%s'\n", func_info->name);

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
    structure_info->fields.alloc = 0;
    structure_info->fields.count = 0;
    structure_info->version = 1U;
    structure_info->source = NULL;
  }
  else {
    MCerror(213, "TODO");
    // // Empty
    // // Clear the current values
    // for (int i = 0; i < structure_info->fields.count; ++i)
    //   if (structure_info->fields.items[i]) {
    //     release_field_info((field_info *)structure_info->fields.items[i]);
    //   }

    // ++structure_info->version;
  }
  register_midge_error_tag("update_or_register_struct_info_from_syntax-2");

  cprintf(structure_info->mc_declared_name, "mc_%s_v%u", structure_info->name, structure_info->version);

  // Set the values parsed
  structure_info->fields.count = 0;
  for (int i = 0; i < struct_ast->structure.fields->count; ++i) {
    field_info *field = (field_info *)malloc(sizeof(field_info));

    field->type_id = (struct_id *)malloc(sizeof(struct_id));
    allocate_and_copy_cstr(field->type_id->identifier, "field_info");
    field->type_id->version = 1U;

    register_midge_error_tag("update_or_register_struct_info_from_syntax-2a");
    mc_syntax_node *field_syntax = struct_ast->structure.fields->items[i];
    switch (field_syntax->field.field_kind) {
    case FIELD_KIND_STANDARD: {
      copy_syntax_node_to_text(field_syntax->field.type_identifier, &field->type);

      field->declarators.alloc = 0;
      field->declarators.count = 0;

      for (int d = 0; d < field_syntax->field.declarators->count; ++d) {
        mc_syntax_node *declarator_syntax = field_syntax->field.declarators->items[d];

        field_declarator_info *declarator = (field_declarator_info *)malloc(sizeof(field_declarator_info));
        copy_syntax_node_to_text(declarator_syntax->field_declarator.name, &declarator->name);
        if (declarator_syntax->field_declarator.type_dereference) {
          declarator->deref_count = declarator_syntax->field_declarator.type_dereference->dereference_sequence.count;
        }
        else {
          declarator->deref_count = 0;
        }

        append_to_collection((void ***)&field->declarators.items, &field->declarators.alloc, &field->declarators.count,
                             declarator);
      }
    } break;
    default: {
      MCerror(310, "NotSupported:%i", field_syntax->field.field_kind);
    }
    }

    register_midge_error_tag("update_or_register_struct_info_from_syntax-2d");

    append_to_collection((void ***)&structure_info->fields.items, &structure_info->fields.alloc,
                         &structure_info->fields.count, field);
    register_midge_error_tag("update_or_register_struct_info_from_syntax-2e");
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
  if (!enum_info) {
    enum_info = (enumeration_info *)malloc(sizeof(enumeration_info));

    attach_enumeration_info_to_owner(owner, enum_info);

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
    enum_member *member = (enum_member *)malloc(sizeof(enum_member));

    copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.identifier, &member->identity);
    if (enum_ast->enumeration.members->items[i]->enum_member.value) {
      copy_syntax_node_to_text(enum_ast->enumeration.members->items[i]->enum_member.value, &member->value);
    }
    else {
      member->value = NULL;
    }

    append_to_collection((void ***)&enum_info->members.items, &enum_info->members.alloc, &enum_info->members.count,
                         member);
  }

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

  // printf("mc_transcription:\n%s||\n", mc_transcription);
  clint_declare(mc_transcription);
  // printf("idfc-5\n");
  char buf[512];
  sprintf(buf, "%s = &%s_mcv%u;", func_info->name, func_info->name, func_info->latest_iteration);
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

  clint_declare(mc_transcription);

  if (definition_info) {
    *definition_info = structure_info;
  }

  register_midge_error_tag("instantiate_struct_definition_from_ast(~)");
  return 0;
}

int instantiate_enum_definition_from_ast(node *definition_owner, source_definition *source, mc_syntax_node *ast,
                                         void **definition_info)
{
  MCerror(297, "TODO");
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
    MCerror(325, "only functions supported atm\n");
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
  register_midge_error_tag("instantiate_all_definitions_from_file()");
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
    case MC_SYNTAX_PREPROCESSOR_DIRECTIVE: {
      break;
    }
    case MC_SYNTAX_FUNCTION: {
      if ((mc_token_type)child->function.code_block->type == MC_TOKEN_SEMI_COLON) {
        // Function Declaration only
        update_or_register_function_info_from_syntax(NULL, child, NULL);
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
        printf("--defined:'%s'\n", child->type_alias.type_descriptor->structure.type_name->text);
        sprintf(buf,
                "#ifdef %s\n"
                "#undef %s\n"
                "#endif\n"
                "#define %s struct %s\n",
                info->mc_declared_name, info->mc_declared_name, info->mc_declared_name, info->mc_declared_name);
        clint_process(buf);
      } break;
      default:
        MCerror(576, "Unhandled type_alias-descriptor-syntax-type:%i", child->type_alias.type_descriptor->type);
        break;
      }

      // alias_info *info;
    } break;
    case MC_SYNTAX_STRUCTURE: {
      struct_info *info;
      instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
      info->source->source_file = lv_source_file;
      printf("--defined:'%s'\n", child->structure.type_name->text);
    } break;
    case MC_SYNTAX_ENUM: {
      enumeration_info *info;
      instantiate_definition(definitions_owner, NULL, child, NULL, (void **)&info);
      info->source->source_file = lv_source_file;
      printf("--defined:'%s'\n", child->enumeration.name->text);
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
        print_syntax_node(child, 0);
        MCerror(576, "Unhandled root-syntax-type:%i", child->type);
      }
      }
    }
    }
  }

  register_midge_error_tag("instantiate_all_definitions_from_file(~)");
  return 0;
}