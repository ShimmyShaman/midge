#include "midge_core.h"

/*mcfuncreplace*/
#define function_info mc_function_info_v1
#define struct_info mc_struct_info_v1
#define node mc_node_v1
/*mcfuncreplace*/

int find_function_info_v1(int argc, void **argv) {
  if (argc != 3) {
    MCerror(-848, "Incorrect argument count");
  }

  function_info **func_info = (function_info **)argv[0];
  node *nodespace = *(node **)argv[1];
  char *function_name = *(char **)argv[2];

  void **mc_dvp;
  int res;

  *func_info = NULL;
  // printf("ffi-nodespace.name:%s\n", nodespace->name);
  for (int i = 0; i < nodespace->function_count; ++i) {

    // printf("ffi-3\n");
    // printf("dope\n");
    function_info *finfo = nodespace->functions[i];

    // printf("ffi-4a\n");
    // printf("ffi-cmp0:%s\n", finfo->name);
    // printf("ffi-cmp1:%s\n", function_name);
    // printf("ffi-4b\n");
    if (strcmp(finfo->name, function_name))
      continue;
    // printf("dwde\n");

    // printf("ffi-5\n");

    // Matches
    *func_info = finfo;
    // printf("find_function_info:set with '%s'\n", finfo->name);
    return 0;
  }
  // printf("dopu\n");

  // printf("ffi-2\n");

  if (nodespace->parent) {
    // Search in the parent nodespace
    void *mc_vargs[3];
    mc_vargs[0] = argv[0];
    mc_vargs[1] = (void *)&nodespace->parent;
    mc_vargs[2] = argv[2];
    MCcall(find_function_info(3, mc_vargs));
  }
  // printf("find_function_info: '%s' could not be found!\n", function_name);
  return 0;
}

int declare_function_pointer_v1(int argc, void **argv) {
  void **mc_dvp;
  int res;
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/

  // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1
  printf("declare_function_pointer_v1()\n");
  char *name = (char *)argv[0];
  char *return_type = (char *)argv[1];

  // printf("dfp-name:%s\n", name);
  // printf("dfp-rett:%s\n", return_type);

  // TODO -- check
  // printf("dfp-0\n");

  // Fill in the function_info and attach to the nodespace
  // function_info *func_info = (function_info *)malloc(sizeof(function_info));
  function_info *func_info = (function_info *)malloc(sizeof(function_info));
  func_info->name = name;
  func_info->latest_iteration = 1U;
  func_info->return_type = return_type;
  func_info->parameter_count = (argc - 2) / 2;
  func_info->parameters = (mc_parameter_info_v1 **)malloc(sizeof(void *) * func_info->parameter_count);
  func_info->variable_parameter_begin_index = -1;
  unsigned int struct_usage_alloc = 2;
  func_info->struct_usage = (mc_struct_info_v1 **)malloc(sizeof(mc_struct_info_v1 *) * struct_usage_alloc);
  func_info->struct_usage_count = 0;
  // printf("dfp-1\n");

  for (int i = 0; i < func_info->parameter_count; ++i) {
    // printf("dfp-2\n");
    mc_parameter_info_v1 *parameter_info = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    // printf("dfp>%p=%s\n", i, (void *)parameters[2 + i * 2 + 0], (char *)parameters[2 + i * 2 + 0]);
    parameter_info->type_name = (char *)argv[2 + i * 2 + 0];

    // printf("dfp-4a\n");
    // printf("chn:%p\n", command_hub->nodespace);
    // printf("ptn:%s\n", parameter_info->type_name);
    // printf("dfp-4b\n");
    void *p_struct_info = NULL;
    MCcall(find_struct_info((void *)command_hub->nodespace, parameter_info->type_name, &p_struct_info));
    if (p_struct_info) {
      struct_info *sinfo = (struct_info *)p_struct_info;
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
        MCcall(append_to_collection((void ***)&func_info->struct_usage, &struct_usage_alloc, &func_info->struct_usage_count,
                                    (void *)sinfo));
      }
      // printf("dfp-6\n");
    } else
      parameter_info->type_version = 0;

    parameter_info->name = (char *)argv[2 + i * 2 + 1];
    func_info->parameters[i] = (mc_parameter_info_v1 *)parameter_info;
    // printf("dfp>set param[%i]=%s %s\n", i, parameter_info->type, parameter_info->name);
  }
  // printf("dfp-7\n");
  MCcall(append_to_collection((void ***)&command_hub->nodespace->functions, &command_hub->nodespace->functions_alloc,
                              &command_hub->nodespace->function_count, (void *)func_info));

  // Cleanup Parameters
  if (func_info->struct_usage_count != struct_usage_alloc) {
    if (func_info->struct_usage_count > 0) {
      struct_info **new_struct_usage = (struct_info **)realloc(func_info->struct_usage, func_info->struct_usage_count);
      if (new_struct_usage == NULL) {
        printf("realloc error\n");
        return -426;
      }
      func_info->struct_usage = new_struct_usage;
    } else {
      struct_usage_alloc = 0;
      free(func_info->struct_usage);
    }
  }
  // printf("dfp-8\n");

  // Declare with clint
  char buf[1024];
  strcpy(buf, "int (*");
  strcat(buf, name);
  strcat(buf, ")(int,void**);");
  printf("dfp>cling_declare:%s\n -- with %i parameters returning %s\n", buf, func_info->parameter_count, func_info->return_type);
  clint_declare(buf);
  // printf("dfp-concludes\n");
  return 0;
}

int initialize_function_v1(int argc, void **argv) {
  void **mc_dvp;
  int res;
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub; // TODO -- replace command_hub instances in code and bring over
                                  // find_struct_info/find_function_info and do the same there.
  /*mcfuncreplace*/
  printf("initialize_function_v1()\n");

  char *function_name = (char *)argv[0];
  char *code = (char *)argv[1];

  MCerror(999, "Expected");
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
  // {
  //   find_function_info()
  // }

  // printf("nodespace->name:%s\n", command_hub->nodespace->name);
  // printf("function_info->name:%s\n", func_info->name);
  // printf("function_info->latest_iteration:%u\n", func_info->latest_iteration);
  // printf("code_block:%c%c%c%c%c...", code_block[0], code_block[1], code_block[2], code_block[3], code_block[4]);

  // Increment function iteration
  ++func_info->latest_iteration;

  // Declare with clint
  char buf[16384];
  const char *TAB = "  ";

  sprintf(buf, "int %s_v%u(int mc_argc, void **mc_argv)\n{\n", func_info->name, func_info->latest_iteration);
  // TODO -- mc_argc count check?
  strcat(buf, TAB);
  strcat(buf, "// Arguments\n");
  strcat(buf, TAB);
  strcat(buf, "int res;\n");
  strcat(buf, TAB);
  strcat(buf, "void **mc_dvp;\n");

  for (int i = 0; i < func_info->parameter_count; ++i) {
    strcat(buf, TAB);

    mc_parameter_info_v1 *parameter = (mc_parameter_info_v1 *)func_info->parameters[i];

    void *p_struct_info;
    MCcall(find_struct_info(command_hub->nodespace, parameter->type_name, &p_struct_info));
    if (p_struct_info) {
      strcat(buf, "declare_and_assign_anon_struct(");
      strcat(buf, parameter->type_name);
      strcat(buf, "_v");
      sprintf(buf + strlen(buf), "%i, ", parameter->type_version);
      strcat(buf, parameter->name);
      strcat(buf, ", ");
      strcat(buf, "mc_argv[");
      sprintf(buf + strlen(buf), "%i]);\n", i);
    } else {
      sprintf(buf + strlen(buf), "%s %s = (%s)mc_argv[%i];\n", parameter->type_name, parameter->name, parameter->type_name, i);
    }
  }
  if (func_info->parameter_count > 0)
    strcat(buf, "\n");

  // const char *identity; const char *type; struct_info *struct_info;
  struct {
    const char *var_name;
    const char *type;
    void *struct_info;
  } declared_types[200];
  int declared_type_count = 0;

  printf("@ifv-5\n");
  // Translate the code-block from script into workable midge-cling C

  return -5224;

  int n = strlen(code);
  for (int i = 0; i < n;) {
    // strcat(buf, code_block);
    switch (code[i]) {
    case ' ':
    case '\t':
      ++i;
      continue;
    default: {
      if (!isalpha(code[i])) {
        MCcall(print_parse_error(code, i, "initialize_function_v1", "default:!isalpha"));
        return -42;
      }
      return -58822;
      // mcqck_translate_script_statement(nodespace, NULL, NULL);
      break;
    }
    }
  }

  strcat(buf, "\n");
  strcat(buf, TAB);
  strcat(buf, "return res;\n");
  strcat(buf, "}");

  printf("ifv>cling_declare:\n%s\n", buf);
  // Declare the method
  clint_declare(buf);

  // Set the method to the function pointer
  sprintf(buf, "%s = &%s_v%u;", func_info->name, func_info->name, func_info->latest_iteration);
  printf("ifv>clint_process:\n%s\n", buf);
  clint_process(buf);

  printf("ifv-concludes\n");
  return 0;
}