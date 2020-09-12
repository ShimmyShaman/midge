/* core_definitions.h */

#ifndef MC_SOURCE_H
#define MC_SOURCE_H

#include "core/c_parser_lexer.h"
#include "midge_common.h"
#include <stddef.h>

extern "C" {
int register_external_definitions_from_file(mc_node *definitions_owner, char *filepath, source_file_info **source_file);
int instantiate_all_definitions_from_file(mc_node *definitions_owner, char *filepath, source_file_info **source_file);

/*
  From code definition: constructs source definition & parses to syntax, registers with hierarchy, and declares the
  definition for immediate use.
  @definition_owner the node in the hierarchy to attach this definition to.
  @code may be NULL only if ast is not, if so it will be generated from the syntax parse.
  @ast may be NULL only if code is not, if so it will be parsed from the code.
  @source may be NULL, if so it will be created.
  @definition_info is OUT. May be NULL, if not dereference will be set with p-to-function_info/struct_info/enum_info
  etc.
*/
int instantiate_definition(mc_node *definition_owner, char *code, mc_syntax_node *ast, source_definition *source,
                           void **definition_info);
};

#endif // MC_SOURCE_H