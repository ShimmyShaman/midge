/* midge_core.c */

#include "midge_core.h"

int print_struct_id(int argc, void **argv)
{
  midgeo struct_id = (midgeo)argv;

  printf("[%s (version:%i)]\n", (char *)struct_id[0], *(uint *)struct_id[1]);

  return 0;
}

int allocate_struct_id_v1(int argc, void **argv)
{
  midgeo out_field = (midgeo)argv[0];
  const char *struct_name = (char *)argv[1];
  uint version = *(uint *)argv[2];

  midgeo field_data = (midgeo)malloc(sizeof(void *) * 2);
  if (!field_data)
  {
    printf("allocate_struct_id(): failed to allocate memory for field_data.\n");
    return -1;
  }

  allocate_from_cstringv(&field_data[0], struct_name);
  allocate_from_uintv(&field_data[1], version);
  *out_field = (void *)field_data;
  return 0;
}

/*int allocate_midge_field_info_v1(int argc, void **argv)
{
    midgeo out_field = (midgeo)argv[0];
    const char *type = (char *)argv[1];
    int pointer_count = *(int *)argv[2];
    const char *name = (char *)argv[3];

    midgeo field_data = (midgeo)malloc(sizeof(void *) * 3);
    allocate_from_cstringv(&field_data[0], type);
    allocate_from_intv(&field_data[1], pointer_count);
    allocate_from_cstringv(&field_data[2], name);
    // printf("pointer_count:%i, *field_data[1]=%i\n", pointer_count, *(int *)field_data[1]);
    *out_field = (void *)field_data;
    return 0;
}

int allocate_from_definition_v1(int argc, void **argv)
{
    midgeo *allocation = (midgeo *)argv[0];
    struct_definition definition = (struct_definition)argv[1];

    int res;

    // printf("allocating %i void*\n", *(int *)definition[1]);
    midgeo alloc = (midgeo)calloc(sizeof(void *), (1 + *(int *)definition[1]));
    if (!alloc)
    {
        printf("allocate_from_definition(): failed to allocate memory for alloc.\n");
        return -1;
    }

    // print_struct_definition(1, definition);
    // print_struct_id(1, (void **)definition[0]);
    // printf("d:%s c:%u\n", (char *)((void **)definition[0])[0], *(uint *)((void **)definition[0])[1]);
    void *vargs[5];
    vargs[0] = (void *)&alloc[0];
    vargs[1] = (char *)((void **)definition[0])[0];
    vargs[2] = (uint *)((void **)definition[0])[1];
    MCcall(allocate_struct_id(3, vargs));

    *allocation = alloc;
    return 0;
}

int obtain_from_index_v1(int argc, void **argv)
{
    // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1

    midgeo *out_var = (midgeo *)argv[0];
    midgeo index_object = (midgeo)argv[1];
    char *name = (char *)argv[2];

    *out_var = NULL;
    while (index_object != NULL)
    {
        // item is 1 : index 2 is left : 3 is right
        int r = strcmp(name, (char *)index_object[1]);
        if (r > 0)
        {
            index_object = (midgeo)index_object[2];
        }
        else if (r < 0)
        {
            index_object = (midgeo)index_object[3];
        }
        else
        {
            // Match!
            *out_var = index_object;
            return 0;
        }
    }

    return 0;
}

int obtain_struct_info_from_index_v1(int argc, void **argv)
{
    // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1

    midgeo *out_var = (midgeo *)argv[0];
    midgeo node = (midgeo)argv[1];
    char *name = (char *)argv[2];

    *out_var = NULL;
    while (node != NULL)
    {
        midgeo index_object = (midgeo)node[4];

        void *vargs[3];
        vargs[0] = argv[0];
        vargs[1] = (void *)index_object;
        vargs[2] = argv[2];
        obtain_from_index(3, vargs);

        if (out_var != NULL)
            return 0;

        // Set node to contextual parent
        node = (midgeo)node[2];
        continue;
    }

    return 0;
}*/

int print_parse_error(const char *const text, int index, const char *const function_name, const char *section_id)
{
  char buf[30];
  int off = 6 - index;
  for (int i = 0; i < 13; ++i)
  {
    if (index - 13 + i < 0)
      buf[i] = ' ';
    else
      buf[i] = text[index - 13 + i];
  }
  buf[13] = '|';
  buf[14] = text[index];
  buf[15] = '|';
  char eof = text[index] == '\0';
  for (int i = 0; i < 13; ++i)
  {
    if (eof)
      buf[16 + i] = ' ';
    else
    {
      eof = text[index + 1 + i] == '\0';
      buf[16 + i] = text[index + 1 + i];
    }
  }
  buf[29] = '\0';

  printf("\n%s>%s#unhandled-char:'%s'\n", function_name, section_id, buf);

  return 0;
}

int parse_past(const char *text, int *index, const char *sequence)
{
  for (int i = 0;; ++i)
  {
    if (sequence[i] == '\0')
    {
      *index += i;
      return 0;
    }
    else if (text[*index + i] == '\0')
    {
      return -1;
    }
    else if (sequence[i] != text[*index + i])
    {
      printf("!parse_past() expected:'%c' was:'%c'\n", sequence[i], text[*index + i]);
      return 1 + i;
    }
  }
}

int parse_past_number(const char *text, int *index, char **output)
{
  for (int i = *index;; ++i)
  {
    if (isdigit(text[i]))
      continue;

    if (i - *index <= 0)
      return -5252;

    *output = (char *)malloc(sizeof(char) * (i - *index + 1));
    strncpy(*output, text + *index, i - *index);
    (*output)[i - *index] = '\0';
    *index = i;
    return 0;
  }

  return 0;
}

int parse_past_character_literal(const char *text, int *index, char **output)
{
  if (text[*index] != '\'')
    return -2482;

  bool prev_escape = false;
  for (int i = *index + 1;; ++i)
  {
    if (!prev_escape && text[i] == '\'')
    {
      ++i;
      *output = (char *)malloc(sizeof(char) * (i - *index + 1));
      strncpy(*output, text + *index, i - *index);
      (*output)[i - *index] = '\0';
      *index = i;
      return 0;
    }

    prev_escape = (text[i] == '\\');
  }

  return 0;
}

int parse_past_identifier(const char *text, int *index, char **identifier, bool include_member_access, bool include_referencing)
{
  int res;
  int o = *index;
  bool hit_alpha = false;
  while (1)
  {
    int doc = 1;
    switch (text[*index])
    {
    case ' ':
    case '\n':
    case '\t':
    case '\0':
      doc = 0;
      break;
    default:
    {
      if (isalpha(text[*index]))
      {
        hit_alpha = true;
        break;
      }
      if (*index > o && isalnum(text[*index]))
        break;
      if (text[*index] == '_')
        break;
      if (include_member_access)
      {
        if (text[*index] == '-' && text[*index + 1] == '>')
        {
          ++*index;
          break;
        }
        if (text[*index] == '.')
          break;
      }
      if (include_referencing)
      {
        if (!hit_alpha)
        {
          if (text[*index] == '&')
            break;
          if (text[*index] == '*')
            break;
        }
      }

      // Identifier end found
      doc = 0;
    }
    break;
    }
    if (!doc)
    {
      if (o == *index)
      {
        MCcall(print_parse_error(text, *index, "parse_past_identifier", "identifier_end"));
        return -148;
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
  return -149;
}

int parse_past_type_identifier(const char *text, int *index, char **identifier)
{
  int res;
  if (text[*index] == '\'')
  {
    // Obtain the type in the literal string
    for (int i = *index + 1;; ++i)
    {
      if (text[i] == '\'')
      {
        *identifier = (char *)malloc(sizeof(char) * (i - *index));
        strncpy(*identifier, text + *index + 1, i - *index - 1);
        (*identifier)[i - *index - 1] = '\0';
        *index = i + 1;
        return 0;
      }
      if (text[i] == '\0')
      {
        MCerror(-25258, "SHOULDN'T");
      }
    }
  }
  else
  {
    MCcall(parse_past_identifier(text, index, identifier, false, false));
    return 0;
  }
}

int set_int_value(int argc, void **argv)
{
  printf("set_int_value()\n");
  int *var = (int *)argv[0];
  int value = *(int *)argv[1];

  *var = value;
  return 0;
}

int increment_int_value(int argc, void **argv)
{
  printf("increment_int_value()\n");
  int *var = (int *)argv[0];

  ++*var;
  return 0;
}

int set_pointer_value(int argc, void **argv)
{
  printf("set_pointer_value()\n");
  void **var = (void **)argv[0];
  void *value = (void *)argv[1];
  printf("was:%p  now:%p\n", *var, value);

  *var = value;
  return 0;
}

int increment_pointer(int argc, void **argv)
{
  printf("increment_pointer()\n");
  unsigned long **var = (unsigned long **)argv[0];

  ++*var;
  return 0;
}

int append_to_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count, void *item)
{
  if (*collection_count + 1 > *collection_alloc)
  {
    unsigned int realloc_amount = *collection_alloc + 8 + *collection_alloc / 3;
    printf("reallocate collection size %i->%i\n", *collection_alloc, realloc_amount);
    void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
    if (new_collection == NULL)
    {
      printf("append_to_collection malloc error\n");
      return -424;
    }

    memcpy(new_collection, *collection, *collection_count * sizeof(void *));
    free(*collection);

    *collection = new_collection;
    *collection_alloc = realloc_amount;
  }

  (*collection)[*collection_count] = item;
  ++*collection_count;
  return 0;
}

int remove_from_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count, int index)
{
  *collection[index] = NULL;
  for (int i = index + 1; i < *collection_count; ++i)
    *collection[i - 1] = *collection[i];

  --*collection_count;
  if (index > 0)
    *collection[*collection_count] = NULL;

  return 0;
}

int find_function_info_v1(int argc, void **argv)
{
  if (argc != 3)
  {
    MCerror(-848, "Incorrect argument count");
  }

  void **function_info = *(void ***)argv[0];
  void *vp_nodespace = *(void **)argv[1];
  char *function_name = *(char **)argv[2];

  void **mc_dvp;
  int res;
  declare_and_assign_anon_struct(node_v1, node, vp_nodespace);

  *function_info = NULL;
  for (int i = 0; i < node->function_count; ++i)
  {
    declare_and_assign_anon_struct(function_info_v1, finfo, node->functions[i]);
    if (strcmp(finfo->name, function_name))
      continue;

    // Matches
    *function_info = (void *)finfo;
    printf("find_function_info:set with '%s'\n", finfo->name);
    return 0;
  }
  printf("find_function_info: '%s' could not be found!\n", function_name);
  return 0;
}

int mcqck_find_function_info(void *vp_nodespace, char *function_name, void **function_info)
{
  void *vargs[3];
  vargs[0] = &function_info;
  vargs[1] = &vp_nodespace;
  vargs[2] = &function_name;
  int res;
  MCcall(find_function_info_v1(3, vargs));
  return 0;
}

int find_struct_info(void *vp_nodespace, const char *const struct_name, void **struct_info)
{
  void **mc_dvp;
  int res;
  declare_and_assign_anon_struct(node_v1, node, vp_nodespace);

  *struct_info = NULL;
  for (int i = 0; i < node->struct_count; ++i)
  {
    declare_and_assign_anon_struct(struct_info_v1, finfo, node->structs[i]);
    if (strcmp(finfo->name, struct_name))
      continue;

    // Matches
    *struct_info = (void *)finfo;
    printf("find_struct_info:set with '%s'\n", finfo->name);
    return 0;
  }
  printf("find_struct_info: '%s' could not be found!\n", struct_name);
  return 0;
}

// int mcqck_translate_script_statement(void *nodespace, char *script_statement, char **translated_statement)
// {
//   int res;
//   void **mc_dvp;
//   char buf[16384];
//   int i = 0;

//   // TODO -- free all created char * strings
//   switch (script_statement[i])
//   {
//   // case 'a':
//   // {
//   //   // ass
//   //   MCcall(parse_past(code, &i, "ass"));
//   //   MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

//   //   // // type
//   //   // char *type_identity;
//   //   // MCcall(parse_past_identifier(code, &i, &type_identity, false, false));
//   //   // MCcall(parse_past(code, &i, " "));

//   //   // Identifier
//   //   char *set_identity;
//   //   MCcall(parse_past_identifier(code, &i, &set_identity, true, true));
//   //   MCcall(parse_past(code, &i, " "));

//   //   // Value-identifier
//   //   char *value_identity;
//   //   MCcall(parse_past_identifier(code, &i, &value_identity, true, true));
//   //   MCcall(parse_past(code, &i, "\n"));

//   //   strcat(buf, TAB);
//   //   sprintf(buf + strlen(buf), "%s = %s;\n", set_identity, value_identity);
//   // }
//   // break;
//   // case 'c':
//   // {
//   //   // cpy
//   //   MCcall(parse_past(code, &i, "cpy"));
//   //   MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

//   //   // type
//   //   char *type_identity;
//   //   if (code[i] == '\'')
//   //   {
//   //     int o = ++i;
//   //     while (code[i] != '\'')
//   //       ++i;
//   //     type_identity = (char *)malloc(sizeof(char) * (i - o + 1));
//   //     strncpy(type_identity, code + o, i - o);
//   //     type_identity[i - o] = '\0';
//   //     printf("type_identity:%s\n", type_identity);
//   //     ++i;
//   //   }
//   //   else
//   //     MCcall(parse_past_identifier(code, &i, &type_identity, false, false));
//   //   MCcall(parse_past(code, &i, " "));

//   //   // Identifier
//   //   char *set_identity;
//   //   MCcall(parse_past_identifier(code, &i, &set_identity, true, false));
//   //   MCcall(parse_past(code, &i, " "));

//   //   // Value-identifier
//   //   char *value_identity;
//   //   MCcall(parse_past_identifier(code, &i, &value_identity, true, false));
//   //   MCcall(parse_past(code, &i, "\n"));

//   //   if (!strcmp(type_identity, "const char *"))
//   //   {
//   //     strcat(buf, TAB);
//   //     sprintf(buf + strlen(buf), "%s = (char *)malloc(sizeof(char) * (strlen(%s) + 1));\n", set_identity, value_identity);
//   //     strcat(buf, TAB);
//   //     sprintf(buf + strlen(buf), "strcpy((char *)%s, %s);\n", set_identity, value_identity);
//   //   }
//   //   else
//   //   {
//   //     printf("initialize_function_V1:>[cpy]>unhandled type identity:%s\n", type_identity);
//   //     return -727;
//   //   }
//   // }
//   // break;
//   case 'd':
//   {
//     // dec
//     MCcall(parse_past(script_statement, &i, "dcl"));
//     MCcall(parse_past(script_statement, &i, " ")); // TODO -- allow tabs too

//     // Identifier
//     char *type_identifier;
//     MCcall(parse_past_identifier(script_statement, &i, &type_identifier, false, false));
//     MCcall(parse_past(script_statement, &i, " "));

//     // Variable Name
//     char *var_name;
//     MCcall(parse_past_identifier(script_statement, &i, &var_name, false, false));
//     if (script_statement[i] != '\n' && script_statement[i] != '\0')
//     {
//       MCerror(-4829, "expected statement end");
//     }

//     // declared_types[declared_type_count].type = type_identifier;
//     // declared_types[declared_type_count].var_name = var_name;

//     // -- Determine if the structure is midge-specified
//     void *p_struct_info = NULL;
//     MCcall(find_struct_info((void *)nodespace, type_identifier, &p_struct_info));
//     if (p_struct_info)
//     {
//       declare_and_assign_anon_struct(struct_info_v1, struct_info, p_struct_info);

//       sprintf(buf + strlen(buf), "declare_and_allocate_anon_struct(%s_v%u, %s, (sizeof(void *) * %u));\n", struct_info->name,
//               struct_info->version, var_name, struct_info->field_count);

//       // declared_types[declared_type_count].struct_info = (void *)struct_info;
//     }
//     else
//     {
//       sprintf(buf + strlen(buf), "%s %s;\n", type_identifier, var_name);

//       // declared_types[declared_type_count].struct_info = NULL;
//     }

//     // ++declared_type_count;
//     free(var_name);
//     free(type_identifier);
//   }
//     return 0;
//   case 'n':
//   {
//     // invoke
//     bool isi = script_statement[i + 2] == 'i';
//     if (isi)
//     {
//       MCcall(parse_past(script_statement, &i, "nvi"));
//     }
//     else
//     {
//       MCcall(parse_past(script_statement, &i, "nvk"));
//     }

//     MCcall(parse_past(script_statement, &i, " ")); // TODO -- allow tabs too

//     // type
//     char *function_identity;
//     MCcall(parse_past_identifier(script_statement, &i, &function_identity, true, false));

//     // Is mc function?
//     void *p_function_info;
//     MCcall(mcqck_find_function_info((void *)nodespace, function_identity, &p_function_info));
//     if (p_function_info)
//     {
//       return -583;
//     }
//     else
//     {
//       strcat(buf, function_identity);
//       strcat(buf, "(");

//       int arg_count = 0;
//       while (script_statement[i] == ' ')
//       {
//         ++i;
//         char *arg_identity;
//         MCcall(parse_past_identifier(script_statement, &i, &arg_identity, true, true));
//         if (arg_count > 0)
//           strcat(buf, ", ");
//         strcat(buf, arg_identity);

//         ++arg_count;
//       }
//     }

//     strcat(buf, ");\n");
//     MCcall(parse_past(script_statement, &i, "\n"));
//   }
//     return 0;
//     // case 'r':
//     // {
//     //   printf("TODO\n");
//     //   return -52828;
//     // }
//     // break;

//   default:
//     MCcall(print_parse_error(script_statement, 0, "mcqck_translate_script_statement", "UnhandledStatement"));
//     return -757;
//   }
// }

int append_to_cstr(unsigned int *allocated_size, char **cstr, const char *extra)
{
  int n = strlen(extra);
  if (strlen(*cstr) + n + 1 >= *allocated_size)
  {
    unsigned int new_allocated_size = *allocated_size + 100 + *allocated_size / 100;
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    memcpy(newptr, *cstr, sizeof(char) * *allocated_size);
    free(*cstr);
    *cstr = newptr;
    *allocated_size = new_allocated_size;
  }

  strcat(*cstr, extra);

  return 0;
}

int mcqck_generate_script_local(void *nodespace, void ***local_index, unsigned int *local_indexes_alloc, unsigned int *local_indexes_count, void *p_script, char *buf,
                                char *type_identifier, char *var_name)
{
  int res;
  void **mc_dvp;

  declare_and_assign_anon_struct(script_v1, script, p_script);

  // Strip type of all deref operators
  char *raw_type_id = NULL;
  for (int i = 0;; ++i)
  {
    if (type_identifier[i] == ' ' || type_identifier[i] == '*' || type_identifier[i] == '\0')
    {
      raw_type_id = (char *)malloc(sizeof(char) * (i + 1));
      strcpy(raw_type_id, type_identifier);
      raw_type_id[i] = '\0';
      break;
    }
  }

  script_local_v1 *kvp;
  allocate_anon_struct(kvp, sizeof_script_local_v1);
  allocate_and_copy_cstr(kvp->type, type_identifier);
  allocate_and_copy_cstr(kvp->identifier, var_name);
  kvp->locals_index = script->local_count;
  kvp->replacement_code = (char *)malloc(sizeof(char) * (64 + strlen(kvp->type)));

  // -- Determine if the structure is midge-specified
  MCcall(find_struct_info(nodespace, raw_type_id, &kvp->struct_info));
  char *size_of_var;
  if (kvp->struct_info)
  {
    declare_and_assign_anon_struct(struct_info_v1, sinfo, kvp->struct_info);

    char *substituted_type = (char *)malloc(sizeof(char) * (strlen(kvp->type) - strlen(raw_type_id) + strlen(sinfo->declared_mc_name) + 1));
    strcpy(substituted_type, sinfo->declared_mc_name);
    strcat(substituted_type, kvp->type + strlen(raw_type_id));
    substituted_type[strlen(kvp->type) - strlen(raw_type_id) + strlen(sinfo->declared_mc_name)] = '\0';
    printf("kt:%s rt:%s dmc:%s st:%s\n", kvp->type, raw_type_id, sinfo->declared_mc_name, substituted_type);

    sprintf(kvp->replacement_code, "(*(%s *)script->locals[%u])", substituted_type, kvp->locals_index);
    // printf("\nkrpstcde:'%s'\n", kvp->replacement_code);

    size_of_var = (char *)malloc(sizeof(char) * (8 + 1 + strlen(substituted_type)));
    sprintf(size_of_var, "sizeof(%s)", substituted_type);

    free(substituted_type);
  }
  else
  {
    sprintf(kvp->replacement_code, "(*(%s *)script->locals[%u])", kvp->type, kvp->locals_index);
    // printf("\nkrpnncde:'%s','%s'\n", type_identifier, kvp->replacement_code);

    size_of_var = (char *)malloc(sizeof(char) * (8 + 1 + strlen(kvp->type)));
    sprintf(size_of_var, "sizeof(%s)", kvp->type);
  }

  ++script->local_count;
  // printf("type:%s identifier:%s lind:%u repcode:%s\n", kvp->type, kvp->identifier, kvp->locals_index, kvp->replacement_code);

  append_to_collection(local_index, local_indexes_alloc, local_indexes_count, kvp);

  sprintf(buf, "script->locals[%u] = (void *)malloc(%s);\n", kvp->locals_index, size_of_var);

  free(raw_type_id);
  free(size_of_var);

  return 0;
}

int mcqck_get_script_local_replace(void *nodespace, void **local_index, unsigned int local_indexes_count, const char *key, char **output)
{
  void **mc_dvp;
  *output = NULL;

  char *primary = NULL;
  int m = -1;
  char breaker = 'n';
  for (int i = 0;; ++i)
  {
    if (!isalnum(key[i]) && key[i] != '_')
    {
      primary = (char *)malloc(sizeof(char) * (i + 1));
      strncpy(primary, key, i);
      primary[i] = '\0';
      m = i;
      breaker = key[i];
      break;
    }
  }
  if (primary == NULL)
  {
    allocate_and_copy_cstr(primary, key);
  }
  script_local_v1 *kvp;
  for (int i = 0; i < local_indexes_count; ++i)
  {
    assign_anon_struct(kvp, local_index[i]);

    if (!strcmp(primary, kvp->identifier))
    {
      // printf("match!: %s=%s:%s\n", primary, kvp->identifier, kvp->replacement_code);
      break;
    }
    else
    {
      // printf("NOmatch!: %s=%s:%s\n", primary, kvp->identifier, kvp->replacement_code);
    }

    kvp = NULL;
  }
  free(primary);
  if (!kvp)
    return 0;

  switch (breaker)
  {
  case '\0':
  {
    // Simple replacement
    *output = kvp->replacement_code;
    return 0;
  }
  case '-':
  {
    // -> Pointer to member
    if (!kvp->struct_info)
    {
      *output = (char *)malloc(sizeof(char) * (strlen(kvp->replacement_code) + strlen(key) - m + 1));
      sprintf(*output, "%s%s", kvp->replacement_code, key + m);
      // printf("*output='%s'\n", *output);
      return 0;
    }

    parse_past(key, &m, "->");
    primary = (char *)malloc(sizeof(char) * (2 + strlen(kvp->replacement_code) + 1));
    sprintf(primary, "%s->", kvp->replacement_code);

    // Determine the child field type
    struct_info_v1 *parent_struct_info;
    assign_anon_struct(parent_struct_info, kvp->struct_info);
    char *secondary = NULL;
    // Get the member name
    breaker = 'n';
    for (int i = m;; ++i)
    {
      if (!isalnum(key[i]) && key[i] != '_')
      {
        secondary = (char *)malloc(sizeof(char) * (i - m + 1));
        strncpy(secondary, key + m, i - m);
        secondary[i - m] = '\0';
        m = i;
        breaker = key[i];
        break;
      }
    }
    switch (breaker)
    {
    case '\0':
    {
      printf("primary:'%s'\n", primary);
      printf("secondary:'%s'\n", secondary);

      *output = (char *)malloc(sizeof(char) * (strlen(primary) + strlen(secondary) + 1));
      strcpy(*output, primary);
      strcat(*output, secondary);
      (*output)[strlen(primary) + strlen(secondary)] = '\0';

      free(primary);
      free(secondary);
      return 0;
    }
    // case '-':
    // {
    // }
    // break;
    default:
    {
      MCerror(24530, "Not handled:'%c'  key=%s", breaker, key);
    }
    }

    // if (n < 0)
    // {
    // }
    // else
    // {
    //   MCerror(24526, "TODO");
    // }

    MCerror(24524, "TODO");
  }
  default:
  {
    MCerror(24528, "Not handled:'%c'  key=%s", breaker, key);
  }
  }
}

int parse_past_singular_expression(void *nodespace, void **local_index, unsigned int local_indexes_count, char *code, int *i, char **output)
{
  int res;
  char *primary, *temp;

  if (isalpha(code[*i]))
  {
    MCcall(parse_past_identifier(code, i, &primary, true, true));

    MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, primary, &temp));

    if (temp)
    {
      free(primary);
      allocate_and_copy_cstr(primary, temp);
    }

    if (code[*i] != '[')
    {
      *output = primary;
      return 0;
    }

    for (int b = 1;; ++b)
    {
      // Parse past the '['
      ++*i;

      char *secondary;
      MCcall(parse_past_identifier(code, i, &secondary, true, true));
      MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, secondary, &temp));
      if (temp)
      {
        free(secondary);
        allocate_and_copy_cstr(secondary, temp);
      }

      temp = (char *)malloc(sizeof(char) * (strlen(primary) + 1 + strlen(secondary) + b + 1));
      strcpy(temp, primary);
      strcat(temp, "[");
      strcat(temp, secondary);
      free(primary);
      free(secondary);
      primary = temp;

      if (code[*i] != '[')
      {
        int k = strlen(primary);
        for (int c = 0; c < b; ++c)
        {
          primary[k + c] = ']';
          ++*i;
        }
        primary[k + b] = '\0';

        *output = primary;
        break;
      }
    }

    return 0;
  }
  else
    switch (code[*i])
    {
    case '"':
      MCerror(7834534, "TODO quotes");
      return 0;
    case '\'':
    {
      MCcall(parse_past_character_literal(code, i, output));
    }
      return 0;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
      MCcall(parse_past_number(code, i, output));
    }
      return 0;
    case '(':
    {
      ++*i;
      MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, i, &temp));

      MCcall(parse_past(code, i, ")"));

      *output = (char *)malloc(sizeof(char) * (2 + strlen(temp) + 1));
      sprintf(*output, "(%s)", temp);
    }
      return 0;
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    {
      char *oper = (char *)malloc(sizeof(char) * 2);
      oper[0] = code[*i];
      oper[1] = '\0';
      ++*i;
      if (code[*i] != ' ')
      {
        MCerror(9877, "BackTODO");
      }
      MCcall(parse_past(code, i, " "));

      // left
      char *left;
      MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, i, &left));
      MCcall(parse_past(code, i, " "));

      // right
      char *right;
      MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, i, &right));

      *output = (char *)malloc(sizeof(char) * (strlen(oper) + 1 + strlen(left) + 1 + strlen(right) + 1));
      sprintf(*output, "%s %s %s", left, oper, right);

      free(left);
      free(oper);
      free(right);
    }
      return 0;
    default:
    {
      MCcall(print_parse_error(code, *i, "parse_past_expression", "first_char"));
      return -7777;
    }
    }

  return 4240;
}

int mcqck_translate_script_code(void *nodespace, void *p_script, char *code)
{
  int res;
  void **mc_dvp;
  declare_and_assign_anon_struct(script_v1, script, p_script);

  unsigned int translation_alloc = 512 + strlen(code) * 13 / 10;
  char *translation = (char *)malloc(sizeof(char) * translation_alloc);
  translation[0] = '\0';
  unsigned int t = 0;

  unsigned int local_indexes_alloc = 20;
  unsigned int local_indexes_count = 0;
  void **local_index = (void **)malloc(sizeof(void *) * local_indexes_alloc);

  script->local_count = 0;
  script->segment_count = 0;

  char buf[1024];
  // -- Parse statements
  int s = -1;
  int i = 8;
  bool loop = true;
  while (loop)
  {
    switch (code[i])
    {
    case ' ':
    case '\t':
    case '\n':
      break;
    case '$':
    {
      // $
      MCcall(parse_past(code, &i, "$asi"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // Identifier
      char *response_identifier;
      MCcall(parse_past_identifier(code, &i, &response_identifier, false, false));
      MCcall(parse_past(code, &i, " "));

      // Variable Name
      char *literal_cstr;
      if (code[i] == '"')
      {
        int j;
        int x = -1;
        bool prev_escape = false;
        for (j = i + 1;; ++j)
        {
          if (code[j] == '"' && !prev_escape)
          {
            x = j + 1;
            break;
          }
          prev_escape = !prev_escape && (code[j] == '\\');
        }

        if (x < 0)
        {
          MCerror(4877, "literal not ended");
        }
        literal_cstr = (char *)malloc(sizeof(char) * (x - i + 1));
        strncpy(literal_cstr, code + i, x - i);
        literal_cstr[x - i] = '\0';

        i += x - i + 1;
      }
      else
      {
        MCerror(4875, "only literal supported");
      }

      if (code[i] != '\n' && code[i] != '\0')
      {
        MCerror(-4864, "expected statement end:'%c'", code[i]);
      }

      ++script->segment_count;
      buf[0] = '\0';
      sprintf(buf, "  printf(\"here-4\\n\");allocate_and_copy_cstr(script->response, %s);\n"
                   "printf(\"seqid:%%u \\n\", script->sequence_uid);\n"
                   "  script->segments_complete = %u;"
                   "  script->awaiting_data_set_index = script->local_count;"
                   "  printf(\"here-6a\\n\");return 0;\n"
                   "segment_%u: printf(\"here-6b\\n\");char *%s = (char *)script->locals[%u];\n",
              literal_cstr, script->segment_count, script->segment_count, response_identifier, script->local_count);
      ++script->local_count;

      append_to_cstr(&translation_alloc, &translation, buf);

      free(literal_cstr);
      //     // declared_types[declared_type_count].type = type_identifier;
      //     // declared_types[declared_type_count].var_name = var_name;
    }
    break;
    case 'a':
    {
      // ass / asi
      MCcall(parse_past(code, &i, "as"));
      // if (code[i] == 'i')
      // {
      //   MCcall(parse_past(code, &i, "i"));
      //   MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      //   // Identifier
      //   char *type_identifier;
      //   MCcall(parse_past_identifier(code, &i, &type_identifier, false, false));
      //   MCcall(parse_past(code, &i, " "));

      //   // Variable Name
      //   char *var_name;
      //   MCcall(parse_past_identifier(code, &i, &var_name, false, false));
      //   MCcall(parse_past(code, &i, " "));

      //   // Any arithmetic operator
      //   char *comparator;
      //   switch (code[i])
      //   {
      //   case '-':
      //   {
      //     allocate_and_copy_cstr(comparator, "-");
      //     ++i;
      //   }
      //   break;
      //   default:
      //     MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "if>operator-switch"));
      //     return 34242;
      //   }
      //   MCcall(parse_past(code, &i, " "));

      //   if (comparator)
      //   {
      //     // left
      //     char *left;
      //     MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, &i, &left));
      //     MCcall(parse_past(code, &i, " "));

      //     // right
      //     char *right;
      //     MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, &i, &right));

      //     buf[0] = '\0';
      //     MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
      //                                        type_identifier, var_name));
      //     append_to_cstr(&translation_alloc, &translation, buf);

      //     buf[0] = '\0';
      //     char *replace_name;
      //     MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_name));
      //     sprintf(buf, "%s = %s %s %s", replace_name, left, comparator, right);

      //     while (code[i] != '\n' && code[i] != '\0')
      //     {
      //       MCcall(parse_past(code, &i, " "));

      //       // Any arithmetic operator
      //       switch (code[i])
      //       {
      //       case '-':
      //       {
      //         allocate_and_copy_cstr(comparator, "-");
      //         ++i;
      //       }
      //       break;
      //       default:
      //         MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "if>operator-switch"));
      //         return 34242;
      //       }
      //       MCcall(parse_past(code, &i, " "));
      //       MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, &i, &right));
      //       sprintf(buf + strlen(buf), " %s %s", comparator, right);
      //     }
      //     MCcall(append_to_cstr(&translation_alloc, &translation, buf));
      //     MCcall(append_to_cstr(&translation_alloc, &translation, ";\n"));
      //   }
      //   else
      //   {
      //     MCerror(-4864, "TODO");
      //   }
      // }
      // else
      if (code[i] == 's')
      {
        MCcall(parse_past(code, &i, "s"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        char *var_identifier;
        MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, &i, &var_identifier));
        MCcall(parse_past(code, &i, " "));

        char *left;
        MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, &i, &left));
        if (code[i] != '\n' && code[i] != '\0')
        {
          MCerror(-4829, "expected statement end");
        }

        // buf[0] = '\0';
        sprintf(buf, "%s = %s;\n", var_identifier, left);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        // free(var_identifier);
        // free(left);
      }
      else
      {
        MCerror(-4866, "TODO");
      }
    }
    break;
    case 'b':
    {
      // brk
      MCcall(parse_past(code, &i, "brk"));
      if (code[i] != '\n' && code[i] != '\0')
      {
        MCerror(-4829, "expected statement end");
      }
      MCcall(append_to_cstr(&translation_alloc, &translation, "break;\n"));
    }
    break;
    case 'd':
    {
      MCcall(parse_past(code, &i, "dc"));
      if (code[i] == 'l')
      {
        // dcl
        MCcall(parse_past(code, &i, "l"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Identifier
        char *type_identifier;
        MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        MCcall(parse_past(code, &i, " "));

        // Variable Name
        char *var_name;
        MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        if (code[i] != '\n' && code[i] != '\0')
        {
          // Array decl
          MCcall(parse_past(code, &i, "["));

          char *left;
          MCcall(parse_past_singular_expression(nodespace, local_index, local_indexes_count, code, &i, &left));
          MCcall(parse_past(code, &i, "]"));
          if (code[i] != '\n' && code[i] != '\0')
          {
            MCerror(-4829, "expected statement end");
          }

          // Add deref to type
          char *derefenced_type_identifier;
          for (int j = strlen(type_identifier) - 1;; --j)
          {
            if (type_identifier[j] == '*')
            {
              derefenced_type_identifier = (char *)malloc(sizeof(char) * (j + 1));
              strcpy(derefenced_type_identifier, type_identifier);
              derefenced_type_identifier[j] = '\0';
              break;
            }
            if (isalpha(type_identifier[j]))
            {
              MCerror(4242, "no pointer for array?");
            }
          }

          // Normal Declaration
          MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
                                             type_identifier, var_name));
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));

          // Allocate the array
          char *replace_var_name;
          MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_var_name));

          sprintf(buf, "%s = (%s)malloc(sizeof(%s) * (%s));\n", replace_var_name, type_identifier, derefenced_type_identifier, left);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));

          free(left);
          free(derefenced_type_identifier);
          printf("Translation:\n%s\n", translation);
        }
        else
        {
          // Normal Declaration
          MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
                                             type_identifier, var_name));
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }
        free(var_name);
        free(type_identifier);
      }
      else if (code[i] == 's')
      {
        // dcs
        MCcall(parse_past(code, &i, "s"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Identifier
        char *type_identifier;
        MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        MCcall(parse_past(code, &i, " "));

        // Variable Name
        char *var_name;
        MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        MCcall(parse_past(code, &i, " "));

        // Set Value
        char *left;
        MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &left));
        if (code[i] != '\n' && code[i] != '\0')
        {
          MCerror(-4829, "expected statement end");
        }

        // Generate Local
        MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
                                           type_identifier, var_name));
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        // Assign
        char *replace_var_name;
        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_var_name));

        sprintf(buf, "%s = %s;\n", replace_var_name, left);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        free(type_identifier);
        free(var_name);
        free(left);
      }
      else
      {
        MCerror(86583, "TODO");
      }
    }
    break;
    case 'e':
    {
      // end
      MCcall(parse_past(code, &i, "end"));
      MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));

      if (code[i] != '\n' && code[i] != '\0')
      {
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too
        MCcall(parse_past(code, &i, "for"));
        MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));

        if (code[i] != '\n' && code[i] != '\0')
        {
          MCerror(-4831, "expected statement end");
        }
      }
    }
    break;
    case 'i':
    {
      // ifs
      // for
      MCcall(parse_past(code, &i, "ifs"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // left
      char *left;
      printf("here-0\n");
      MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &left));
      printf("left:%s\n", left);
      MCcall(parse_past(code, &i, " "));

      // comparison operator
      char *comparator;
      switch (code[i])
      {
      case '=':
      {
        if (code[i + 1] != '=')
        {
          MCerror(585282, "can't have just one =");
        }
        i += 2;
        comparator = (char *)malloc(sizeof(char) * 3);
        strcpy(comparator, "==");
      }
      break;
      case '<':
      case '>':
      {
        int len = 1;
        if (code[i + 1] == '=')
          len = 2;
        comparator = (char *)malloc(sizeof(char) * (len + 1));
        comparator[0] = code[i];
        if (len > 1)
          comparator[1] = code[i + 1];
        comparator[len] = '\0';
        i += len;
      }
      break;
      default:
        MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "if>comparator-switch"));
        return 858528;
      }
      MCcall(parse_past(code, &i, " "));

      // right
      char *right;
      MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &right));
      if (code[i] != '\n' && code[i] != '\0')
      {
        MCerror(-4864, "expected statement end:'%c'", code[i]);
      }

      buf[0] = '\0';
      sprintf(buf, "if(%s %s %s) {\n", left, comparator, right);
      printf("ifs:%s", buf);
      MCcall(append_to_cstr(&translation_alloc, &translation, buf));

      free(left);
      free(comparator);
      free(right);
    }
    break;
    case 'f':
    {
      // for i 0 command_length
      // for
      MCcall(parse_past(code, &i, "for"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // iterator
      char *iterator;
      MCcall(parse_past_identifier(code, &i, &iterator, false, false));
      MCcall(parse_past(code, &i, " "));

      // initiate
      char *initiate, *initiate_final;
      if (isalpha(code[i]))
      {
        MCcall(parse_past_identifier(code, &i, &initiate, false, false));

        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, initiate, &initiate_final));
        if (!initiate_final)
          initiate_final = initiate;
      }
      else if (code[i] == '"')
      {
        MCerror(118492, "TODO quotes");
      }
      else if (isdigit(code[i]))
      {
        parse_past_number(code, &i, &initiate);
        initiate_final = initiate;
      }
      else
      {
        MCerror(118492, "TODO what");
      }

      MCcall(parse_past(code, &i, " "));

      // maximum
      char *maximum, *maximum_final;
      if (isalpha(code[i]))
      {
        MCcall(parse_past_identifier(code, &i, &maximum, false, false));

        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, maximum, &maximum_final));
        if (!maximum_final)
          maximum_final = maximum;
      }
      else if (code[i] == '"')
      {
        MCerror(78678, "TODO quotes");
      }
      else if (isdigit(code[i]))
      {
        parse_past_number(code, &i, &initiate);
      }
      else
      {
        MCerror(373785, "TODO what");
      }
      if (code[i] != '\n' && code[i] != '\0')
      {
        MCerror(-4864, "expected statement end:'%c'", code[i]);
      }

      append_to_cstr(&translation_alloc, &translation, "{\n");
      buf[0] = '\0';
      char *int_cstr;
      allocate_and_copy_cstr(int_cstr, "int");
      MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
                                         int_cstr, iterator));
      append_to_cstr(&translation_alloc, &translation, buf);
      char *iterator_replace;
      MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, iterator, &iterator_replace));

      buf[0] = '\0';
      sprintf(buf, "for(%s = %s; %s < %s; ++%s) {\n", iterator_replace, initiate_final, iterator_replace, maximum_final, iterator_replace);
      append_to_cstr(&translation_alloc, &translation, buf);

      free(initiate);
      free(maximum);
    }
    break;
    case 'm':
    {
      // msi
      MCcall(parse_past(code, &i, "ms"));
      if (code[i] == 'i')
      {
        MCcall(parse_past(code, &i, "i"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Type Identifier
        char *type_identifier;
        MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        MCcall(parse_past(code, &i, " "));

        // Variable Name
        char *var_name;
        MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        MCcall(parse_past(code, &i, " "));

        // Set Value
        char *sizeof_expr;
        MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &sizeof_expr));
        if (code[i] != '\n' && code[i] != '\0')
        {
          MCerror(-4829, "expected statement end");
        }

        // Script Local
        MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
                                           type_identifier, var_name));
        append_to_cstr(&translation_alloc, &translation, buf);

        char *replace_name;
        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_name));

        // Statement
        char *trimmed_type_identifier = (char *)malloc(sizeof(char) * (strlen(type_identifier)));
        strncpy(trimmed_type_identifier, type_identifier, strlen(type_identifier) - 1);
        trimmed_type_identifier[strlen(type_identifier) - 1] = '\0';

        sprintf(buf, "%s = (%s)malloc(sizeof(%s) * (%s));\n", replace_name, type_identifier, trimmed_type_identifier, sizeof_expr);
        free(trimmed_type_identifier);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        // // Any arithmetic operator
        // char *comparator;
        // switch (code[i])
        // {
        // case '-':
        // case '+':
        // case '*':
        // case '/':
        // {
        //   comparator = (char *)malloc(sizeof(char) * 2);
        //   comparator[0] = code[i];
        //   comparator[1] = '\0';
        //   ++i;
        // }
        // break;
        // default:
        //   MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "if>operator-switch"));
        //   return 34242;
        // }
        // MCcall(parse_past(code, &i, " "));

        // if (comparator)
        // {
        //   // left
        //   char *left;
        //   MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &left));
        //   MCcall(parse_past(code, &i, " "));

        //   // right
        //   char *right;
        //   MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &right));

        //   buf[0] = '\0';
        //   MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
        //                                      type_identifier, var_name));
        //   append_to_cstr(&translation_alloc, &translation, buf);

        //   buf[0] = '\0';
        //   char *replace_name;
        //   MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_name));
        //   char *trimmed_type_identifier = (char *)malloc(sizeof(char) * (strlen(type_identifier)));
        //   strncpy(trimmed_type_identifier, type_identifier, strlen(type_identifier) - 1);
        //   trimmed_type_identifier[strlen(type_identifier) - 1] = '\0';
        //   sprintf(buf, "%s = malloc(sizeof(%s) * (%s %s %s", replace_name, trimmed_type_identifier, left, comparator, right);

        //   while (code[i] != '\n' && code[i] != '\0')
        //   {
        //     MCcall(parse_past(code, &i, " "));

        //     // Any arithmetic operator
        //     switch (code[i])
        //     {
        //     case '-':
        //     case '+':
        //     case '*':
        //     case '/':
        //     {
        //       comparator = (char *)malloc(sizeof(char) * 2);
        //       comparator[0] = code[i];
        //       comparator[1] = '\0';
        //       ++i;
        //     }
        //     break;
        //     default:
        //       MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "msi>operator-switch"));
        //       return 34242;
        //     }
        //     MCcall(parse_past(code, &i, " "));
        //     MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &right));
        //     sprintf(buf + strlen(buf), " %s %s", comparator, right);
        //   }
        //   MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        //   MCcall(append_to_cstr(&translation_alloc, &translation, "));\n"));
        // }
        // else
        // {
        //   MCerror(-4864, "TODO");
        // }
      }
      else
      {
        MCerror(-4866, "TODO");
      }
    }
    break;
    case 'n':
    {
      // nvi
      MCcall(parse_past(code, &i, "nv"));
      if (code[i] == 'i')
      {
        MCcall(parse_past(code, &i, "i"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Identifier
        char *type_identifier;
        MCcall(parse_past_type_identifier(code, &i, &type_identifier));
        MCcall(parse_past(code, &i, " "));

        // Variable Name
        char *var_name;
        MCcall(parse_past_identifier(code, &i, &var_name, false, false));
        MCcall(parse_past(code, &i, " "));

        MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
                                           type_identifier, var_name));
        append_to_cstr(&translation_alloc, &translation, buf);

        char *replace_name;
        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_name));
        // printf("nvi gen replace_name:%s=%s\n", var_name, replace_name);

        // Invoke
        // Function Name
        char *function_name;
        MCcall(parse_past_identifier(code, &i, &function_name, true, false));

        function_info_v1 *function_info;
        mcqck_find_function_info((void *)nodespace, function_name, (void **)&function_info);
        if (!function_info)
        {
          sprintf(buf, "%s = %s(", replace_name, function_name);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }
        else
        {
          if (!strcmp(function_info->return_type, "void"))
          {
            MCerror(-1002, "compile error: cannot assign from a void function!");
          }
          else
          {
            sprintf(buf, "mc_vargs[0] = (void *)&%s;\n", replace_name);
            MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          }
        }

        int arg_index = 0;
        while (code[i] != '\n' && code[i] != '\0')
        {
          MCcall(parse_past(code, &i, " "));

          char *arg_name;
          MCcall(parse_past_identifier(code, &i, &arg_name, true, true));
          MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, arg_name, &replace_name));
          char *arg_entry = arg_name;
          if (replace_name)
            arg_entry = replace_name;
          if (function_info)
            sprintf(buf, "mc_vargs[%i] = (void *)&%s;\n", arg_index + 1, arg_entry);
          else
            sprintf(buf, "%s%s", arg_index ? ", " : "", arg_entry);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          ++arg_index;
          free(arg_name);
        }

        if (function_info)
        {
          sprintf(buf, "%s(%i, mc_vargs", function_name, arg_index + 1);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }

        MCcall(append_to_cstr(&translation_alloc, &translation, ");\n"));

        free(function_name);
      }
      else if (code[i] == 'k')
      {
        MCcall(parse_past(code, &i, "k"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Invoke
        // Function Name
        char *function_name;
        MCcall(parse_past_identifier(code, &i, &function_name, true, false));
        MCcall(append_to_cstr(&translation_alloc, &translation, function_name));
        MCcall(append_to_cstr(&translation_alloc, &translation, "("));

        bool first_arg = true;
        while (code[i] != '\n' && code[i] != '\0')
        {
          MCcall(parse_past(code, &i, " "));

          char *argument;
          MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &argument));

          buf[0] = '\0';
          sprintf(buf, "%s%s", first_arg ? "" : ", ", argument);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          first_arg = false;
          free(argument);
        }
        MCcall(append_to_cstr(&translation_alloc, &translation, ");\n"));
      }
      else
      {
        MCerror(-4248, "TODO");
      }
    }
    break;
    case 's':
    {
      // set
      MCcall(parse_past(code, &i, "set"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // Identifier
      char *type_identifier;
      MCcall(parse_past_type_identifier(code, &i, &type_identifier));
      MCcall(parse_past(code, &i, " "));

      // Variable Name
      char *var_name;
      MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &var_name));
      MCcall(parse_past(code, &i, " "));

      // Value Name
      char *value_expr;
      MCcall(parse_past_singular_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &value_expr));
      if (code[i] != '\n' && code[i] != '\0')
      {
        MCerror(-4829, "expected statement end");
      }

      if (!strcmp(type_identifier, "int"))
      {
        buf[0] = '\0';
        sprintf(buf, "%s = %s;\n", var_name, value_expr);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));
      }
      else
      {
        MCerror(52885, "NotYetSupported:%s", type_identifier);
      }
    }
    break;
    case '\0':
      loop = false;
      break;
    default:
    {
      // printf("\ntranslation:\n%s\n\n", translation);
      MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "UnhandledStatement"));
      return -4857;
    }
    break;
    }

    ++i;
  }

  unsigned int declaration_alloc = 1024 + translation_alloc;
  char *declaration = (char *)malloc(sizeof(char) * declaration_alloc);
  sprintf(declaration, "int %s%u(void *p_script) {\n"
                       "  int res;\n"
                       "  void **mc_dvp;\n"
                       "  void *mc_vargs[64];\n"
                       "\n"
                       "  declare_and_assign_anon_struct(script_v1, script, p_script);\n"
                       "  declare_and_assign_anon_struct(node_v1, global, script->arguments[0]);\n"
                       "  declare_and_assign_anon_struct(node_v1, nodespace, script->arguments[1]);\n"
                       "  const char *command = (const char *)script->arguments[2];\n\n"
                       "  switch(script->segments_complete)\n"
                       "  {\n"
                       "  case 0:\n"
                       "    break;\n",
          SCRIPT_NAME_PREFIX, script->script_uid);
  for (int i = 1; i <= script->segment_count; ++i)
  {
    sprintf(buf, "  case %i: goto segment_%i;\n", i, i);
    append_to_cstr(&declaration_alloc, &declaration, buf);
  }
  append_to_cstr(&declaration_alloc, &declaration,
                 "  default:\n"
                 "    return 0;\n"
                 "  }\n\n");
  ++script->segment_count;
  append_to_cstr(&declaration_alloc, &declaration, translation);
  append_to_cstr(&declaration_alloc, &declaration, "\n"
                                                   "// Script Execution Finished\n"
                                                   "script->segments_complete = script->segment_count;\n"
                                                   "printf(\"script-concluding\\n\");\n"
                                                   "return 0;\n}");

  script->locals = (void **)calloc(sizeof(void *), script->local_count);

  printf("script_declaration:\n%s\n\n", declaration);

  clint_declare(declaration);

  free(translation);
  free(declaration);
  return 0;
}

int initialize_function_v1(int argc, void **argv)
{
  printf("initialize_function_v1()\n");
  void **mc_dvp;
  int res;

  declare_and_assign_anon_struct(node_v1, nodespace, argv[0]);
  declare_and_assign_anon_struct(function_info_v1, function_info, argv[1]);
  char *code = (char *)argv[2];

  printf("nodespace->name:%s\n", nodespace->name);
  printf("function_info->name:%s\n", function_info->name);
  printf("function_info->latest_iteration:%u\n", function_info->latest_iteration);
  // printf("code_block:%c%c%c%c%c...", code_block[0], code_block[1], code_block[2], code_block[3], code_block[4]);

  // Increment function iteration
  ++function_info->latest_iteration;

  // Declare with clint
  char buf[16384];
  const char *TAB = "  ";

  sprintf(buf, "int %s_v%u(int mc_argc, void **mc_argv)\n{\n", function_info->name, function_info->latest_iteration);
  // TODO -- mc_argc count check?
  strcat(buf, TAB);
  strcat(buf, "// Arguments\n");
  strcat(buf, TAB);
  strcat(buf, "int res;\n");
  strcat(buf, TAB);
  strcat(buf, "void **mc_dvp;\n");

  for (int i = 0; i < function_info->parameter_count; ++i)
  {
    strcat(buf, TAB);

    declare_and_assign_anon_struct(parameter_info_v1, parameter, function_info->parameters[i]);

    void *p_struct_info;
    MCcall(find_struct_info(nodespace, parameter->type_name, &p_struct_info));
    if (p_struct_info)
    {
      strcat(buf, "declare_and_assign_anon_struct(");
      strcat(buf, parameter->type_name);
      strcat(buf, "_v");
      sprintf(buf + strlen(buf), "%i, ", parameter->type_version);
      strcat(buf, parameter->name);
      strcat(buf, ", ");
      strcat(buf, "mc_argv[");
      sprintf(buf + strlen(buf), "%i]);\n", i);
    }
    else
    {
      sprintf(buf + strlen(buf), "%s %s = (%s)mc_argv[%i];\n", parameter->type_name, parameter->name, parameter->type_name, i);
    }
  }
  if (function_info->parameter_count > 0)
    strcat(buf, "\n");

  // const char *identity; const char *type; struct_info *struct_info;
  struct
  {
    const char *var_name;
    const char *type;
    void *struct_info;
  } declared_types[200];
  int declared_type_count = 0;

  printf("@ifv-5\n");
  // Translate the code-block from script into workable midge-cling C

  return -5224;

  int n = strlen(code);
  for (int i = 0; i < n;)
  {
    //strcat(buf, code_block);
    switch (code[i])
    {
    case ' ':
    case '\t':
      ++i;
      continue;
    default:
    {
      if (!isalpha(code[i]))
      {
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
  sprintf(buf, "%s = &%s_v%u;", function_info->name, function_info->name, function_info->latest_iteration);
  printf("ifv>clint_process:\n%s\n", buf);
  clint_process(buf);

  printf("ifv-concludes\n");
  return 0;
}

int declare_function_pointer_v1(int argc, void **argv)
{
  // printf("declare_function_pointer_v1()\n");
  // TODO -- not meant for usage with struct versions other than function_info_v1 && node_v1

  void **mc_dvp;
  int res;

  declare_and_assign_anon_struct(node_v1, nodespace, argv[0]);
  int parameter_count = *(int *)argv[1];
  midgeo parameters = (midgeo)argv[2];
  int variable_parameter_begin_index = *(int *)argv[3];

  // printf("::6 Params\n");
  // for (int i = 0; i < 6; ++i)
  // {
  //     printf("-%i:%s\n", i, (char *)parameters[i]);
  // }

  char *name = (char *)parameters[0];
  char *return_type = (char *)parameters[1];

  // printf("dfp>nodespace:%s\n", nodespace->name);
  // printf("dfp>param_count:%i\n", parameter_count);
  // printf("dfp>name:%s\n", name);
  // printf("dfp>return_type:%s\n", return_type);

  // TODO -- check

  // Fill in the function_info and attach to the nodespace
  declare_and_allocate_anon_struct(function_info_v1, function_info, sizeof_function_info_v1);
  function_info->name = name;
  function_info->latest_iteration = 0U;
  function_info->return_type = return_type;
  function_info->parameter_count = parameter_count;
  function_info->parameters = (void **)malloc(sizeof(void *) * parameter_count);
  function_info->variable_parameter_begin_index = variable_parameter_begin_index;
  unsigned int struct_usage_alloc = 2;
  function_info->struct_usage = (void **)malloc(sizeof(void *) * struct_usage_alloc);
  function_info->struct_usage_count = 0;

  for (int i = 0; i < function_info->parameter_count; ++i)
  {
    declare_and_allocate_anon_struct(parameter_info_v1, parameter_info, sizeof_parameter_info_v1);
    // printf("dfp>%p=%s\n", i, (void *)parameters[2 + i * 2 + 0], (char *)parameters[2 + i * 2 + 0]);
    parameter_info->type_name = (char *)parameters[2 + i * 2 + 0];

    void *p_struct_info = NULL;
    MCcall(find_struct_info((void *)nodespace, parameter_info->type_name, &p_struct_info));
    declare_and_assign_anon_struct(struct_info_v1, struct_info, p_struct_info);
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
    }
    else
      parameter_info->type_version = 0;

    parameter_info->name = (char *)parameters[2 + i * 2 + 1];
    function_info->parameters[i] = (void *)parameter_info;
    // printf("dfp>set param[%i]=%s %s\n", i, parameter_info->type, parameter_info->name);
  }
  MCcall(append_to_collection(&nodespace->functions, &nodespace->functions_alloc, &nodespace->function_count, function_info));

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

  // Declare with clint
  char buf[1024];
  strcpy(buf, "int (*");
  strcat(buf, name);
  strcat(buf, ")(int,void**);");
  printf("dfp>cling_declare:%s\n", buf);
  clint_declare(buf);
  printf("dfp-concludes\n");
  return 0;
}

int mcqck_temp_create_process_declare_function_pointer(midgeo *process_unit)
{
  const int dfp_varg_count = 4; // nodespace, params_count, input_data
                                // node-parent, name, return-type, params_count, parameters(field_info)

  midgeary dfp_vargs = (midgeary)malloc(sizeof(void *) * (1 + dfp_varg_count));
  allocate_from_intv(&dfp_vargs[0], dfp_varg_count);
  allocate_from_intv(&dfp_vargs[2], 0);
  midgeo parameter_data = (midgeo)calloc(sizeof(void *), 200);
  void **ptr_current_data = (void **)malloc(sizeof(void *) * 1);
  *ptr_current_data = (void *)&parameter_data[0];
  dfp_vargs[3] = parameter_data;
  allocate_from_intv(&dfp_vargs[4], 0);

  // process_parameter_data = &parameter_data[0];
  // p_process_param_count = (int *)dfp_vargs[4];

  // printf("addr of current_data[0]   :%p\n", &parameter_data[0]);
  // printf("ptr_current_data_points_to:%p\n", *ptr_current_data);

  // printf("pr_prm_pointer:%p\n", (char *)*(void **)process_parameter_data);
  // char *pparams = (char *)*process_parameter_data;
  // for (int z = 0; z < *p_process_param_count; ++z)
  // {
  //     printf("prm[%i]:%s", z, pparams);
  //     pparams += sizeof(unsigned long);
  // }
  void **mc_dvp;

  declare_and_allocate_anon_struct(process_unit_v1, process_unit_reset_data_pointer, sizeof_process_unit_v1);
  // declare_and_allocate_anon_struct(process_unit_v1, process_unit_function_name, sizeof_process_unit_v1);
  // process_unit_v1 *process_unit_function_name;
  // put_data = (void **)malloc(sizeof_process_unit_v1);
  // printf("address:%p\n", put_data);

  // assign_anon_struct(process_unit_function_name, put_data);
  // printf("pufnAddr:%p\n", process_unit_function_name);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_function_name, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_return_type, sizeof_process_unit_v1);
  // put_data = (void **)process_unit_function_name;
  // printf("address:%p\n", put_data);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_reset_params_count, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_type, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_name, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_increment_param_count, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_invoke, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_add_context_param, sizeof_process_unit_v1);

  process_unit_reset_data_pointer->type = PROCESS_UNIT_INVOKE;
  process_unit_reset_data_pointer->data = (void *)&set_pointer_value;
  midgeary invoke_args = (midgeary)malloc(sizeof(void *) * (1 + 2));
  allocate_from_intv(&invoke_args[0], 2);
  invoke_args[1] = (void *)ptr_current_data;
  invoke_args[2] = (void *)&parameter_data[0];
  process_unit_reset_data_pointer->data2 = (void *)invoke_args;
  process_unit_reset_data_pointer->next = (void *)process_unit_function_name;
  process_unit_reset_data_pointer->debug = "process_unit_reset_data_pointer";

  process_unit_function_name->type = PROCESS_UNIT_INTERACTION_INCR_DPTR;
  allocate_from_cstringv(&process_unit_function_name->data, "Function Name:");
  process_unit_function_name->data2 = (void *)ptr_current_data;
  process_unit_function_name->next = (void *)process_unit_return_type;
  process_unit_function_name->debug = "process_unit_function_name";

  process_unit_return_type->type = PROCESS_UNIT_INTERACTION_INCR_DPTR;
  allocate_from_cstringv(&process_unit_return_type->data, "Return Type:");
  process_unit_return_type->data2 = (void *)ptr_current_data;
  process_unit_return_type->next = (void *)process_unit_reset_params_count;
  process_unit_return_type->debug = "process_unit_return_type";

  // declare_and_assign_anon_struct(process_unit_v1, put, put_data);
  // printf("ptr_current_data_points_to7:%p\n", *((void **)put->data2));

  // unsigned long **var = (unsigned long **)ptr_current_data;
  // ++*var;
  int res;
  // // increment_pointer(1, (void **)put->data2);
  // var = (unsigned long **)*(void **)put->data2;
  // ++var;
  // MCcall(increment_pointer(1, &put->data2));
  // *(void **)vbut[0] += sizeof(void *);
  // unsigned long **var = (unsigned long **)vbut[0];
  // ++*var;

  // printf("ptr_current_data_points_to:%p\n", *ptr_current_data);
  // printf("ptr_current_data_points_to8:%p\n", *((void **)put->data2));
  // set_pointer_value(2, &invoke_args[1]);
  // printf("ptr_current_data_points_to9:%p\n", *((void **)put->data2));

  // set_pointer_value(2, &invoke_args[1]);
  // // put_data = (void *)process_unit_function_name;
  // process_unit_v1 *put;
  // mc_dvp = (void **)&put;
  // printf("put_data:%p\n", put_data);
  // *mc_dvp = (void *)(put_data);
  // printf("put:%p\n", put);
  // printf("debug:%s\n", put->debug);
  // printf("ptr_current_data_points_to:%p\n", **((void ***)put->data2));

  process_unit_reset_params_count->type = PROCESS_UNIT_INVOKE;
  process_unit_reset_params_count->data = (void *)&set_int_value;
  invoke_args = (midgeary)malloc(sizeof(void *) * (1 + 2));
  allocate_from_intv(&invoke_args[0], 2);
  invoke_args[1] = (void *)dfp_vargs[2];
  allocate_from_intv(&invoke_args[2], 0);
  process_unit_reset_params_count->data2 = (void *)invoke_args;
  process_unit_reset_params_count->next = (void *)process_unit_type;
  process_unit_reset_params_count->debug = "process_unit_reset_params_count";

  process_unit_type->type = PROCESS_UNIT_BRANCHING_INTERACTION;
  allocate_from_cstringv(&process_unit_type->data, "Parameter Type:");
  const int PARAMETER_TYPE_BRANCH_COUNT = 3;
  allocate_from_intv(&process_unit_type->data2, PARAMETER_TYPE_BRANCH_COUNT);
  midgeary branches = (midgeary)malloc(sizeof(void *) * PARAMETER_TYPE_BRANCH_COUNT);
  allocate_from_intv(&branches[0], PARAMETER_TYPE_BRANCH_COUNT);
  process_unit_type->next = (void *)branches;
  process_unit_type->debug = "process_unit_type";

  declare_and_allocate_anon_struct(branch_unit_v1, branch_end, sizeof_branch_unit_v1);
  branch_end->type = BRANCHING_INTERACTION_IGNORE_DATA;
  branch_end->match = "end";
  branch_end->next = (void *)process_unit_add_context_param;
  branches[0] = (void *)branch_end;

  declare_and_allocate_anon_struct(branch_unit_v1, branch_indicate_params, sizeof_branch_unit_v1);
  branch_indicate_params->type = BRANCHING_INTERACTION_INVOKE;
  branch_indicate_params->match = "params";
  process_unit_reset_params_count->data = (void *)&set_int_value;
  invoke_args = (midgeary)malloc(sizeof(void *) * (1 + 2));
  allocate_from_intv(&invoke_args[0], 2);
  invoke_args[1] = (void *)dfp_vargs[4];
  invoke_args[2] = (void *)dfp_vargs[2];
  branch_indicate_params->next = (void *)process_unit_type;
  branches[1] = (void *)branch_indicate_params;

  declare_and_allocate_anon_struct(branch_unit_v1, branch_default, sizeof_branch_unit_v1);
  branch_default->type = BRANCHING_INTERACTION_INCR_DPTR;
  branch_default->match = NULL;
  branch_default->data = (void *)ptr_current_data;
  branch_default->next = (void *)process_unit_name;
  branches[2] = (void *)branch_default;

  process_unit_name->type = PROCESS_UNIT_INTERACTION_INCR_DPTR;
  allocate_from_cstringv(&process_unit_name->data, "Parameter Name:");
  process_unit_name->data2 = (void *)ptr_current_data;
  process_unit_name->next = (void *)process_unit_increment_param_count;
  process_unit_name->debug = "process_unit_name";

  process_unit_increment_param_count->type = PROCESS_UNIT_INVOKE;
  process_unit_increment_param_count->data = (void *)&increment_int_value;
  invoke_args = (midgeary)malloc(sizeof(void *) * (1 + 2));
  allocate_from_intv(&invoke_args[0], 1);
  invoke_args[1] = (void *)dfp_vargs[2];
  process_unit_increment_param_count->data2 = (void *)invoke_args;
  process_unit_increment_param_count->next = (void *)process_unit_type;
  process_unit_increment_param_count->debug = "process_unit_increment_param_count";

  process_unit_add_context_param->type = PROCESS_UNIT_SET_CONTEXTUAL_DATA;
  allocate_from_intv(&process_unit_add_context_param->data, PROCESS_CONTEXTUAL_DATA_NODESPACE);
  process_unit_add_context_param->data2 = (void *)&dfp_vargs[1];
  process_unit_add_context_param->next = process_unit_invoke;
  process_unit_add_context_param->debug = "process_unit_add_context_param";

  process_unit_invoke->type = PROCESS_UNIT_INVOKE;
  process_unit_invoke->data = (void *)&declare_function_pointer_v1;
  process_unit_invoke->data2 = (void *)dfp_vargs;
  process_unit_invoke->next = NULL;
  process_unit_invoke->debug = "process_unit_invoke";

  *process_unit = (void **)process_unit_reset_data_pointer;

  // printf("put_data_ptr-2:%p\n", (char *)**(void ***)((void **)put->data2)[1]);

  // printf("end-mcqck_temp_create_process_declare_function_pointer()\n");
  return 0;
}

int mcqck_temp_create_process_initialize_function(midgeo *process_unit)
{
  const int dfp_varg_count = 3; // nodespace, function_info, code_block

  midgeary dfp_vargs = (midgeary)malloc(sizeof(void *) * (1 + dfp_varg_count));
  allocate_from_intv(&dfp_vargs[0], dfp_varg_count);
  dfp_vargs[1] = NULL;
  dfp_vargs[2] = NULL;
  dfp_vargs[3] = NULL;

  void **mc_dvp;

  declare_and_allocate_anon_struct(process_unit_v1, process_unit_function_name, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_add_context_param, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_get_func_info_set, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_code_block, sizeof_process_unit_v1);
  declare_and_allocate_anon_struct(process_unit_v1, process_unit_invoke, sizeof_process_unit_v1);

  midgeo parameter_data = (midgeo)calloc(sizeof(void *), 200);
  void **ptr_current_data = (void **)malloc(sizeof(void *) * 1);
  *ptr_current_data = (void *)&parameter_data[0];
  char **function_name_storage = (char **)malloc(sizeof(char *) * 1);
  // void **ptr_function_name_storage = (void **)malloc(sizeof(void *) * 1);
  // *ptr_function_name_storage = (void *)function_name_storage;

  process_unit_function_name->type = PROCESS_UNIT_INTERACTION;
  allocate_from_cstringv(&process_unit_function_name->data, "Function Name:");
  process_unit_function_name->data2 = (void *)function_name_storage;
  process_unit_function_name->next = (void *)process_unit_add_context_param;
  process_unit_function_name->debug = "process_unit_function_name";

  process_unit_add_context_param->type = PROCESS_UNIT_SET_CONTEXTUAL_DATA;
  allocate_from_intv(&process_unit_add_context_param->data, PROCESS_CONTEXTUAL_DATA_NODESPACE);
  process_unit_add_context_param->data2 = (void *)&dfp_vargs[1];
  process_unit_add_context_param->next = process_unit_get_func_info_set;
  process_unit_add_context_param->debug = "process_unit_add_context_param";

  process_unit_get_func_info_set->type = PROCESS_UNIT_SET_NODESPACE_FUNCTION_INFO;
  process_unit_get_func_info_set->data = (void *)function_name_storage;
  process_unit_get_func_info_set->data2 = (void *)&dfp_vargs[2];
  process_unit_get_func_info_set->next = (void *)process_unit_code_block;
  process_unit_get_func_info_set->debug = "process_unit_get_func_info_set";

  process_unit_code_block->type = PROCESS_UNIT_INTERACTION;
  allocate_from_cstringv(&process_unit_code_block->data, "Enter Code:\n");
  process_unit_code_block->data2 = (void *)&dfp_vargs[3];
  process_unit_code_block->next = (void *)process_unit_invoke;
  process_unit_code_block->debug = "process_unit_code_block";

  process_unit_invoke->type = PROCESS_UNIT_INVOKE;
  process_unit_invoke->data = (void *)&initialize_function_v1;
  process_unit_invoke->data2 = (void *)dfp_vargs;
  process_unit_invoke->next = NULL;
  process_unit_invoke->debug = "process_unit_invoke";

  *process_unit = (void **)process_unit_function_name;

  // printf("put_data_ptr-2:%p\n", (char *)**(void ***)((void **)put->data2)[1]);

  // printf("end-mcqck_temp_create_process_initialize_function()\n");
  return 0;
}

int submit_user_command(int argc, void **argsv);
int (*mc_dummy_function_pointer)(int, void **);
int mc_main(int argc, const char *const *argv)
{
  int sizeof_void_ptr = sizeof(void *);
  if (sizeof_void_ptr != sizeof(int *) || sizeof_void_ptr != sizeof(char *) || sizeof_void_ptr != sizeof(uint *) || sizeof_void_ptr != sizeof(const char *) ||
      sizeof_void_ptr != sizeof(void **) || sizeof_void_ptr != sizeof(mc_dummy_function_pointer) || sizeof_void_ptr != sizeof(&find_function_info_v1) || sizeof_void_ptr != sizeof(unsigned long))
  {
    printf("pointer sizes aren't equal!!!\n");
    return -1;
  }
  int res;
  void **mc_dvp;

  declare_and_allocate_anon_struct(struct_info_v1, parameter_info_definition_v1, sizeof_struct_info_v1);
  { // TYPE:DEFINITION parameter_info
    parameter_info_definition_v1->struct_id = NULL;
    parameter_info_definition_v1->name = "parameter_info";
    parameter_info_definition_v1->version = 1U;
    parameter_info_definition_v1->declared_mc_name = "mc_parameter_info_v1";
    parameter_info_definition_v1->field_count = 5;
    parameter_info_definition_v1->fields = (void **)calloc(sizeof(void *), parameter_info_definition_v1->field_count);
    parameter_info_definition_v1->sizeof_cstr = "sizeof_parameter_info_v1";

    parameter_info_v1 *field;
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    parameter_info_definition_v1->fields[0] = field;
    field->type_name = "struct_info";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "struct_id";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    parameter_info_definition_v1->fields[1] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "type_name";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    parameter_info_definition_v1->fields[2] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "type_version";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    parameter_info_definition_v1->fields[3] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "type_deref_count";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    parameter_info_definition_v1->fields[4] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "name";
  }

  declare_and_allocate_anon_struct(struct_info_v1, struct_info_definition_v1, sizeof_struct_info_v1);
  { // TYPE:DEFINITION struct_info
    struct_info_definition_v1->struct_id = NULL;
    struct_info_definition_v1->name = "struct_info";
    struct_info_definition_v1->version = 1U;
    parameter_info_definition_v1->declared_mc_name = "mc_struct_info_v1";
    struct_info_definition_v1->field_count = 7;
    struct_info_definition_v1->fields = (void **)calloc(sizeof(void *), struct_info_definition_v1->field_count);
    struct_info_definition_v1->sizeof_cstr = "sizeof_struct_info_v1";

    // FUNCTION_INFO STRUCT INFO
    // #define struct_info_v1              \
//     struct                          \
//     {                               \
//         struct_info *struct_id;                \
//         const char *name;           \
//         unsigned int version;       \
        // const char *declared_mc_name; \
//         unsigned int field_count;   \
//         void **fields;              \
//         const char *sizeof_cstr;    \
//     }
    parameter_info_v1 *field;
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    struct_info_definition_v1->fields[0] = field;
    field->type_name = "struct_info";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "struct_id";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    struct_info_definition_v1->fields[1] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "type_name";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    struct_info_definition_v1->fields[2] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "version";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    struct_info_definition_v1->fields[3] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "declared_mc_name";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    struct_info_definition_v1->fields[4] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "field_count";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    struct_info_definition_v1->fields[5] = field;
    field->type_name = "parameter_info";
    field->type_version = 0U;
    field->type_deref_count = 2;
    field->name = "fields";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    struct_info_definition_v1->fields[6] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "sizeof_cstr";
  }

  declare_and_allocate_anon_struct(struct_info_v1, function_info_definition_v1, sizeof_struct_info_v1);
  { // TYPE:DEFINITION function_info
    function_info_definition_v1->struct_id = NULL;
    function_info_definition_v1->name = "function_info";
    function_info_definition_v1->version = 1U;
    function_info_definition_v1->declared_mc_name = "mc_function_info_v1";
    function_info_definition_v1->field_count = 9;
    function_info_definition_v1->fields = (void **)calloc(sizeof(void *), function_info_definition_v1->field_count);
    function_info_definition_v1->sizeof_cstr = "sizeof_function_info_v1";

    // FUNCTION_INFO STRUCT INFO
    // #define function_info_v1                             \
//     struct                                           \
//     {                                                \
//         void *struct_id;                             \
//         const char *name;                            \
//         unsigned int latest_iteration;               \
//         const char *return_type;                     \
//         unsigned int parameter_count;                \
//         void **parameters;                           \
//         unsigned int variable_parameter_begin_index; \
//         unsigned int struct_usage_count;             \
//         void **struct_usage;                         \
//     }
    parameter_info_v1 *field;
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[0] = field;
    field->type_name = "struct_info";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "struct_id";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[1] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "name";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[2] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "latest_iteration";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[3] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "return_type";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[4] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "parameter_count";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[5] = field;
    field->type_name = "parameter_info";
    field->type_version = 0U;
    field->type_deref_count = 2;
    field->name = "parameters";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[6] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "variable_parameter_begin_index";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[7] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "struct_usage_count";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    function_info_definition_v1->fields[8] = field;
    field->type_name = "struct_id";
    field->type_version = 0U;
    field->type_deref_count = 2;
    field->name = "struct_usage";
  }

  declare_and_allocate_anon_struct(function_info_v1, find_function_info_definition_v1, sizeof_function_info_v1);
  {
    find_function_info_definition_v1->struct_id = NULL;
    find_function_info_definition_v1->name = "find_function_info";
    find_function_info_definition_v1->latest_iteration = 1U;
    find_function_info_definition_v1->return_type = "function_info *";
    find_function_info_definition_v1->parameter_count = 2;
    find_function_info_definition_v1->parameters = (void **)malloc(sizeof(void *) * find_function_info_definition_v1->parameter_count);
    find_function_info_definition_v1->variable_parameter_begin_index = -1;
    find_function_info_definition_v1->struct_usage_count = 0;
    find_function_info_definition_v1->struct_usage = NULL;

    parameter_info_v1 *field;
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    find_function_info_definition_v1->parameters[0] = field;
    field->type_name = "node";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "nodespace";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    find_function_info_definition_v1->parameters[1] = field;
    field->type_name = "char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "function_name";
  }

  declare_and_allocate_anon_struct(function_info_v1, declare_function_pointer_definition_v1, sizeof_function_info_v1);
  {
    declare_function_pointer_definition_v1->struct_id = NULL;
    declare_function_pointer_definition_v1->name = "declare_function_pointer";
    declare_function_pointer_definition_v1->latest_iteration = 1U;
    declare_function_pointer_definition_v1->return_type = "void";
    declare_function_pointer_definition_v1->parameter_count = 2;
    declare_function_pointer_definition_v1->parameters = (void **)malloc(sizeof(void *) * declare_function_pointer_definition_v1->parameter_count);
    declare_function_pointer_definition_v1->variable_parameter_begin_index = -1;
    declare_function_pointer_definition_v1->struct_usage_count = 0;
    declare_function_pointer_definition_v1->struct_usage = NULL;

    parameter_info_v1 *field;
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    declare_function_pointer_definition_v1->parameters[0] = field;
    field->type_name = "node";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "nodespace";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    declare_function_pointer_definition_v1->parameters[1] = field;
    field->type_name = "int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "parameter_count";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    declare_function_pointer_definition_v1->parameters[2] = field;
    field->type_name = "parameter_info";
    field->type_version = 1U;
    field->type_deref_count = 2;
    field->name = "parameters";
    allocate_anon_struct(field, sizeof_parameter_info_v1);
    declare_function_pointer_definition_v1->parameters[3] = field;
    field->type_name = "int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "variable_parameter_begin_index";
  }

  // NODE STRUCT INFO
  // declare_and_allocate_anon_struct(struct_info_v1, node_definition_v1, sizeof_struct_info_v1);
  // node_definition_v1->struct_id.identifier = "struct_info";
  // node_definition_v1->struct_id.version = 1U;
  // node_definition_v1->name = "node";
  // node_definition_v1->version = 1U;
  // node_definition_v1->field_count = 11;
  // node_definition_v1->fields = (void **)calloc(sizeof(void *), 7);
  // node_definition_v1->sizeof_cstr = "sizeof_node_v1";

  // declare_and_allocate_anon_struct(parameter_info_v1, field0, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field0;
  // field0->type.identifier = "const char *";
  // field0->type.version = 0U;
  // field0->name = "name";
  // declare_and_allocate_anon_struct(parameter_info_v1, field1, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field1;
  // field1->type.identifier = "node";
  // field1->type.version = 1U;
  // field1->name = "parent";
  // declare_and_allocate_anon_struct(parameter_info_v1, field2, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field2;
  // field2->type.identifier = "unsigned int";
  // field2->type.version = 0U;
  // field2->name = "functions_alloc";
  // declare_and_allocate_anon_struct(parameter_info_v1, field3, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field3;
  // field3->type.identifier = "unsigned int";
  // field3->type.version = 0U;
  // field3->name = "function_count";
  // declare_and_allocate_anon_struct(parameter_info_v1, field4, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field4;
  // field4->type.identifier = "void **";
  // field4->type.version = 0U;
  // field4->name = "functions";
  // declare_and_allocate_anon_struct(parameter_info_v1, field5, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field5;
  // field5->type.identifier = "unsigned int";
  // field5->type.version = 0U;
  // field5->name = "structs_alloc";
  // declare_and_allocate_anon_struct(parameter_info_v1, field6, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field6;
  // field6->type.identifier = "unsigned int";
  // field6->type.version = 0U;
  // field6->name = "struct_count";
  // declare_and_allocate_anon_struct(parameter_info_v1, field7, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field7;
  // field7->type.identifier = "void **";
  // field7->type.version = 0U;
  // field7->name = "structs";
  // declare_and_allocate_anon_struct(parameter_info_v1, field8, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field8;
  // field8->type.identifier = "unsigned int";
  // field8->type.version = 0U;
  // field8->name = "child_count";
  // declare_and_allocate_anon_struct(parameter_info_v1, field9, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field9;
  // field9->type.identifier = "unsigned int";
  // field9->type.version = 0U;
  // field9->name = "children_alloc";
  // declare_and_allocate_anon_struct(parameter_info_v1, field10, sizeof_parameter_info_v1);
  // node_definition_v1->fields[0] = field10;
  // field10->type.identifier = "void **";
  // field10->type.version = 0U;
  // field10->name = "children";

  // Instantiate: node global;
  declare_and_allocate_anon_struct(node_v1, global, sizeof_node_v1);
  global->name = "global";
  global->parent = NULL;
  global->functions_alloc = 40;
  global->functions = (void **)calloc(sizeof(void *), global->functions_alloc);
  global->function_count = 0;
  global->structs_alloc = 40;
  global->structs = (void **)calloc(sizeof(void *), global->structs_alloc);
  global->struct_count = 0;
  global->children_alloc = 40;
  global->children = (void **)calloc(sizeof(void *), global->children_alloc);
  global->child_count = 0;

  MCcall(append_to_collection(&global->structs, &global->structs_alloc, &global->struct_count, (void *)parameter_info_definition_v1));
  MCcall(append_to_collection(&global->structs, &global->structs_alloc, &global->struct_count, (void *)struct_info_definition_v1));
  MCcall(append_to_collection(&global->structs, &global->structs_alloc, &global->struct_count, (void *)function_info_definition_v1));
  // MCcall(append_to_collection(&global->structs, &global->structs_alloc, &global->struct_count, (void *)node_definition_v1));

  MCcall(append_to_collection(&global->functions, &global->functions_alloc, &global->function_count, (void *)find_function_info_definition_v1));
  clint_process("int (*find_function_info)(int, void **);");
  clint_process("find_function_info = &find_function_info_v1;");
  MCcall(append_to_collection(&global->functions, &global->functions_alloc, &global->function_count, (void *)declare_function_pointer_definition_v1));
  clint_process("int (*declare_function_pointer)(int, void **);");
  clint_process("declare_function_pointer = &declare_function_pointer_v1;");

  // TODO -- Instantiate version 2 of declare_function_pointer (with struct usage)

  // Execute commands
  declare_and_allocate_anon_struct(command_hub_v1, command_hub, sizeof_command_hub_v1);
  command_hub->global_node = global;
  command_hub->nodespace = global;
  command_hub->process_matrix = (midgeo)malloc(sizeof_void_ptr * (2 + 4000));
  command_hub->focused_issue_stack_alloc = 16;
  command_hub->focused_issue_stack = (void **)malloc(sizeof(void *) * command_hub->focused_issue_stack_alloc);
  command_hub->focused_issue_stack_count = 0;
  command_hub->focused_issue_activated = false;
  command_hub->uid_counter = 2000;
  command_hub->active_scripts_alloc = 16;
  command_hub->active_scripts = (void **)malloc(sizeof(void *) * command_hub->active_scripts_alloc);
  command_hub->active_script_count = 0;

  declare_and_allocate_anon_struct(template_collection_v1, template_collection, sizeof_template_collection_v1);
  template_collection->templates_alloc = 400;
  template_collection->template_count = 0;
  template_collection->templates = (void **)malloc(sizeof(void *) * template_collection->templates_alloc);
  command_hub->template_collection = (void *)template_collection;

  midgeo template_process = (midgeo)malloc(sizeof_void_ptr * 2);
  allocate_from_cstringv(&template_process[0], "invoke declare_function_pointer");
  MCcall(mcqck_temp_create_process_declare_function_pointer((midgeo *)&template_process[1]));
  MCcall(append_to_collection(&template_collection->templates, &template_collection->templates_alloc, &template_collection->template_count, (void *)template_process));

  template_process = (midgeo)malloc(sizeof_void_ptr * 2);
  allocate_from_cstringv(&template_process[0], "invoke initialize_function");
  MCcall(mcqck_temp_create_process_initialize_function((midgeo *)&template_process[1]));
  MCcall(append_to_collection(&template_collection->templates, &template_collection->templates_alloc, &template_collection->template_count, (void *)template_process));

  const char *commands =
      "construct_and_attach_child_node|"
      "demo|"
      // ---- BEGIN SEQUENCE ----
      "invoke declare_function_pointer|"
      "demo|"
      // ---- BEGIN SEQUENCE ----
      ".script\n"
      "dcl int space_index\n"
      "nvi int command_length strlen command\n"
      "for i 0 command_length\n"
      "ifs command[i] == ' '\n"
      "set int space_index i\n"
      "brk\n"
      "end\n"
      "end for\n"
      "dcs int command_remaining_length - command_length - space_index 1\n"
      "msi 'char *' function_name + command_remaining_length 1\n"
      "nvk strcpy function_name (+ command + space_index 1)\n"
      "ass function_name[command_remaining_length] '\\0'\n"
      "nvi 'function_info *' finfo find_function_info nodespace function_name\n"
      ""
      "dcs int rind 0\n"
      "dcl 'char *' responses[32]\n"
      ""
      "dcs int linit finfo->parameter_count\n"
      "|"
      "midgequit|";
  "ifs finfo->variable_parameter_begin_index >= 0\n"
  "ass linit finfo->variable_parameter_begin_index\n"
  "end\n"
  "for i 0 linit\n"
  "dcl char provocation[512]\n"
  "nvk strcpy provocation finfo->parameters[i]->name\n"
  "nvk strcat provocation \": \"\n"
  "$ASI responses[rind] provocation\n"
  "ass rind + rind 1\n"
  "end for\n"
  "ifs finfo->variable_parameter_begin_index >= 0\n"
  "dcs int pind finfo->variable_parameter_begin_index\n"
  "whl 1\n"
  "dcl char provocation[512]\n"
  "nvk strcpy provocation finfo->parameters[pind]->name\n"
  "nvk strcat provocation \": \"\n"
  "$ASI responses[rind] provocation\n"
  "ass rind + rind 1\n"
  "ass pind + pind 1\n"
  "ass pind % pind finfo->parameter_count\n"
  "ifs pind < finfo->variable_parameter_begin_index\n"
  "ass pind finfo->variable_parameter_begin_index\n"
  "end\n"
  "end\n"
  "end\n"
  ""
  "nvk $SVL function_name $SYA rind &responses\n"
  "|"
  "midgequit|";
  // void declare_function_pointer(char *function_name, char *return_type, [char *parameter_type, char *parameter_name]...);
  // What is the name of the function?
  "construct_and_attach_child_node|"
  // Return Type:
  "void|"
  // Parameter type:
  "node|"
  // Parameter name:
  "parent|"
  // Parameter type:
  "const char *|"
  // Parameter name:
  "node_name|"
  // Parameter 1 type:
  "end|"
  // ---- SEQUENCE TRANSITION ----
  "invoke initialize_function|"
  // What is the name of the function you wish to initialize?
  "construct_and_attach_child_node|"
  // int construct_and_attach_child_node(node parent, cstr node_name )
  // write code:
  "dec node child\n"
  "cpy 'char *' child->name node_name\n"
  "ass child->parent parent\n"
  "inv void append_to_collection &parent->children &parent->children_alloc &parent->child_count child\n|"
  // ---- SEQUENCE TRANSITION ----
  "invoke construct_and_attach_child_node|"
  "";

  // node_v1 *node;
  // "create function\n"
  // // Uncertain response: Type another command or type demobegin to demostrate it
  // "demobegin\n"
  // "demoend\n"
  // "create node\n"
  // "construct_and_attach_child_node\n";
  printf("\n:> ");
  int n = strlen(commands);
  int s = 0;
  char cstr[2048];
  void *vargs[12]; // TODO -- count
  for (int i = 0; i < n; ++i)
  {
    if (commands[i] != '|')
      continue;
    strncpy(cstr, commands + s, i - s);
    cstr[i - s] = '\0';
    s = i + 1;

    vargs[0] = (void *)command_hub;
    vargs[4] = (void *)cstr;

    if (!strcmp(cstr, "midgequit"))
    {
      printf("midgequit\n");
      break;
    }

    // printf("%s\n", cstr);
    MCcall(submit_user_command(12, vargs));

    // if (*(int *)interaction_context[0] == INTERACTION_CONTEXT_BROKEN)
    // {
    //   printf("\nUNHANDLED_COMMAND_SEQUENCE\n");
    //   break;
    // }
    // if (reply != NULL)
    // {
    //   printf("%s", reply);
    // }
  }

  printf("\n\n</midge_core>\n");
  return 0;
}

int process_matrix_register_action(void *p_command_hub, void *p_process_action);
int command_hub_submit_process_action(void *p_command_hub, void *p_process_action);
int command_hub_process_outstanding_actions(void *p_command_hub);
int systems_process_command_hub_issues(void *p_command_hub, void **p_response_action);
int systems_process_command_hub_scripts(void *p_command_hub, void **p_response_action);

int submit_user_command(int argc, void **argsv)
{
  void **mc_dvp;
  int res;

  declare_and_assign_anon_struct(command_hub_v1, command_hub, argsv[0]);
  char *command = (char *)argsv[4];

  // Format the User Response as an action
  declare_and_allocate_anon_struct(process_action_v1, process_action, sizeof_process_action_v1);
  if (command_hub->focused_issue_stack_count == 0)
  {
    process_action->sequence_uid = command_hub->uid_counter;
    ++command_hub->uid_counter;

    process_action->type = PROCESS_ACTION_USER_UNPROVOKED_COMMAND;
    allocate_and_copy_cstr(process_action->dialogue, command);
    process_action->history = NULL;
  }
  else
  {
    // Pop the focused issue from the stack
    declare_and_assign_anon_struct(process_action_v1, focused_issue, command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1]);
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    switch (focused_issue->type)
    {
    case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
    {
      process_action->sequence_uid = focused_issue->sequence_uid;
      process_action->type = PROCESS_ACTION_USER_UNRESOLVED_RESPONSE;

      allocate_and_copy_cstr(process_action->dialogue, command);
      process_action->history = focused_issue;
    }
    break;
    case PROCESS_ACTION_PM_DEMO_INITIATION:
    {
      process_action->sequence_uid = focused_issue->sequence_uid;
      process_action->type = PROCESS_ACTION_USER_UNPROVOKED_COMMAND;

      allocate_and_copy_cstr(process_action->dialogue, command);
      process_action->history = focused_issue;
    }
    break;
    case PROCESS_ACTION_SCRIPT_QUERY:
    {
      process_action->sequence_uid = focused_issue->sequence_uid;
      process_action->type = PROCESS_ACTION_USER_SCRIPT_RESPONSE;

      allocate_and_copy_cstr(process_action->dialogue, command);
      process_action->history = focused_issue;
    }
    break;
    default:
      MCerror(-828, "Unhandled PM-query-type:%i", focused_issue->type);
    }
  }

  // Process command and any/all system responses
  while (1)
  {
    // Affect the command hub
    MCcall(process_matrix_register_action(command_hub, process_action));
    MCcall(command_hub_submit_process_action(command_hub, process_action));

    // Process the action
    MCcall(command_hub_process_outstanding_actions(command_hub));

    // Formulate system responses
    void *p_response_action;
    MCcall(systems_process_command_hub_scripts(command_hub, &p_response_action));
    if (!p_response_action)
    {
      MCcall(systems_process_command_hub_issues(command_hub, &p_response_action));
    }
    if (!p_response_action)
      break;
    assign_anon_struct(process_action, p_response_action);
  }

  // Send control back to the user
  return 0;
}

int process_matrix_register_action(void *p_command_hub, void *p_process_action)
{
  return 0;
}

int command_hub_submit_process_action(void *p_command_hub, void *p_process_action)
{
  void **mc_dvp;
  declare_and_assign_anon_struct(command_hub_v1, command_hub, p_command_hub);
  declare_and_assign_anon_struct(process_action_v1, process_action, p_process_action);

  // printf("submitting:%i\n", process_action->type);
  append_to_collection(&command_hub->focused_issue_stack, &command_hub->focused_issue_stack_alloc, &command_hub->focused_issue_stack_count,
                       process_action);
  command_hub->focused_issue_activated = false;

  return 0;
}

int command_hub_process_outstanding_actions(void *p_command_hub)
{
  void **mc_dvp;
  declare_and_assign_anon_struct(command_hub_v1, command_hub, p_command_hub);
  if (command_hub->focused_issue_activated || command_hub->focused_issue_stack_count == 0)
    return 0;

  // Process the foremost focused issue
  declare_and_assign_anon_struct(process_action_v1, focused_issue, command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1]);

  switch (focused_issue->type)
  {
  case PROCESS_ACTION_USER_UNPROVOKED_COMMAND:
  case PROCESS_ACTION_USER_UNRESOLVED_RESPONSE:
  case PROCESS_ACTION_USER_SCRIPT_ENTRY:
  {
    // Print to terminal
    printf("%s\n", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;
  }
  break;
  case PROCESS_ACTION_USER_SCRIPT_RESPONSE:
  {
    // Print to terminal
    printf("%s\n", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;
  }
  break;
  case PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS:
  {
    if (focused_issue->dialogue != NULL)
    {
      // Print to terminal
      printf("%s\n", focused_issue->dialogue);
      command_hub->focused_issue_activated = true;
    }
  }
  break;
  case PROCESS_ACTION_SCRIPT_QUERY:
  {
    // Print to terminal
    printf("%s\n", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;
  }
  break;
  case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
  case PROCESS_ACTION_PM_DEMO_INITIATION:
  {
    // Print to terminal
    printf("%s\n", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;

    // Indicate user response
    printf(":> ");
  }
  break;
  default:
    MCerror(-151, "UnhandledType:%i", focused_issue->type)
  }

  return 0;
}

int systems_process_command_hub_scripts(void *p_command_hub, void **p_response_action)
{
  *p_response_action = NULL;
  void **mc_dvp;
  int res;
  declare_and_assign_anon_struct(command_hub_v1, command_hub, p_command_hub);

  if (command_hub->active_script_count == 0)
    return 0;

  for (int i = 0; i < command_hub->active_script_count; ++i)
  {
    declare_and_assign_anon_struct(script_v1, script, command_hub->active_scripts[i]);

    if (script->awaiting_data_set_index >= 0)
      continue;

    // printf("script->sequence_uid=%u\n", script->sequence_uid);
    if (script->segments_complete > script->segment_count)
    {
      // Cleanup
      MCerror(5482, "TODO");
    }
    // Execute the script
    char buf[1024];
    strcpy(buf, SCRIPT_NAME_PREFIX);

    char **output = NULL;
    printf("script entered: %u: %i / %i\n", script->sequence_uid, script->segments_complete, script->segment_count);
    sprintf(buf, "{\n"
                 "void *p_script = (void *)%p;\n"
                 "%s%u(p_script);\n"
                 "}",
            script, SCRIPT_NAME_PREFIX, script->script_uid);
    clint_process(buf);
    printf("script exited: %u: %i / %i\n", script->sequence_uid, script->segments_complete, script->segment_count);

    if (script->segments_complete >= script->segment_count)
    {
      // Cleanup & continue

      if (script->arguments[2])
        free(script->arguments[2]);
      free(script->arguments);

      // Locals should be freed at end of script
      for (int j = 0; j < script->local_count; ++j)
        if (script->locals[j])
          free(script->locals[j]);
      free(script->locals);

      if (script->response)
        free(script->response);

      continue;
    }

    if (!script->response)
    {
      MCerror(542852, "TODO: response=%s", script->response);
    }

    // Process the Focused Issue
    declare_and_assign_anon_struct(process_action_v1, focused_issue, command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1]);

    if (focused_issue->type != PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS)
    {
      // MCerror(42425, "TODO"); // Cleanup
      remove_from_collection(&command_hub->active_scripts, &command_hub->active_scripts_alloc, &command_hub->active_script_count, i);
      --i;
      continue;
    }

    // Pop the focused issue from the stack
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    // Begin Query
    declare_and_allocate_anon_struct(process_action_v1, script_query, sizeof_process_action_v1);
    script_query->sequence_uid = focused_issue->sequence_uid;
    script_query->type = PROCESS_ACTION_SCRIPT_QUERY;
    allocate_and_copy_cstr(script_query->dialogue, script->response);
    script_query->history = (void *)focused_issue;

    *p_response_action = script_query;
  }
  return 0;
}

int systems_process_command_hub_issues(void *p_command_hub, void **p_response_action)
{
  *p_response_action = NULL;
  int res;
  void **mc_dvp;
  declare_and_assign_anon_struct(command_hub_v1, command_hub, p_command_hub);

  if (command_hub->focused_issue_stack_count == 0)
    return 0;

  // Process the Focused Issue
  declare_and_assign_anon_struct(process_action_v1, focused_issue, command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1]);
  declare_and_assign_anon_struct(template_collection_v1, template_collection, command_hub->template_collection);

  // Templates first
  switch (focused_issue->type)
  {
  case PROCESS_ACTION_SCRIPT_QUERY:
    // case PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS:
    {
      //   // Do not process these commands
      //   // Send to other systems
      return 0;
    }
  case PROCESS_ACTION_USER_SCRIPT_RESPONSE:
  {
    // Pop the focused issue from the stack
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    // Obtain the script that queried the response
    script_v1 *script = NULL;
    for (int i = 0; i < command_hub->active_script_count; ++i)
    {
      assign_anon_struct(script, command_hub->active_scripts[i]);
      if (focused_issue->sequence_uid == script->sequence_uid)
        break;
      script = NULL;
    }
    if (script == NULL)
    {
      MCerror(8284, "");
    }

    // Set the requested data on the script
    allocate_from_cstringv(&script->locals[script->awaiting_data_set_index], focused_issue->dialogue);
    script->awaiting_data_set_index = -1;

    // Set corresponding issue
    declare_and_allocate_anon_struct(process_action_v1, script_issue, sizeof_process_action_v1);
    script_issue->sequence_uid = focused_issue->sequence_uid;
    script_issue->type = PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS;
    script_issue->history = (void *)focused_issue;
    script_issue->dialogue = NULL;

    *p_response_action = script_issue;

    return 0;
  }
  case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
  case PROCESS_ACTION_PM_DEMO_INITIATION:
  {
    // Do not process these commands
    // Send back to user
    return 0;
  }
  case PROCESS_ACTION_USER_UNPROVOKED_COMMAND:
  {
    // Pop the focused issue from the stack
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    // Script Execution Request
    if (!strncmp(focused_issue->dialogue, ".script", 7))
    {
      // Create the script
      declare_and_allocate_anon_struct(script_v1, script, sizeof_script_v1);
      // script->execution_state = SCRIPT_EXECUTION_STATE_INITIAL;
      // script->next_statement_index = -1;
      script->sequence_uid = focused_issue->sequence_uid;
      ++command_hub->uid_counter;
      script->script_uid = command_hub->uid_counter;
      script->segments_complete = 0;
      script->awaiting_data_set_index = -1;
      script->arguments = (void **)malloc(sizeof(void *) * 3);
      script->response = NULL;

      // -- Submit contextual arguments

      // -- -- Global
      script->arguments[0] = command_hub->global_node;
      // assign_anon_struct(variable, malloc(sizeof(void *) * 2));
      // allocate_and_copy_cstr(variable->name, "global_node");
      // variable->value = command_hub->global_node;
      // append_to_collection(&script->arguments, &script->arguments_alloc, &script->argument_count, variable);

      // -- -- Nodespace
      script->arguments[1] = command_hub->nodespace;
      // assign_anon_struct(variable, malloc(sizeof(void *) * 2));
      // allocate_and_copy_cstr(variable->name, "nodespace");
      // variable->value = command_hub->nodespace;
      // append_to_collection(&script->arguments, &script->arguments_alloc, &script->argument_count, variable);

      // -- -- Previous User Command
      if (focused_issue->history)
      {
        declare_and_assign_anon_struct(process_action_v1, previous_issue, focused_issue->history);
        switch (previous_issue->type)
        {
        case PROCESS_ACTION_PM_DEMO_INITIATION:
        {
          script->arguments[2] = previous_issue->data.demonstrated_command;
          // assign_anon_struct(variable, malloc(sizeof(void *) * 2));
          // allocate_and_copy_cstr(variable->name, "command");
          // allocate_from_cstringv(&variable->value, previous_issue->data.demonstrated_command);

          // append_to_collection(&script->arguments, &script->arguments_alloc, &script->argument_count, variable);
        }
        break;
        default:
        {
          MCerror(-825, "unhandled type:%i", previous_issue->type);
        }
        }
      }
      else
      {
        script->arguments[2] = NULL;
        // assign_anon_struct(variable, malloc(sizeof(void *) * 2));
        // allocate_and_copy_cstr(variable->name, "command");
        // allocate_from_cstringv(&variable->value, "null");

        // append_to_collection(&script->arguments, &script->arguments_alloc, &script->argument_count, variable);
      }

      // -- Parse statements
      MCcall(mcqck_translate_script_code(command_hub->nodespace, (void *)script, focused_issue->dialogue));

      // Set corresponding issue
      declare_and_allocate_anon_struct(process_action_v1, script_issue, sizeof_process_action_v1);
      script_issue->sequence_uid = focused_issue->sequence_uid;
      script_issue->type = PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS;
      script_issue->history = (void *)focused_issue;
      allocate_and_copy_cstr(script_issue->dialogue, "Initiating Script...");

      // Submit the script
      append_to_collection(&command_hub->active_scripts, &command_hub->active_scripts_alloc, &command_hub->active_script_count, script);

      // Set as response action
      *p_response_action = (void *)script_issue;
      return 0;
    }

    // Attempt to find the action the user is commanding

    // -- Find a suggestion from the process matrix
    // printf("##########################################\n");
    // printf("Begin Template Process:%s\n", process[0]);
    // MCcall(handle_process(argc, argsv));

    // -- Couldn't find one
    // -- Request direction
    declare_and_allocate_anon_struct(process_action_v1, request_guidance_issue, sizeof_process_action_v1);
    request_guidance_issue->type = PROCESS_ACTION_PM_UNRESOLVED_COMMAND;
    request_guidance_issue->sequence_uid = focused_issue->sequence_uid;
    request_guidance_issue->history = (void *)focused_issue;
    allocate_and_copy_cstr(request_guidance_issue->dialogue, "Unresolved Command: type 'demo' to demonstrate.");

    *p_response_action = (void *)request_guidance_issue;

    return 0;
  }
  case PROCESS_ACTION_USER_UNRESOLVED_RESPONSE:
  {
    // Pop the focused issue from the stack
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    if (!strcmp(focused_issue->dialogue, "demo"))
    {
      ++command_hub->uid_counter;
      focused_issue->sequence_uid = command_hub->uid_counter;

      // Begin demonstration
      declare_and_allocate_anon_struct(process_action_v1, demo_issue, sizeof_process_action_v1);
      demo_issue->sequence_uid = focused_issue->sequence_uid;
      demo_issue->type = PROCESS_ACTION_PM_DEMO_INITIATION;
      demo_issue->history = (void *)focused_issue;

      // Obtain the command being demonstrated
      {
        if (focused_issue->history == NULL)
        {
          MCerror(-852, "demo historical issue shouldn't be NULL");
        }
        declare_and_assign_anon_struct(process_action_v1, historical_issue, focused_issue->history);
        if (historical_issue->history == NULL)
        {
          MCerror(-853, "demo historical issue 2 shouldn't be NULL");
        }
        assign_anon_struct(historical_issue, historical_issue->history);

        allocate_and_copy_cstr(demo_issue->data.demonstrated_command, historical_issue->dialogue);
      }
      const char *DIALOGUE_PREFIX = "Demonstrating '";
      const char *DIALOGUE_POSTFIX = "' (type 'end' to end).";
      demo_issue->dialogue = (char *)malloc(sizeof(char) * (strlen(demo_issue->data.demonstrated_command) + strlen(DIALOGUE_PREFIX) + strlen(DIALOGUE_POSTFIX) + 1));
      strcpy(demo_issue->dialogue, DIALOGUE_PREFIX);
      strcat(demo_issue->dialogue, demo_issue->data.demonstrated_command);
      strcat(demo_issue->dialogue, DIALOGUE_POSTFIX);

      // Return the original command to the stack
      if (focused_issue->history == NULL)
      {
        MCerror(-882, "demo -1 should not be missing");
      }
      declare_and_assign_anon_struct(process_action_v1, unresolved_command, focused_issue->history);
      if (unresolved_command->history == NULL)
      {
        MCerror(-883, "demo -2 should not be missing");
      }
      append_to_collection(&command_hub->focused_issue_stack, &command_hub->focused_issue_stack_alloc, &command_hub->focused_issue_stack_count,
                           unresolved_command->history);

      // Add a demonstration process on top of the focused issue stack
      *p_response_action = (void *)demo_issue;
    }
    else
    {
      // Look at resubmitting this as an unprovoked command
      MCerror(-842, "TODO");
    }
    return 0;
  }
  default:
    if (focused_issue->type == 8)
    {
      MCerror(-241, "Temp got to end of script:%i", focused_issue->type)
    }
    MCerror(-241, "UnhandledType:%i", focused_issue->type)
  }

  return -242;
}

// return 0;

// int res = 0;
// *reply = NULL;

// if (*(int *)interaction_context[0] == INTERACTION_CONTEXT_BROKEN)
// {
//   return -1;
// }

// if (*(int *)interaction_context[0] == INTERACTION_CONTEXT_BLANK)
// {
//   // No real context
//   // User Submits Commmand without prompt
//   int n = *(int *)template_collection[1];
//   for (int i = 0; i < n; ++i)
//   {
//     midgeo process = (midgeo)template_collection[2 + i];
//     if (!strcmp((char *)process[0], command))
//     {
//       *(int *)interaction_context[0] = INTERACTION_CONTEXT_PROCESS;
//       *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//       interaction_context[1] = process;
//       interaction_context[2] = process[1];

//       printf("##########################################\n");
//       printf("Begin Process:%s\n", process[0]);
//       MCcall(handle_process(argc, argsv));
//       return 0;
//     }
//   }
//   // else if (!strcmp("demobegin", command))
//   // {
//   //     *reply = "[Demo:\n";
//   //     strcpy(*reply, "[Demo: \"");

//   //     history[0] strcat(*reply, (char *)history) return 0;
//   // }

//   *(int *)interaction_context[0] = INTERACTION_CONTEXT_BROKEN;
//   return 0;
// }
// else if (*(int *)interaction_context[0] == INTERACTION_CONTEXT_PROCESS)
// {
//   declare_and_assign_anon_struct(process_unit_v1, process_unit, interaction_context[2]);
//   *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_POSTREPLY;
//   // printf("process_unit:%u\n", process_unit->type);

//   // Handle reaction from previous unit
//   MCcall(handle_process(argc, argsv));
// }

// return res;
// }

// int handle_process(int argc, void **argsv)
// {
//   void **mc_dvp;
//   int res;

//   // Arguments
//   midgeo process_matrix = (midgeo)argsv[0];
//   midgeo template_collection = (midgeo)argsv[1];
//   midgeo interaction_context = (midgeo)argsv[2];
//   declare_and_assign_anon_struct(node_v1, nodespace, argsv[3]);
//   // History:
//   // [0] -- linked list cstr : most recent set
//   char *command = (char *)argsv[4];
//   char **reply = (char **)argsv[5];

//   declare_and_assign_anon_struct(process_unit_v1, process_unit, interaction_context[2]);
//   printf("handle_process()\n");

//   int loop = 1;
//   while (loop)
//   {
//     printf("icontext:%i process_unit:%i:%s\n", *(int *)interaction_context[3], process_unit->type, process_unit->debug);
//     // declare_and_assign_anon_struct(process_unit_v1, put, put_data);
//     // printf("ptr_current_data_points_to0:%p\n", *((void **)put->data2));

//     if (*(int *)interaction_context[3] == INTERACTION_PROCESS_STATE_INITIAL)
//     {
//       // Perform instigation for next process_unit
//       if (process_unit == NULL)
//       {
//         *(int *)interaction_context[0] = INTERACTION_CONTEXT_BLANK;
//       }

//       // process next unit
//       switch (process_unit->type)
//       {
//       case PROCESS_UNIT_INTERACTION_INCR_DPTR:
//       case PROCESS_UNIT_INTERACTION:
//       {
//         *reply = (char *)process_unit->data;

//         loop = 0;
//       }
//       break;
//       case PROCESS_UNIT_BRANCHING_INTERACTION:
//       {
//         *reply = (char *)process_unit->data;
//         loop = 0;
//       }
//       break;
//       case PROCESS_UNIT_INVOKE:
//       {
//         // printf("ptr_current_data_points_to10:%p\n", *((void **)put->data2));
//         // No provocation
//         *reply = NULL;

//         int (*fptr)(int, void **) = (int (*)(int, void **))process_unit->data;
//         midgeary data = (midgeary)process_unit->data2;
//         MCcall(fptr(*(int *)data[0], &data[1]));

//         interaction_context[2] = process_unit->next;
//         assign_anon_struct(process_unit, process_unit->next);
//         *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//         // printf("ptr_current_data_points_to11:%p\n", *((void **)put->data2));
//       }
//       break;
//       case PROCESS_UNIT_SET_CONTEXTUAL_DATA:
//       {
//         // printf("gethere-0\n");
//         // No provocation
//         *reply = NULL;
//         switch (*(int *)process_unit->data)
//         {
//         case PROCESS_CONTEXTUAL_DATA_NODESPACE:
//         {
//           // printf("gethere-1\n");
//           void **p_data = (void **)process_unit->data2;
//           *p_data = (void *)nodespace;
//           // printf("gethere-2\n");
//         }
//         break;

//         default:
//           allocate_from_intv(&interaction_context[0], INTERACTION_CONTEXT_BROKEN);
//           printf("Unhandled process_unit type:%i\n", process_unit->type);
//           return -33;
//         }

//         interaction_context[2] = process_unit->next;
//         assign_anon_struct(process_unit, process_unit->next);
//         *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//         // printf("gethere-3\n");
//       }
//       break;
//       case PROCESS_UNIT_SET_NODESPACE_FUNCTION_INFO:
//       {
//         // Find the function_info in the current nodespace
//         MCcall(find_function_info(nodespace, *(char **)process_unit->data, (void **)process_unit->data2));

//         // No provocation
//         *reply = NULL;

//         interaction_context[2] = process_unit->next;
//         assign_anon_struct(process_unit, process_unit->next);
//         *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//       }
//       break;

//       default:
//         allocate_from_intv(&interaction_context[0], INTERACTION_CONTEXT_BROKEN);
//         printf("Unhandled process_unit type:%i\n", process_unit->type);
//         return -7;
//       }
//     }
//     else if (*(int *)interaction_context[3] == INTERACTION_PROCESS_STATE_POSTREPLY)
//     {
//       switch (process_unit->type)
//       {
//       case PROCESS_UNIT_INTERACTION_INCR_DPTR:
//       {
//         // printf("ptr_current_data_points_to2:%p\n", *((void **)put->data2));
//         // printf("pr_prm_pointer:%p\n", (char *)*process_parameter_data);
//         // printf("data2_pointer:%p\n", (char *)**(void ***)process_unit->data2);
//         // printf("put_data_ptr-2:%p\n", (char *)**(void ***)((void **)put->data2)[1]);

//         char *str_allocation = (char *)malloc(sizeof(char) * (strlen(command) + 1));
//         strcpy(str_allocation, command);
//         **(void ***)process_unit->data2 = str_allocation;
//         // printf("stv:%s set to %p\n", (char *)**(void ***)process_unit->data2, *(void **)process_unit->data2);
//         MCcall(increment_pointer(1, &process_unit->data2));

//         // printf("ptr_current_data_points_to3:%p\n", *((void **)put->data2));
//         // printf("pr_prm_pointer:%p\n", (char *)*process_parameter_data);
//         // printf("data2_pointer:%p\n", (char *)**((void ***)process_unit->data2));
//         // printf("put_data_ptr-2:%p\n", (char *)**(void ***)((void **)put->data2)[1]);

//         interaction_context[2] = process_unit->next;
//         assign_anon_struct(process_unit, process_unit->next);
//         *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//       }
//       break;
//       case PROCESS_UNIT_INTERACTION:
//       {
//         *(char **)process_unit->data2 = (char *)malloc(sizeof(char) * (strlen(command) + 1));
//         strcpy(*(char **)process_unit->data2, command);
//         // printf("PROCCESSUINT copied:%s\n", *(char **)process_unit->data2);

//         interaction_context[2] = process_unit->next;
//         assign_anon_struct(process_unit, process_unit->next);
//         *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//       }
//       break;
//       case PROCESS_UNIT_BRANCHING_INTERACTION:
//       {
//         // printf("pub-0\n");
//         midgeo branch_ary = (midgeo)process_unit->next;
//         int branch_ary_size = *(int *)process_unit->data2;

//         for (int i = 0; i < branch_ary_size; ++i)
//         {
//           // printf("pub-1:%i\n", i);
//           declare_and_assign_anon_struct(branch_unit_v1, branch, branch_ary[i]);
//           if (branch->match != NULL && strcmp(branch->match, command))
//             continue;

//           switch (branch->type)
//           {
//           case BRANCHING_INTERACTION_IGNORE_DATA:
//           {
//             interaction_context[2] = branch->next;
//             assign_anon_struct(process_unit, branch->next);
//             *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//           }
//           break;
//           case BRANCHING_INTERACTION_INCR_DPTR:
//           {
//             char *str_allocation = (char *)malloc(sizeof(char) * (strlen(command) + 1));
//             strcpy(str_allocation, command);
//             **(void ***)branch->data = str_allocation;
//             // printf("stv:%s set to %p\n", (char *)**(void ***)branch->data, *(void **)branch->data);
//             MCcall(increment_pointer(1, &branch->data));

//             // printf("ptr_current_data_points_to5:%p\n", *((void **)put->data2));
//             // strcpy((char *)*((void **)branch->data), command);
//             // MCcall(increment_pointer(1, &branch->data));
//             // printf("ptr_current_data_points_to6:%p\n", *((void **)put->data2));

//             interaction_context[2] = branch->next;
//             assign_anon_struct(process_unit, branch->next);
//             *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//           }
//           break;
//           case BRANCHING_INTERACTION_INVOKE:
//           {
//             int (*fptr)(int, void **) = (int (*)(int, void **))process_unit->data;
//             midgeary data = (midgeary)branch->data;
//             MCcall(fptr(*(int *)data[0], &data[1]));

//             interaction_context[2] = branch->next;
//             assign_anon_struct(process_unit, branch->next);
//             *(int *)interaction_context[3] = INTERACTION_PROCESS_STATE_INITIAL;
//           }
//           break;

//           default:
//             allocate_from_intv(&interaction_context[0], INTERACTION_CONTEXT_BROKEN);
//             printf("Unhandled process_unit:branch type:%i\n", branch->type);
//             return -11;
//           }
//           break;
//         }
//       }
//       break;
//       case PROCESS_UNIT_SET_NODESPACE_FUNCTION_INFO:
//       case PROCESS_UNIT_SET_CONTEXTUAL_DATA:
//       case PROCESS_UNIT_INVOKE:
//       {
//         printf("shouldn't get here: handle_process():PROCESS_UNIT_INVOKE\n");
//         return -24;
//       }
//       break;

//       default:
//         allocate_from_intv(&interaction_context[0], INTERACTION_CONTEXT_BROKEN);
//         printf("Unhandled process_unit type:%i\n", process_unit->type);
//         return -5;
//       }
//     }
//     else
//     {
//       printf("unhandled interaction_process_state:%i\n", *(int *)interaction_context[3]);
//       return -19;
//     }

//     if (process_unit == NULL)
//     {
//       *(int *)interaction_context[0] = INTERACTION_CONTEXT_BLANK;
//       break;
//     }
//   }

//   printf("end-handle_process()\n");
//   return 0;
// }