/* mcl_type_defs.h */

#ifndef MCL_TYPE_DEFS_H
#define MCL_TYPE_DEFS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

// #include "cling/Interpreter/Interpreter.h"
// #include "cling/Interpreter/Transaction.h"

// static cling::Interpreter *clint;

#include "midge.h"

// std::vector<cling::Transaction *> transactions;
std::map<std::string, cling::Transaction *> definedTypes;
void define_structure(std::string typeName, std::string definition)
{
  std::map<std::string, cling::Transaction *>::iterator it = definedTypes.find(typeName);
  if (it != definedTypes.end())
  {
    std::cout << "type " << typeName << " already exists!" << std::endl;

    uint wastid = clint->getLatestTransaction()->getUniqueID();
    //std::cout << "current=" << interp->getLatestTransaction()->getUniqueID() << std::endl;
    //interp->getLatestTransaction()->dump();
    int iter = 0;
    while (clint->getLatestTransaction()->getUniqueID() > it->second->getUniqueID())
    {
      ++iter;
      clint->unload(1);
      //interp->getLatestTransaction()->dump();
    }

    if (clint->getLatestTransaction()->getUniqueID() != it->second->getUniqueID())
      throw 343;
    clint->unload(1);
  }

  cling::Transaction *transaction = nullptr;
  clint->declare(definition, &transaction);
  if (!transaction)
    throw 111;
  else
  {
    //std::cout << "Transaction:" << transaction->getUniqueID() << std::endl;
    //transaction->dump();
    // transactions.push_back(transaction);
    definedTypes[typeName] = transaction;
  }
}

typedef struct function_parameter
{
  char *type;
  char *name;
  bool requires_addressing;
} function_parameter;

typedef struct defined_function
{
  int version;
  const char *name;

  function_parameter *params;
  int params_count;
  const char *given_code;
} defined_function;

void append_substring(char *dest, char *src, int n)
{
  for (int i = 0; i < n; ++i)
  {
    dest[i] = src[i];
  }
  dest[n] = '\0';
}

std::map<std::string, defined_function *> defined_functions;
void add_code_block(char *decl, defined_function *df, const char *block)
{
  int dlen = strlen(decl);
  int blen = strlen(block);

  // Search for identifiers
  int ids = 0, lwi = 0;
  bool in_identifier = false;
  for (int i = 0; i <= blen; ++i)
  {
    bool ident_found = false;
    if (block[i] == ' ' || block[i] == '\n' || block[i] == '\t')
    {
      if (!in_identifier)
        continue;

      // Identifier found
      ident_found = true;
    }
    if (block[i] == ';' || block[i] == '\0')
    {
      if (!in_identifier)
        continue;

      // Identifier found
      ident_found = true;
    }

    if (in_identifier)
    {
      if (!ident_found && (isalnum(block[i] || block[i] == '_')))
        continue;

      // Identifier!
      ident_found = true;
    }
    else
    {
      if (isalpha(block[i]))
      {
        ids = i;
        in_identifier = true;
      }
      continue;
    }

    if (block[i] == '(')
    {
      // Function Call Identifier
      //   char call_name[256];
      //   strncpy(call_name, block + t, i - t);
      //   call_name[i - t] = '\0';

      //   std::map<std::string, defined_function *> it = defined_functions.find(call_name);
      //   if (it == defined_functions.end())
      //     continue;

      //   defined_function *sf = it->second;
      //   char nstname[10];
      //   printf("found-call:(%i:%i):%s\n", t, i - t, call_name);
      //   {
      //     // Declare the array of void pointers
      //     char nst[100];
      //     char nstnum[7];
      //     const char *initial = "void *_mdge_";
      //     strcpy(nst, initial);
      //     sprintf(nstnum, "%i", varyd);
      //     strcat(nst, nstnum);

      //     // -- sidebar : set name of array for other pointers to access
      //     strcpy(nstname, initial);
      //     strcat(nstname, nstnum);

      //     strcat(nst, "[");
      //     memset(nstnum, '\0', 7);
      //     sprintf(nstnum, "%i", sf->params_count);
      //     strcat(nst, nstnum);
      //     strcat(nst, "];\n");
      //     strcat(decl, nst);
      //   }
      //   for (int j = 0; j < sf->params_count; ++j)
      //   {
      //     char nst[2048];
      //     char nstnum[7];
      //     strcpy(nst, nstname);
      //     strcat(nst, "[");
      //     sprintf(nstnum, "%i", j);
      //     strcat(nst, nstnum);
      //     strcat(nst, "] = (void *)");
      //   }
      //   ++varyd;
      // }
      // strcat(decl, "}\n");
    }
    else
    {
      for (int j = 0; j < df->params_count; ++j)
      {
        if (!df->params[j].requires_addressing)
          continue;

        // Compare
        bool equals = false;
        for (int c = 0;; ++c)
        {
          if (df->params[j].name[c] == '\0')
          {
            equals = true;
            break;
          }
          if (df->params[j].name[c] != decl[ids + c])
            break;
        }
        if (!equals)
          continue;

        // Insert a dereferencer and copy the rest of the block forwards
        decl[ids] = '*';
        decl[ids + 1] = '\0';
        // strcat(decl, block[ids - dlen])++ i;
      }
    }
  }
}

void add_params_definitions(char *decl, defined_function *df, const char *params)
{
  // (Re)Allocation
  if (df->params)
  {
    for (int i = 0; i < df->params_count; ++i)
    {
      free(df->params[i].type);
      free(df->params[i].name);
    }
    free(df->params);
    df->params = NULL;
  }
  int allocated_params = 20;
  df->params = (function_parameter *)malloc(sizeof(function_parameter) * allocated_params);
  df->params_count = 0;

  // Iterate
  int n = strlen(params);
  int s = 0, t = 0;
  if (n > 0)
  {
    int u = 0, p = 0;
    for (int i = 0; i <= n; ++i)
    {
      if (t == s)
      {
        if (params[i] == ' ' || params[i] == '&' || params[i] == '*')
        {
          int mod = 0;
          while (params[i + 1] == '&' || params[i + 1] == '*')
          {
            mod = 1;
            ++i;
          }
          t = i + mod;
          u = mod ? t : (i + 1);
        }
      }
      else
      {
        if (params[i] == ',' || params[i] == '\0')
        {
          // Set
          if (df->params_count >= allocated_params)
          {
            allocated_params = allocated_params + 4 + allocated_params / 2;
            df->params = (function_parameter *)realloc(df->params, sizeof(function_parameter) * allocated_params);
          }
          df->params[df->params_count].type = (char *)malloc(sizeof(char) * (t - s));
          strncpy(df->params[df->params_count].type, params + s, t - s);
          df->params[df->params_count].type[t - s] = '\0';
          df->params[df->params_count].name = (char *)malloc(sizeof(char) * (i - u));
          strncpy(df->params[df->params_count].name, params + u, i - u);
          df->params[df->params_count].name[i - u] = '\0';

          // Check addressing
          if (df->params[df->params_count].type[t - s - 1] == '*')
            df->params[df->params_count].requires_addressing = false;
          else
          {
            char *fch = strchr(df->params[df->params_count].type, '&');
            if (fch != NULL)
            {
              printf("ERROR >> Pass By Reference not supported.");
              return;
            }
            df->params[df->params_count].requires_addressing = true;
          }
          printf("param_type:'%s' param_name:'%s' requires_addressing:'%s'\n", df->params[df->params_count].type, df->params[df->params_count].name,
                 df->params[df->params_count].requires_addressing ? "true" : "false");

          // Add to declaration
          const char *TAB = "  ";
          strcat(decl, TAB);
          strcat(decl, df->params[df->params_count].type);
          if (df->params[df->params_count].requires_addressing)
            strcat(decl, " *");
          strcat(decl, df->params[df->params_count].name);
          strcat(decl, " = (");
          strcat(decl, df->params[df->params_count].type);
          if (df->params[df->params_count].requires_addressing)
            strcat(decl, " *");
          strcat(decl, ")p_vargs[");
          sprintf(decl + strlen(decl), "%i", p);
          strcat(decl, "];\n");

          // Reset
          ++df->params_count;
          ++p;
          if (params[i] == ',')
          {
            // Remove any further whitespace
            while (params[i + 1] == '\n' || params[i + 1] == ' ' || params[i + 1] == '\t')
              ++i;
          }
          s = t = i + 1;
        }
      }
    }
    strcat(decl, "\n");
  }
  if (allocated_params > df->params_count)
  {
    df->params = (function_parameter *)realloc(df->params, sizeof(function_parameter) * df->params_count);
  }
}

void define_function(const char *return_type, const char *name, const char *params, const char *block)
{
  if (strcmp(return_type, "void"))
  {
    printf("Only allowed void returning functions atm.");
    return;
  }

  const char *TAB = "  ";
  char decl[16384];
  std::map<std::string, defined_function *>::iterator it = defined_functions.find(name);
  defined_function *df;

  // Declare Function Pointer
  if (it == defined_functions.end())
  {
    strcpy(decl, "static void (*");
    strcat(decl, name);
    strcat(decl, ")(void **);");
    clint->declare(decl);
    printf("%s\n", decl);

    df = (defined_function *)malloc(sizeof df);
    df->version = 0;
    df->params = NULL;
    defined_functions[name] = df;
  }
  else
    df = it->second;

  // Set version and function name postfix
  int version = ++df->version;
  df->name = name;
  df->given_code = block;

  char verstr[7];
  strcpy(verstr, "_v");
  sprintf(verstr + 2, "%i", version);

  // Form the function declaration
  // -- header
  strcpy(decl, "void ");
  strcat(decl, name);
  strcat(decl, verstr);
  strcat(decl, "(void **p_vargs) {\n");

  // -- params
  // add_params_definitions(decl, df, params);

  // -- code-block
  add_code_block(decl, df, block);

  // Declare function
  printf("decl:%s\n", decl);
  clint->declare(decl);

  // Set pointer to function
  strcpy(decl, name);
  strcat(decl, " = &");
  strcpy(decl, name);
  strcat(decl, verstr);
  strcat(decl, ";");
  clint->process(decl);
}

#define PRCE(CALL)                       \
  res = CALL;                            \
  if (res != 0)                          \
  {                                      \
    printf("error@" #CALL ":%i\n", res); \
    return res;                          \
  }

enum parsing_context
{
  PARSING_CONTEXT_ROOT = 1,
};

typedef struct parsing_state
{
  const char *text;
  parsing_context context;
  int index;
  bool end;
} parsing_state;

enum token
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
};

int parse_past_empty_text(parsing_state *ps)
{
  while (true)
  {
    switch (ps->text[ps->index])
    {
    case ' ':
    case '\n':
    case '\t':
      ++ps->index;
      continue;
    case '\0':
      ps->end = true;
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
  while (true)
  {
    bool doc = true;
    switch (ps->text[ps->index])
    {
    case ' ':
    case '\n':
    case '\t':
      doc = false;
      break;
    case '\0':
      printf("parse_identifier: unexpected eof");
      return -1;
    default:
      if (!isalpha(ps->text[ps->index]) && (ps->index > o && !isalnum(ps->text[ps->index]) && ps->text[ps->index] != '_'))
        doc = false;
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
  while (true)
  {
    bool doc = true;
    switch (ps->text[ps->index])
    {
    case ' ':
    case '\n':
    case '\t':
      doc = false;
      break;
    case '\0':
      ps->end = true;
      doc = false;
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

int peek_token(token *ptk, parsing_state *ps)
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

enum c_node_type
{
  CNODE_FILE_ROOT = 100,
  CNODE_INCLUDE,
  CNODE_FUNCTION,
  CNODE_VARIABLE,
  CNODE_CODE_BLOCK,
};

typedef struct c_node
{
  c_node_type type;
  int data_count, data_allocated;
  void **data;
} c_node;

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

  bool *is_processed = (bool *)malloc(sizeof(bool));
  *is_processed = false;
  add_child_data(code_block, (void *)is_processed);

  int curly_bracket_depth = 1;
  int origin = ps->index;
  bool loop = true;
  for (int i = ps->index; loop; ++i)
  {
    switch (ps->text[i])
    {
    case '{':
      ++curly_bracket_depth;
      break;
    case '}':
    {
      --curly_bracket_depth;
      loop = curly_bracket_depth > 0;
      if (!loop)
        ps->index = origin;
    }
    break;
    case '\0':
    {
      printf("parse_code_block>loop: unexpected EOF\n");
      return -1;
    }
    default:
      continue;
    }
  }

  int code_size_in_bytes = sizeof(char) * (ps->index - origin);
  char *unprocessed_code_block = (char *)malloc(code_size_in_bytes + sizeof(char));
  memcpy(unprocessed_code_block, ps->text + origin, code_size_in_bytes);
  unprocessed_code_block[ps->index - origin] = '\0';
  printf("unprocessed_code_block(%i):\n%s\n", code_size_in_bytes, unprocessed_code_block);

  PRCE(parse_past(ps, "}"));
  PRCE(parse_past_empty_text(ps));

  // Place-keeper for the processed code block
  add_child_data(code_block, (void *)NULL);
  return 0;
}

int parse_function(parsing_state *ps, c_node *function)
{
  int res;
  PRCE(parse_past(ps, "("));

  int *param_count = (int *)malloc(sizeof(int));
  *param_count = 0;
  PRCE(add_child_data(function, (void *)param_count));

  int allocated = 4;
  function_parameter **fparams = (function_parameter **)malloc(sizeof(function_parameter *) * allocated);
  if (!fparams)
    return -1;

  // Parse through the parameters
  while (true)
  {
    // Parse
    function_parameter *fp = (function_parameter *)malloc(sizeof(function_parameter));

    PRCE(parse_identifier(ps, &fp->type));
    PRCE(parse_past_empty_text(ps));
    if (isalpha(ps->text[ps->index]))
    {
      fp->requires_addressing = true;
    }
    else
    {
      switch (ps->text[ps->index])
      {
      case '*':
      {
        fp->requires_addressing = false;
        int deref_count = 1;
        while (ps->text[++ps->index] == '*')
          ++deref_count;
        int new_type_str_len = strlen(fp->type) + 1 + deref_count;
        char *new_type_str = (char *)malloc(sizeof(char) * new_type_str_len);
        strcpy(new_type_str, fp->type);
        free(fp->type);
        fp->type = new_type_str;
        fp->type[new_type_str_len - 2 - deref_count] = ' ';
        for (int i = 0; i < deref_count; ++i)
          fp->type[new_type_str_len - 2 - i] = '*';
        fp->type[new_type_str_len - 1] = '\0';

        PRCE(parse_past_empty_text(ps));
      }
      break;
      default:
        printf("parse_function/unsupported-0 char:%c\n", ps->text[ps->index]);
      }
    }

    PRCE(parse_identifier(ps, &fp->name));

    // Add to array
    if (*param_count == allocated)
    {
      allocated = allocated + 4 + allocated / 10;
      fparams = (function_parameter **)realloc(fparams, sizeof(function_parameter *) * allocated);
    }
    fparams[*param_count] = fp;
    ++*param_count;

    // Look forward
    PRCE(parse_past_empty_text(ps));
    if (ps->text[ps->index] == ',')
    {
      ++ps->index;
      PRCE(parse_past_empty_text(ps));
      continue;
    }
    else if (ps->text[ps->index] == ')')
    {
      ++ps->index;
      PRCE(parse_past_empty_text(ps));
      break;
    }
    else
    {
      printf("parse_function/unsupported-1 char:%c\n", ps->text[ps->index]);
    }
  }

  if (*param_count != allocated)
    fparams = (function_parameter **)realloc(fparams, sizeof(function_parameter *) * *param_count);
  PRCE(add_child_data(function, (void *)fparams));

  c_node *code_block = (c_node *)calloc(sizeof(c_node), 1);
  code_block->type = CNODE_CODE_BLOCK;
  PRCE(parse_code_block(ps, code_block));
  PRCE(add_child_data(function, (void *)code_block));

  return 0;
}

int parse_file_text(c_node *root_node, const char *txt)
{
  int res;
  struct parsing_state ps = {txt, PARSING_CONTEXT_ROOT, 0, false};

  int allocated_children = 8, child_count = 0;
  c_node **children = (c_node **)malloc(sizeof(c_node *));

  while (!ps.end)
  {
    token tok;
    if (peek_token(&tok, &ps))
      break; // Unhandled Error
    switch (tok)
    {
    case TOKEN_INCLUDE:
    {
      PRCE(parse_past(&ps, "#include"));
      PRCE(parse_past_empty_text(&ps));

      if (ps.text[ps.index] != '<' && ps.text[ps.index] != '"')
      {
        // print_error(&ps, "token_include");
        printf("unexpected char 52525:%c\n", ps.text[ps.index]);
        return -1;
      }

      char *statement;
      PRCE(parse_contiguous_segment(&ps, &statement));

      c_node *child = (c_node *)calloc(sizeof(c_node), 1);
      child->type = CNODE_INCLUDE;
      child->data = (void **)(&statement);
      child->data_count = 1;

      PRCE(add_child_data(root_node, (void *)child));
    }
    break;
    case TOKEN_INT_KEYWORD:
    {
      PRCE(parse_past(&ps, "int"));
      PRCE(parse_past_empty_text(&ps));

      char *identifier;
      PRCE(parse_identifier(&ps, &identifier));

      switch (ps.text[ps.index])
      {
      case '(':
      {
        // Function
        c_node *function = (c_node *)calloc(sizeof(c_node), 1);
        function->type = CNODE_FUNCTION;

        char *return_type = (char *)calloc(sizeof(char), 4);
        strcpy(return_type, "int\0");
        PRCE(add_child_data(function, (void *)return_type));

        PRCE(add_child_data(function, (void *)identifier));

        PRCE(parse_function(&ps, function));
      }
      break;
      // case ';':
      //   parse_variable(&ps, &child, "int", identifier);
      //   break;
      default:
        print_parse_error(&ps, "parse_file_text", "TOKEN_INT");
        return -1;
      }
    }
    break;
    default:
    {
      print_parse_error(&ps, "parse_file_text", "root_switch");
      return -1;
    }
    }
  }

  root_node->data = (void **)children;
  root_node->data_count = child_count;
  return 0;
}

typedef struct structure_definition
{
  const char *name;
  cling::Transaction transaction;

} structure_definition;

int load_mc_file(const char *filepath, bool error_on_redefinition)
{
  if (sizeof(int *) != sizeof(c_node *) || sizeof(int *) != sizeof(char *) || sizeof(int *) != sizeof(function_parameter *) || sizeof(int *) != sizeof(bool *))
  {
    printf("violation of program architecture. Specified pointer types must be of the same size!\n");
    return -1;
  }

  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen(filepath, "rb");
  if (!fp)
    perror(filepath), exit(1);

  fseek(fp, 0L, SEEK_END);
  lSize = ftell(fp);
  rewind(fp);

  /* allocate memory for entire content */
  buffer = (char *)calloc(1, lSize + 1);
  if (!buffer)
    fclose(fp), fputs("memory alloc fails", stderr), exit(1);

  /* copy the file into the buffer */
  if (1 != fread(buffer, lSize, 1, fp))
    fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

  // Parse
  c_node root = {
      .type = CNODE_FILE_ROOT,
      .data_count = 0,
      .data_allocated = 0,
      .data = NULL,
  };
  parse_file_text(&root, buffer);
  // TODO -- free c_nodes

  fclose(fp);
  free(buffer);
  return 0;
}

void redef()
{
  // -- Redefinition
  load_mc_file("/home/jason/midge/src/mc_exp.c", true);
  // clint->process("mcmain();");
  return;

  /* 
void mvk_init_instance(VkResult *result, vk_render_state *p_vkrs, char const *const app_short_name)
{
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  app_info.pApplicationName = app_short_name;
  app_info.applicationVersion = 1;
  app_info.pEngineName = app_short_name;
  app_info.engineVersion = 1;
  app_info.apiVersion = VK_API_VERSION_1_0;

  // -- Layers & Extensions --
  p_vkrs->instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  p_vkrs->instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = NULL;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledLayerCount = p_vkrs->instance_layer_names.size();
  inst_info.ppEnabledLayerNames = p_vkrs->instance_layer_names.size() ? p_vkrs->instance_layer_names.data() : NULL;
  inst_info.enabledExtensionCount = p_vkrs->instance_extension_names.size();
  inst_info.ppEnabledExtensionNames = p_vkrs->instance_extension_names.data();

  printf("create VkInstance...");
  *result = vkCreateInstance(&inst_info, NULL, &p_vkrs->inst);
  assert(*result == VK_SUCCESS);
  printf("SUCCESS\n");
} */

  // const char *mc_add_to_num = "func add_to_num(int *v, int e) { *v += 4 * e; }";
  // define_function("void", "add_to_num", "int *v, int e", "  *v += 4 * e;\n");
  // clint->declare("void add_to_num (void **vargs) {"
  //                "  int *v = (int *)vargs[0];\n"
  //                "  int *e = (int *)vargs[1];\n"
  //                "  \n"
  //                "  *v += 4 * *e;\n }");

  // const char *mc_set_num = "func set_num(int **var, int value) { *var = (int *)malloc(sizeof(int)); *var = value; }";
  // define_function("void", "set_num", "int **var, int value", "  *var = (int *)malloc(sizeof(int));\n  *var = value;\n)");
  // clint->declare("void set_num (void **vargs) {"
  //                "  int **var = (int **)vargs[0];\n"
  //                "  int *value = (int *)vargs[1];\n"
  //                "  \n"
  //                "  *var = (int *)malloc(sizeof(int));\n"
  //                "  **var = *value;\n"
  //                "}");

  // const char *mc_puffy = "func puffy() {"
  //                        "  int *v;"
  //                        "  set_num(&v, 6);"
  //                        "  add_to_num(&v, 2);"
  //                        "  printf(\"out:%i\\n\", v);\n"
  //                        "}";
  // define("void", "puffy", "",
  //        "  int *v;"
  //        "  set_num(&v, 6);"
  //        "  add_to_num(&v, 2);"
  //        "  printf(\"out:%i\\n\", v);\n");
  // clint->declare("void puffy() {"
  //                "  int *v;\n"
  //                "\n"
  //                "  void *vargs_0[2];\n"
  //                "  vargs_0[0] = (void *)&v;\n"
  //                "  int mcliteral_0 = 6;"
  //                "  vargs_0[1] = (void *)&mcliteral_0;\n"
  //                "  set_num(vargs_0);"
  //                "\n"
  //                "  void *vargs_1[2];\n"
  //                "  vargs_1[0] = (void *)v;\n"
  //                "  int mcliteral_1 = 2;"
  //                "  vargs_1[1] = (void *)&mcliteral_1;\n"
  //                "  add_to_num(vargs_1);"
  //                "  printf(\"out:%i\\n\", *v);\n"
  //                "\n"
  //                "  free(v);"
  //                "}");

  clint->process("puffy()");
  // define_function("void", "add_to_num", "int *v", "  *v += 4;\n");
  // define_function("void", "puffy", "",
  //                 "  int v = 3;\n"
  //                 "  add_to_num(&v);\n"
  //                 "  printf(\"out:%i\\n\", v);\n");
  // code("int v = 3;");
  // code("add_to_num(&v);");
  // define_method("add_to_num", "void add_to_num(int *v) { *v += 19; }");
  // code("add_to_num(&v);");

  // code("printf(\"Initial >v=%i\\n\", v);");

  // define structure
  // define_structure("shaver", "typedef struct shaver { float battery_life; } shaver;");

  // define_method("do_stuff", "void do_stuff(shaver *s) { s->battery_life -= 7; }");

  // code("shaver s;");
  // code("s.battery_life = 100;");
  // code("do_stuff(&s);");
  // code("printf(\"Before>s.battery_life=%.2f\\n\", s.battery_life);");

  // define_structure("shaver", "typedef struct shaver { float battery_life; float condition_multiplier; } shaver;");
  // code("printf(\"After >s.battery_life=%i\\n\", s.battery_life);");
}

void redef2()
{
  // -- Redefinition
  // define structure
  define_structure("shaver", "typedef struct shaver { float battery_life; } shaver;");

  // define method
  clint->declare("void *shaver_display_routine(void *vargp) {"
                 "  void **vargs = (void **)vargp;"
                 "  mthread_info *thr = *(mthread_info **)vargs[0];"
                 "  shaver *s = (shaver *)vargs[1];"
                 "  "
                 "  float last_measure = 120.f;"
                 "  while(!thr->should_exit) {"
                 "    if(thr->should_pause && hold_mthread(thr))"
                 "      break;"
                 "    if(last_measure - s->battery_life > 1.f) {"
                 "      last_measure = s->battery_life;"
                 "      printf(\"battery-life:%.2f\\n\", s->battery_life);"
                 "    }"
                 "    usleep(2000);"
                 "  }"
                 "  "
                 "  thr->has_concluded = true;"
                 "  return NULL;"
                 "}");
  clint->declare("void *shaver_update_routine(void *vargp) {"
                 "  void **vargs = (void **)vargp;"
                 "  mthread_info *thr = *(mthread_info **)vargs[0];"
                 "  shaver *s = (shaver *)vargs[1];"
                 "  "
                 "  int ms = 0;"
                 "  while(!thr->should_exit && ms < 10000) {"
                 "    if(thr->should_pause && hold_mthread(thr))"
                 "      break;"
                 "    usleep(50000);"
                 "    ms += 50;"
                 "    s->battery_life = 0.9999f * s->battery_life - 0.00007f * ms;"
                 "  }"
                 "  "
                 "  thr->has_concluded = true;"
                 "  return NULL;"
                 "}");

  // Begin
  clint->process("mthread_info *rthr, *uthr;");
  clint->process("shaver s_data = { .battery_life = 83.4f };");
  clint->process("void *args[2];");
  clint->process("args[1] = &s_data;");

  clint->process("args[0] = &rthr;");
  clint->process("begin_mthread(shaver_display_routine, &rthr, args);");
  clint->process("args[0] = &uthr;");
  clint->process("begin_mthread(shaver_update_routine, &uthr, args);");

  // Pause
  clint->process("int iterations = 0;");
  clint->process("while(!uthr->has_concluded && iterations < 1000) { usleep(4000); ++iterations; }");
  clint->process("printf(\"pausing...\\n\");");
  clint->process("pause_mthread(rthr, false);");
  clint->process("pause_mthread(uthr, false);");
  clint->process("while(!rthr->has_paused || !uthr->has_paused) usleep(1);");
  clint->process("printf(\"paused for 3 seconds.\\n\");");

  // Redefine
  // redefine structure in main thread
  define_structure("shaver", "typedef struct shaver { float battery_life; float condition_multiplier; } shaver;");

  // Resume
  clint->process("iterations = 0;");
  clint->process("while(!uthr->has_concluded && iterations < 1000) { usleep(3000); ++iterations; }");
  clint->process("printf(\"resuming...\");");
  clint->process("unpause_mthread(rthr, false);");
  clint->process("unpause_mthread(uthr, false);");
  clint->process("printf(\"resumed!\\n\");");

  // End
  clint->process("while(!uthr->has_concluded) usleep(1);");
  clint->process("printf(\"ending...\\n\");");
  clint->process("end_mthread(rthr);");
  clint->process("end_mthread(uthr);");
  clint->process("printf(\"success!\\n\");");
}

#endif // MCL_TYPE_DEFS_H