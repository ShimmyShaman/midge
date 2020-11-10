/* c_parser_lexer.h */

#ifndef C_PARSER_LEXER_H
#define C_PARSER_LEXER_H

#include "core/core_definitions.h"

typedef enum mc_token_type {
  MC_TOKEN_NULL = 0,
  MC_TOKEN_NULL_CHARACTER,
  MC_TOKEN_PREPROCESSOR_KEYWORD_DEFINE,
  MC_TOKEN_PREPROCESSOR_KEYWORD_INCLUDE,
  MC_TOKEN_PREPROCESSOR_KEYWORD_IFNDEF,
  MC_TOKEN_PREPROCESSOR_KEYWORD_ENDIF,
  MC_TOKEN_PREPROCESSOR_KEYWORD_VARIADIC_ARGS,
  MC_TOKEN_STAR_CHARACTER,
  MC_TOKEN_ESCAPE_CHARACTER,
  MC_TOKEN_MULTIPLY_OPERATOR,
  MC_TOKEN_DEREFERENCE_OPERATOR,
  MC_TOKEN_IDENTIFIER,
  MC_TOKEN_SQUARE_OPENING_BRACKET,
  MC_TOKEN_SQUARE_CLOSING_BRACKET,
  MC_TOKEN_OPENING_BRACKET,
  MC_TOKEN_CLOSING_BRACKET,
  MC_TOKEN_SEMI_COLON,
  MC_TOKEN_COLON,
  MC_TOKEN_DECREMENT_OPERATOR,
  MC_TOKEN_INCREMENT_OPERATOR,
  MC_TOKEN_POINTER_OPERATOR,
  MC_TOKEN_ASSIGNMENT_OPERATOR,
  MC_TOKEN_LOGICAL_NOT_OPERATOR,
  MC_TOKEN_SUBTRACT_OPERATOR,
  MC_TOKEN_PLUS_OPERATOR,
  MC_TOKEN_DIVIDE_OPERATOR,
  MC_TOKEN_MODULO_OPERATOR,
  MC_TOKEN_TERNARY_OPERATOR,
  MC_TOKEN_SUBTRACT_AND_ASSIGN_OPERATOR,
  MC_TOKEN_PLUS_AND_ASSIGN_OPERATOR,
  MC_TOKEN_MULTIPLY_AND_ASSIGN_OPERATOR,
  MC_TOKEN_DIVIDE_AND_ASSIGN_OPERATOR,
  MC_TOKEN_MODULO_AND_ASSIGN_OPERATOR,
  MC_TOKEN_TYPEDEF_KEYWORD,
  MC_TOKEN_EXTERN_KEYWORD,
  MC_TOKEN_IF_KEYWORD,
  MC_TOKEN_ELSE_KEYWORD,
  MC_TOKEN_GOTO_KEYWORD,
  MC_TOKEN_WHILE_KEYWORD,
  MC_TOKEN_DO_KEYWORD,
  MC_TOKEN_FOR_KEYWORD,
  MC_TOKEN_SWITCH_KEYWORD,
  MC_TOKEN_CONTINUE_KEYWORD,
  MC_TOKEN_BREAK_KEYWORD,
  MC_TOKEN_RETURN_KEYWORD,
  MC_TOKEN_CONST_KEYWORD,
  MC_TOKEN_STATIC_KEYWORD,
  MC_TOKEN_SIZEOF_KEYWORD,
  MC_TOKEN_OFFSETOF_KEYWORD,
  MC_TOKEN_VA_ARG_WORD,
  MC_TOKEN_VA_LIST_WORD,
  MC_TOKEN_VA_START_WORD,
  MC_TOKEN_VA_END_WORD,
  MC_TOKEN_CURLY_OPENING_BRACKET,
  MC_TOKEN_CURLY_CLOSING_BRACKET,
  MC_TOKEN_NEW_LINE,
  MC_TOKEN_TAB_SEQUENCE,
  MC_TOKEN_SPACE_SEQUENCE,
  MC_TOKEN_LINE_COMMENT,
  MC_TOKEN_MULTI_LINE_COMMENT,
  MC_TOKEN_DECIMAL_POINT,
  MC_TOKEN_NUMERIC_LITERAL,
  MC_TOKEN_STRING_LITERAL,
  MC_TOKEN_CHAR_LITERAL,
  MC_TOKEN_COMMA,
  MC_TOKEN_LESS_THAN_OR_EQUAL_OPERATOR,
  MC_TOKEN_ARROW_OPENING_BRACKET,
  MC_TOKEN_MORE_THAN_OR_EQUAL_OPERATOR,
  MC_TOKEN_ARROW_CLOSING_BRACKET,
  MC_TOKEN_BITWISE_LEFT_SHIFT_OPERATOR,
  MC_TOKEN_BITWISE_LEFT_SHIFT_AND_ASSIGN_OPERATOR,
  MC_TOKEN_BITWISE_RIGHT_SHIFT_OPERATOR,
  MC_TOKEN_BITWISE_RIGHT_SHIFT_AND_ASSIGN_OPERATOR,
  MC_TOKEN_LOGICAL_AND_OPERATOR,
  MC_TOKEN_BINARY_AND_ASSIGNMENT_OPERATOR,
  MC_TOKEN_AMPERSAND_CHARACTER,
  MC_TOKEN_LOGICAL_OR_OPERATOR,
  MC_TOKEN_BINARY_OR_ASSIGNMENT_OPERATOR,
  MC_TOKEN_BITWISE_NOT_OPERATOR,
  MC_TOKEN_BITWISE_XOR_OPERATOR,
  MC_TOKEN_BITWISE_OR_OPERATOR,
  MC_TOKEN_EQUALITY_OPERATOR,
  MC_TOKEN_INEQUALITY_OPERATOR,
  MC_TOKEN_CASE_KEYWORD,
  MC_TOKEN_DEFAULT_KEYWORD,
  MC_TOKEN_STRUCT_KEYWORD,
  MC_TOKEN_UNION_KEYWORD,
  MC_TOKEN_ENUM_KEYWORD,
  MC_TOKEN_VOID_KEYWORD,
  MC_TOKEN_CHAR_KEYWORD,
  MC_TOKEN_INT_KEYWORD,
  MC_TOKEN_SIGNED_KEYWORD,
  MC_TOKEN_UNSIGNED_KEYWORD,
  MC_TOKEN_FLOAT_KEYWORD,
  MC_TOKEN_LONG_KEYWORD,
  MC_TOKEN_SHORT_KEYWORD,
  MC_TOKEN_EXCLUSIVE_MAX_VALUE = 200,
} mc_token_type;

typedef enum mc_syntax_node_type {
  MC_SYNTAX_ROOT = MC_TOKEN_EXCLUSIVE_MAX_VALUE,
  MC_SYNTAX_FILE_ROOT,
  MC_SYNTAX_PREPROCESSOR_DIRECTIVE_IFNDEF,
  MC_SYNTAX_PREPROCESSOR_DIRECTIVE_INCLUDE,
  MC_SYNTAX_PREPROCESSOR_DIRECTIVE_DEFINE,
  MC_SYNTAX_EXTERN_C_BLOCK,
  MC_SYNTAX_FUNCTION,
  MC_SYNTAX_TYPE_ALIAS,
  MC_SYNTAX_STRUCTURE,
  MC_SYNTAX_UNION,
  MC_SYNTAX_ENUM,
  MC_SYNTAX_ENUM_MEMBER,
  MC_SYNTAX_NESTED_TYPE_DECLARATION,
  MC_SYNTAX_CODE_BLOCK,
  MC_SYNTAX_STATEMENT_LIST,
  MC_SYNTAX_FOR_STATEMENT,
  MC_SYNTAX_WHILE_STATEMENT,
  MC_SYNTAX_IF_STATEMENT,
  MC_SYNTAX_SWITCH_STATEMENT,
  MC_SYNTAX_SWITCH_SECTION,
  MC_SYNTAX_SWITCH_CASE_LABEL,
  MC_SYNTAX_SWITCH_DEFAULT_LABEL,
  MC_SYNTAX_CONTINUE_STATEMENT,
  MC_SYNTAX_BREAK_STATEMENT,
  MC_SYNTAX_EXPRESSION_STATEMENT,
  MC_SYNTAX_GOTO_STATEMENT,
  MC_SYNTAX_LABEL_STATEMENT,
  MC_SYNTAX_DECLARATION_STATEMENT,
  MC_SYNTAX_LOCAL_VARIABLE_DECLARATION,
  MC_SYNTAX_LOCAL_VARIABLE_DECLARATOR,
  MC_SYNTAX_LOCAL_VARIABLE_ARRAY_INITIALIZER,
  MC_SYNTAX_LOCAL_VARIABLE_ASSIGNMENT_INITIALIZER,
  MC_SYNTAX_ASSIGNMENT_EXPRESSION,
  MC_SYNTAX_RETURN_STATEMENT,
  MC_SYNTAX_INVOCATION,

  MC_SYNTAX_SUPERNUMERARY,
  MC_SYNTAX_INCLUDE_SYSTEM_HEADER_IDENTITY,
  MC_SYNTAX_TYPE_IDENTIFIER,
  MC_SYNTAX_FUNCTION_POINTER_DECLARATOR,
  MC_SYNTAX_FUNCTION_POINTER_DECLARATION,
  MC_SYNTAX_DEREFERENCE_SEQUENCE,
  MC_SYNTAX_PARAMETER_DECLARATION,
  MC_SYNTAX_FIELD_DECLARATION,
  MC_SYNTAX_FIELD_DECLARATOR,
  MC_SYNTAX_STRING_LITERAL_EXPRESSION,
  MC_SYNTAX_CAST_EXPRESSION,
  MC_SYNTAX_PARENTHESIZED_EXPRESSION,
  MC_SYNTAX_TYPE_INITIALIZER,
  MC_SYNTAX_INITIALIZER_EXPRESSION,
  MC_SYNTAX_SIZEOF_EXPRESSION,
  MC_SYNTAX_OFFSETOF_EXPRESSION,
  MC_SYNTAX_VA_ARG_EXPRESSION,
  MC_SYNTAX_VA_LIST_STATEMENT,
  MC_SYNTAX_VA_START_STATEMENT,
  MC_SYNTAX_VA_END_STATEMENT,
  MC_SYNTAX_PREPENDED_UNARY_EXPRESSION,
  MC_SYNTAX_LOGICAL_EXPRESSION,
  MC_SYNTAX_BITWISE_EXPRESSION,
  MC_SYNTAX_TERNARY_CONDITIONAL,
  MC_SYNTAX_RELATIONAL_EXPRESSION,
  MC_SYNTAX_OPERATIONAL_EXPRESSION,
  MC_SYNTAX_MEMBER_ACCESS_EXPRESSION,
  MC_SYNTAX_ELEMENT_ACCESS_EXPRESSION,
  MC_SYNTAX_FIXREMENT_EXPRESSION,
  MC_SYNTAX_DEREFERENCE_EXPRESSION,
} mc_syntax_node_type;

typedef struct mc_syntax_node mc_syntax_node;
typedef struct mc_syntax_node_list {
  unsigned int alloc;
  unsigned int count;
  struct mc_syntax_node **items;
} mc_syntax_node_list;

struct mc_syntax_node {
  mc_syntax_node_type type;
  mc_syntax_node *parent;
  struct {
    int line, col;
    int index;
  } begin;
  union {
    // Text -- only used by MC_TOKENS -- All syntax tokens use below fields (as appropriate)
    char *text;
    struct {
      mc_syntax_node_list *children;
      union {
        struct {
          mc_syntax_node *return_type_identifier;
          // May be null indicating no dereference operators
          mc_syntax_node *return_type_dereference;
          mc_syntax_node *name;
          mc_syntax_node_list *parameters;
          mc_syntax_node *code_block;
        } function;
        struct {
          mc_syntax_node_list *declarations;
        } extern_block;
        struct {
          mc_syntax_node *type_descriptor;
          mc_syntax_node *alias;
        } type_alias;
        struct {
          mc_syntax_node *type_name;
          mc_syntax_node_list *fields;
        } structure;
        struct {
          mc_syntax_node *type_name;
          mc_syntax_node_list *fields;
        } union_decl;
        struct {
          mc_syntax_node *name;
          mc_syntax_node *type_identity;
          mc_syntax_node_list *members;
        } enumeration;
        struct {
          mc_syntax_node *identifier;
          mc_syntax_node *value_expression;
        } enum_member;
        struct {
          mc_syntax_node *declaration;
          mc_syntax_node_list *declarators;
        } nested_type;
        struct {
          mc_syntax_node *identifier;
          mc_syntax_node_list *groupopt;
        } preprocess_ifndef;
        struct {

        } preprocess_include;
        struct {
          preprocessor_define_type statement_type;
          mc_syntax_node_list *replacement_list;
          mc_syntax_node *identifier;
        } preprocess_define;
        struct {
          bool is_system_header_search;
          mc_syntax_node *filepath;
        } include_directive;
        struct {
          unsigned int count;
        } dereference_sequence;
        struct {
          parameter_kind type;
          mc_syntax_node *type_identifier;
          // May be NULL indicating no dereference operators (will be NULL for variable_args)
          mc_syntax_node *type_dereference;
          // Will be NULL if variable_args or function_pointer
          mc_syntax_node *name;
          mc_syntax_node *function_pointer;
        } parameter;
        struct {
          field_kind type;
          union {
            mc_syntax_node *type_identifier;
            mc_syntax_node *nested_struct;
            mc_syntax_node *nested_union;
            mc_syntax_node *function_pointer;
          };
          mc_syntax_node_list *declarators;
        } field;
        struct {
          mc_syntax_node *type_dereference;
          mc_syntax_node *name;
          mc_syntax_node *array_size;
          mc_syntax_node *function_pointer;
        } field_declarator;
        struct {
          mc_syntax_node *fp_dereference;
          mc_syntax_node *name;
          mc_syntax_node_list *parameters;
        } fptr_declarator;
        struct {
          mc_syntax_node *return_type_identifier;
          // May be null indicating no dereference operators
          mc_syntax_node *return_type_dereference;
          mc_syntax_node *declarator;
        } fptr_declaration;
        struct {
          mc_syntax_node *type_modifier;
          mc_syntax_node *type_identifier;
        } modified_type;
        struct {
          mc_syntax_node_list *statements;
        } statement_list;
        struct {
          mc_syntax_node *statement_list;
        } code_block;
        struct {
          mc_syntax_node *initialization;
          mc_syntax_node *conditional;
          mc_syntax_node *fix_expression;
          mc_syntax_node *loop_statement;
        } for_statement;
        struct {
          bool do_first;
          mc_syntax_node *conditional;
          mc_syntax_node *do_statement;
        } while_statement;
        struct {
          mc_syntax_node *conditional;
          mc_syntax_node *do_statement;
          mc_syntax_node *else_continuance;
        } if_statement;
        struct {
          mc_syntax_node *conditional;
          mc_syntax_node_list *sections;
        } switch_statement;
        struct {
          mc_syntax_node *expression;
        } expression_statement;
        struct {
          mc_syntax_node *declaration;
        } declaration_statement;
        struct {
          mc_syntax_node *label;
        } goto_statement;
        struct {
          mc_syntax_node *label;
        } label_statement;
        struct {
          mc_syntax_node_list *labels;
          mc_syntax_node *statement_list;
        } switch_section;
        struct {
          mc_syntax_node *constant;
        } switch_case_label;
        struct {
          mc_syntax_node *type_identifier;
          mc_syntax_node_list *declarators;
        } local_variable_declaration;
        struct {
          // May be null indicating no dereference operators
          mc_syntax_node *type_dereference;
          mc_syntax_node *variable_name;
          mc_syntax_node *function_pointer;
          mc_syntax_node *initializer;
        } local_variable_declarator;
        struct {
          mc_syntax_node *size_expression;
          mc_syntax_node *assignment_expression;
        } local_variable_array_initializer;
        struct {
          mc_syntax_node *value_expression;
        } local_variable_assignment_initializer;
        struct {
          mc_syntax_node_list *list;
        } initializer_expression;
        struct {
          mc_syntax_node *function_identity;
          mc_syntax_node_list *arguments;
        } invocation;
        struct {
          mc_syntax_node *expression;
        } return_statement;
        struct {
          mc_syntax_node *variable;
          mc_syntax_node *assignment_operator;
          mc_syntax_node *value_expression;
        } assignment_expression;
        struct {
          mc_syntax_node *variable;
          mc_syntax_node *assignment_operator;
          mc_syntax_node *value_expression;
        } arithmetic_assignment;
        struct {
          mc_syntax_node *left;
          mc_syntax_node *logical_operator;
          mc_syntax_node *right;
        } logical_expression;
        struct {
          mc_syntax_node *condition;
          mc_syntax_node *true_expression;
          mc_syntax_node *false_expression;
        } ternary_conditional;
        struct {
          mc_syntax_node *left;
          mc_syntax_node *relational_operator;
          mc_syntax_node *right;
        } relational_expression;
        struct {
          mc_syntax_node *left;
          mc_syntax_node *operational_operator;
          mc_syntax_node *right;
        } operational_expression;
        struct {
          mc_syntax_node *left;
          mc_syntax_node *bitwise_operator;
          mc_syntax_node *right;
        } bitwise_expression;
        struct {
          mc_syntax_node *primary;
          mc_syntax_node *access_operator;
          mc_syntax_node *identifier;
        } member_access_expression;
        struct {
          mc_syntax_node *primary;
          mc_syntax_node *access_expression;
        } element_access_expression;
        struct {
          mc_syntax_node *primary;
          mc_syntax_node *fix_operator;
          bool is_postfix;
        } fixrement_expression;
        struct {
          mc_syntax_node *prepend_operator;
          mc_syntax_node *unary_expression;
        } prepended_unary;
        struct {
          mc_syntax_node *deref_sequence;
          mc_syntax_node *unary_expression;
        } dereference_expression;
        struct {
          mc_syntax_node *identifier;
          bool is_const;
          bool is_static;
          bool has_struct_prepend;
          // -1 for unspecified (implicit signed), 0 for unsigned, 1 for explicit signed
          int is_signed;
          mc_syntax_node *size_modifiers;
        } type_identifier;
        // struct {
        //   mc_syntax_node *type_dereference;
        //   mc_syntax_node *identifier;
        //   mc_syntax_node_list *parameters;
        // } function_pointer_declarator;
        struct {
          mc_syntax_node *type_identifier;
          // May be null indicating no dereference operators
          mc_syntax_node *type_dereference;
          mc_syntax_node *expression;
        } cast_expression;
        struct {
          mc_syntax_node *expression;
        } parenthesized_expression;
        struct {
          mc_syntax_node *type_identifier;
          // May be null indicating no dereference operators
          mc_syntax_node *type_dereference;
        } sizeof_expression;
        struct {
          mc_syntax_node *type_identifier;
          // May be null indicating no dereference operators
          mc_syntax_node *type_dereference;
          mc_syntax_node *field_identity;
        } offsetof_expression;
        struct {
          mc_syntax_node *list_identity;
          mc_syntax_node *type_identifier;
          // May be null indicating no dereference operators
          mc_syntax_node *type_dereference;
        } va_arg_expression;
        struct {
          mc_syntax_node *list_identity;
        } va_list_expression;
      };
    };
  };
};

int print_syntax_node(mc_syntax_node *syntax_node, int depth);
int copy_syntax_node_to_text(mc_syntax_node *syntax_node, char **output);
int parse_definition_to_syntax_tree(char *code, mc_syntax_node **ast);
int parse_file_to_syntax_tree(char *code, mc_syntax_node **file_ast);
const char *get_mc_syntax_token_type_name(mc_syntax_node_type type);

#endif // C_PARSER_LEXER_H