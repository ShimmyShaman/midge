#include "core/midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

int convert_return_type_string(char *return_type, char **return_type_name, unsigned int *return_type_deref_count)
{
  int n = strlen(return_type);
  *return_type_deref_count = 0;
  int i;
  for (i = n - 1; i > 0; --i) {
    if (return_type[i] == '*')
      ++*return_type_deref_count;
    else if (return_type[i] == ' ')
      continue;
    else
      break;
  }
  allocate_and_copy_cstrn(*return_type_name, return_type, i + 1);

  return 0;
}

int declare_function_pointer_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1
  printf("declare_function_pointer_v1()\n");
  char *name = *(char **)argv[0];
  char *return_type = *(char **)argv[1];

  char *return_type_name;
  unsigned int return_type_deref_count;
  MCcall(convert_return_type_string(return_type, &return_type_name, &return_type_deref_count));

  // TODO -- check
  // printf("dfp-0\n");

  // Fill in the function_info and attach to the nodespace
  // function_info *func_info = (function_info *)malloc(sizeof(function_info));
  function_info *func_info = (function_info *)malloc(sizeof(function_info));
  func_info->name = name;
  func_info->latest_iteration = 0U;
  func_info->return_type.name = return_type_name;
  func_info->return_type.deref_count = return_type_deref_count;
  func_info->parameter_count = (argc - 2) / 2;
  func_info->parameters = (mc_parameter_info_v1 **)malloc(sizeof(void *) * func_info->parameter_count);
  func_info->variable_parameter_begin_index = -1;
  uint struct_usage_alloc = 0;
  func_info->struct_usage = NULL;
  func_info->struct_usage_count = 0;
  // printf("dfp-1\n");

  for (int i = 0; i < func_info->parameter_count; ++i) {
    // printf("dfp-2\n");
    mc_parameter_info_v1 *parameter_info = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    parameter_info->is_function_pointer = false;
    // printf("dfp>%p=%s\n", i, (void *)parameters[2 + i * 2 + 0], (char *)parameters[2 + i * 2 + 0]);
    char *type_name;
    allocate_and_copy_cstr(type_name, *(char **)argv[2 + i * 2 + 0]);

    // Strip the type name of dereference operators
    parameter_info->type_deref_count = 0;
    while (true) {
      int k = strlen(type_name) - 1;
      if (k < 0) {
        MCerror(108, "arg error");
      }

      if (type_name[k] == ' ') {
        // Do nothing
      }
      else if (type_name[k] == '*') {
        ++parameter_info->type_deref_count;
      }
      else
        break;

      // Strip of last character
      char *temp;
      allocate_and_copy_cstrn(temp, type_name, k);
      free(type_name);
      type_name = temp;
    }
    parameter_info->type_name = type_name;

    printf("dfp-4a\n");
    // printf("chn:%p\n", command_hub->nodespace);
    // printf("ptn:%s\n", parameter_info->type_name);
    //  printf("dfp-4b\n");
    mc_struct_info_v1 *p_struct_info;
    {
      void *mc_vargs[3];
      mc_vargs[0] = (void *)&command_hub->nodespace;
      mc_vargs[1] = (void *)&parameter_info->type_name;
      mc_vargs[2] = (void *)&p_struct_info;
      MCcall(find_struct_info(3, mc_vargs));
    }
    if (!strcmp(parameter_info->type_name, "node") && !p_struct_info) {
      MCerror(156, "DEBUG Its not finding node in find_struct_info");
    }
    if (p_struct_info) {
      struct_info *sinfo = (struct_info *)p_struct_info;
      allocate_and_copy_cstr(parameter_info->mc_declared_type, sinfo->declared_mc_name);
      parameter_info->type_version = sinfo->version;

      int already_added = 0;
      for (int j = 0; j < func_info->struct_usage_count; ++j) {
        struct_info *existing = (struct_info *)func_info->struct_usage[j];

        if (!strcmp(parameter_info->type_name, existing->name)) {
          already_added = 1;
          break;
        }
      }
      if (!already_added) {
        MCcall(append_to_collection((void ***)&func_info->struct_usage, &struct_usage_alloc,
                                    &func_info->struct_usage_count, (void *)sinfo));
      }
      // printf("dfp-6\n");
    }
    else {
      parameter_info->mc_declared_type = NULL;
      parameter_info->type_version = 0;
    }

    char *param_name;
    allocate_and_copy_cstr(param_name, *(char **)argv[2 + i * 2 + 1]);
    parameter_info->name = param_name;

    func_info->parameters[i] = (mc_parameter_info_v1 *)parameter_info;
    // printf("dfp>set param[%i]=%s %s\n", i, parameter_info->type, parameter_info->name);
  }

  // Add the function info to the nodespace
  MCcall(append_to_collection((void ***)&command_hub->nodespace->functions, &command_hub->nodespace->functions_alloc,
                              &command_hub->nodespace->function_count, (void *)func_info));

  // Cleanup Parameters
  if (func_info->struct_usage_count != struct_usage_alloc) {
    if (func_info->struct_usage_count > 0) {
      struct_info **new_struct_usage = (struct_info **)malloc(sizeof(struct_info *) * func_info->struct_usage_count);
      memcpy((void *)new_struct_usage, func_info->struct_usage, sizeof(struct_info *) * func_info->struct_usage_count);

      free(func_info->struct_usage);
      func_info->struct_usage = new_struct_usage;
    }
    else {
      struct_usage_alloc = 0;
      free(func_info->struct_usage);
      func_info->struct_usage = NULL;
    }
  }

  // Declare with clint
  char buf[1024];
  strcpy(buf, "int (*");
  strcat(buf, name);
  strcat(buf, ")(int,void**);");
  printf("dfp>cling_declare:%s\n -- with %i parameters returning %s\n", buf, func_info->parameter_count,
         func_info->return_type.name);
  clint_declare(buf);
  // printf("dfp-concludes\n");
  return 0;
}

/* Conforms the given type_identity to a mc_type_identity if it is available. Searches first the given func_info,
 * failing that then searches the current nodespace. else just returns the given type
 * @nodespace : *(node **) Return Value.
 * @func_info : *(function_info **) The function info, may be NULL.
 * @type_identity : *(const char **) The type name to check for
 * @conformed_type_result : (conformed_type_result **) Return Value.
 */
int conform_type_identity_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  printf("conform_type_identity()\n");
  if (argc != 4) {
    return 537;
  }

  // Parameters
  node *nodespace = (node *)argv[0];
  function_info *func_info = *(function_info **)argv[1];
  const char *const type_identity = *(const char *const *)argv[2];
  char **conformed_type_identity = (char **)argv[3];

  // if (!strcmp(type_identity, "node")) {
  //   allocate_and_copy_cstr((*conformed_type_identity), "mc_node_v1 *");
  //   return 0;
  // }
  printf("type_identity='%s'\n", type_identity);

  char *finalized_identity;
  allocate_and_copy_cstr(finalized_identity, type_identity);

  // Strip the type identity of all deref operators
  int type_deref_count = 0;
  while (true) {
    int k = strlen(finalized_identity) - 1;
    if (k < 0) {
      MCerror(108, "arg error");
    }

    if (finalized_identity[k] == ' ') {
      // Do nothing
    }
    else if (finalized_identity[k] == '*') {
      ++type_deref_count;
    }
    else
      break;

    // Strip of last character
    char *temp;
    allocate_and_copy_cstrn(temp, finalized_identity, k);
    free(finalized_identity);
    finalized_identity = temp;
  }

  bool matched = false;
  if (func_info) {
    // printf("ctn-0  %s\n", func_info->name);
    // Check for utilized version in function info
    for (int i = 0; i < func_info->struct_usage_count; ++i) {
      if (!strcmp(finalized_identity, func_info->struct_usage[i]->name)) {
        // printf("ctn-1\n");
        // Match!
        matched = true;
        free(finalized_identity);
        allocate_and_copy_cstr(finalized_identity, func_info->struct_usage[i]->declared_mc_name);

        // MC structures require another dereference
        // ++type_deref_count;
        break;
      }
    }
  }

  if (!matched) {
    // Look for type in nodespace
    mc_struct_info_v1 *p_struct_info;
    {
      void *mc_vargs[3];
      mc_vargs[0] = (void *)&command_hub->nodespace;
      mc_vargs[1] = (void *)&finalized_identity;
      mc_vargs[2] = (void *)&p_struct_info;
      MCcall(find_struct_info(3, mc_vargs));
    }
    printf("cti-could %sfind type '%s'\n", p_struct_info ? "" : "NOT ", finalized_identity);
    if (p_struct_info) {
      struct_info *str_info = (struct_info *)p_struct_info;
      matched = true;

      // MC structures require another dereference
      // ++type_deref_count;

      // printf("ctn-3\n");
      // Change Name
      free(finalized_identity);
      allocate_and_copy_cstr(finalized_identity, str_info->declared_mc_name);

      // Add reference to function infos struct usages
      if (func_info) {
        // printf("ctn-4\n");
        struct_info **new_collection =
            (struct_info **)malloc(sizeof(struct_info *) * (func_info->struct_usage_count + 1));
        if (func_info->struct_usage_count) {
          memcpy((void *)new_collection, func_info->struct_usage,
                 sizeof(struct_info *) * func_info->struct_usage_count);
        }
        new_collection[func_info->struct_usage_count] = str_info;
        if (func_info->struct_usage)
          free(func_info->struct_usage);

        // printf("ctn-6\n");
        func_info->struct_usage_count = func_info->struct_usage_count + 1;
        func_info->struct_usage = new_collection;
      }
    }
  }
  // printf("ctn-8\n");

  // Re-add any deref operators
  if (type_deref_count) {
    char *temp = (char *)malloc(sizeof(char) * (strlen(finalized_identity) + 1 + type_deref_count + 1));
    strcpy(temp, finalized_identity);
    strcat(temp, " ");
    for (int i = 0; i < type_deref_count; ++i)
      strcat(temp, "*");

    free(finalized_identity);
    finalized_identity = temp;
  }

  // No modification, use the original value
  printf("finalized_identity='%s'\n", finalized_identity);
  allocate_and_copy_cstr((*conformed_type_identity), finalized_identity);
  free(finalized_identity);

  return 0;
}

#define RETURN_VALUE_IDENTIFIER "mc_return_value"

int instantiate_function_v1(int argc, void **argv)
{
  register_midge_error_tag("instantiate_function_v1()");

  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/
  // printf("~instantiate()\n");

  char *function_name = *(char **)argv[0];
  char *midge_c = *(char **)argv[1];

  // Find the function info
  mc_function_info_v1 *func_info = NULL;
  {
    void *mc_vargs[3];
    mc_vargs[0] = (void *)&func_info;
    mc_vargs[1] = (void *)&command_hub->nodespace;
    mc_vargs[2] = (void *)&function_name;
    MCcall(find_function_info(3, mc_vargs));
    if (!func_info) {
      MCerror(184, "cannot find function info for function_name=%s", function_name);
    }
  }
  // printf("@ifv-0\n");

  // Translate the code-block from script into workable midge-cling C
  // char *midge_c;
  // {
  //   void *mc_vargs[3];
  //   mc_vargs[0] = (void *)&midge_c;
  //   mc_vargs[1] = (void *)&func_info;
  //   mc_vargs[2] = (void *)&script;
  //   MCcall(parse_script_to_mc(3, mc_vargs));
  // }

  // printf("@ifv-1\n");

  // // const char *identity; const char *type; struct_info *struct_info;
  // struct {
  //   const char *var_name;
  //   const char *type;
  //   void *struct_info;
  // } declared_types[200];
  // int declared_type_count = 0;

  // Construct the function identifier
  const char *function_identifier_format = "%s_v%u";
  char func_identity_buf[256];
  func_identity_buf[0] = '\0';
  sprintf(func_identity_buf, function_identifier_format, func_info->name, func_info->latest_iteration);

  // printf("@ifv-2\n");
  // Construct the function parameters
  char param_buf[4096];
  param_buf[0] = '\0';
  for (int i = 0; i < func_info->parameter_count; ++i) {
    if (func_info->parameters[i]->is_function_pointer) {

      // TODO -- include mc_structs in these parameters

      if (strncmp("int (**", func_info->parameters[i]->full_function_pointer_declaration, 7)) {
        MCerror(715, "TODO");
      }
      if (strncmp(")(int, void **)",
                  func_info->parameters[i]->full_function_pointer_declaration +
                      (strlen(func_info->parameters[i]->full_function_pointer_declaration) - 15),
                  16)) {
        printf("ffpd:'%s'\n", func_info->parameters[i]->full_function_pointer_declaration);
        printf("ffpd:'%s'\n", func_info->parameters[i]->full_function_pointer_declaration +
                                  (strlen(func_info->parameters[i]->full_function_pointer_declaration) - 15));
        MCerror(721, "TODO");
      }
      sprintf(param_buf + strlen(param_buf), "  %s = *(int (***)(int, void **))argv[%i];\n",
              func_info->parameters[i]->full_function_pointer_declaration, i);

      // struct_info *find_struct_info(node *nodespace);

      // mc_struct_info *(*fsi)(node *) = *(mc_struct_info *(**)(node *))args[0];???

      continue;
    }

    // MC conformed type name
    char *parameter_type;
    {
      struct_info *type_struct_info;

      void *mc_vargs[3];
      mc_vargs[0] = (void *)&command_hub->nodespace;
      mc_vargs[1] = (void *)&func_info->parameters[i]->type_name;
      mc_vargs[2] = (void *)&type_struct_info;
      MCcall(find_struct_info(3, mc_vargs));
      // printf("@ifv-2 func_info->parameters[i]->type_name:%s\n", func_info->parameters[i]->type_name);

      if (type_struct_info) {
        allocate_and_copy_cstr(parameter_type, type_struct_info->declared_mc_name);
        // if (!func_info->parameters[i]->type_deref_count) {
        //   MCerror(734, "TODO some deref acrobatics to deal with mc_types");
        // }
      }
      else {
        allocate_and_copy_cstr(parameter_type, func_info->parameters[i]->type_name);
      }
    }
    // printf("@ifv-2a\n");
    // printf("func_info->parameter_count:%i\n", func_info->parameters[0]->type_deref_count);

    // Deref
    char derefbuf[24];
    for (int j = 0; j < func_info->parameters[i]->type_deref_count; ++j)
      derefbuf[j] = '*';
    derefbuf[func_info->parameters[i]->type_deref_count] = '\0';
    // printf("@ifv-2b\n");

    // Decl
    sprintf(param_buf + strlen(param_buf), "  %s %s%s = ", parameter_type, derefbuf, func_info->parameters[i]->name);

    // Deref + 1 (unless its the return value)
    derefbuf[0] = ' ';
    for (int j = 0; j < func_info->parameters[i]->type_deref_count; ++j)
      derefbuf[1 + j] = '*';
    derefbuf[1 + func_info->parameters[i]->type_deref_count] = '*';
    derefbuf[1 + func_info->parameters[i]->type_deref_count + 1] = '\0';

    // printf("@ifv-2c\n");
    // Assignment
    sprintf(param_buf + strlen(param_buf), "*(%s%s)argv[%i];\n", parameter_type, derefbuf, i);

    free(parameter_type);
  }
  // printf("@ifv-5\n");

  // Append return-value
  if (func_info->return_type.deref_count || strcmp(func_info->return_type.name, "void")) {
    // MC conformed type name
    char *return_type;
    {
      struct_info *type_struct_info;

      void *mc_vargs[3];
      mc_vargs[0] = (void *)&command_hub->nodespace;
      mc_vargs[1] = (void *)&func_info->return_type.name;
      mc_vargs[2] = (void *)&type_struct_info;
      MCcall(find_struct_info(3, mc_vargs));

      if (type_struct_info) {
        allocate_and_copy_cstr(return_type, type_struct_info->declared_mc_name);
        if (!func_info->return_type.deref_count) {
          MCerror(734, "TODO some deref acrobatics to deal with mc_types");
        }
      }
      else {
        allocate_and_copy_cstr(return_type, func_info->return_type.name);
      }
    }
    // printf("ifv-return_type:'%s' func_info->return_type.deref_count:'%u'\n", return_type,
    //        func_info->return_type.deref_count);

    // Deref
    char derefbuf[24];
    for (int j = 0; j < 1 + func_info->return_type.deref_count; ++j)
      derefbuf[j] = '*';
    derefbuf[1 + func_info->return_type.deref_count] = '\0';

    // Decl
    sprintf(param_buf + strlen(param_buf), "  %s %s%s = ", return_type, derefbuf, RETURN_VALUE_IDENTIFIER);

    // Deref + 1 (unless its the return value)
    derefbuf[0] = ' ';
    for (int j = 0; j < func_info->return_type.deref_count; ++j)
      derefbuf[1 + j] = '*';
    derefbuf[1 + func_info->return_type.deref_count] = '*';
    derefbuf[1 + func_info->return_type.deref_count + 1] = '\0';

    // Assignment
    sprintf(param_buf + strlen(param_buf), "(%s%s)argv[%i];\n", return_type, derefbuf, func_info->parameter_count);

    free(return_type);
  }
  register_midge_error_tag("instantiate_function_v1()-8");

  // printf("@ifv-8\n");
  // Declare the function
  const char *function_declaration_format = "int %s(int argc, void **argv) {\n"
                                            "\n"
                                            "  register_midge_error_tag(\"%s()\");\n"
                                            "\n"
                                            "  // midge Command Hub\n"
                                            "  mc_command_hub_v1 *comman"
                                            "d_hub = command_hub;\n" // TODO -- maybe, just directly replace all
                                                                     // command hub references with the casted pointer
                                            "\n"
                                            "  // Function Parameters\n"
                                            "%s"
                                            "\n"
                                            "  // Function Code\n"
                                            "%s"
                                            "\n"
                                            "  register_midge_error_tag(\"%s(~)\");\n"
                                            "\n"
                                            "  return 0;\n"
                                            "}";
  int function_declaration_length = snprintf(NULL, 0, function_declaration_format, func_identity_buf, func_identity_buf,
                                             param_buf, midge_c, func_identity_buf);
  char *function_declaration = (char *)malloc(sizeof(char) * (function_declaration_length + 1));
  sprintf(function_declaration, function_declaration_format, func_identity_buf, func_identity_buf, param_buf, midge_c,
          func_identity_buf);

  // Declare the function
  printf("ifv>cling_declare:\n%s\n", function_declaration);
  // printf("ifv>cling_declare:'%s'\n", func_identity_buf);
  MCcall(clint_declare(function_declaration));

  // Set the method to the function pointer
  char decl_buf[1024];
  sprintf(decl_buf, "%s = &%s;", func_info->name, func_identity_buf);
  // printf("ifv>clint_process:\n%s\n", decl_buf);
  MCcall(clint_process(decl_buf));

  free(function_declaration);
  // free(midge_c);
  // printf("ifv-concludes\n");
  register_midge_error_tag("instantiate_function_v1(~)");
  return 0;
}

int _parse_past_conformed_type_identifier(function_info *func_info, char *code, int *i, char **conformed_type_identity)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  *conformed_type_identity = NULL;

  if (!isalpha(code[*i])) {
    MCcall(print_parse_error(code, *i, "_parse_past_conformed_type_identifier", ""));
    MCerror(459, "Type must begin with alpha character was:'%c'", code[*i]);
  }

  char *type_identity;
  int s = *i;
  for (;; ++*i) {
    if (code[*i] == '\0') {
      MCerror(772, "ERROR");
    }
    if (!isalnum(code[*i]) && code[*i] != '_') {
      break;
    }
  }
  type_identity = (char *)malloc(sizeof(char) * (*i - s + 1));
  strncpy(type_identity, code + s, *i - s);
  type_identity[*i - s] = '\0';

  {
    char *mc_conformed_type;
    void *mc_vargs[4];
    mc_vargs[0] = (void *)&command_hub->nodespace;
    mc_vargs[1] = (void *)&func_info;
    mc_vargs[2] = (void *)&type_identity;
    mc_vargs[3] = (void *)&mc_conformed_type;
    MCcall(conform_type_identity(4, mc_vargs));
    // printf("@ifv-5\n");
    // printf("ifv:paramName:'%s' conformed_type_name:'%s'\n", func_info->parameters[i]->type_name,
    // conformed_type_name);

    if (mc_conformed_type) {
      // Require an extra dereference for parameters
      free(type_identity);
      mc_pprintf(&type_identity, "%s *", mc_conformed_type);
      free(mc_conformed_type);
    }
  }

  *conformed_type_identity = type_identity;
  return 0;
}

int parse_past_conformed_type_declaration(function_info *owner, char *code, int *i, char **type_declaration)
{
  *type_declaration = NULL;

  bool leadingConst = !strncmp(code + *i, "const", 5);
  if (leadingConst) {
    MCcall(parse_past(code, i, "const"));
    MCcall(parse_past_empty_text(code, i));
  }

  char *primitive_modifier = NULL;
  if (!strncmp(code + *i, "unsigned", 8)) {
    allocate_and_copy_cstr(primitive_modifier, "unsigned ");
    MCcall(parse_past(code, i, "unsigned"));
    MCcall(parse_past_empty_text(code, i));
  }

  MCcall(_parse_past_conformed_type_identifier(owner, code, i, type_declaration));
  MCcall(parse_past_empty_text(code, i));

  // printf("ppctd-0:'%s'\n", *type_declaration);
  if (primitive_modifier) {
    char *prependedModStr = (char *)malloc(sizeof(char) * (strlen(primitive_modifier) + strlen(*type_declaration) + 1));
    strcpy(prependedModStr, primitive_modifier);
    strcat(prependedModStr, *type_declaration);
    free(*type_declaration);
    *type_declaration = prependedModStr;
  }
  if (leadingConst) {
    char *prependedConstStr = (char *)malloc(sizeof(char) * (6 + strlen(*type_declaration) + 1));
    strcpy(prependedConstStr, "const ");
    strcat(prependedConstStr, *type_declaration);
    free(*type_declaration);
    *type_declaration = prependedConstStr;
  }

  // printf("ppctd-1:'%s'\n", *type_declaration);
  uint type_declaration_alloc = strlen(*type_declaration) + 1;
  while (1) {
    if (!strncmp(code + *i, "const", 5)) {
      MCcall(append_to_cstr(&type_declaration_alloc, type_declaration, " const"));
      MCcall(parse_past(code, i, "const"));
      MCcall(parse_past_empty_text(code, i));
      // printf("ppctd-2:'%s'\n", *type_declaration);
      continue;
    }

    uint type_deref_count = 0;
    while (code[*i] == '*') {
      ++type_deref_count;
      ++*i;
      MCcall(parse_past_empty_text(code, i));
    }
    if (type_deref_count) {
      MCcall(append_to_cstr(&type_declaration_alloc, type_declaration, " "));
      for (int j = 0; j < type_deref_count; ++j) {
        MCcall(append_to_cstr(&type_declaration_alloc, type_declaration, "*"));
      }
      // printf("ppctd-3:'%s'\n", *type_declaration);
      continue;
    }
    break;
  }
  // printf("ppctd-f:'%s'\n", *type_declaration);

  return 0;
}

int parse_past_expression(function_info *func_info, void *nodespace, char *code, int *i, char **output)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/
  char *primary, *temp;

  if (isalpha(code[*i])) {

    MCcall(parse_past_identifier(code, i, &primary, false, true));

    if (!strcmp(primary, "command_hub")) {
      int len = snprintf(NULL, 0, "((mc_command_hub_v1 *)%p)", command_hub);
      free(primary);
      primary = (char *)malloc(sizeof(char) * (len + 1));
      sprintf(primary, "((mc_command_hub_v1 *)%p)", command_hub);
    }

    switch (code[*i]) {
    case '[': {
      for (int b = 1;; ++b) {
        // Parse past the '['
        ++*i;

        char *secondary;
        parse_past_expression(func_info, (void *)nodespace, code, i, &secondary);

        temp = (char *)malloc(sizeof(char) * (strlen(primary) + 1 + strlen(secondary) + b + 1));
        strcpy(temp, primary);
        strcat(temp, "[");
        strcat(temp, secondary);
        free(primary);
        free(secondary);
        primary = temp;

        if (code[*i] != '[') {
          int k = strlen(primary);
          for (int c = 0; c < b; ++c) {
            primary[k + c] = ']';
            ++*i;
          }
          primary[k + b] = '\0';

          switch (code[*i]) {
          case ' ':
          case '\0':
          case '\n':
            *output = primary;
            return 0;
            break;
          case '-': {
            MCcall(parse_past(code, i, "->"));
            MCcall(parse_past_identifier(code, i, &secondary, true, true));
            if (code[*i] != '[') {
              temp = (char *)malloc(sizeof(char) * (strlen(primary) + 2 + strlen(secondary) + 1));
              strcpy(temp, primary);
              strcat(temp, "->");
              strcat(temp, secondary);
              free(primary);
              free(secondary);
              *output = temp;
              return 0;
            }
            MCerror(326, "TODO ");
          }
          default:
            print_parse_error(code, *i, "parse_past_expression", "default:break;");
            MCerror(329, "TODO '%c'", code[*i]);
          }
        }
      }
    } break;
    case '-': {
      parse_past(code, i, "->");
      parse_past_expression(func_info, nodespace, code, i, &temp);
      int len = snprintf(NULL, 0, "%s->%s", primary, temp);
      *output = (char *)malloc(sizeof(char) * (len + 1));
      sprintf(*output, "%s->%s", primary, temp);
      // printf("primary:'%s'\n", primary);
      // printf("temp:'%s'\n", temp);
      free(primary);
      free(temp);
      return 0;
    }
    default: {
      *output = primary;
      return 0;
    }
    }
  }
  else
    switch (code[*i]) {
    case '"': {
      MCcall(mc_parse_past_literal_string(code, i, output));

      return 0;
    }
    case '\'': {
      MCcall(parse_past_character_literal(code, i, output));

      return 0;
    }
    case '&': {
      ++*i;
      MCcall(parse_past_expression(func_info, (void *)nodespace, code, i, &primary));
      temp = (char *)malloc(sizeof(char) * (strlen(primary) + 2));
      strcpy(temp, "&");
      strcat(temp, primary);
      temp[strlen(primary) + 1] = '\0';

      free(primary);
      *output = temp;

      return 0;
    }
    case '(': {
      // Assume to be a casting - do containment other ways
      parse_past(code, i, "(");
      MCcall(parse_past_conformed_type_declaration(func_info, code, i, &primary));
      parse_past(code, i, ")");

      MCcall(parse_past_expression(func_info, (void *)nodespace, code, i, &temp));

      *output = (char *)malloc(sizeof(char) * (2 + strlen(primary) + strlen(temp) + 1));
      sprintf(*output, "(%s)%s", primary, temp);
      free(primary);
      free(temp);

      return 0;
    }
    case '!': {
      ++*i;
      MCcall(parse_past_expression(func_info, (void *)nodespace, code, i, &primary));
      temp = (char *)malloc(sizeof(char) * (strlen(primary) + 2));
      strcpy(temp, "!");
      strcat(temp, primary);
      temp[strlen(primary) + 1] = '\0';

      free(primary);
      *output = temp;

      return 0;
    }
    case '$': {
      ++*i;

      MCcall(parse_past_expression(func_info, nodespace, code, i, output));

      char *temp = (char *)malloc(sizeof(char) * (strlen(*output) + 2));
      sprintf(temp, "$%s", *output);
      free(*output);
      *output = temp;

      return 0;
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      MCcall(parse_past_number(code, i, output));

      return 0;
    }
    case '+':
    case '-':
    case '*':
    case '/':
    case '%': {
      char *oper = (char *)malloc(sizeof(char) * 2);
      oper[0] = code[*i];
      oper[1] = '\0';
      ++*i;
      if (code[*i] != ' ') {
        MCerror(404, "BackTODO");
      }
      MCcall(parse_past(code, i, " "));

      // left
      char *left;
      MCcall(parse_past_expression(func_info, nodespace, code, i, &left));
      MCcall(parse_past(code, i, " "));

      // right
      char *right;
      MCcall(parse_past_expression(func_info, nodespace, code, i, &right));

      *output = (char *)malloc(sizeof(char) * (strlen(oper) + 1 + strlen(left) + 1 + strlen(right) + 1));
      sprintf(*output, "%s %s %s", left, oper, right);

      free(left);
      free(oper);
      free(right);
    }
      return 0;
    default: {
      MCcall(print_parse_error(code, *i, "parse_past_expression", "first_char"));
      return -427;
    }
    }

  MCerror(432, "Incorrectly Handled");
}

int create_default_mc_struct_v1(int argc, void **argv)
{
  // printf("@cdms-1\n");
  // Arguments
  void **ptr_output = (void **)argv[0];
  char *type_name = *(char **)argv[1];

  if (!strcmp(type_name, "mc_node_v1 *")) {
    // printf("@cdms-2\n");
    mc_node_v1 *data = (mc_node_v1 *)calloc(sizeof(mc_node_v1), 1);
    data->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
    data->struct_id->identifier = "mc_node_v1";
    data->struct_id->version = 1U;
    data->name = NULL;
    data->parent = NULL;
    data->functions_alloc = 1;
    data->function_count = 0;
    data->functions = (mc_function_info_v1 **)malloc(sizeof(mc_function_info_v1 *) * data->functions_alloc);
    data->structs_alloc = 1;
    data->struct_count = 0;
    data->structs = (mc_struct_info_v1 **)malloc(sizeof(mc_struct_info_v1 *) * data->structs_alloc);
    data->children_alloc = 1;
    data->child_count = 0;
    data->children = (mc_node_v1 **)malloc(sizeof(mc_node_v1 *) * data->children_alloc);

    *ptr_output = (void *)data;
    // printf("@cdms-3\n");
    return 0;
  }
  MCerror(616, "Unrecognized type:%s", type_name);
}

int parse_script_to_mc_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  char **output = (char **)argv[0];
  function_info *func_info = *(function_info **)argv[1];
  char *code = *(char **)argv[2];

  // Parse the script one statement at a time
  unsigned int translation_alloc = 80;
  char *translation = (char *)malloc(sizeof(char) * translation_alloc);
  translation[0] = '\0';
  char buf[2048];
  int i = 0;
  int code_len = strlen(code);
  while (i <= code_len) {
    switch (code[i]) {
    case 'a': {
      // ass / asi
      MCcall(parse_past(code, &i, "ass"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      char *dest_expr;
      MCcall(parse_past_expression(func_info, command_hub->nodespace, code, &i, &dest_expr));
      MCcall(parse_past(code, &i, " "));

      char *src_expr;
      MCcall(parse_past_expression(func_info, command_hub->nodespace, code, &i, &src_expr));
      if (code[i] != '\n' && code[i] != '\0') {
        MCerror(-4829, "expected statement end");
      }

      sprintf(buf, "%s = %s;\n", dest_expr, src_expr);
      MCcall(append_to_cstr(&translation_alloc, &translation, buf));

      // free(var_identifier);
      // free(left);
    } break;
    case 'c': {
      // cpy - deep copy of known types
      MCcall(parse_past(code, &i, "cpy"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // Type Identifier
      char *type_identifier;
      MCcall(parse_past_conformed_type_declaration(func_info, code, &i, &type_identifier));
      MCcall(parse_past(code, &i, " "));

      // Destination Name
      char *dest_expr;
      MCcall(parse_past_expression(func_info, command_hub->nodespace, code, &i, &dest_expr));
      MCcall(parse_past(code, &i, " "));

      // Source Name
      char *src_expr;
      MCcall(parse_past_expression(func_info, command_hub->nodespace, code, &i, &src_expr));
      if (code[i] != '\n' && code[i] != '\0') {
        MCerror(589, "expected end of statement");
      }

      if (!strcmp(type_identifier, "char *")) {
        sprintf(buf,
                "{\n"
                "  char *mc_tmp_constchar = (char *)malloc(sizeof(char) * (strlen(%s) + 1));\n"
                "  strcpy(mc_tmp_constchar, %s);\n"
                "  %s = mc_tmp_constchar;\n"
                "}\n",
                src_expr, src_expr, dest_expr);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));
      }
      else {
        MCerror(727, "Unsupported deep copy type");
      }

      free(type_identifier);
      free(dest_expr);
      free(src_expr);
    } break;
    case 'd': {
      MCcall(parse_past(code, &i, "dc"));
      if (code[i] == 'd') {
        // dcd - initialization of configured mc_structures to default values
        MCcall(parse_past(code, &i, "d"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // parse_past(code, &i, "'node *' child");
        // break;
        // Identifier
        char *type_identifier;
        MCcall(parse_past_conformed_type_declaration(func_info, code, &i, &type_identifier));
        MCcall(parse_past(code, &i, " "));
        // parse_past(code, &i, "child");
        // break;

        // Variable Name
        char *var_name;
        MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(589, "expected end of statement");
        }

        // Normal Declaration
        sprintf(buf, "%s %s;\n", type_identifier, var_name);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        // Call for default allocation
        sprintf(buf,
                "{\n"
                // "printf(\"@innercdmscall-1\\n\");"
                "  void *mc_vargs[2];\n"
                "  mc_vargs[0] = (void *)&%s;\n"
                "  const char *mc_type_name = \"%s\";\n"
                "  mc_vargs[1] = (void *)&mc_type_name;\n"
                "  MCcall(create_default_mc_struct(2, mc_vargs));\n"
                "}\n",
                var_name, type_identifier);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        free(var_name);
        free(type_identifier);
      }
      else if (code[i] == 'l') {
        MCerror(1366, "TODO");
        // // dcl
        // MCcall(parse_past(code, &i, "l"));
        // MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // // Identifier
        // char *type_identifier;
        // MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        // MCcall(parse_past(code, &i, " "));

        // // Variable Name
        // char *var_name;
        // MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        // if (code[i] != '\n' && code[i] != '\0') {
        //   // Array decl
        //   MCcall(parse_past(code, &i, "["));

        //   char *left;
        //   MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &left));
        //   MCcall(parse_past(code, &i, "]"));
        //   if (code[i] != '\n' && code[i] != '\0') {
        //     MCerror(-4829, "expected statement end");
        //   }

        //   // Add deref to type
        //   char *new_type_identifier = (char *)malloc(sizeof(char) * (strlen(type_identifier) + 2));
        //   strcpy(new_type_identifier, type_identifier);
        //   strcat(new_type_identifier, "*");
        //   new_type_identifier[strlen(type_identifier) + 1] = '\0';

        //   // Normal Declaration
        //   MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc,
        //   &local_indexes_count, script,
        //                                      buf, local_scope_depth, new_type_identifier, var_name));
        //   // printf("dcl-here-0\n");
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   // printf("dcl-here-1\n");

        //   // Allocate the array
        //   char *replace_var_name;
        //   MCcall(
        //       mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name,
        //       &replace_var_name));
        //   // printf("dcl-here-2\n");

        //   sprintf(buf, "%s = (%s)malloc(sizeof(%s) * (%s));\n", replace_var_name, new_type_identifier,
        //   type_identifier, left);
        //   // printf("dcl-here-3\n");
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   // printf("dcl-here-4\n");

        //   free(left);
        //   // printf("dcl-here-5\n");
        //   free(new_type_identifier);
        //   // printf("dcl-here-6\n");
        //   // printf("Translation:\n%s\n", translation);
        //   // printf("dcl-here-7\n");
        // }
        // else {
        //   // Normal Declaration
        //   MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc,
        //   &local_indexes_count, script,
        //                                      buf, local_scope_depth, type_identifier, var_name));
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        // }
        // free(var_name);
        // free(type_identifier);
      }
      else if (code[i] == 's') {
        MCerror(641, "TODO");
        //   // dcs
        //   MCcall(parse_past(code, &i, "s"));
        //   MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        //   // Identifier
        //   char *type_identifier;
        //   MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        //   MCcall(parse_past(code, &i, " "));

        //   // Variable Name
        //   char *var_name;
        //   MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        //   MCcall(parse_past(code, &i, " "));

        //   // Set Value
        //   char *left;
        //   MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &left));
        //   if (code[i] != '\n' && code[i] != '\0') {
        //     MCerror(-4829, "expected statement end");
        //   }

        //   // Generate Local
        //   MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc,
        //   &local_indexes_count, script,
        //                                      buf, local_scope_depth, type_identifier, var_name));
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        //   // Assign
        //   char *replace_var_name;
        //   MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name,
        //   &replace_var_name));

        //   sprintf(buf, "%s = %s;\n", replace_var_name, left);
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        //   free(type_identifier);
        //   free(var_name);
        //   free(left);
        // }
        // else {
        //   MCerror(678, "TODO");
        // }
      }
    } break;
    case 'n': {
      MCcall(parse_past(code, &i, "nv"));
      if (code[i] == 'i') {
        MCerror(286, "TODO");
        // // nvi
        // MCcall(parse_past(code, &i, "i"));
        // MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // // Type Identifier
        // char *type_identifier;
        // MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        // MCcall(parse_past(code, &i, " "));

        // // Variable Name
        // char *var_name;
        // MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        // MCcall(parse_past(code, &i, " "));

        // MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc,
        // &local_indexes_count, script,
        //                                    buf, local_scope_depth, type_identifier, var_name));
        // append_to_cstr(&translation_alloc, &translation, buf);

        // char *replace_name;
        // MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name,
        // &replace_name));
        // // printf("nvi gen replace_name:%s=%s\n", var_name, replace_name);

        // // Invoke
        // // Function Name
        // char *function_name;
        // MCcall(parse_past_identifier(code, &i, &function_name, true, false));

        // // printf("dopeh\n");
        // function_info_v1 *func_info;
        // {
        //   void *mc_vargs[3];
        //   mc_vargs[0] = (void *)&func_info;
        //   mc_vargs[1] = (void *)&nodespace;
        //   mc_vargs[2] = (void *)&function_name;
        //   find_function_info(3, mc_vargs);
        // }
        // // printf("dopey\n");
        // if (func_info) {
        //   if (!strcmp(func_info->return_type, "void")) {
        //     MCerror(-1002, "compile error: cannot assign from a void function!");
        //   }
        //   else {
        //     sprintf(buf,
        //             "{\n"
        //             "  mc_vargs[0] = (void *)&%s;\n",
        //             replace_name);
        //     MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   }
        // }
        // else {
        //   sprintf(buf, "%s = %s(", replace_name, function_name);
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        // }

        // int arg_index = 0;
        // while (code[i] != '\n' && code[i] != '\0') {
        //   MCcall(parse_past(code, &i, " "));

        //   char *argument;
        //   MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i,
        //   &argument));
        //   // MCcall(parse_past_identifier(code, &i, &argument, true, true));
        //   // MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, argument,
        //   // &replace_name)); char *arg_entry = argument; if (replace_name)
        //   //   arg_entry = replace_name;
        //   if (func_info) {
        //     if (argument[0] == '$') {
        //       sprintf(buf,
        //               "  char *mc_context_data_%i;\n"
        //               "  MCcall(get_process_contextual_data(script_instance->contextual_action, \"%s\", (void "
        //               "**)&mc_context_data_%i));\n"
        //               "  mc_vargs[%i] = (void *)&mc_context_data_%i;\n",
        //               arg_index + 1, argument + 1, arg_index + 1, arg_index + 1, arg_index + 1);
        //     }
        //     else {
        //       sprintf(buf, "mc_vargs[%i] = (void *)&%s;\n", arg_index + 1, argument);
        //     }
        //   }
        //   else {
        //     if (argument[0] == '$') {
        //       MCerror(4829, "NOT YET IMPLEMENTED");
        //     }
        //     sprintf(buf, "%s%s", arg_index ? ", " : "", argument);
        //   }
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   ++arg_index;
        //   free(argument);
        // }

        // if (func_info) {

        //   // append_to_cstr(&translation_alloc, &translation, "  printf(\"here-22  =%s\\n\", (char
        //   *)mc_vargs[2]);\n"); sprintf(buf, "  %s(%i, mc_vargs", function_name, arg_index + 1);
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        // }

        // MCcall(append_to_cstr(&translation_alloc, &translation, ");\n"));
        // if (func_info)
        //   MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));
        // // MCcall(append_to_cstr(&translation_alloc, &translation, "  printf(\"here-31\\n\");\n"));

        // free(function_name);
      }
      else if (code[i] == 'k') {
        // nvk
        MCcall(parse_past(code, &i, "k"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Invoke
        // Function
        char *function_name;
        MCcall(parse_past_identifier(code, &i, &function_name, true, false));
        MCcall(append_to_cstr(&translation_alloc, &translation, function_name));
        MCcall(append_to_cstr(&translation_alloc, &translation, "("));

        function_info *func_info;
        {
          void *mc_vargs[3];
          mc_vargs[0] = (void *)&func_info;
          mc_vargs[1] = (void *)&command_hub->nodespace;
          mc_vargs[2] = (void *)&function_name;
          find_function_info(3, mc_vargs);
        }
        if (func_info) {
          // Return Parameter
          sprintf(buf, "{\n"
                       "  mc_vargs[0] = NULL;\n");
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }
        else {
          sprintf(buf, "%s(", function_name);
        }

        // Arguments
        int arg_index = 0;
        while (code[i] != '\n' && code[i] != '\0') {
          MCcall(parse_past(code, &i, " "));

          char *argument;
          MCcall(parse_past_expression(func_info, (void *)command_hub->nodespace, code, &i, &argument));

          // printf("argument='%s'\n", argument);

          if (func_info) {
            sprintf(buf, "  mc_vargs[%i] = (void *)&%s;\n", arg_index + 1, argument);
          }
          else {
            sprintf(buf, "%s%s", arg_index ? ", " : "", argument);
          }
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          ++arg_index;
          free(argument);
        }

        if (func_info) {
          // append_to_cstr(&translation_alloc, &translation, "  printf(\"here-22  =%s\\n\", (char *)mc_vargs[2]);\n");
          sprintf(buf, "  %s(%i, mc_vargs", func_info->name, arg_index + 1);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }

        MCcall(append_to_cstr(&translation_alloc, &translation, ");\n"));
        if (func_info)
          MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));
        // MCcall(append_to_cstr(&translation_alloc, &translation, "  printf(\"here-31\\n\");\n"));

        free(function_name);
      }
      else {
        MCerror(742, "TODO");
      }
    } break;
    case '\n': {
      MCcall(parse_past(code, &i, "\n"));
    } break;
    case ' ': {
      MCcall(parse_past(code, &i, " "));
    } break;
    case '\t': {
      MCcall(parse_past(code, &i, "\t"));
    } break;
    case '\0': {
      // Trim translation
      *output = translation;
      return 0;
    }
    default: {
      // printf("\ntranslation:\n%s\n\n", translation);
      MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "UnhandledStatement"));
      return 285;
    }
    }
  }

  MCerror(279, "Improperly terminated script argument");
}

int parse_past_mc_identifier(const char *text, int *index, char **identifier, bool include_member_access,
                             bool include_referencing)
{
  int o = *index;
  bool hit_alpha = false;
  while (1) {
    int doc = 1;
    switch (text[*index]) {
    case ' ':
    case '\n':
    case '\t':
    case '\0':
    case '=':
      doc = 0;
      break;
    default: {
      if (isalpha(text[*index])) {
        hit_alpha = true;
        break;
      }
      if (*index > o && isalnum(text[*index]))
        break;
      if (text[*index] == '_')
        break;
      if (include_member_access) {
        if (text[*index] == '-' && text[*index + 1] == '>') {
          ++*index;
          break;
        }
        if (text[*index] == '.')
          break;
      }
      if (include_referencing) {
        if (!hit_alpha) {
          if (text[*index] == '&')
            break;
          if (text[*index] == '*')
            break;
        }
      }

      // Identifier end found
      doc = 0;
    } break;
    }
    if (!doc) {
      if (o == *index) {
        MCcall(print_parse_error(text, *index, "parse_past_mc_identifier", "identifier_end"));
        MCerror(1634, "error");
      }

      *identifier = (char *)calloc(sizeof(char), *index - o + 1);
      strncpy(*identifier, text + o, *index - o);
      (*identifier)[*index - o] = '\0';
      return 0;
    }
    if (text[*index] == '\0')
      return 0;
    ++*index;
  }
  return -1742;
}

int transcribe_past(char const *const code, int *index, uint *transcription_alloc, char **transcription,
                    char const *const sequence)
{
  for (int i = 0;; ++i) {
    if (sequence[i] == '\0') {
      MCcall(append_to_cstrn(transcription_alloc, transcription, code + *index, i));
      *index += i;
      return 0;
    }
    else if (code[*index + i] == '\0') {
      return -1;
    }
    else if (sequence[i] != code[*index + i]) {
      print_parse_error(code, *index + i, "see_below", "");
      printf("!parse_past() expected:'%c' was:'%c'\n", sequence[i], code[*index + i]);
      return 1 + i;
    }
  }
}

int peek_mc_token(char *code, int i, uint tokens_ahead, mc_token *output);
int peek_mc_token(char *code, int i, uint tokens_ahead, mc_token *output)
{
  MCcall(parse_past_empty_text(code, &i));
  // printf("peek_mc_token(): %u:'%c'\n", tokens_ahead, code[i]);
  switch (code[i]) {
  case '-': {
    if (code[i + 1] == '>') {
      if (!tokens_ahead) {
        output->type = MC_TOKEN_POINTER_OPERATOR;
        allocate_and_copy_cstr(output->text, "->");
        output->start_index = i;
        return 0;
      }

      MCcall(peek_mc_token(code, i + 2, tokens_ahead - 1, output));
      return 0;
    }
    else if (code[i + 1] == '-') {
      if (!tokens_ahead) {
        output->type = MC_TOKEN_DECREMENT_OPERATOR;
        allocate_and_copy_cstr(output->text, "--");
        output->start_index = i;
        return 0;
      }

      MCcall(peek_mc_token(code, i + 2, tokens_ahead - 1, output));
      return 0;
    }

    if (!tokens_ahead) {
      output->type = MC_TOKEN_SUBTRACT_OPERATOR;
      allocate_and_copy_cstr(output->text, "-");
      output->start_index = i;
      return 0;
    }

    MCcall(peek_mc_token(code, i + 1, tokens_ahead - 1, output));
    return 0;
  }
  case '*': {
    if (!tokens_ahead) {
      output->type = MC_TOKEN_STAR_CHARACTER;
      allocate_and_copy_cstr(output->text, "*");
      output->start_index = i;
      return 0;
    }
    MCcall(peek_mc_token(code, i + 1, tokens_ahead - 1, output));
    return 0;
  }
  case '(': {
    if (!tokens_ahead) {
      output->type = MC_TOKEN_OPENING_BRACKET;
      allocate_and_copy_cstr(output->text, "(");
      output->start_index = i;
      return 0;
    }
    MCcall(peek_mc_token(code, i + 1, tokens_ahead - 1, output));
    return 0;
  }
  case ';': {
    if (!tokens_ahead) {
      output->type = MC_TOKEN_SEMI_COLON;
      allocate_and_copy_cstr(output->text, ";");
      output->start_index = i;
      return 0;
    }
    MCcall(peek_mc_token(code, i + 1, tokens_ahead - 1, output));
    return 0;
  }
  case '[': {
    if (!tokens_ahead) {
      output->type = MC_TOKEN_SQUARE_OPENING_BRACKET;
      allocate_and_copy_cstr(output->text, "[");
      output->start_index = i;
      return 0;
    }
    MCcall(peek_mc_token(code, i + 1, tokens_ahead - 1, output));
    return 0;
  }
  case '=': {
    if (code[i + 1] == '=') {
      if (!tokens_ahead) {
        output->type = MC_TOKEN_EQUALITY_OPERATOR;
        allocate_and_copy_cstr(output->text, "==");
        output->start_index = i;
        return 0;
      }

      MCcall(peek_mc_token(code, i + 2, tokens_ahead - 1, output));
      return 0;
    }

    if (!tokens_ahead) {
      output->type = MC_TOKEN_ASSIGNMENT_OPERATOR;
      allocate_and_copy_cstr(output->text, "=");
      output->start_index = i;
      return 0;
    }

    MCcall(peek_mc_token(code, i + 1, tokens_ahead - 1, output));
    return 0;
  }
  default: {
    if (isalpha(code[i])) {
      int s = i;
      while (isalnum(code[i]) || code[i] == '_')
        ++i;

      int slen = i - s;
      {
        // Keywords
        if (slen == 2 && !strncmp(code + s, "if", slen)) {
          if (!tokens_ahead) {
            output->type = MC_TOKEN_IF_KEYWORD;
            allocate_and_copy_cstrn(output->text, code + s, slen);
            output->start_index = s;
            return 0;
          }
          else {
            MCcall(peek_mc_token(code, i, tokens_ahead - 1, output));
            return 0;
          }
        }
        if (slen == 4 && !strncmp(code + s, "else", slen)) {
          if (!tokens_ahead) {
            output->type = MC_TOKEN_ELSE_KEYWORD;
            allocate_and_copy_cstrn(output->text, code + s, slen);
            output->start_index = s;
            return 0;
          }
          else {
            MCcall(peek_mc_token(code, i, tokens_ahead - 1, output));
            return 0;
          }
        }
        if (slen == 5 && !strncmp(code + s, "while", slen)) {
          if (!tokens_ahead) {
            output->type = MC_TOKEN_WHILE_KEYWORD;
            allocate_and_copy_cstrn(output->text, code + s, slen);
            output->start_index = s;
            return 0;
          }
          else {
            MCcall(peek_mc_token(code, i, tokens_ahead - 1, output));
            return 0;
          }
        }
        if (slen == 6 && !strncmp(code + s, "switch", slen)) {
          if (!tokens_ahead) {
            output->type = MC_TOKEN_SWITCH_KEYWORD;
            allocate_and_copy_cstrn(output->text, code + s, slen);
            output->start_index = s;
            return 0;
          }
          else {
            MCcall(peek_mc_token(code, i, tokens_ahead - 1, output));
            return 0;
          }
        }
        if (slen == 6 && !strncmp(code + s, "return", slen)) {
          if (!tokens_ahead) {
            output->type = MC_TOKEN_RETURN_KEYWORD;
            allocate_and_copy_cstrn(output->text, code + s, slen);
            output->start_index = s;
            return 0;
          }
          else {
            MCcall(peek_mc_token(code, i, tokens_ahead - 1, output));
            return 0;
          }
        }
        if (slen == 5 && !strncmp(code + s, "const", slen)) {
          if (!tokens_ahead) {
            output->type = MC_TOKEN_CONST_KEYWORD;
            allocate_and_copy_cstrn(output->text, code + s, slen);
            output->start_index = s;
            return 0;
          }
          else {
            MCcall(peek_mc_token(code, i, tokens_ahead - 1, output));
            return 0;
          }
        }
      }

      if (!tokens_ahead) {
        output->type = MC_TOKEN_IDENTIFIER;
        allocate_and_copy_cstrn(output->text, code + s, slen);
        output->start_index = s;
        return 0;
      }

      MCcall(peek_mc_token(code, i, tokens_ahead - 1, output));
      return 0;
    }

    // Unhandled
    MCcall(print_parse_error(code, i, "peek_mc_token", "default"));
    MCerror(2837, "TODO:'%c'", code[i]);
  }
  }

  MCerror(2996, "Flow error -- all cases end in return");
}

int parse_expression_lacking_midge_function_call(function_info *owner, char *code, int *i, char **expression)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  *expression = NULL;

  for (int j = *i;; ++j) {
    switch (code[j]) {
    case '\0': {
      MCerror(2043, "TODO:was:'%c'", code[j]);
    } break;
    case ')':
    case ']':
    case ',':
    case ';': {
      // Expression is free of function call - transcribe directly
      *expression = (char *)malloc(sizeof(char) * (j - *i + 1));
      strncpy(*expression, code + *i, j - *i);
      (*expression)[j - *i] = '\0';
      *i = j;
      return 0;
    }
    case '[': {
      ++j;
      char *innerExpression;
      MCcall(parse_expression_lacking_midge_function_call(owner, code, &j, &innerExpression));
      if (!innerExpression) {
        free(innerExpression);
        return 0;
      }
      free(innerExpression);
      MCcall(parse_past(code, &j, "]"));
      --j;
      // Just continue
    } break;
    case '(': {
      int p = j - 1;
      while (p >= *i && code[p] == ' ') {
        --p;
      }
      if (p <= *i || (!isalnum(code[p]) && code[p] != '_')) {
        // Treat it as a cast
        MCcall(parse_past(code, &j, "("));
        char *innerExpression;
        MCcall(parse_expression_lacking_midge_function_call(owner, code, &j, &innerExpression));
        if (!innerExpression) {
          free(innerExpression);
          return 0;
        }
        free(innerExpression);
        MCcall(parse_past(code, &j, ")"));
        --j;

        // free(declared_type);
        break;
      }

      // Function call
      // -- but the get the name
      while (isalnum(code[p]) || code[p] == '_') {
        --p;
      }
      ++p;
      char *function_name = (char *)malloc(sizeof(char) * (j - p + 1));
      strncpy(function_name, code + p, j - p);
      function_name[j - p] = '\0';

      function_info *func_info;
      {
        void *mc_vargs[3];
        mc_vargs[0] = (void *)&func_info;
        mc_vargs[1] = (void *)&command_hub->nodespace;
        mc_vargs[2] = (void *)&function_name;
        find_function_info(3, mc_vargs);
      }
      free(function_name);

      if (func_info) {
        return 0;
      }

      // Non-midge function -- continue
      MCcall(parse_past(code, &j, "("));
      MCcall(parse_past_empty_text(code, &j));
      while (1) {
        if (code[j] == '\0') {
          MCerror(2247, "EOF");
        }

        char *argExpression;
        MCcall(parse_expression_lacking_midge_function_call(owner, code, &j, &argExpression));
        // printf("argExpression:'%s'\n", argExpression);
        if (!argExpression) {
          free(argExpression);
          return 0;
        }
        free(argExpression);
        MCcall(parse_past_empty_text(code, &j));
        if (code[j] == ',') {
          ++j;
          MCcall(parse_past_empty_text(code, &j));
          continue;
        }
        break;
      }
      MCcall(parse_past(code, &j, ")"));
      --j;
      break;
    }
    // case '\'': {
    //   ++j;
    //   if (code[j] == '\\') {
    //     ++j;
    //   }
    //   ++j;
    //   MCcall(parse_past(code, &j, "'"));
    // } break;
    default:
      break;
    }
  }

  return 0;
}

int transcribe_expression(function_info *owner, char *code, int *i, uint *transcription_alloc, char **transcription);
int transcribe_bracketed_expression(function_info *owner, char *code, int *i, uint *transcription_alloc,
                                    char **transcription)
{
  char *expression;
  MCcall(parse_expression_lacking_midge_function_call(owner, code, i, &expression));
  if (expression) {
    append_to_cstr(transcription_alloc, transcription, expression);
    return 0;
  }

  MCcall(parse_past(code, i, "("));
  MCcall(parse_past_empty_text(code, i));

  // Determine the type of statement
  mc_token token0;
  MCcall(peek_mc_token(code, *i, 0, &token0));
  switch (token0.type) {
  case MC_TOKEN_IDENTIFIER: {
    mc_token token1;
    MCcall(peek_mc_token(code, *i, 1, &token1));
    switch (token1.type) {
    case MC_TOKEN_STAR_CHARACTER: {
      mc_token token2;
      MCcall(peek_mc_token(code, *i, 1, &token2));
      switch (token2.type) {
      case MC_TOKEN_STAR_CHARACTER: {
        MCerror(1884, "type cast TODO");
      } break;
      default: {
        MCcall(print_parse_error(code, *i, "transcribe_bracketed_expression", ""));
        MCerror(1881, "MC_TOKEN_IDENTIFIER:STAR:%s:%s", get_mc_token_type_name(token2.type), token2.text);
      }
      }
      free(token2.text);
    } break;
    default: {
      MCcall(print_parse_error(code, *i, "transcribe_bracketed_expression", ""));
      MCerror(1881, "MC_TOKEN_IDENTIFIER:%s:%s", get_mc_token_type_name(token1.type), token1.text);
    }
    }
    free(token1.text);
  } break;
  case MC_TOKEN_CONST_KEYWORD: {
    // Type cast
    char *type_declaration;
    MCcall(parse_past_conformed_type_declaration(owner, code, i, &type_declaration));
    MCcall(parse_past_empty_text(code, i));
    MCcall(parse_past(code, i, ")"));

    mc_token token1;
    MCcall(peek_mc_token(code, *i, 1, &token1));
    switch (token1.type) {
    case MC_TOKEN_IDENTIFIER: {
      mc_token token2;
      MCcall(peek_mc_token(code, *i, 2, &token2));
      switch (token2.type) {
      // case MC_TOKEN_SQUARE_OPENING_BRACKET: {
      //   char *identifier;
      //   MCcall(parse_past_identifier(code, i, &identifier, true, true));
      //   MCcall(parse_past_empty_text(code, i));

      //   // Transcribe it all
      //   MCcall(append_to_cstr(transcription_alloc, transcription, "("));
      //   MCcall(append_to_cstr(transcription_alloc, transcription, type_declaration));
      //   MCcall(append_to_cstr(transcription_alloc, transcription, ")"));
      //   MCcall(append_to_cstr(transcription_alloc, transcription, identifier));
      //   MCcall(append_to_cstr(transcription_alloc, transcription, " "));

      //   free(identifier);
      // } break;
      default: {
        MCcall(print_parse_error(code, *i, "transcribe_bracketed_expression", ":CONST:KEYWORD_OR_NAME"));
        MCerror(1748, "Unsupported-token:%i:%s", token2.type, token2.text);
      }
      }

      free(token2.text);
    } break;
    default: {
      MCcall(print_parse_error(code, *i, "transcribe_bracketed_expression", ""));
      MCerror(1926, "MC_TOKEN_CONST_KEYWORD:KEYWORD_OR_NAME:%s:%s", get_mc_token_type_name(token1.type), token1.text);
    }
    }

    free(type_declaration);
    free(token1.text);
  } break;
  case MC_TOKEN_SUBTRACT_OPERATOR: {
    // Expression
    MCcall(append_to_cstr(transcription_alloc, transcription, "("));
    MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
    MCcall(parse_past_empty_text(code, i));
    MCcall(transcribe_past(code, i, transcription_alloc, transcription, ")"));
  } break;
  default: {
    MCcall(print_parse_error(code, *i, "transcribe_bracketed_expression", ""));
    MCerror(1942, "MC_TOKEN_SUBTRACT_OPERATOR:%s:%s", get_mc_token_type_name(token0.type), token0.text);
  }
  }

  free(token0.text);

  return 0;
}

int transcribe_expression(function_info *owner, char *code, int *i, uint *transcription_alloc, char **transcription)
{
  int s = *i;

  switch (code[*i]) {
  case '"': {
    char *literal_string;
    MCcall(mc_parse_past_literal_string(code, i, &literal_string));
    MCcall(append_to_cstr(transcription_alloc, transcription, literal_string));
    free(literal_string);

    // TODO "unconcatenated strings" "like this"
  } break;
  case '\'': {
    char *literal_char;
    MCcall(parse_past_character_literal(code, i, &literal_char));
    MCcall(append_to_cstr(transcription_alloc, transcription, literal_char));
    free(literal_char);
  } break;
  case '*': {
    MCcall(transcribe_past(code, i, transcription_alloc, transcription, "*"));
    MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
  } break;
  case '&': {
    MCcall(transcribe_past(code, i, transcription_alloc, transcription, "&"));
    MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
  } break;
  case '(': {
    MCcall(transcribe_bracketed_expression(owner, code, i, transcription_alloc, transcription));
  } break;
  default: {
    if (isalpha(code[*i]) || code[*i] == '_') {
      int s = *i;
      while (1) {
        // printf("code[*i]=='%c'\n", code[*i]);
        switch (code[*i]) {
        case '[': {
          MCcall(append_to_cstrn(transcription_alloc, transcription, code + s, *i - s));
        } break;
        case '-': {
          // printf("housed '-'\n");
          if (code[*i + 1] == '>') {
            *i += 2;
            continue;
          }
          MCerror(1556, "TODO");
        } break;
        case '.': {
          ++*i;
          continue;
        }
        case ']':
          // printf("']': after:'%c%c'\n", (code + *i + 1)[0], (code + *i + 1)[1]);
          // printf("']': compare %i<>%i,%i,%i\n", (code + *i + 1)[0], '-', '-', '-');
          // if (code[*i + 1] == '.' || !strncmp(code + *i + 1, "->", 2)) {
          //   ++*i;
          //   continue;
          // }
          MCcall(append_to_cstrn(transcription_alloc, transcription, code + s, *i - s));
          break;
        case ' ':
        case ',':
        case ';':
        case ')':
          MCcall(append_to_cstrn(transcription_alloc, transcription, code + s, *i - s));
          break;
        case '(': {
          // Uh oh
          // check it isn't sizeof
          if (*i - s == 6 && !strncmp(code + s, "sizeof", 6)) {
            MCcall(append_to_cstr(transcription_alloc, transcription, "sizeof"));
            MCcall(transcribe_past(code, i, transcription_alloc, transcription, "("));

            // Type cast
            char *type_declaration;
            MCcall(parse_past_conformed_type_declaration(owner, code, i, &type_declaration));
            MCcall(parse_past_empty_text(code, i));
            MCcall(append_to_cstr(transcription_alloc, transcription, type_declaration));
            MCcall(transcribe_past(code, i, transcription_alloc, transcription, ")"));
            break;
          }
          print_parse_error(code, s, "transcribe_expression", "");
          MCerror(1836, "TODO");
        } break;
        default:
          if (isalnum(code[*i]) || code[*i] == '_') {
            ++*i;
            continue;
          }
          MCcall(print_parse_error(code, *i, "transcribe_expression", "initial-identifier"));
          MCerror(1829, "unhandled flow");
        }
        break;
      }
      break;
    }
    else if (isdigit(code[*i])) {
      char *number;
      MCcall(parse_past_number(code, i, &number));
      MCcall(append_to_cstr(transcription_alloc, transcription, number));
      free(number);
      break;
    }

    MCcall(print_parse_error(code, *i, "transcribe_expression", "initial"));
    MCerror(1562, "unhandled flow");
  }
  }

  // After Initial
  while (1) {
    switch (code[*i]) {
    case ' ': {
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, " "));
      continue;
    }
    case '=': {
      if (code[*i + 1] != '=') {
        MCerror(1736, "TODO");
      }

      // Equality Expression
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, "=="));
      MCcall(parse_past_empty_text(code, i));
      MCcall(append_to_cstr(transcription_alloc, transcription, " "));

      MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
      continue;
    }
    case '.': {
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, "."));
      MCcall(parse_past_empty_text(code, i));
      MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
      continue;
    }
    case '-': {
      if (code[*i + 1] == '>') {
        MCcall(transcribe_past(code, i, transcription_alloc, transcription, "->"));
        MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
        continue;
      }
      else {
        // Do nothing, skip to next case-statement block
      }
    }
    case '+':
    case '/':
    case '*':
    case '%': {
      char buf[4];
      sprintf(buf, "%c", code[*i]);
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, buf));
      MCcall(parse_past_empty_text(code, i));
      MCcall(append_to_cstr(transcription_alloc, transcription, " "));

      MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
      continue;
    }
    case '<':
    case '>': {
      char buf[4];
      sprintf(buf, "%c", code[*i]);
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, buf));
      if (code[*i] == '=') {
        MCcall(transcribe_past(code, i, transcription_alloc, transcription, "="));
      }
      MCcall(parse_past_empty_text(code, i));
      MCcall(append_to_cstr(transcription_alloc, transcription, " "));

      MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
      continue;
    }
    case ',':
    case ')':
    case ']':
    case ';':
      return 0;
    case '[': {
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, "["));
      MCcall(transcribe_expression(owner, code, i, transcription_alloc, transcription));
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, "]"));
      continue;
    }
    default:
      MCcall(print_parse_error(code, *i, "transcribe_expression", "after-initial"));
      MCerror(1909, "unhandled flow");
    }
  }

  return 0;
}

int transcribe_error_statement(char *code, int *i, uint *transcription_alloc, char **transcription)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  MCcall(parse_past(code, i, "ERR("));

  // Obtain the error code
  int s = *i;
  for (;; ++*i) {
    if (!isalnum(code[*i]) && code[*i] != '_') {
      break;
    }
  }

  const char *MC_ERROR_PREPEND = "MCERROR_";
  int error_def_len = strlen(MC_ERROR_PREPEND) + *i - s;
  char *error_name;
  allocate_and_copy_cstrn(error_name, code + s, *i - s);
  char *error_def = (char *)malloc(sizeof(char) * (error_def_len + 1));
  strcpy(error_def, MC_ERROR_PREPEND);
  strncat(error_def, code + s, *i - s);
  error_def[error_def_len] = '\0';

  // Ensure the error type has a definition
  {
    bool defined;
    char buf[1024];
    sprintf(buf,
            "{\n"
            "  #ifdef %s\n"
            "    *((bool *)%p) = true;\n"
            "  #else\n"
            "    *((bool *)%p) = false;\n"
            "  #endif\n"
            "}",
            error_def, &defined, &defined);
    clint_process(buf);

    if (!defined) {
      sprintf(buf, "#define %s %u", error_def, command_hub->error_definition_index++);
      clint_process(buf);
    }
  }

  MCcall(parse_past_empty_text(code, i));

  // Transcribe
  MCcall(append_to_cstr(transcription_alloc, transcription, "  printf(\"\\n----------------\\n  ERROR-:"));
  MCcall(append_to_cstr(transcription_alloc, transcription, error_name));
  MCcall(append_to_cstr(transcription_alloc, transcription, ":>\");\n"));

  if (code[*i] == ',') {
    MCcall(append_to_cstr(transcription_alloc, transcription, "  dprintf("));

    // format & vargs
    MCcall(parse_past_empty_text(code, i));
    ++*i;
    s = *i;
    for (;; ++*i) {
      if (code[*i] == ')') {
        break;
      }
    }
    MCcall(append_to_cstrn(transcription_alloc, transcription, code + s, *i - s));

    MCcall(append_to_cstr(transcription_alloc, transcription, "\"\\n\");\n"));
  }

  // TODO -- some kinda call stack?

  // The end of the statement & transcribe return
  MCcall(parse_past(code, i, ");"));
  MCcall(append_to_cstr(transcription_alloc, transcription, "  return "));
  MCcall(append_to_cstr(transcription_alloc, transcription, error_def));
  MCcall(append_to_cstr(transcription_alloc, transcription, ";\n"));

  return 0;
}

int transcribe_for_statement(function_info *owner, char *code, int *i, uint *transcription_alloc, char **transcription)
{
  int s = *i;

  for (;; ++*i) {
    if (code[*i] == '{') {
      ++*i;
      break;
    }
  }
  MCcall(append_to_cstrn(transcription_alloc, transcription, code + s, *i - s));
  MCcall(append_to_cstr(transcription_alloc, transcription, "\n"));

  MCcall(transcribe_c_block_to_mc(owner, code, i, transcription_alloc, transcription));
  // printf("returned from cblock: tforstatement\n");

  // print_parse_error(code, *i, "trans-forstatem", "before-final_curly");
  MCcall(parse_past(code, i, "}"));

  MCcall(append_to_cstr(transcription_alloc, transcription, "}\n"));

  // MCcall(append_to_cstr(transcription_alloc, transcription, code+ s, n));

  // MCcall(parse_past(code, i, "for"));
  // MCcall(parse_past_empty_text(code, i));
  // MCcall(parse_past(code, i, "("));
  // MCcall(parse_past_empty_text(code, i));

  // char *iterator_type = NULL;
  // char *iterator_identity = NULL;
  // if (code[*i] == ';') {
  //   MCerror(1536, "TODO - no init");
  // }
  // else {
  //   MCcall(parse_past_conformed_type_declaration(NULL, code, i, &iterator_type));
  //   MCcall(parse_past_empty_text(code, i));
  //   if (code[*i] == '=') {
  //     MCerror(1544, "TODO - assign case");
  //   }

  // }
  // MCcall(parse_past_mc_identifier(code, i, &identifier, true, false));

  return 0;
}

int transcribe_comment(function_info *owner, char *code, int *i, uint *transcription_alloc, char **transcription)
{
  MCcall(transcribe_past(code, i, transcription_alloc, transcription, "/"));
  int s = *i;
  if (code[*i] == '/') {
    // Line Comment
    for (;; ++*i) {
      if (code[*i] == '\n' || code[*i] == '\0') {
        break;
      }
    }
  }
  else if (code[*i] == '*') {
    // Multi-Line Comment
    for (;; ++*i) {
      if (code[*i] == '\0')
        break;
      if ((code[*i] == '*' && code[*i + 1] == '/')) {
        *i += 2;
        break;
      }
    }
  }
  else {
    MCerror(2129, "Not Supported");
  }

  MCcall(append_to_cstrn(transcription_alloc, transcription, code + s, *i - s));
  MCcall(append_to_cstr(transcription_alloc, transcription, "\n"));
  return 0;
}

int transcribe_if_statement(function_info *owner, char *code, int *i, uint *transcription_alloc, char **transcription)
{
  // Header
  MCcall(parse_past(code, i, "if"));
  MCcall(parse_past_empty_text(code, i));
  MCcall(parse_past(code, i, "("));
  MCcall(parse_past_empty_text(code, i));

  char *expression;
  MCcall(parse_expression_lacking_midge_function_call(owner, code, i, &expression));
  if (!expression) {

    // TODO TODO TODO
    // ASSUMING THAT none of the code will contain a midge function in them, so transcribing directly
    int bracketCount = 1;
    for (int j = *i; bracketCount; ++j) {
      // printf("bracketCount:%i\n", bracketCount);
      switch (code[j]) {
      case '\0':
      case '{': {
        // printf("bracketCount-f:%i\n", bracketCount);
        MCcall(print_parse_error(code, j, "--transcribe_if_statement", "direct-transcribe"));
        MCerror(2127, "FORMAT PROBLEM");
      } break;
      case ')': {
        --bracketCount;
        if (!bracketCount) {
          expression = (char *)malloc(sizeof(char) * (j - *i + 1));
          strncpy(expression, code + *i, j - *i);
          expression[j - *i] = '\0';
          *i = j;
        }
      } break;
      case '(': {
        ++bracketCount;
      } break;
      default:
        break;
      }
    }

    // printf("if_statement:Direct_parse_expression:'%s'\n", expression);
  }

  MCcall(parse_past_empty_text(code, i));
  MCcall(parse_past(code, i, ")"));
  MCcall(parse_past_empty_text(code, i));

  MCcall(append_to_cstr(transcription_alloc, transcription, "if ("));
  MCcall(append_to_cstr(transcription_alloc, transcription, expression));
  MCcall(append_to_cstr(transcription_alloc, transcription, ") {\n"));

  if (code[*i] != '{') {
    MCcall(print_parse_error(code, *i, "transcribe_if_statement", "expecting curly"));
    MCerror(1948, "NOT supporting unbracketed if statements yet");
  }

  MCcall(parse_past(code, i, "{"));
  MCcall(parse_past_empty_text(code, i));

  MCcall(transcribe_c_block_to_mc(owner, code, i, transcription_alloc, transcription));
  // printf("returned from cblock: ifstate\n");
  MCcall(parse_past(code, i, "}"));
  MCcall(append_to_cstr(transcription_alloc, transcription, "}\n"));

  while (1) {

    int p = *i;
    MCcall(parse_past_empty_text(code, &p));
    if (code[p] == 'e' && code[p + 1] == 'l' && code[p + 2] == 's' && code[p + 3] == 'e') {
      MCcall(parse_past_empty_text(code, i));
      MCcall(parse_past(code, i, "else"));
      MCcall(parse_past_empty_text(code, i));

      if (code[*i] == 'i' && code[*i + 1] == 'f') {
        MCcall(append_to_cstr(transcription_alloc, transcription, "else "));
        MCcall(transcribe_if_statement(owner, code, i, transcription_alloc, transcription));
      }
      else {
        if (code[*i] != '{') {
          MCerror(1948, "NOT supporting unbracketed else statements yet");
        }
        MCcall(parse_past(code, i, "{"));
        MCcall(append_to_cstr(transcription_alloc, transcription, "else {\n"));

        MCcall(transcribe_c_block_to_mc(owner, code, i, transcription_alloc, transcription));
        // printf("returned from cblock: ifstatement\n");

        MCcall(parse_past(code, i, "}"));
        MCcall(append_to_cstr(transcription_alloc, transcription, "}\n"));
      }
    }
    else {
      break;
    }
  }
  return 0;
}

int transcribe_while_statement(function_info *owner, char *code, int *i, uint *transcription_alloc,
                               char **transcription)
{
  // Header
  MCcall(parse_past(code, i, "while"));
  MCcall(parse_past_empty_text(code, i));
  MCcall(parse_past(code, i, "("));
  MCcall(parse_past_empty_text(code, i));

  char *expression;
  MCcall(parse_expression_lacking_midge_function_call(owner, code, i, &expression));
  if (!expression) {

    // TODO TODO TODO
    // ASSUMING THAT none of the code will contain a midge function in them, so transcribing directly
    int bracketCount = 1;
    for (int j = *i; bracketCount; ++j) {
      // printf("bracketCount:%i\n", bracketCount);
      switch (code[j]) {
      case '\0':
      case '{': {
        // printf("bracketCount-f:%i\n", bracketCount);
        MCcall(print_parse_error(code, j, "--transcribe_while_statement", "direct-transcribe"));
        MCerror(2127, "FORMAT PROBLEM");
      } break;
      case ')': {
        --bracketCount;
        if (!bracketCount) {
          expression = (char *)malloc(sizeof(char) * (j - *i + 1));
          strncpy(expression, code + *i, j - *i);
          expression[j - *i] = '\0';
          *i = j;
        }
      } break;
      case '(': {
        ++bracketCount;
      } break;
      default:
        break;
      }
    }
  }

  MCcall(parse_past_empty_text(code, i));
  MCcall(parse_past(code, i, ")"));
  MCcall(parse_past_empty_text(code, i));

  MCcall(append_to_cstr(transcription_alloc, transcription, "while ("));
  MCcall(append_to_cstr(transcription_alloc, transcription, expression));
  MCcall(append_to_cstr(transcription_alloc, transcription, ") {\n"));

  if (code[*i] != '{') {
    MCcall(print_parse_error(code, *i, "transcribe_while_statement", "expecting curly"));
    MCerror(1948, "NOT supporting unbracketed while statements yet");
  }

  MCcall(parse_past(code, i, "{"));
  MCcall(parse_past_empty_text(code, i));

  MCcall(transcribe_c_block_to_mc(owner, code, i, transcription_alloc, transcription));
  // printf("returned from cblock: while\n");
  MCcall(parse_past(code, i, "}"));
  MCcall(append_to_cstr(transcription_alloc, transcription, "}\n"));

  return 0;
}

int transcribe_switch_statement(function_info *owner, char *code, int *i, uint *transcription_alloc,
                                char **transcription)
{
  // Header
  MCcall(parse_past(code, i, "switch"));
  MCcall(parse_past_empty_text(code, i));
  MCcall(parse_past(code, i, "("));
  MCcall(parse_past_empty_text(code, i));

  char *expression;
  MCcall(parse_expression_lacking_midge_function_call(owner, code, i, &expression));
  if (!expression) {

    // TODO TODO TODO
    // ASSUMING THAT none of the code will contain a midge function in them, so transcribing directly
    int bracketCount = 1;
    for (int j = *i; bracketCount; ++j) {
      // printf("bracketCount:%i\n", bracketCount);
      switch (code[j]) {
      case '\0':
      case '{': {
        // printf("bracketCount-f:%i\n", bracketCount);
        MCcall(print_parse_error(code, j, "--transcribe_if_statement", "direct-transcribe"));
        MCerror(2127, "FORMAT PROBLEM");
      } break;
      case ')': {
        --bracketCount;
        if (!bracketCount) {
          expression = (char *)malloc(sizeof(char) * (j - *i + 1));
          strncpy(expression, code + *i, j - *i);
          expression[j - *i] = '\0';
          *i = j;
        }
      } break;
      case '(': {
        ++bracketCount;
      } break;
      default:
        break;
      }
    }

    printf("switch_statement:Direct_parse_expression:'%s'\n", expression);
    MCcall(parse_past_empty_text(code, i));
    MCcall(parse_past(code, i, ")"));
    MCcall(parse_past_empty_text(code, i));
  }
  else {
    printf("swtch:expression lacking function call:'%s'\n", expression);

    MCcall(parse_past_empty_text(code, i));
    MCcall(parse_past(code, i, ")"));
    MCcall(parse_past_empty_text(code, i));
  }

  MCcall(append_to_cstr(transcription_alloc, transcription, "switch ("));
  MCcall(append_to_cstr(transcription_alloc, transcription, expression));
  MCcall(append_to_cstr(transcription_alloc, transcription, ") {\n"));

  if (code[*i] != '{') {
    MCerror(1948, "NOT supporting unbracketed if statements yet");
  }

  MCcall(parse_past(code, i, "{"));
  MCcall(parse_past_empty_text(code, i));

  while (code[*i] != '}') {
    // Hunt for 'case' or 'default:'
    if (code[*i] == 'c') {
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, "case "));
      MCcall(parse_past_empty_text(code, i));
      int s = *i;
      for (;; ++*i) {
        if (code[*i] == '\0') {
          MCerror(2294, "FORMAT error");
        }
        if (!isalnum(code[*i]) && code[*i] != '_') {
          break;
        }
      }
      MCcall(append_to_cstrn(transcription_alloc, transcription, code + s, *i - s));
      MCcall(parse_past_empty_text(code, i));
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, ":"));
      MCcall(parse_past_empty_text(code, i));

      if (!strncmp(code + (*i), "case ", 5)) {
        continue;
      }
    }
    else if (code[*i] == '/') {
      MCcall(transcribe_comment(owner, code, i, transcription_alloc, transcription));
      MCcall(parse_past_empty_text(code, i));
      continue;
    }
    else {
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, "default:"));
      MCcall(parse_past_empty_text(code, i));
    }

    // For now... expect a bracket after
    MCcall(transcribe_past(code, i, transcription_alloc, transcription, "{"));
    MCcall(append_to_cstr(transcription_alloc, transcription, "\n"));
    MCcall(parse_past_empty_text(code, i));

    MCcall(transcribe_c_block_to_mc(owner, code, i, transcription_alloc, transcription));

    MCcall(transcribe_past(code, i, transcription_alloc, transcription, "}"));
    MCcall(append_to_cstr(transcription_alloc, transcription, "\n"));
    MCcall(parse_past_empty_text(code, i));

    // break / continue / default or case
    if (code[*i] == 'b') {
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, "break"));
      MCcall(parse_past_empty_text(code, i));
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, ";"));
      MCcall(append_to_cstr(transcription_alloc, transcription, "\n"));
      MCcall(parse_past_empty_text(code, i));
    }
    else if (code[*i] == 'd' || (code[*i] == 'c' && !strncmp(code + (*i), "case ", 5))) {
      continue;
    }
    else if (code[*i] == 'c') {
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, "continue"));
      MCcall(parse_past_empty_text(code, i));
      MCcall(transcribe_past(code, i, transcription_alloc, transcription, ";"));
      MCcall(append_to_cstr(transcription_alloc, transcription, "\n"));
      MCcall(parse_past_empty_text(code, i));
    }
    else if (code[*i] == '/') {
      continue;
    }
    else if (code[*i] == '}') {
      break;
    }
    else {
      MCerror(2346, "FORMAT-error");
    }
  }

  MCcall(transcribe_past(code, i, transcription_alloc, transcription, "}"));
  MCcall(append_to_cstr(transcription_alloc, transcription, "\n"));
  MCcall(parse_past_empty_text(code, i));

  return 0;
}

int render_global_node_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // For the global node (and whole screen)
  image_render_details *sequence;
  MCcall(obtain_image_render_request(command_hub->renderer.image_render_queue, &sequence));
  sequence->image_width = APPLICATION_SET_WIDTH;
  sequence->image_height = APPLICATION_SET_HEIGHT;
  sequence->clear_color = COLOR_DARK_SLATE_GRAY;
  sequence->render_target = NODE_RENDER_TARGET_PRESENT;

  element_render_command *element_cmd;

  for (int i = 0; i < command_hub->global_node->child_count; ++i) {
    mc_node_v1 *child = (mc_node_v1 *)command_hub->global_node->children[i];
    if (child->type == NODE_TYPE_VISUAL && child->data.visual.visible && child->data.visual.image_resource_uid) {
      MCcall(obtain_element_render_command(sequence, &element_cmd));
      element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
      element_cmd->x = child->data.visual.bounds.x;
      element_cmd->y = child->data.visual.bounds.y;
      element_cmd->textured_rect_info.width = child->data.visual.bounds.width;
      element_cmd->textured_rect_info.height = child->data.visual.bounds.height;
      element_cmd->textured_rect_info.texture_uid = child->data.visual.image_resource_uid;
    }
  }

  // MCcall(obtain_element_render_command(sequence, &element_cmd));
  // element_cmd->type = RENDER_COMMAND_TEXTURED_RECTANGLE;
  // element_cmd->x = command_hub->interactive_console->bounds.x;
  // element_cmd->y = command_hub->interactive_console->bounds.y;
  // element_cmd->textured_rect_info.width = command_hub->interactive_console->bounds.width;
  // element_cmd->textured_rect_info.height = command_hub->interactive_console->bounds.height;
  // element_cmd->textured_rect_info.texture_uid = command_hub->interactive_console->visual.image_resource_uid;

  return 0;
}

int code_editor_update_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
                                  /*mcfuncreplace*/

  printf("code_editor_update_v1-a\n");
  frame_time *elapsed = *(frame_time **)argv[0];
  mc_node_v1 *fedit = (mc_node_v1 *)argv[1];

  mc_code_editor_state_v1 *state = (mc_code_editor_state_v1 *)fedit->extra;

  // bool shouldCursorBeVisible= elapsed->app_nsec > 500000000L;
  // if(state->cursorVisible != shouldCursorBeVisible){

  // }

  return 0;
}

int read_editor_text_into_cstr(mc_code_editor_state_v1 *state, char **output)
{
  mc_str *code_from_code_editor;
  MCcall(mc_alloc_str(&code_from_code_editor));

  int s = 0;
  char *code = state->code.rtf->text;
  for (int i = 0;; ++i) {
    if (code[i] == '\0') {
      if (i - s > 0) {
        mc_append_to_strn(code_from_code_editor, code + s, i - s);
      }
      break;
    }
    else if (code[i] == '[') {
      if (code[i + 1] == '[') {
        ++i;
        continue;
      }
      if (i - s > 0) {
        mc_append_to_strn(code_from_code_editor, code + s, i - s);
      }

      // Find the close bracket
      for (;; ++i) {
        if (code[i] == '\0') {
          *output = NULL;
          printf("ERROR");
          return 88;
        }
        else if (code[i] == ']') {
          s = i + 1;
          break;
        }
      }
    }
  }

  // printf("read_editor_text_into_cstr:\n%s||\n", code_from_code_editor->text);
  *output = code_from_code_editor->text;

  mc_release_str(code_from_code_editor, false);

  return 0;
}

int define_struct_from_code_editor(mc_code_editor_state_v1 *state)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // Change the source data
  free(state->source_data->code);
  read_editor_text_into_cstr(state, &state->source_data->code);

  mc_struct_info_v1 *defined_struct;
  parse_struct_definition(command_hub, state->source_data, &defined_struct);

  printf("dsfce-0\n");
  if (!state->source_data || state->source_data->type != mc_source_definition_STRUCTURE) {
    // New Definition
    MCerror(4851, "TODO");
  }
  else {
    // Compare to see if they are effectively different
    mc_struct_info_v1 *editor_struct = (mc_struct_info_v1 *)state->source_data;

    bool definition_changed = false;
    if (strcmp(editor_struct->name, defined_struct->name)) {
      definition_changed = true;
      MCerror(4720, "TODO");
    }
    printf("dsfce-1\n");

    // Look for removed fields
    uint editor_struct_fields_alloc = editor_struct->field_count;
    for (int i = 0; i < editor_struct->field_count; ++i) {
      mc_parameter_info_v1 *field_info = (mc_parameter_info_v1 *)editor_struct->fields[i];

      printf("dsfce-2\n");
      mc_parameter_info_v1 *equivalent = NULL;
      for (int j = 0; j < defined_struct->field_count; ++j) {
        mc_parameter_info_v1 *defined_field_info = (mc_parameter_info_v1 *)defined_struct->fields[j];
        if (strcmp(field_info->name, defined_field_info->name) ||
            strcmp(field_info->type_name, defined_field_info->type_name) ||
            field_info->type_deref_count != defined_field_info->type_deref_count) {
          continue;
        }

        equivalent = defined_field_info;
        break;
      }
      if (equivalent) {
        continue;
      }

      // Field is missing in new definition
      definition_changed = true;

      printf("field %s (%ux*)%s has been removed in redefinition\n", field_info->type_name,
             field_info->type_deref_count, field_info->name);
      remove_from_collection((void ***)&editor_struct->fields, &editor_struct->field_count,
                             i);
    }
    printf("dsfce-3\n");

    // Look for added fields
    for (int i = 0; i < defined_struct->field_count; ++i) {
      mc_parameter_info_v1 *defined_field_info = (mc_parameter_info_v1 *)defined_struct->fields[i];

      mc_parameter_info_v1 *equivalent = NULL;
      for (int j = 0; j < editor_struct->field_count; ++j) {
        mc_parameter_info_v1 *field_info = (mc_parameter_info_v1 *)editor_struct->fields[j];
        printf("dsfce-3b\n");
        if (strcmp(field_info->name, defined_field_info->name) ||
            strcmp(field_info->type_name, defined_field_info->type_name) ||
            field_info->type_deref_count != defined_field_info->type_deref_count) {
          continue;
        }
        printf("dsfce-4\n");

        equivalent = defined_field_info;
        break;
      }
      if (equivalent) {
        continue;
      }

      // Field is added in new definition
      definition_changed = true;

      // Add it
      insert_in_collection((void ***)&editor_struct->fields, &editor_struct_fields_alloc, &editor_struct->field_count,
                           i, defined_field_info);
    }

    printf("dsfce-5\n");
    if (!definition_changed) {
      // No update needed
      printf("structure definition not changed...\n");
      return 0;
    }

    printf("dsfce-8\n");
    // Redefinition
    ++editor_struct->version;
    declare_struct_from_info(command_hub, editor_struct);
  }

  return 0;
}

/* Obtains the function info from the given definition, or updates it if @command_hub_function_info is NOT NULL */
int obtain_function_info_from_definition_v1(char *function_definition_text, function_info **registered_function_info)
{
  // Parse function definition
  mc_syntax_node *fsyntax;
  MCcall(parse_mc_to_syntax_tree(function_definition_text, &fsyntax, false));

  // MCcall(print_syntax_node(fsyntax, 0));

  release_syntax_node(fsyntax);

  return 0;
}

int parse_and_process_function_definition_v1(mc_mc_source_definition_v1 *mc_source_definition,
                                             function_info **function_definition, bool skip_clint_declaration)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  // printf("mc_source_definition->code:\n%s\n", mc_source_definition->code);

  // Parse the function return type & name
  int index = 0;

  MCcall(parse_past_empty_text(mc_source_definition->code, &index));
  if (mc_source_definition->code[index] == '*') {
    print_parse_error(mc_source_definition->code, index, "read_function_definition_from_editor",
                      "return-type that uses deref handle not implemented");
  }

  struct {
    char *name;
    uint deref_count;
  } return_type;
  MCcall(parse_past_type_declaration_text(mc_source_definition->code, &index, &return_type.name));
  MCcall(parse_past_empty_text(mc_source_definition->code, &index));
  MCcall(parse_past_dereference_sequence(mc_source_definition->code, &index, &return_type.deref_count));

  char *function_name;
  MCcall(parse_past_mc_identifier(mc_source_definition->code, &index, &function_name, false, false));
  MCcall(parse_past_empty_text(mc_source_definition->code, &index));
  MCcall(parse_past(mc_source_definition->code, &index, "("));
  MCcall(parse_past_empty_text(mc_source_definition->code, &index));

  // Obtain the information for the function
  function_info *func_info;
  {
    // Check if the function info already exists in the namespace
    void *mc_vargs[3];
    mc_vargs[0] = (void *)&func_info;
    mc_vargs[1] = (void *)&command_hub->global_node; // TODO -- from state?
    mc_vargs[2] = (void *)&function_name;
    MCcall(find_function_info(3, mc_vargs));
  }

  if (func_info) {
    // Increment function iteration
    ++func_info->latest_iteration;

    // Free previous resources
    for (int i = 0; i < func_info->struct_usage_count; ++i) {
      mc_struct_info_v1 *str = (mc_struct_info_v1 *)func_info->struct_usage[i];
      if (str->struct_id) {
      } // TODO
      // TODO free_struct(str);
    }
    // parameters

    func_info->source = mc_source_definition;
    // if (func_info->mc_code)
    //   free(func_info->mc_code); // OR can it also be script code??
    // func_info->mc_code = NULL;

    free(function_name);
  }
  else {
    // Create
    func_info = (function_info *)malloc(sizeof(function_info));
    func_info->struct_id = NULL; // TODO
    func_info->source = mc_source_definition;
    func_info->latest_iteration = 1;
    func_info->name = function_name;
    func_info->struct_usage_count = 0;
    func_info->struct_usage = NULL;

    // Attach to global -- TODO
    MCcall(append_to_collection((void ***)&command_hub->global_node->functions,
                                &command_hub->global_node->functions_alloc, &command_hub->global_node->function_count,
                                func_info));

    // Declare with cling
    // if (!skip_clint_declaration) {
    char buf[1024];
    sprintf(buf, "int (*%s)(int, void **);", func_info->name);
    clint_process(buf);
    // }
  }

  // printf("papfd-0\n");
  func_info->struct_usage_count = 0;
  func_info->parameter_count = 0;
  func_info->variable_parameter_begin_index = -1;
  func_info->return_type.name = return_type.name;
  func_info->return_type.deref_count = return_type.deref_count;
  // printf("papfd-1 func_info->return_type.name='%s', func_info->return_type.deref_count='%u'\n",
  //        func_info->return_type.name, func_info->return_type.deref_count);

  // Parse the parameters
  struct {
    char *type_name;
    uint type_deref_count;
    char *name;
  } parameters[32];
  uint parameter_count = 0;

  while (mc_source_definition->code[index] != ')') {
    if (parameter_count) {
      MCcall(parse_past(mc_source_definition->code, &index, ","));
      MCcall(parse_past_empty_text(mc_source_definition->code, &index));
    }
    MCcall(parse_past_type_declaration_text(mc_source_definition->code, &index, &parameters[parameter_count].type_name));
    MCcall(parse_past_empty_text(mc_source_definition->code, &index));
    MCcall(parse_past_dereference_sequence(mc_source_definition->code, &index,
                                           &parameters[parameter_count].type_deref_count));
    MCcall(parse_past_empty_text(mc_source_definition->code, &index));

    MCcall(parse_past_mc_identifier(mc_source_definition->code, &index, &parameters[parameter_count].name, false, false));
    MCcall(parse_past_empty_text(mc_source_definition->code, &index));
    ++parameter_count;
  }
  MCcall(parse_past(mc_source_definition->code, &index, ")"));
  MCcall(parse_past_empty_text(mc_source_definition->code, &index));

  // printf("papfd-2\n");
  func_info->parameter_count = parameter_count;
  func_info->parameters = (mc_parameter_info_v1 **)malloc(sizeof(mc_parameter_info_v1 *) * parameter_count);
  for (int p = 0; p < parameter_count; ++p) {
    mc_parameter_info_v1 *parameter = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    parameter->struct_id = NULL; // TODO
    parameter->is_function_pointer = false;
    parameter->type_version = 0U; // TODO
    parameter->type_name = parameters[p].type_name;
    parameter->type_deref_count = parameters[p].type_deref_count;
    parameter->name = parameters[p].name;

    struct_info *sinfo;
    {
      void *mc_vargs[3];
      mc_vargs[0] = &command_hub->nodespace;
      mc_vargs[1] = &parameter->type_name;
      mc_vargs[2] = &sinfo;
      find_struct_info(3, mc_vargs);
    }
    if (sinfo) {
      allocate_and_copy_cstr(parameter->mc_declared_type, sinfo->declared_mc_name);
    }
    else {
      parameter->mc_declared_type = NULL;
    }

    func_info->parameters[p] = parameter;
  }

  MCcall(parse_past(mc_source_definition->code, &index, "{"));
  // MCcall(parse_past_empty_text(mc_source_definition->code, &index));

  // printf("papfd-3\n");
  // Find the index of the last closing curly bracket
  int last_curly_index = strlen(mc_source_definition->code) - 1;
  {
    bool found_curly = false;
    while (1) {
      // printf(":%c:\n", mc_source_definition->code[last_curly_index]);
      if (mc_source_definition->code[last_curly_index] == '}') {
        --last_curly_index;
        while (mc_source_definition->code[last_curly_index] == ' ' && mc_source_definition->code[last_curly_index] == '\n' &&
               mc_source_definition->code[last_curly_index] == '\t') {
          --last_curly_index;
        }
        break;
      }

      --last_curly_index;
    }
  }

  if (last_curly_index <= index) {
    MCerror(4126, "TODO");
  }

  // printf("papfd-4\n");
  char *code_block = (char *)malloc(sizeof(char) * (last_curly_index - index + 1));
  strncpy(code_block, mc_source_definition->code + index, last_curly_index - index);
  code_block[last_curly_index - index] = '\0';
  // func_info-> = code_block;

  // printf("papfd-5\n");
  *function_definition = func_info;
  return 0;
}

// int read_and_declare_function_from_editor(mc_code_editor_state_v1 *state, function_info **defined_function_info)
// {
//   /*mcfuncreplace*/
//   mc_command_hub_v1 *command_hub;
//   /*mcfuncreplace*/

//   uint code_allocation = 64;
//   char *function_header;
//   char *code_from_code_editor = (char *)malloc(sizeof(char) * code_allocation);
//   code_from_code_editor[0] = '\0';
//   uint bracket_count = 0;
//   bool hit_code_block = false;
//   for (int i = 0; i < state->text->lines_count; ++i) {
//     int line_start_index = 0;
//     for (int j = 0;; ++j) {
//       if (state->text->lines[i][j] == '\0') {
//         if (j - line_start_index > 0) {
//           append_to_cstr(&code_allocation, &code_from_code_editor, state->text->lines[i] + line_start_index);
//         }
//         append_to_cstr(&code_allocation, &code_from_code_editor, "\n");
//         break;
//       }
//       if (state->text->lines[i][j] == '{') {
//         ++bracket_count;
//         if (!hit_code_block) {
//           hit_code_block = true;
//           if (j - line_start_index > 0) {
//             append_to_cstr(&code_allocation, &code_from_code_editor, state->text->lines[i] + line_start_index);
//           }
//           function_header = code_from_code_editor;
//           code_from_code_editor = (char *)malloc(sizeof(char) * code_allocation);
//           code_from_code_editor[0] = '\0';
//           line_start_index = j + 1;
//         }
//       }
//       else if (state->text->lines[i][j] == '}') {
//         --bracket_count;
//         if (!bracket_count && hit_code_block) {
//           // End
//           // -- Copy up to now
//           if (j - line_start_index > 0) {
//             char line_to_now[j - line_start_index + 1];
//             strncpy(line_to_now, state->text->lines[i] + line_start_index, j - line_start_index);
//             line_to_now[j] = '\0';
//             append_to_cstr(&code_allocation, &code_from_code_editor, line_to_now);
//           }

//           // -- Break from upper loop
//           i = state->text->lines_count;
//           break;
//         }
//       }
//     }
//   }

//   // Parse the function return type & name
//   char *return_type;
//   int index = 0;
//   MCcall(parse_past_conformed_type_declaration(NULL, function_header, &index, &return_type));

//   MCcall(parse_past_empty_text(function_header, &index));
//   if (function_header[index] == '*') {
//     print_parse_error(function_header, index, "read_function_definition_from_editor",
//                       "return-type that uses deref handle not implemented");
//   }

//   char *function_name;
//   MCcall(parse_past_mc_identifier(function_header, &index, &function_name, false, false));
//   MCcall(parse_past_empty_text(function_header, &index));
//   MCcall(parse_past(function_header, &index, "("));
//   MCcall(parse_past_empty_text(function_header, &index));

//   // Obtain the information for the function
//   function_info *func_info;
//   {
//     // Check if the function info already exists in the namespace
//     void *mc_vargs[3];
//     mc_vargs[0] = (void *)&func_info;
//     mc_vargs[1] = (void *)&command_hub->global_node; // TODO -- from state?
//     mc_vargs[2] = (void *)&function_name;
//     MCcall(find_function_info(3, mc_vargs));
//   }

//   if (func_info) {
//     // Free previous resources
//     for (int i = 0; i < func_info->struct_usage_count; ++i) {
//       mc_struct_info_v1 *str = (mc_struct_info_v1 *)func_info->struct_usage[i];
//       if (str->struct_id) {
//       } // TODO
//       // TODO free_struct(str);
//     }
//     // parameters

//     // if (func_info->mc_code)
//     //   free(func_info->mc_code); // OR can it also be script code??
//     // func_info->mc_code = NULL;

//     free(function_name);
//   }
//   else {
//     // Create
//     func_info = (function_info *)malloc(sizeof(function_info));
//     func_info->struct_id = NULL; // TODO
//     func_info->source_file = NULL;
//     func_info->name = function_name;
//     func_info->latest_iteration = 0;
//     func_info->struct_usage_count = 0;
//     func_info->struct_usage = NULL;

//     // Attach to global -- TODO
//     MCcall(append_to_collection((void ***)&command_hub->global_node->functions,
//                                 &command_hub->global_node->functions_alloc,
//                                 &command_hub->global_node->function_count, func_info));

//     // Declare with cling
//     char buf[1024];
//     sprintf(buf, "int (*%s)(int, void **);", func_info->name);
//     clint_process(buf);
//   }

//   printf("radffe-0\n");
//   func_info->struct_usage_count = 0;
//   func_info->parameter_count = 0;
//   func_info->variable_parameter_begin_index = -1;
//   func_info->mc_code = code_from_code_editor;

//   MCcall(convert_return_type_string(return_type, &func_info->return_type.name, &func_info->return_type.deref_count));
//   free(return_type);

//   // printf("radffe-1\n");
//   // Parse the parameters
//   struct {
//     char *type;
//     uint type_deref_count;
//     char *name;
//   } parameters[32];
//   uint parameter_count = 0;

//   while (function_header[index] != ')') {
//     if (parameter_count) {
//       MCcall(parse_past(function_header, &index, ","));
//       MCcall(parse_past_empty_text(function_header, &index));
//     }
//     MCcall(
//         parse_past_conformed_type_declaration(func_info, function_header, &index,
//         &parameters[parameter_count].type));
//     MCcall(parse_past_empty_text(function_header, &index));
//     parameters[parameter_count].type_deref_count = 0;
//     while (function_header[index] == '*') {
//       ++parameters[parameter_count].type_deref_count;
//       ++index;
//       MCcall(parse_past_empty_text(function_header, &index));
//     }

//     MCcall(parse_past_mc_identifier(function_header, &index, &parameters[parameter_count].name, false, false));
//     MCcall(parse_past_empty_text(function_header, &index));
//     ++parameter_count;
//   }

//   printf("radffe-2\n");
//   func_info->parameter_count = parameter_count;
//   func_info->parameters = (mc_parameter_info_v1 **)malloc(sizeof(mc_parameter_info_v1 *) * parameter_count);
//   for (int p = 0; p < parameter_count; ++p) {
//     mc_parameter_info_v1 *parameter = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
//     parameter->struct_id = NULL;  // TODO
//     parameter->type_version = 0U; // TODO
//     parameter->type_name = parameters[p].type;
//     parameter->type_deref_count = parameters[p].type_deref_count;
//     parameter->name = parameters[p].name;

//     func_info->parameters[p] = parameter;
//   }
//   free(function_header);

//   *defined_function_info = func_info;

//   // printf("radffe-3\n");

//   return 0;
// }