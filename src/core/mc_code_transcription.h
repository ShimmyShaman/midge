/* mc_code_transcription.h */

#ifndef MC_CODE_TRANSCRIPITION_H
#define MC_CODE_TRANSCRIPITION_H

#include "midge_common.h"

#include "core/c_parser_lexer.h"

extern "C" {
int transcribe_function_to_mc(function_info *func_info, mc_syntax_node *function_ast, char **mc_transcription);
int transcribe_enumeration_to_mc(enumeration_info *enum_info, mc_syntax_node *enumeration_ast, char **mc_transcription);
int transcribe_struct_to_mc(struct_info *structure_info, mc_syntax_node *struct_ast, char **mc_transcription);
}

#endif // MC_CODE_TRANSCRIPITION_H