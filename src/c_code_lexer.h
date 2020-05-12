/* c_code_lexer.h */

#ifndef C_CODE_LEXER
#define C_CODE_LEXER

#include "stdlib.h"
#include "stdio.h"
#include "ctype.h"

#define bool unsigned char
#define mctrue 0xEE
#define mcfalse 0x00

typedef enum parsing_context
{
    PARSING_CONTEXT_ROOT = 1,
    PARSING_CONTEXT_CODE_BLOCK
} parsing_context;

typedef struct parsing_state
{
    const char *text;
    parsing_context context;
    int index;
    bool end;
} parsing_state;

typedef enum token
{
    TOKEN_UNKNOWN = 0,
    TOKEN_ENDOFTEXT,
    TOKEN_INCLUDE,
    TOKEN_STRUCT_KEYWORD,
    TOKEN_INT_KEYWORD,
    TOKEN_PREPROC_DEFINE,
    TOKEN_PREPROC_IF,
    TOKEN_PREPROC_IFNDEF,
    TOKEN_PREPROC_ENDIF,
} token;

typedef enum c_node_type
{
    CNODE_FILE_ROOT = 100,
    CNODE_INCLUDE,
    CNODE_FUNCTION,
    CNODE_VARIABLE,
    CNODE_CODE_BLOCK,
} c_node_type;

typedef struct c_node
{
    c_node_type type;
    int data_count, data_allocated;
    void **data;
} c_node;

#define PRCE(CALL)                                 \
    res = CALL;                                    \
    if (res != 0)                                  \
    {                                              \
        printf("lexer_error@" #CALL ":%i\n", res); \
        return res;                                \
    }

int parse_past_empty_text(parsing_state *ps)
{
    while (mctrue)
    {
        switch (ps->text[ps->index])
        {
        case ' ':
        case '\n':
        case '\t':
            ++ps->index;
            continue;
        case '\0':
            ps->end = mctrue;
            return 0;
        default:
            return 0;
        }
    }
}

int strvncmp(parsing_state *ps, const char *str)
{
    for (int i = 0;; ++i)
        if (str[i] == '\0' || ps->text[ps->index + i] == '\0')
            return 0;
        else if (str[i] != ps->text[ps->index + i])
            return i;
    return 0;
}

int parse_past(parsing_state *ps, const char *str)
{
    for (int i = 0;; ++i)
    {
        if (str[i] == '\0')
        {
            ps->index += i;
            return 0;
        }
        else if (ps->text[ps->index + i] == '\0')
        {
            return -1;
        }
        else if (str[i] != ps->text[ps->index + i])
        {
            printf("expected:'%c' was:'%c'\n", str[i], ps->text[ps->index + i]);
            return 1 + i;
        }
    }
}

int parse_identifier(parsing_state *ps, char **out_seg)
{
    int o = ps->index;
    while (mctrue)
    {
        bool doc = mctrue;
        switch (ps->text[ps->index])
        {
        case ' ':
        case '\n':
        case '\t':
            doc = mcfalse;
            break;
        case '\0':
            printf("parse_identifier: unexpected eof");
            return -1;
        default:
            if (!isalpha(ps->text[ps->index]) && (ps->index > o && !isalnum(ps->text[ps->index]) && ps->text[ps->index] != '_'))
                doc = mcfalse;
            break;
        }
        if (!doc)
        {
            if (o == ps->index)
                return -1;

            *out_seg = (char *)calloc(sizeof(char), ps->index - o + 1);
            strncpy(*out_seg, ps->text + o, ps->index - o);
            (*out_seg)[ps->index - o] = '\0';
            return 0;
        }
        ++ps->index;
    }
    return -1;
}

int parse_contiguous_segment(parsing_state *ps, char **out_seg)
{
    int o = ps->index;
    while (mctrue)
    {
        bool doc = mctrue;
        switch (ps->text[ps->index])
        {
        case ' ':
        case '\n':
        case '\t':
            doc = mcfalse;
            break;
        case '\0':
            ps->end = mctrue;
            doc = mcfalse;
            break;
        default:
            break;
        }
        if (!doc)
        {
            if (o == ps->index)
                return -1;

            *out_seg = (char *)malloc(sizeof(char) * (ps->index - o + 1));
            strncpy(*out_seg, ps->text + o, ps->index - o);
            (*out_seg)[ps->index - o] = '\0';
            return 0;
        }
        ++ps->index;
    }
    return -1;
}

int peek_token(parsing_state *ps, token *ptk)
{
    parse_past_empty_text(ps);

    switch (ps->context)
    {
    case PARSING_CONTEXT_ROOT:
    {
        switch (ps->text[ps->index])
        {
        case '#':
        {
            if (!strvncmp(ps, "#include"))
            {
                *ptk = TOKEN_INCLUDE;
                return 0;
            }
            else if (!strvncmp(ps, "#ifndef"))
            {
                *ptk = TOKEN_PREPROC_IFNDEF;
                return 0;
            }
            else if (!strvncmp(ps, "#endif"))
            {
                *ptk = TOKEN_PREPROC_ENDIF;
                return 0;
            }

            printf("unknown preproc 2ndchar:%c\n", ps->text[ps->index + 1]);
            return -1;
        }
        case 'i':
        {
            if (!strvncmp(ps, "int"))
            {
                *ptk = TOKEN_INT_KEYWORD;
                return 0;
            }

            printf("unknown i 2ndchar:%c\n", ps->text[ps->index + 1]);
            return -1;
        }
        default:
        {
            printf("unknown ROOT firstchar:%c\n", ps->text[ps->index]);
            return -1;
        }
        }
        // if (!isalpha(ps->text[ps->index]))
        // {
        //   printf("unexpected char:%c\n", ps->text[ps->index]);
        // }
    }
    break;
    case PARSING_CONTEXT_CODE_BLOCK:
    {
        switch (ps->text[ps->index])
        {
        default:
        {
            printf("unknown CODE_BLOCK firstchar:%c\n", ps->text[ps->index]);
            return -1;
        }
        }
    }
    break;
    default:
    {
        printf("unknown context: %i", ps->context);
        return -1;
    }
    }
}

int print_parse_error(parsing_state *ps, const char *function_name, const char *section_id)
{
    char *buf = (char *)malloc(sizeof(char) * 16);
    if (!buf)
        return -1;

    strncpy(buf, ps->text + ps->index - 6, 6);
    strncpy(buf + 6, "|", 1);
    strncpy(buf + 7, ps->text + ps->index, 1);
    strncpy(buf + 8, "|", 1);
    strncpy(buf + 9, ps->text + ps->index + 1, 6);
    buf[15] = '\0';

    printf("%s>%s#unhandled-char:%s\n", function_name, section_id, buf);

    free(buf);
    return 0;
}

int add_child_data(c_node *parent, void *child)
{
    // printf("here-5\n");
    if (parent->data_count < parent->data_allocated)
    {
        // printf("here-6\n");
        parent->data[parent->data_count] = child;
        ++parent->data_count;
        return 0;
    }

    // Resize
    // printf("here-7:%i:%i:%p\n", parent->data_count, parent->data_allocated, parent->data);
    parent->data_allocated = parent->data_allocated + 4 + parent->data_allocated / 10;
    void **new_allocation = (void **)realloc(parent->data, sizeof(void **) * parent->data_allocated);
    if (!new_allocation)
    {
        // printf("here-8\n");
        printf("error allocating new c_node data\n");
        return -1;
    }
    // printf("here-9\n");
    parent->data = new_allocation;
    return 0;
}

int parse_code_block(parsing_state *ps, c_node *code_block)
{
    int res;
    PRCE(parse_past(ps, "{"));

    parsing_context previous_parsing_context = ps->context;
    ps->context = PARSING_CONTEXT_CODE_BLOCK;

    bool *is_processed = (bool *)malloc(sizeof(bool));
    *is_processed = mcfalse;
    add_child_data(code_block, (void *)is_processed);

    int curly_bracket_depth = 1;
    int origin = ps->index;
    bool loop = mctrue;

    while (1)
    {
        token tok;
        PRCE(peek_token(ps, &tok))

        switch (tok)
        {
        default:
        {
            print_parse_error(ps, "parse_code_block", "code_block_switch");
            return -1;
        }
        }
    }
    // for (int i = ps->index; loop; ++i)
    // {
    //     token token;
    //     peek_token(&token, ps);
    //     switch (token)
    //     {
    //     default:
    //         break;
    //     }
    //     switch (ps->text[i])
    //     {
    //     case '{':
    //         ++curly_bracket_depth;
    //         break;
    //     case '}':
    //     {
    //         --curly_bracket_depth;
    //         loop = curly_bracket_depth > 0;
    //         if (!loop)
    //             ps->index = i;
    //     }
    //     break;
    //     case '\0':
    //     {
    //         printf("parse_code_block>loop: unexpected EOF\n");
    //         return -1;
    //     }
    //     case ' ':
    //     {
    //         parse_token_empty(ps, code_block);
    //         ps->index = i;
    //     }
    //     break;
    //     default:
    //         printf("unexpected char:'%c'\n", ps->text[ps->index]);
    //         // int print_parse_error(parsing_state *ps, const char *function_name, const char *section_id)
    //         return -5;
    //     }
    // }

    // int code_size_in_bytes = sizeof(char) * (ps->index - origin);
    // char *unprocessed_code_block = (char *)malloc(code_size_in_bytes + sizeof(char) * 1);
    // memcpy(unprocessed_code_block, ps->text + origin, code_size_in_bytes);
    // unprocessed_code_block[ps->index - origin] = '\0';
    // printf("unprocessed_code_block(%i):\n'%s'\n", code_size_in_bytes, unprocessed_code_block);

    PRCE(parse_past(ps, "}"));
    PRCE(parse_past_empty_text(ps));

    // Place-keeper for the processed code block
    ps->context = previous_parsing_context;
    return 0;
}

#endif // C_CODE_LEXER