#include "midge_core.h"

int declare_function_pointer_v1(int argc, void **argv)
{
  void **mc_dvp;
  int res;
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
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
  declare_and_allocate_anon_struct(function_info_v1, function_info, sizeof_function_info_v1);
  function_info->name = name;
  function_info->latest_iteration = 1U;
  function_info->return_type = return_type;
  function_info->parameter_count = (argc - 2) / 2;
  function_info->parameters = (mc_parameter_info_v1 **)malloc(sizeof(void *) * function_info->parameter_count);
  function_info->variable_parameter_begin_index = -1;
  unsigned int struct_usage_alloc = 2;
  function_info->struct_usage = (void **)malloc(sizeof(void *) * struct_usage_alloc);
  function_info->struct_usage_count = 0;
  // printf("dfp-1\n");

  for (int i = 0; i < function_info->parameter_count; ++i)
  {
    // printf("dfp-2\n");
    declare_and_allocate_anon_struct(parameter_info_v1, parameter_info, sizeof_parameter_info_v1);
    // printf("dfp>%p=%s\n", i, (void *)parameters[2 + i * 2 + 0], (char *)parameters[2 + i * 2 + 0]);
    parameter_info->type_name = (char *)argv[2 + i * 2 + 0];

    // printf("dfp-4a\n");
    // printf("chn:%p\n", command_hub->nodespace);
    // printf("ptn:%s\n", parameter_info->type_name);
    // printf("dfp-4b\n");
    void *p_struct_info = NULL;
    MCcall(find_struct_info((void *)command_hub->nodespace, parameter_info->type_name, &p_struct_info));
    // printf("dfp-5\n");
    declare_and_assign_anon_struct(struct_info_v1, struct_info, p_struct_info);
    // printf("dfp-3\n");
    if (struct_info)
    {
      parameter_info->type_version = struct_info->version;

      int already_added = 0;
      for (int j = 0; j < function_info->struct_usage_count; ++j)
      {
        declare_and_assign_anon_struct(struct_info_v1, existing, function_info->struct_usage[j]);

        if (!strcmp(parameter_info->type_name, existing->name))
        {
          already_added = 1;
          break;
        }
      }
      if (!already_added)
      {
        MCcall(append_to_collection(&function_info->struct_usage, &struct_usage_alloc, &function_info->struct_usage_count, (void *)struct_info));
      }
      // printf("dfp-6\n");
    }
    else
      parameter_info->type_version = 0;

    parameter_info->name = (char *)argv[2 + i * 2 + 1];
    function_info->parameters[i] = (mc_parameter_info_v1 *)parameter_info;
    // printf("dfp>set param[%i]=%s %s\n", i, parameter_info->type, parameter_info->name);
  }
  // printf("dfp-7\n");
  MCcall(append_to_collection(&command_hub->nodespace->functions, &command_hub->nodespace->functions_alloc, &command_hub->nodespace->function_count, function_info));

  // Cleanup Parameters
  if (function_info->struct_usage_count != struct_usage_alloc)
  {
    if (function_info->struct_usage_count > 0)
    {
      void **new_struct_usage = (void **)realloc(function_info->struct_usage, function_info->struct_usage_count);
      if (new_struct_usage == NULL)
      {
        printf("realloc error\n");
        return -426;
      }
      function_info->struct_usage = new_struct_usage;
    }
    else
    {
      struct_usage_alloc = 0;
      free(function_info->struct_usage);
    }
  }
  // printf("dfp-8\n");

  // Declare with clint
  char buf[1024];
  strcpy(buf, "int (*");
  strcat(buf, name);
  strcat(buf, ")(int,void**);");
  printf("dfp>cling_declare:%s\n -- with %i parameters returning %s", buf, function_info->parameter_count, function_info->return_type);
  clint_declare(buf);
  // printf("dfp-concludes\n");
  return 0;
}