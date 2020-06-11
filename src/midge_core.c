/* midge_core.c */

#include "midge_core.h"

int print_struct_id(int argc, void **argv)
{
  midgeo struct_id = (midgeo)argv;

  printf("[struct:%s (version:%i)]\n", (char *)struct_id[0], *(uint *)struct_id[1]);

  return 0;
}

int print_parse_error(const char *const text, int index, const char *const function_name, const char *section_id)
{
  char buf[30];
  int off = 6 - index;
  for (int i = 0; i < 13; ++i) {
    if (index - 13 + i < 0)
      buf[i] = ' ';
    else
      buf[i] = text[index - 13 + i];
  }
  buf[13] = '|';
  buf[14] = text[index];
  buf[15] = '|';
  char eof = text[index] == '\0';
  for (int i = 0; i < 13; ++i) {
    if (eof)
      buf[16 + i] = ' ';
    else {
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
  for (int i = 0;; ++i) {
    if (sequence[i] == '\0') {
      *index += i;
      return 0;
    }
    else if (text[*index + i] == '\0') {
      return -1;
    }
    else if (sequence[i] != text[*index + i]) {
      printf("!parse_past() expected:'%c' was:'%c'\n", sequence[i], text[*index + i]);
      return 1 + i;
    }
  }
}

int parse_past_number(const char *text, int *index, char **output)
{
  for (int i = *index;; ++i) {
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
  for (int i = *index + 1;; ++i) {
    if (!prev_escape && text[i] == '\'') {
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

int mc_parse_past_literal_string(const char *text, int *index, char **output)
{
  if (text[*index] != '"')
    return -2482;

  bool prev_escape = false;
  for (int j = *index + 1;; ++j) {
    if (!prev_escape && text[j] == '"') {
      ++j;
      *output = (char *)malloc(sizeof(char) * (j - *index + 1));
      strncpy(*output, text + *index, j - *index);
      (*output)[j - *index] = '\0';
      *index = j;
      return 0;
    }

    prev_escape = (text[j] == '\\');
  }

  return 0;
}

int parse_past_identifier(const char *text, int *index, char **identifier, bool include_member_access, bool include_referencing)
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
  if (text[*index] == '\'') {
    // Obtain the type in the literal string
    for (int i = *index + 1;; ++i) {
      if (text[i] == '\'') {
        *identifier = (char *)malloc(sizeof(char) * (i - *index));
        strncpy(*identifier, text + *index + 1, i - *index - 1);
        (*identifier)[i - *index - 1] = '\0';
        *index = i + 1;
        return 0;
      }
      if (text[i] == '\0') {
        MCerror(-25258, "SHOULDN'T");
      }
    }
  }
  else {
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

int init_void_collection_v1(mc_void_collection_v1 **output)
{
  *output = (mc_void_collection_v1 *)malloc(sizeof(mc_void_collection_v1));
  (*output)->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
  (*output)->struct_id->identifier = "void_collection";
  (*output)->struct_id->version = 1U;
  (*output)->count = 0;
  (*output)->allocated = 2;
  (*output)->items = (void **)malloc(sizeof(void *) * (*output)->allocated);

  return 0;
}

int get_process_contextual_data(mc_process_action_v1 *contextual_action, const char *const key, void **value)
{
  *value = NULL;
  // printf("entered gpcd()\n");
  while (contextual_action) {
    // printf("contextual_action(%u)->contextual_data_count:%i\n", contextual_action->object_uid,
    //        contextual_action->contextual_data_count);
    // printf("here31\n");
    for (int i = 0; i < contextual_action->contextual_data->count; ++i) {
      // printf("here34\n");
      // printf("comparing %s<>%s:%s\n", key, ((mc_key_value_pair_v1 *)contextual_action->contextual_data[i])->key,
      //        (char *)((mc_key_value_pair_v1 *)contextual_action->contextual_data[i])->value);
      if (!strcmp(key, ((mc_key_value_pair_v1 *)contextual_action->contextual_data->items[i])->key)) {
        *value = ((mc_key_value_pair_v1 *)contextual_action->contextual_data->items[i])->value;
        // printf("here35\n");
        return 0;
      }
      // printf("here36\n");
    }

    if (contextual_action->contextual_issue == NULL) {
      // printf("gpcd-contextual_issue=NULL; break\n");
      break;
    }

    if (contextual_action->previous_issue != NULL) {
      // printf("gpcd-contextual_action = contextual_action->previous_issue;\n");
      contextual_action = contextual_action->previous_issue;
    }
    else {
      // printf("gpcd-contextual_action = contextual_action->contextual_issue;\n");
      contextual_action = contextual_action->contextual_issue;
    }
  }
  // printf("left gpcd()\n");
  return 0;
}

int append_to_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count, void *item)
{
  if (*collection_count + 1 > *collection_alloc) {
    unsigned int realloc_amount = *collection_alloc + 8 + *collection_alloc / 3;
    // printf("reallocate collection size %i->%i\n", *collection_alloc, realloc_amount);
    void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
    if (new_collection == NULL) {
      MCerror(304, "append_to_collection malloc error");
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

int find_struct_info(void *vp_nodespace, const char *const struct_name, void **struct_info)
{
  mc_node_v1 *node = (mc_node_v1 *)vp_nodespace;
  // declare_and_assign_anon_struct(node_v1, node, vp_nodespace);

  *struct_info = NULL;
  for (int i = 0; i < node->struct_count; ++i) {
    if (strcmp(node->structs[i]->name, struct_name))
      continue;

    // Matches
    *struct_info = (void *)node->structs[i];
    // printf("find_struct_info:set with '%s'\n", finfo->name);
    return 0;
  }

  // if (node->parent)
  // {
  //   // Search in the parent nodespace
  //   MCcall(find_struct_info(node->parent, struct_name, struct_info));
  // }

  // printf("find_struct_info: '%s' could not be found in nodespace=%s!\n", struct_name, node->name);
  return 0;
}

int release_process_action(mc_process_action_v1 *process_action) { free(process_action); return 0; }
// int mcqck_translate_script_statement(void *nodespace, char *script_statement, char **translated_statement)
// {
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
//   //     sprintf(buf + strlen(buf), "%s = (char *)malloc(sizeof(char) * (strlen(%s) + 1));\n", set_identity,
//   value_identity);
//   //     strcat(buf, TAB);
//   //     sprintf(buf + strlen(buf), "strcpy((char *)%s, %s);\n", set_identity, value_identity);
//   //   }
//   //   else
//   //   {
//   //     printf("instantiate_function_V1:>[cpy]>unhandled type identity:%s\n", type_identity);
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

//       sprintf(buf + strlen(buf), "declare_and_allocate_anon_struct(%s_v%u, %s, (sizeof(void *) * %u));\n",
//       struct_info->name,
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
  if (strlen(*cstr) + n + 1 >= *allocated_size) {
    unsigned int new_allocated_size = n + *allocated_size + 16 + (n + *allocated_size) / 10;
    // printf("atc-3 : new_allocated_size:%u\n", new_allocated_size);
    char *newptr = (char *)malloc(sizeof(char) * new_allocated_size);
    // printf("atc-4\n");
    memcpy(newptr, *cstr, sizeof(char) * *allocated_size);
    // printf("atc-5\n");
    free(*cstr);
    // printf("atc-6\n");
    *cstr = newptr;
    // printf("atc-7\n");
    *allocated_size = new_allocated_size;
    // printf("atc-8\n");
  }

  // printf("atc-9\n");
  strcat(*cstr, extra);
  // printf("atc-10\n");

  return 0;
}

int mcqck_generate_script_local(void *nodespace, void ***local_index, unsigned int *local_indexes_alloc,
                                unsigned int *local_indexes_count, void *p_script, char *buf, int scope_depth,
                                char *type_identifier, char *var_name)
{
  mc_script_v1 *script = (mc_script_v1 *)p_script;

  // Reuse out-of-scope declarations, and check for in-scope conflicts
  for (int j = 0; j < *local_indexes_count; ++j) {
    mc_script_local_v1 *local = (mc_script_local_v1 *)(*local_index)[j];
    // printf("type:%s identifier:%s lind:%u repcode:%s\n", local->type, local->identifier, local->locals_index,
    // local->replacement_code);
    //   printf("var_name:%s\n", var_name);
    // printf("here-2\n");
    if (!strcmp(var_name, local->identifier)) {
      // printf("here-6\n");
      if (local->scope_depth >= 0) {
        // Variable Identity Conflict with active local
        MCerror(241414, "Variable with same identity already declared in this scope");
      }
      else if (!strcmp(type_identifier, local->type)) {
        // Reset it
        local->scope_depth = scope_depth;
        buf[0] = '\0';
        return 0;
      }
      else {
        MCerror(8582, "Reusing out of scope function local again. TODO in future allowing this");
      }
    }
    // printf("here-4\n");
  }
  // printf("here-3\n");

  // Strip type of all deref operators
  char *raw_type_id = NULL;
  for (int i = 0;; ++i) {
    if (type_identifier[i] == ' ' || type_identifier[i] == '*' || type_identifier[i] == '\0') {
      raw_type_id = (char *)malloc(sizeof(char) * (i + 1));
      strncpy(raw_type_id, type_identifier, i);
      raw_type_id[i] = '\0';
      break;
    }
  }

  mc_script_local_v1 *scr_local = (mc_script_local_v1 *)malloc(sizeof(mc_script_local_v1));
  allocate_and_copy_cstr(scr_local->type, type_identifier);
  allocate_and_copy_cstr(scr_local->identifier, var_name);
  scr_local->locals_index = script->local_count;
  scr_local->scope_depth = scope_depth;
  scr_local->replacement_code = (char *)malloc(sizeof(char) * (64 + strlen(scr_local->type)));

  // -- Determine if the structure is midge-specified
  MCcall(find_struct_info(nodespace, raw_type_id, &scr_local->struct_info));
  char *size_of_var;
  if (scr_local->struct_info) {
    mc_struct_info_v1 *sinfo = (mc_struct_info_v1 *)scr_local->struct_info;

    char *substituted_type =
        (char *)malloc(sizeof(char) * (strlen(scr_local->type) - strlen(raw_type_id) + strlen(sinfo->declared_mc_name) + 1));
    strcpy(substituted_type, sinfo->declared_mc_name);
    strcat(substituted_type, scr_local->type + strlen(raw_type_id));
    substituted_type[strlen(scr_local->type) - strlen(raw_type_id) + strlen(sinfo->declared_mc_name)] = '\0';
    // printf("kt:%s rt:%s dmc:%s st:%s\n", kvp->type, raw_type_id, sinfo->declared_mc_name, substituted_type);

    sprintf(scr_local->replacement_code, "(*(%s *)script_instance->locals[%u])", substituted_type, scr_local->locals_index);
    // printf("\nkrpstcde:'%s'\n", kvp->replacement_code);

    size_of_var = (char *)malloc(sizeof(char) * (8 + 1 + strlen(substituted_type)));
    sprintf(size_of_var, "sizeof(%s)", substituted_type);

    free(substituted_type);
  }
  else {
    sprintf(scr_local->replacement_code, "(*(%s *)script_instance->locals[%u])", scr_local->type, scr_local->locals_index);
    // printf("\nkrpnncde:'%s','%s'\n", type_identifier, kvp->replacement_code);

    size_of_var = (char *)malloc(sizeof(char) * (8 + 1 + strlen(scr_local->type)));
    sprintf(size_of_var, "sizeof(%s)", scr_local->type);
  }

  ++script->local_count;
  // printf("type:%s identifier:%s lind:%u repcode:%s\n", kvp->type, kvp->identifier, kvp->locals_index,
  // kvp->replacement_code);

  append_to_collection(local_index, local_indexes_alloc, local_indexes_count, scr_local);

  sprintf(buf, "script_instance->locals[%u] = (void *)malloc(%s);\n", scr_local->locals_index, size_of_var);

  free(raw_type_id);
  free(size_of_var);

  return 0;
}

int mcqck_get_script_local_replace(void *nodespace, void **local_index, unsigned int local_indexes_count, const char *key,
                                   char **output)
{
  void **mc_dvp;
  *output = NULL;
  // "nvk strcpy provocation finfo->parameters[pind]->name\n"

  char *primary = NULL;
  int m = -1;
  char breaker = 'n';
  for (int i = 0;; ++i) {
    if (!isalnum(key[i]) && key[i] != '_') {
      primary = (char *)malloc(sizeof(char) * (i + 1));
      strncpy(primary, key, i);
      primary[i] = '\0';
      m = i;
      breaker = key[i];
      break;
    }
  }
  if (primary == NULL) {
    allocate_and_copy_cstr(primary, key);
  }
  mc_script_local_v1 *kvp;
  for (int i = 0; i < local_indexes_count; ++i) {
    assign_anon_struct(kvp, local_index[i]);

    if (!strcmp(primary, kvp->identifier)) {
      // printf("match!: %s=%s:%s\n", primary, kvp->identifier, kvp->replacement_code);
      break;
    }
    else {
      // printf("NOmatch!: %s=%s:%s\n", primary, kvp->identifier, kvp->replacement_code);
    }

    kvp = NULL;
  }
  free(primary);
  if (!kvp)
    return 0;

  switch (breaker) {
  case '\0': {
    // Simple replacement
    *output = kvp->replacement_code;
    return 0;
  }
  case '-': {
    // -> Pointer to member
    if (!kvp->struct_info) {
      *output = (char *)malloc(sizeof(char) * (strlen(kvp->replacement_code) + strlen(key) - m + 1));
      sprintf(*output, "%s%s", kvp->replacement_code, key + m);
      // printf("*output='%s'\n", *output);
      return 0;
    }

    parse_past(key, &m, "->");
    primary = (char *)malloc(sizeof(char) * (2 + strlen(kvp->replacement_code) + 1));
    sprintf(primary, "%s->", kvp->replacement_code);

    // Determine the child field type
    mc_struct_info_v1 *parent_struct_info;
    assign_anon_struct(parent_struct_info, kvp->struct_info);
    char *secondary = NULL;
    // Get the member name
    breaker = 'n';
    for (int i = m;; ++i) {
      if (!isalnum(key[i]) && key[i] != '_') {
        secondary = (char *)malloc(sizeof(char) * (i - m + 1));
        strncpy(secondary, key + m, i - m);
        secondary[i - m] = '\0';
        m = i;
        breaker = key[i];
        break;
      }
    }
    switch (breaker) {
    case '\0': {
      // printf("primary:'%s'\n", primary);
      // printf("secondary:'%s'\n", secondary);

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
    default: {
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
  default: {
    MCerror(24528, "Not handled:'%c'  key=%s", breaker, key);
  }
  }
}

int parse_past_script_expression(void *nodespace, void **local_index, unsigned int local_indexes_count, char *code, int *i,
                                 char **output)
{
  char *primary, *temp;

  if (isalpha(code[*i])) {

    MCcall(parse_past_identifier(code, i, &primary, true, true));
    // printf("primary:%s code[*i]:'%c'\n", primary, code[*i]);

    MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, primary, &temp));

    if (temp) {
      free(primary);
      allocate_and_copy_cstr(primary, temp);
    }
    if (code[*i] != '[') {
      *output = primary;
      return 0;
    }

    for (int b = 1;; ++b) {
      // Parse past the '['
      ++*i;

      char *secondary;
      parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, i, &secondary);

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
          MCerror(42452, "TODO ");
        }
        default:
          MCerror(3252353, "TODO '%c'", code[*i]);
        }
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
    case '!': {
      ++*i;
      MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, i, &primary));
      temp = (char *)malloc(sizeof(char) * (strlen(primary) + 2));
      strcpy(temp, "!");
      strcat(temp, primary);
      temp[strlen(primary) + 1] = '\0';

      free(primary);
      *output = temp;

      return 0;
    }
    case '@': {
      // Script context variable
      ++*i;

      MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, i, output));

      char *temp = (char *)malloc(sizeof(char) * (strlen(*output) + 2));
      sprintf(temp, "@%s", *output);
      free(*output);
      // printf("@@@@ ='%s'\n", temp);
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
    // case '(': {
    //   ++*i;
    //   MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, i, &temp));

    //   MCcall(parse_past(code, i, ")"));

    //   *output = (char *)malloc(sizeof(char) * (2 + strlen(temp) + 1));
    //   sprintf(*output, "(%s)", temp);

    //   return 0;
    // }
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
        MCerror(9877, "BackTODO");
      }
      MCcall(parse_past(code, i, " "));

      // left
      char *left;
      MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, i, &left));
      MCcall(parse_past(code, i, " "));

      // right
      char *right;
      MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, i, &right));

      *output = (char *)malloc(sizeof(char) * (strlen(oper) + 1 + strlen(left) + 1 + strlen(right) + 1));
      sprintf(*output, "%s %s %s", left, oper, right);

      free(left);
      free(oper);
      free(right);
    }
      return 0;
    default: {
      MCcall(print_parse_error(code, *i, "parse_past_script_expression", "first_char"));
      return -7777;
    }
    }

  return 4240;
}

int mcqck_translate_script_code(void *nodespace, mc_script_v1 *script, char *code)
{
  unsigned int translation_alloc = strlen(code);
  char *translation = (char *)malloc(sizeof(char) * translation_alloc);
  translation[0] = '\0';
  unsigned int t = 0;

  unsigned int local_indexes_alloc = 20;
  unsigned int local_indexes_count = 0;
  void **local_index = (void **)malloc(sizeof(void *) * local_indexes_alloc);
  int local_scope_depth = 0;

  script->local_count = 0;
  script->segment_count = 0;

  int debug_statement_index = 0;

  char buf[2048];
  // -- Parse statements
  int s = -1;
  int i = 0;
  bool loop = true;
  while (loop) {
    // printf("i:%i  '%c'\n", i, code[i]);
    // sprintf(buf, "printf(\"statement %i:\\n\");\n", debug_statement_index++);
    // MCcall(append_to_cstr(&translation_alloc, &translation, buf));
    switch (code[i]) {
    case ' ':
    case '\t':
    case '\n':
      break;
    case '$': {
      MCcall(parse_past(code, &i, "$"));
      switch (code[i]) {
      case 'p': {
        // $pi - provocation interaction
        MCcall(parse_past(code, &i, "pi"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Identifier
        char *response_location;
        MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &response_location));
        MCcall(parse_past(code, &i, " "));

        // Variable Name
        char *provocation;
        MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &provocation));
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(-4864, "expected statement end:'%c'", code[i]);
        }

        ++script->segment_count;
        buf[0] = '\0';
        sprintf(buf,
                "  \n// Script Provocation-Response Break\n"
                // "  printf(\"here-4\\n\");\n"
                "  allocate_and_copy_cstr(script_instance->response, %s);\n"
                // "printf(\"seqid:%%u \\n\", script_instance->sequence_uid);\n"
                "  script_instance->segments_complete = %u;\n"
                "  script_instance->awaiting_data_set_index = %u;\n"
                // "  printf(\"here-6a\\n\");\n"
                "  return 0;\n"
                "segment_%u:\n"
                // "printf(\"here-6b\\n\");\n"
                "  %s = (char *)script_instance->locals[%u];\n"
                "  if(%s[0] == '@') {\n"
                "    char *mc_context_data;"
                "    MCcall(get_process_contextual_data(script_instance->contextual_action, %s + 1,\n"
                "           (void **)&mc_context_data));\n"
                "    if(!mc_context_data) {\n"
                "      MCerror(1027, \"Missing process context data for '%%s'\", %s);"
                "    }\n"
                "    free(%s);\n"
                "    %s = mc_context_data;\n"
                "  }\n",
                provocation, script->segment_count, script->local_count, script->segment_count, response_location,
                script->local_count, response_location, response_location, response_location, response_location,
                response_location);
        ++script->local_count;

        append_to_cstr(&translation_alloc, &translation, buf);

        free(response_location);
        free(provocation);
      } break;
      case 'n': {
        // $nv - invocation of function with variable name
        MCcall(parse_past(code, &i, "nv"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        MCcall(append_to_cstr(&translation_alloc, &translation, "{\n"));
        MCcall(append_to_cstr(&translation_alloc, &translation, "  char mcsfnv_buf[2048];\n"));
        // MCcall(append_to_cstr(&translation_alloc, &translation, "  printf(\"here-1\\n\");\n"));
        MCcall(append_to_cstr(&translation_alloc, &translation, "  int mcsfnv_arg_count = 0;\n"));
        MCcall(append_to_cstr(&translation_alloc, &translation, "  sprintf(mcsfnv_buf, \"{\\n\");\n"));
        MCcall(append_to_cstr(&translation_alloc, &translation,
                              "  sprintf(mcsfnv_buf + strlen(mcsfnv_buf), \"void *mcsfnv_vargs[128];\\n\");\n\n"));

        // Function Name
        char *function_name_identifier;
        MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i,
                                            &function_name_identifier));

        MCcall(append_to_cstr(&translation_alloc, &translation,
                              "  mc_function_info_v1 *mcsfnv_function_info;\n"
                              "  {\n"
                              "    void *mc_vargs[3];\n"
                              "    mc_vargs[0] = (void *)&mcsfnv_function_info;\n"
                              "    mc_vargs[1] = (void *)&nodespace;\n"));
        if (function_name_identifier[0] == '@') {
          sprintf(buf,
                  "    char *mc_context_data_2;\n"
                  "    MCcall(get_process_contextual_data(script_instance->contextual_action, \"%s\",\n"
                  "           (void **)&mc_context_data_2));\n"
                  "    if(!mc_context_data_2) {\n"
                  "      return -5;"
                  "    }\n"
                  "    mc_vargs[2] = (void *)&mc_context_data_2;\n",
                  function_name_identifier + 1);
          // TODO deal with maybe duplicated / free strings?
        }
        else {
          sprintf(buf, "    mc_vargs[2] = (void *)&%s;\n", function_name_identifier);
        }
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        sprintf(buf, "    find_function_info(3, mc_vargs);\n"
                     "  }\n"
                     "  if (mcsfnv_function_info) {\n"
                     //  "    // Invoking a midge function, pass command_hub as first argument\n"
                     //  "    sprintf(mcsfnv_buf + strlen(mcsfnv_buf), \"mcsfnv_vargs[0] = (void *)%%p;\\n\",
                     //  command_hub);\n" "    ++mcsfnv_arg_count;\n"
                     "  }\n"
                     "  else\n"
                     "    throw -233;\n\n");
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        // Form the arguments
        while (code[i] != '\n' && code[i] != '\0') {
          MCcall(parse_past(code, &i, " "));

          if (code[i] == '$') {
            MCcall(parse_past(code, &i, "$ya"));
            MCcall(parse_past(code, &i, " "));

            char *array_count_identifier;
            MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i,
                                                &array_count_identifier));
            MCcall(parse_past(code, &i, " "));
            sprintf(buf, "  for (int mc_ii = 0; mc_ii < %s; ++mc_ii) {\n", array_count_identifier);
            MCcall(append_to_cstr(&translation_alloc, &translation, buf));

            char *array_identifier;
            MCcall(
                parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &array_identifier));
            sprintf(buf,
                    "    sprintf(mcsfnv_buf + strlen(mcsfnv_buf), \"mcsfnv_vargs[%%i] = (void *)%%p;\\n\",\n"
                    "            mcsfnv_arg_count, &%s[mc_ii]);\n",
                    array_identifier);
            MCcall(append_to_cstr(&translation_alloc, &translation, buf));

            MCcall(append_to_cstr(&translation_alloc, &translation, "    ++mcsfnv_arg_count;\n"));
            MCcall(append_to_cstr(&translation_alloc, &translation, "  }\n"));

            free(array_count_identifier);
            free(array_identifier);
          }
          else {
            MCerror(-1037, "TODO this case");
            // char *argument;
            // MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &argument));
            // // MCcall(parse_past_identifier(code, &i, &argument, true, true));
            // // MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, argument,
            // &replace_name));
            // // char *arg_entry = argument;
            // // if (replace_name)
            // //   arg_entry = replace_name;
            // if (function_info)
            //   sprintf(buf, "mc_vargs[%i] = (void *)&%s;\n", argument_count + 1, argument);
            // else
            //   sprintf(buf, "%s%s", argument_count ? ", " : "", argument);
            // MCcall(append_to_cstr(&translation_alloc, &translation, buf));
            // ++argument_count;
            // free(argument);
          }
        }

        if (function_name_identifier[0] == '@') {
          sprintf(buf,
                  "  char *mcsfnv_function_name;\n"
                  "  MCcall(get_process_contextual_data(script_instance->contextual_action, \"%s\", (void "
                  "**)&mcsfnv_function_name));\n"
                  "  if(!mcsfnv_function_name) {\n"
                  "    MCerror(42424, \"InScriptGen: Couldn't find '%%s' in process action contextual data\", "
                  "mcsfnv_function_name);\n"
                  "  }\n",
                  function_name_identifier + 1);
        }
        else {
          sprintf(buf, "  char *mcsfnv_function_name = %s;\n", function_name_identifier);
        }
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        // TODO haven't implemented return
        MCcall(append_to_cstr(&translation_alloc, &translation, "  int mc_script_func_res;\n"));
        sprintf(buf, "  sprintf(mcsfnv_buf + strlen(mcsfnv_buf), \"int *mc_script_func_res = (int *)%%p;\\n"
                     "  *mc_script_func_res = %%s(%%i, mcsfnv_vargs);\","
                     " &mc_script_func_res, mcsfnv_function_name, mcsfnv_arg_count);\n");
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        MCcall(append_to_cstr(&translation_alloc, &translation, "  sprintf(mcsfnv_buf + strlen(mcsfnv_buf), \"}\\n\");\n"));
        // sprintf(buf, "  printf(\"\\n%s\\n\", mcsfnv_buf);\n");
        // MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        MCcall(append_to_cstr(&translation_alloc, &translation, "  clint_process(mcsfnv_buf);\n"));
        MCcall(append_to_cstr(&translation_alloc, &translation, "  sprintf(mcsfnv_buf + strlen(mcsfnv_buf), \"}\\n\");\n"));
        sprintf(buf, "  if (mc_script_func_res) {\n"
                     "    printf(\"--function '%%s' error:%%i\\n\", mcsfnv_function_name, mc_script_func_res);\n"
                     "    return mc_script_func_res;\n"
                     "}\n\n");
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));

        free(function_name_identifier);
      } break;
      default: {
        // printf("\ntranslation:\n%s\n\n", translation);
        MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "$ unhandled"));
        return -4857;
      } break;
      }
    } break;
    case 'a': {
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
      //     MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &left));
      //     MCcall(parse_past(code, &i, " "));

      //     // right
      //     char *right;
      //     MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &right));

      //     buf[0] = '\0';
      //     MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc,
      //     &local_indexes_count, script, buf,
      //                                        type_identifier, var_name));
      //     append_to_cstr(&translation_alloc, &translation, buf);

      //     buf[0] = '\0';
      //     char *replace_name;
      //     MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name,
      //     &replace_name)); sprintf(buf, "%s = %s %s %s", replace_name, left, comparator, right);

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
      //       MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &right));
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
      if (code[i] == 's') {
        MCcall(parse_past(code, &i, "s"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        char *var_identifier;
        MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &var_identifier));
        MCcall(parse_past(code, &i, " "));

        char *left;
        MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &left));
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(-4829, "expected statement end");
        }

        // buf[0] = '\0';
        sprintf(buf, "%s = %s;\n", var_identifier, left);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        // free(var_identifier);
        // free(left);
      }
      else {
        MCerror(-4866, "TODO");
      }
    } break;
    case 'b': {
      // brk
      MCcall(parse_past(code, &i, "brk"));
      if (code[i] != '\n' && code[i] != '\0') {
        MCerror(-4829, "expected statement end");
      }
      MCcall(append_to_cstr(&translation_alloc, &translation, "break;\n"));
    } break;
    case 'd': {
      MCcall(parse_past(code, &i, "dc"));
      if (code[i] == 'l') {
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
        if (code[i] != '\n' && code[i] != '\0') {
          // Array decl
          MCcall(parse_past(code, &i, "["));

          char *left;
          MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &left));
          MCcall(parse_past(code, &i, "]"));
          if (code[i] != '\n' && code[i] != '\0') {
            MCerror(-4829, "expected statement end");
          }

          // Add deref to type
          char *new_type_identifier = (char *)malloc(sizeof(char) * (strlen(type_identifier) + 2));
          strcpy(new_type_identifier, type_identifier);
          strcat(new_type_identifier, "*");
          new_type_identifier[strlen(type_identifier) + 1] = '\0';

          // Normal Declaration
          MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script,
                                             buf, local_scope_depth, new_type_identifier, var_name));
          // printf("dcl-here-0\n");
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          // printf("dcl-here-1\n");

          // Allocate the array
          char *replace_var_name;
          MCcall(
              mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_var_name));
          // printf("dcl-here-2\n");

          sprintf(buf, "%s = (%s)malloc(sizeof(%s) * (%s));\n", replace_var_name, new_type_identifier, type_identifier, left);
          // printf("dcl-here-3\n");
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          // printf("dcl-here-4\n");

          free(left);
          // printf("dcl-here-5\n");
          free(new_type_identifier);
          // printf("dcl-here-6\n");
          // printf("Translation:\n%s\n", translation);
          // printf("dcl-here-7\n");
        }
        else {
          // Normal Declaration
          MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script,
                                             buf, local_scope_depth, type_identifier, var_name));
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }
        free(var_name);
        free(type_identifier);
      }
      else if (code[i] == 's') {
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
        MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &left));
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(-4829, "expected statement end");
        }

        // Generate Local
        MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script,
                                           buf, local_scope_depth, type_identifier, var_name));
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
      else {
        MCerror(86583, "TODO");
      }
    } break;
    case 'e': {
      if (code[i + 1] == 'n') {
        // end
        MCcall(parse_past(code, &i, "end"));
        MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));

        --local_scope_depth;

        if (code[i] != '\n' && code[i] != '\0') {
          MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too
          MCcall(parse_past(code, &i, "for"));
          MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));

          --local_scope_depth;

          if (code[i] != '\n' && code[i] != '\0') {
            MCerror(-4831, "expected statement end");
          }
        }

        // Manage variable scope
        for (int j = 0; j < local_indexes_count; ++j) {
          mc_script_local_v1 *local = (mc_script_local_v1 *)local_index[j];
          if (local->scope_depth > local_scope_depth) {
            // Disable the local variable
            local->scope_depth = -1;
          }
        }
      }
      else if (code[i + 1] == 'r') {
        // end
        MCcall(parse_past(code, &i, "err"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        char *error_code;
        MCcall(parse_past_number(code, &i, &error_code));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        char *error_message;
        MCcall(parse_past_script_expression(nodespace, local_index, local_indexes_count, code, &i, &error_message));
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(-4831, "expected statement end");
        }

        sprintf(buf, "{\n  MCerror(%s, %s);\n}\n", error_code, error_message);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        free(error_code);
        free(error_message);
      }
      else {
        MCerror(1380, "Unhandled statement:'%c'", code[i]);
      }

    } break;
    case 'i': {
      // ifs
      MCcall(parse_past(code, &i, "ifs"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // left
      char *left;
      // printf("here-0\n");
      MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &left));
      // printf("left:%s\n", left);
      if (code[i] != ' ') {
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(-4887, "expected statement end");
        }

        sprintf(buf, "if(%s) {\n", left);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));
      }
      else {
        MCcall(parse_past(code, &i, " "));

        // comparison operator
        char *comparator;
        switch (code[i]) {
        case '=': {
          if (code[i + 1] != '=') {
            MCerror(585282, "can't have just one =");
          }
          i += 2;
          comparator = (char *)malloc(sizeof(char) * 3);
          strcpy(comparator, "==");
        } break;
        case '<':
        case '>': {
          int len = 1;
          if (code[i + 1] == '=')
            len = 2;
          comparator = (char *)malloc(sizeof(char) * (len + 1));
          comparator[0] = code[i];
          if (len > 1)
            comparator[1] = code[i + 1];
          comparator[len] = '\0';
          i += len;
        } break;
        default:
          MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "if>comparator-switch"));
          return 858528;
        }
        MCcall(parse_past(code, &i, " "));

        // right
        char *right;
        MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &right));
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(-4864, "expected statement end:'%c'", code[i]);
        }

        sprintf(buf, "if(%s %s %s) {\n", left, comparator, right);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));

        free(comparator);
        free(right);
      }

      // Increment Scope Depth
      ++local_scope_depth;

      free(left);
    } break;
    case 'f': {
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
      if (isalpha(code[i])) {
        MCcall(parse_past_identifier(code, &i, &initiate, false, false));

        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, initiate, &initiate_final));
        if (!initiate_final)
          initiate_final = initiate;
      }
      else if (code[i] == '"') {
        MCerror(118492, "TODO quotes");
      }
      else if (isdigit(code[i])) {
        parse_past_number(code, &i, &initiate);
        initiate_final = initiate;
      }
      else {
        MCerror(118492, "TODO what");
      }

      MCcall(parse_past(code, &i, " "));

      // maximum
      char *maximum, *maximum_final;
      if (isalpha(code[i])) {
        MCcall(parse_past_identifier(code, &i, &maximum, false, false));

        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, maximum, &maximum_final));
        if (!maximum_final)
          maximum_final = maximum;
      }
      else if (code[i] == '"') {
        MCerror(78678, "TODO quotes");
      }
      else if (isdigit(code[i])) {
        parse_past_number(code, &i, &initiate);
      }
      else {
        MCerror(373785, "TODO what");
      }
      if (code[i] != '\n' && code[i] != '\0') {
        MCerror(-4864, "expected statement end:'%c'", code[i]);
      }

      append_to_cstr(&translation_alloc, &translation, "{\n");
      ++local_scope_depth;

      char *int_cstr;
      allocate_and_copy_cstr(int_cstr, "int");
      MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script, buf,
                                         local_scope_depth, int_cstr, iterator));
      append_to_cstr(&translation_alloc, &translation, buf);
      char *iterator_replace;
      MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, iterator, &iterator_replace));

      sprintf(buf, "for(%s = %s; %s < %s; ++%s) {\n", iterator_replace, initiate_final, iterator_replace, maximum_final,
              iterator_replace);
      append_to_cstr(&translation_alloc, &translation, buf);
      ++local_scope_depth;

      free(initiate);
      free(maximum);
    } break;
    case 'm': {
      // msi
      MCcall(parse_past(code, &i, "ms"));
      if (code[i] == 'i') {
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
        MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &sizeof_expr));
        if (code[i] != '\n' && code[i] != '\0') {
          MCerror(-4829, "expected statement end");
        }

        // Script Local
        MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script,
                                           buf, local_scope_depth, type_identifier, var_name));
        append_to_cstr(&translation_alloc, &translation, buf);

        char *replace_name;
        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_name));

        // Statement
        char *trimmed_type_identifier = (char *)malloc(sizeof(char) * (strlen(type_identifier)));
        strncpy(trimmed_type_identifier, type_identifier, strlen(type_identifier) - 1);
        trimmed_type_identifier[strlen(type_identifier) - 1] = '\0';

        sprintf(buf, "%s = (%s)malloc(sizeof(%s) * (%s));\n", replace_name, type_identifier, trimmed_type_identifier,
                sizeof_expr);
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
        //   MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &left));
        //   MCcall(parse_past(code, &i, " "));

        //   // right
        //   char *right;
        //   MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &right));

        //   buf[0] = '\0';
        //   MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc,
        //   &local_indexes_count, script, buf,
        //                                      type_identifier, var_name));
        //   append_to_cstr(&translation_alloc, &translation, buf);

        //   buf[0] = '\0';
        //   char *replace_name;
        //   MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name,
        //   &replace_name)); char *trimmed_type_identifier = (char *)malloc(sizeof(char) * (strlen(type_identifier)));
        //   strncpy(trimmed_type_identifier, type_identifier, strlen(type_identifier) - 1);
        //   trimmed_type_identifier[strlen(type_identifier) - 1] = '\0';
        //   sprintf(buf, "%s = malloc(sizeof(%s) * (%s %s %s", replace_name, trimmed_type_identifier, left, comparator,
        //   right);

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
        //     MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &right));
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
      else {
        MCerror(-4866, "TODO");
      }
    } break;
    case 'n': {
      MCcall(parse_past(code, &i, "nv"));
      if (code[i] == 'i') {
        // nvi
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

        MCcall(mcqck_generate_script_local((void *)nodespace, &local_index, &local_indexes_alloc, &local_indexes_count, script,
                                           buf, local_scope_depth, type_identifier, var_name));
        append_to_cstr(&translation_alloc, &translation, buf);

        char *replace_name;
        MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, var_name, &replace_name));
        // printf("nvi gen replace_name:%s=%s\n", var_name, replace_name);

        // Invoke
        // Function Name
        char *function_name;
        MCcall(parse_past_identifier(code, &i, &function_name, true, false));

        // printf("dopeh\n");
        mc_function_info_v1 *func_info;
        {
          void *mc_vargs[3];
          mc_vargs[0] = (void *)&func_info;
          mc_vargs[1] = (void *)&nodespace;
          mc_vargs[2] = (void *)&function_name;
          find_function_info(3, mc_vargs);
        }
        // printf("dopey\n");
        if (func_info) {
          if (!strcmp(func_info->return_type, "void")) {
            MCerror(-1002, "compile error: cannot assign from a void function!");
          }
          else {
            sprintf(buf,
                    "{\n"
                    "  mc_vargs[0] = (void *)&%s;\n",
                    replace_name);
            MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          }
        }
        else {
          sprintf(buf, "%s = %s(", replace_name, function_name);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }

        int arg_index = 0;
        while (code[i] != '\n' && code[i] != '\0') {
          MCcall(parse_past(code, &i, " "));

          char *argument;
          MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &argument));
          // MCcall(parse_past_identifier(code, &i, &argument, true, true));
          // MCcall(mcqck_get_script_local_replace((void *)nodespace, local_index, local_indexes_count, argument,
          // &replace_name)); char *arg_entry = argument; if (replace_name)
          //   arg_entry = replace_name;
          if (func_info) {
            if (argument[0] == '@') {
              sprintf(buf,
                      "  char *mc_context_data_%i;\n"
                      "  MCcall(get_process_contextual_data(script_instance->contextual_action, \"%s\", (void "
                      "**)&mc_context_data_%i));\n"
                      "  if(!mc_context_data_%i) {\n"
                      "    MCerror(1813, \"Missing process context data for '%s'\");"
                      "  }\n"
                      "  mc_vargs[%i] = (void *)&mc_context_data_%i;\n",
                      arg_index + 1, argument + 1, arg_index + 1, arg_index + 1, argument, arg_index + 1, arg_index + 1);
            }
            else {
              sprintf(buf, "mc_vargs[%i] = (void *)&%s;\n", arg_index + 1, argument);
            }
          }
          else {
            if (argument[0] == '@') {
              MCerror(4829, "NOT YET IMPLEMENTED");
            }
            sprintf(buf, "%s%s", arg_index ? ", " : "", argument);
          }
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          ++arg_index;
          free(argument);
        }

        if (func_info) {

          // append_to_cstr(&translation_alloc, &translation, "  printf(\"here-22  =%s\\n\", (char *)mc_vargs[2]);\n");
          sprintf(buf, "  %s(%i, mc_vargs", function_name, arg_index + 1);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
        }

        MCcall(append_to_cstr(&translation_alloc, &translation, ");\n"));
        if (func_info)
          MCcall(append_to_cstr(&translation_alloc, &translation, "}\n"));
        // MCcall(append_to_cstr(&translation_alloc, &translation, "  printf(\"here-31\\n\");\n"));

        free(function_name);
      }
      else if (code[i] == 'k') {
        // nvk
        MCcall(parse_past(code, &i, "k"));
        MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

        // Invoke
        // Function Name
        char *function_name;
        MCcall(parse_past_identifier(code, &i, &function_name, true, false));
        MCcall(append_to_cstr(&translation_alloc, &translation, function_name));
        MCcall(append_to_cstr(&translation_alloc, &translation, "("));

        bool first_arg = true;
        while (code[i] != '\n' && code[i] != '\0') {
          MCcall(parse_past(code, &i, " "));

          char *argument;
          MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &argument));

          sprintf(buf, "%s%s", first_arg ? "" : ", ", argument);
          MCcall(append_to_cstr(&translation_alloc, &translation, buf));
          first_arg = false;
          free(argument);
        }
        MCcall(append_to_cstr(&translation_alloc, &translation, ");\n"));
      }
      else {
        MCerror(-4248, "TODO");
      }
    } break;
    case 's': {
      // set
      MCcall(parse_past(code, &i, "set"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // Identifier
      char *type_identifier;
      MCcall(parse_past_type_identifier(code, &i, &type_identifier));
      MCcall(parse_past(code, &i, " "));

      // Variable Name
      char *var_name;
      MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &var_name));
      MCcall(parse_past(code, &i, " "));

      // Value Name
      char *value_expr;
      MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &value_expr));
      if (code[i] != '\n' && code[i] != '\0') {
        MCerror(-4829, "expected statement end");
      }

      if (!strcmp(type_identifier, "int")) {
        buf[0] = '\0';
        sprintf(buf, "%s = %s;\n", var_name, value_expr);
        MCcall(append_to_cstr(&translation_alloc, &translation, buf));
      }
      else {
        MCerror(52885, "NotYetSupported:%s", type_identifier);
      }
    } break;
    case 'w': {
      // set
      MCcall(parse_past(code, &i, "whl"));
      MCcall(parse_past(code, &i, " ")); // TODO -- allow tabs too

      // Variable Name
      char *conditional;
      MCcall(parse_past_script_expression((void *)nodespace, local_index, local_indexes_count, code, &i, &conditional));
      if (code[i] != '\n' && code[i] != '\0') {
        MCerror(-4829, "expected statement end");
      }

      sprintf(buf, "while(%s) {\n", conditional);
      MCcall(append_to_cstr(&translation_alloc, &translation, buf));

      free(conditional);
    } break;
    case '\0':
      loop = false;
      break;
    default: {
      // printf("\ntranslation:\n%s\n\n", translation);
      MCcall(print_parse_error(code, i, "mcqck_translate_script_code", "UnhandledStatement"));
      return -4857;
    } break;
    }

    ++i;
  }

  unsigned int declaration_alloc = 1024 + translation_alloc;
  char *declaration = (char *)malloc(sizeof(char) * declaration_alloc);
  sprintf(declaration,
          "int %s(mc_script_instance_v1 *script_instance) {\n"
          "  void *mc_vargs[128];\n"
          "\n\n"
          "  mc_node_v1 *nodespace = (mc_node_v1 *)script_instance->command_hub->nodespace;"
          // "  const char *command = script_instance->contextual_command;\n\n"
          "  switch(script_instance->segments_complete)\n"
          "  {\n"
          "  case 0:\n"
          "    break;\n",
          script->created_function_name);
  for (int i = 1; i <= script->segment_count; ++i) {
    sprintf(buf, "  case %i: goto segment_%i;\n", i, i);
    append_to_cstr(&declaration_alloc, &declaration, buf);
  }
  append_to_cstr(&declaration_alloc, &declaration,
                 "  default:\n"
                 "    return 0;\n"
                 "  }\n\n");
  ++script->segment_count;
  append_to_cstr(&declaration_alloc, &declaration, translation);
  append_to_cstr(&declaration_alloc, &declaration,
                 "\n"
                 "// Script Execution Finished\n"
                 "script_instance->segments_complete = script_instance->script->segment_count;\n"
                 //  "printf(\"script-concluding\\n\");\n"
                 "return 0;\n}");

  // printf("script_declaration:\n%s\n\n", declaration);

  clint_declare(declaration);

  free(translation);
  free(declaration);
  return 0;
}

int print_process_unit(mc_process_unit_v1 *process_unit, int detail_level, int print_children, int indent)
{
  if (indent > 0)
    printf("-%i--", indent);
  for (int i = 1; i < indent; ++i)
    printf("----");
  printf(":");
  switch (process_unit->type) {
  case PROCESS_MATRIX_SAMPLE:
    printf("SAMPLE\n");
    break;
  case PROCESS_MATRIX_NODE:
    printf("NODE(%i)\n", process_unit->process_unit_field_differentiation_index);
    break;
  default:
    MCerror(2009, "forget");
  }

  if (detail_level > 1) {

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("actType:%s\n", get_action_type_string(process_unit->action->type));
  }

  if (detail_level > 2) {

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("actDiag:'%s'\n", process_unit->action->dialogue == NULL ? "(null)" : process_unit->action->dialogue);

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("contType:%s\n", get_action_type_string(process_unit->continuance->type));

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("contDiag:'%s'\n", process_unit->continuance->dialogue == NULL ? "(null)" : process_unit->continuance->dialogue);
  }

  if (detail_level > 3) {

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("prevType:'%s'\n", get_action_type_string(process_unit->previous_issue->type));

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("prevDlg:'%s'\n", process_unit->previous_issue->dialogue == NULL ? "(null)" : process_unit->previous_issue->dialogue);

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("seqRootType:'%s'\n", get_action_type_string(process_unit->sequence_root_issue->type));

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("seqRootDlg:'%s'\n",
           process_unit->sequence_root_issue->dialogue == NULL ? "(null)" : process_unit->sequence_root_issue->dialogue);

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("contextType:'%s'\n", get_action_type_string(process_unit->contextual_issue->type));

    for (int i = 0; i < indent; ++i)
      printf("    ");
    printf("contextDlg:'%s'\n",
           process_unit->contextual_issue->dialogue == NULL ? "(null)" : process_unit->contextual_issue->dialogue);
  }

  // Children
  switch (process_unit->type) {
  case PROCESS_MATRIX_SAMPLE:
    return 0;
  case PROCESS_MATRIX_NODE:
    if (print_children) {
      for (int i = 0; i < process_unit->children->count; ++i)
        print_process_unit((mc_process_unit_v1 *)process_unit->children->items[i], print_children, print_children, indent + 1);
    }
    else {
      for (int i = 0; i < indent; ++i)
        printf("    ");
      printf("child_branches:%i\n", process_unit->children->count);
    }
    return 0;
  default:
    MCerror(2009, "forget");
  }
}

int init_core_functions(mc_command_hub_v1 *command_hub);
int init_process_matrix(mc_command_hub_v1 *command_hub);
int init_command_hub_process_matrix(mc_command_hub_v1 *command_hub);
int submit_user_command(int argc, void **argsv);
int (*mc_dummy_function_pointer)(int, void **);
int mc_main(int argc, const char *const *argv)
{
  printf("mm-0\n");
  int sizeof_void_ptr = sizeof(void *);
  if (sizeof_void_ptr != sizeof(int *) || sizeof_void_ptr != sizeof(char *) || sizeof_void_ptr != sizeof(uint *) ||
      sizeof_void_ptr != sizeof(const char *) || sizeof_void_ptr != sizeof(void **) ||
      sizeof_void_ptr != sizeof(mc_dummy_function_pointer) || sizeof_void_ptr != sizeof(&mc_main) ||
      sizeof_void_ptr != sizeof(unsigned long)) {
    printf("pointer sizes aren't equal!!!\n");
    return -1;
  }

  mc_struct_info_v1 *parameter_info_definition_v1 = (mc_struct_info_v1 *)malloc(sizeof(mc_struct_info_v1));
  { // TYPE:DEFINITION parameter_info
    parameter_info_definition_v1->struct_id = NULL;
    parameter_info_definition_v1->name = "parameter_info";
    parameter_info_definition_v1->version = 1U;
    parameter_info_definition_v1->declared_mc_name = "mc_parameter_info_v1";
    parameter_info_definition_v1->field_count = 5;
    parameter_info_definition_v1->fields = (void **)calloc(sizeof(void *), parameter_info_definition_v1->field_count);
    parameter_info_definition_v1->sizeof_cstr = "sizeof_parameter_info_v1";

    mc_parameter_info_v1 *field;
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    parameter_info_definition_v1->fields[0] = field;
    field->type_name = "struct_info";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "struct_id";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    parameter_info_definition_v1->fields[1] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "type_name";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    parameter_info_definition_v1->fields[2] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "type_version";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    parameter_info_definition_v1->fields[3] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "type_deref_count";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    parameter_info_definition_v1->fields[4] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "name";
  }

  mc_struct_info_v1 *struct_info_definition_v1 = (mc_struct_info_v1 *)malloc(sizeof(mc_struct_info_v1));
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
    mc_parameter_info_v1 *field;
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    struct_info_definition_v1->fields[0] = field;
    field->type_name = "struct_info";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "struct_id";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    struct_info_definition_v1->fields[1] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "type_name";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    struct_info_definition_v1->fields[2] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "version";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    struct_info_definition_v1->fields[3] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "declared_mc_name";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    struct_info_definition_v1->fields[4] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "field_count";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    struct_info_definition_v1->fields[5] = field;
    field->type_name = "parameter_info";
    field->type_version = 0U;
    field->type_deref_count = 2;
    field->name = "fields";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    struct_info_definition_v1->fields[6] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "sizeof_cstr";
  }

  mc_struct_info_v1 *function_info_definition_v1 = (mc_struct_info_v1 *)malloc(sizeof(mc_struct_info_v1));
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
    mc_parameter_info_v1 *field;
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[0] = field;
    field->type_name = "struct_info";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "struct_id";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[1] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "name";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[2] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "latest_iteration";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[3] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "return_type";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[4] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "parameter_count";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[5] = field;
    field->type_name = "parameter_info";
    field->type_version = 0U;
    field->type_deref_count = 2;
    field->name = "parameters";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[6] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "variable_parameter_begin_index";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[7] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "struct_usage_count";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    function_info_definition_v1->fields[8] = field;
    field->type_name = "struct_id";
    field->type_version = 0U;
    field->type_deref_count = 2;
    field->name = "struct_usage";
  }

  printf("mm-1\n");
  mc_struct_info_v1 *node_definition_v1 = (mc_struct_info_v1 *)malloc(sizeof(mc_struct_info_v1));
  { // TYPE:DEFINITION function_info
    node_definition_v1->struct_id = NULL;
    node_definition_v1->name = "node";
    node_definition_v1->version = 1U;
    node_definition_v1->declared_mc_name = "mc_node_v1";
    node_definition_v1->field_count = 12;
    node_definition_v1->fields = (void **)calloc(sizeof(void *), node_definition_v1->field_count);
    node_definition_v1->sizeof_cstr = "sizeof_node_v1"; // TODO -- remove this

    // #define node_v1                          \
//     struct                               \
//     {                                    \
        // mc_struct_id_v1 *struct_id;      \
        // const char *name;                \
        // mc_node_v1 *parent;              \
        // unsigned int functions_alloc;    \
        // unsigned int function_count;     \
        // mc_function_info_v1 **functions; \
        // unsigned int structs_alloc;      \
        // unsigned int struct_count;       \
        // mc_struct_info_v1 **structs;     \
        // unsigned int children_alloc;     \
        // unsigned int child_count;        \
        // mc_node_v1 **children;           \
//     }

    mc_parameter_info_v1 *field;
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[0] = field;
    field->type_name = "struct_id";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "struct_id";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[1] = field;
    field->type_name = "const char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "name";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[2] = field;
    field->type_name = "node";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "parent";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[3] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "functions_alloc";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[4] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "function_count";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[5] = field;
    field->type_name = "function_info";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "functions";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[6] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "structs_alloc";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[7] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "struct_count";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[8] = field;
    field->type_name = "struct_info";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "structs";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[9] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "children_alloc";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[10] = field;
    field->type_name = "unsigned int";
    field->type_version = 0U;
    field->type_deref_count = 0;
    field->name = "child_count";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    node_definition_v1->fields[11] = field;
    field->type_name = "node";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "children";
  }

  // Instantiate: node global;
  mc_node_v1 *global = (mc_node_v1 *)malloc(sizeof(mc_node_v1));
  global->name = "global";
  global->parent = NULL;
  global->functions_alloc = 40;
  global->functions = (mc_function_info_v1 **)calloc(sizeof(mc_function_info_v1 *), global->functions_alloc);
  global->function_count = 0;
  global->structs_alloc = 40;
  global->structs = (mc_struct_info_v1 **)calloc(sizeof(mc_struct_info_v1 *), global->structs_alloc);
  global->struct_count = 0;
  global->children_alloc = 40;
  global->children = (mc_node_v1 **)calloc(sizeof(mc_node_v1 *), global->children_alloc);
  global->child_count = 0;

  MCcall(append_to_collection((void ***)&global->structs, &global->structs_alloc, &global->struct_count,
                              (void *)parameter_info_definition_v1));
  MCcall(append_to_collection((void ***)&global->structs, &global->structs_alloc, &global->struct_count,
                              (void *)struct_info_definition_v1));
  MCcall(append_to_collection((void ***)&global->structs, &global->structs_alloc, &global->struct_count,
                              (void *)function_info_definition_v1));
  MCcall(append_to_collection((void ***)&global->structs, &global->structs_alloc, &global->struct_count,
                              (void *)node_definition_v1));

  // Execute commands
  mc_command_hub_v1 *command_hub = (mc_command_hub_v1 *)calloc(sizeof(mc_command_hub_v1), 1);
  command_hub->global_node = global;
  command_hub->nodespace = global;
  MCcall(init_command_hub_process_matrix(command_hub));
  command_hub->focused_issue_stack_alloc = 16;
  command_hub->focused_issue_stack = (void **)malloc(sizeof(void *) * command_hub->focused_issue_stack_alloc);
  command_hub->focused_issue_stack_count = 0;
  command_hub->focused_issue_activated = false;
  command_hub->uid_counter = 2000;
  command_hub->scripts_alloc = 32;
  command_hub->scripts = (void **)malloc(sizeof(void *) * command_hub->scripts_alloc);
  command_hub->scripts_count = 0;
  command_hub->script_instances_alloc = 16;
  command_hub->script_instances = (void **)malloc(sizeof(void *) * command_hub->script_instances_alloc);
  command_hub->script_instances_count = 0;

  // declare_and_allocate_anon_struct(template_collection_v1, template_collection, sizeof_template_collection_v1);
  // template_collection->templates_alloc = 400;
  // template_collection->template_count = 0;
  // template_collection->templates = (void **)malloc(sizeof(void *) * template_collection->templates_alloc);
  // command_hub->template_collection = (void *)template_collection;

  // midgeo template_process = (midgeo)malloc(sizeof_void_ptr * 2);
  // allocate_from_cstringv(&template_process[0], "invoke declare_function_pointer");
  // MCcall(mcqck_temp_create_process_declare_function_pointer((midgeo *)&template_process[1]));
  // MCcall(append_to_collection(&template_collection->templates, &template_collection->templates_alloc,
  // &template_collection->template_count, (void *)template_process));

  // template_process = (midgeo)malloc(sizeof_void_ptr * 2);
  // allocate_from_cstringv(&template_process[0], "invoke instantiate_function");
  // MCcall(mcqck_temp_create_process_instantiate_function((midgeo *)&template_process[1]));
  // MCcall(append_to_collection(&template_collection->templates, &template_collection->templates_alloc,
  // &template_collection->template_count, (void *)template_process));

  printf("mm-2\n");
  // Parse & Declare/add Core functions in midge_core_functions.c
  MCcall(init_core_functions(command_hub));
  MCcall(init_process_matrix(command_hub));
  // return 0;

  printf("mm-3\n");
  const char *commands =
      // Create invoke function script
      ".createScript\n"
      "nvi 'function_info *' finfo find_function_info nodespace @function_to_invoke\n"
      "ifs !finfo\n"
      "err 10 \"Could not find function_info for specified function\"\n"
      "end\n"
      ""
      "dcs int rind 0\n"
      "dcl 'char *' responses[32]\n"
      ""
      "dcs int linit finfo->parameter_count\n"
      "ifs finfo->variable_parameter_begin_index >= 0\n"
      "ass linit finfo->variable_parameter_begin_index\n"
      "end\n"
      "for i 0 linit\n"
      "dcl char provocation[512]\n"
      "nvk strcpy provocation finfo->parameters[i]->name\n"
      "nvk strcat provocation \": \"\n"
      "$pi responses[rind] provocation\n"
      "ass rind + rind 1\n"
      "end for\n"
      // "nvk printf \"func_name:%s\\n\" finfo->name\n"
      "ifs finfo->variable_parameter_begin_index >= 0\n"
      "dcs int pind finfo->variable_parameter_begin_index\n"
      "whl 1\n"
      "dcl char provocation[512]\n"
      "nvk strcpy provocation finfo->parameters[pind]->name\n"
      "nvk strcat provocation \": \"\n"
      "$pi responses[rind] provocation\n"
      "nvi bool end_it strcmp responses[rind] \"finish\"\n"
      "ifs !end_it\n"
      "brk\n"
      "end\n"
      // "nvk printf \"responses[1]='%s'\\n\" responses[1]\n"
      "ass rind + rind 1\n"
      "ass pind + pind 1\n"
      "ass pind % pind finfo->parameter_count\n"
      "ifs pind < finfo->variable_parameter_begin_index\n"
      "ass pind finfo->variable_parameter_begin_index\n"
      "end\n"
      "end\n"
      "end\n"
      "$nv @function_to_invoke $ya rind responses\n"
      "|"
      "invoke_function_with_args|"
      // Create Function Sequence
      "create function construct_and_attach_child_node|"
      "demo create function @{function_name=construct_and_attach_child_node}|"
      // -- DEMO declare function
      "invoke declare_function_pointer|"
      "demo invoke @{function_to_invoke=declare_function_pointer}|"
      ".runScript invoke_function_with_args|"
      // ---- SCRIPT SEQUENCE ----
      // ---- void declare_function_pointer(char *function_name, char *return_type, [char *parameter_type,
      // ---- char *parameter_name]...);
      // > function_name:
      "@function_name|"
      // > return_type:
      "void|"
      // > parameter_type:
      "const char *|"
      // > parameter_name:
      "node_name|"
      // > Parameter 1 type:
      "finish|"
      // ---- END SCRIPT SEQUENCE ----
      "enddemo|"
      // -- END DEMO invoke $function_to_invoke
      // -- invoke initialize function
      "invoke instantiate_function|"
      "@function_name|"
      // "nvk printf \"got here, node_name=%s\\n\" node_name\n"
      "dcd node * child\n"
      "cpy char * child->name node_name\n"
      "ass child->parent command_hub->nodespace\n"
      "nvk append_to_collection (void ***)&child->parent->children &child->parent->children_alloc &child->parent->child_count "
      "(void *)child\n"
      "|"
      "enddemo|"
      "midgequit|"
      // -- END DEMO create function $function_name
      "invoke construct_and_attach_child_node|"
      "command_interface_node|"
      "create function print_word|"
      "@function_name|"
      "void|"
      "char *|"
      "word|"
      "finish|"
      ""
      "midgequit|";

  // Command Loop
  printf("\n:> ");
  int n = strlen(commands);
  int s = 0;
  char cstr[2048];
  mc_process_action_v1 *suggestion = NULL;
  void *vargs[12]; // TODO -- count
  for (int i = 0; i < n; ++i) {
    if (commands[i] != '|')
      continue;
    strncpy(cstr, commands + s, i - s);
    cstr[i - s] = '\0';
    s = i + 1;

    vargs[0] = (void *)command_hub;
    vargs[4] = (void *)cstr;
    vargs[6] = (void *)&suggestion;

    if (!strcmp(cstr, "midgequit")) {
      printf("midgequit\n");
      break;
    }

    printf("========================================\n");
    if (suggestion) {
      printf("%s]%s\n>: ", get_action_type_string(suggestion->type), suggestion->dialogue);
      release_process_action(suggestion);
      suggestion = NULL;
    }
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

  if (global->child_count > 0) {
    mc_node_v1 *child = (mc_node_v1 *)global->children[0];
    printf("\n>> global has a child named %s!\n", child->name);
  }
  // printf("\n\nProcess Matrix:\n");
  // print_process_unit(command_hub->process_matrix, 5, 5, 1);

  printf("\n\n</midge_core>\n");
  return 0;
}

int construct_process_action(mc_command_hub_v1 *command_hub, process_action_indent_movement process_movement,
                             mc_process_action_v1 *current_issue, process_action_type type, const char *const dialogue,
                             void *data, mc_process_action_v1 **output);
int construct_process_unit_from_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 *action, mc_process_unit_v1 **output);
int construct_completion_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 *current_focused_issue,
                                char const *const dialogue, bool force_resolution, mc_process_action_v1 **output);

int format_user_response(mc_command_hub_v1 *command_hub, char *command, mc_process_action_v1 **command_action);
int process_matrix_register_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 *process_action);
int command_hub_submit_process_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 *action);
int command_hub_process_outstanding_actions(mc_command_hub_v1 *command_hub);
int systems_process_command_hub_issues(mc_command_hub_v1 *command_hub, void **p_response_action);
int systems_process_command_hub_scripts(mc_command_hub_v1 *command_hub, void **p_response_action);
int suggest_user_process_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 **out_suggestion);

int submit_user_command(int argc, void **argsv)
{
  // printf("suc-0\n");
  mc_command_hub_v1 *command_hub = (mc_command_hub_v1 *)argsv[0];
  char *command = (char *)argsv[4];
  mc_process_action_v1 **suggestion = (mc_process_action_v1 **)argsv[6];

  // Format the User Response as an action
  mc_process_action_v1 *process_action;
  MCcall(format_user_response(command_hub, command, &process_action));

  // printf("suc-1\n");
  // Process command and any/all system responses
  while (1) {
    // printf("suc:loop_begin:process_action=%s\n", get_action_type_string(process_action->type));
    // printf("suc:loop_begin:focused_issue_stack_count=%u\n", command_hub->focused_issue_stack_count);
    {
      // Register the previous action with the process matrix
      mc_process_action_v1 *action = process_action->previous_issue;
      if (action == NULL) {
        action = process_action->contextual_issue;
      }
      if (action != NULL) {
        while (action->next_issue->object_uid != process_action->object_uid) {
          action = action->next_issue;
        }

        MCcall(process_matrix_register_action(command_hub, action));
      }
    }

    // Affect the command hub
    MCcall(command_hub_submit_process_action(command_hub, process_action));

    // printf("suc-2\n");
    // Process the action
    MCcall(command_hub_process_outstanding_actions(command_hub));

    // printf("suc-3\n");
    // Formulate system responses
    void *p_response_action = NULL;
    MCcall(systems_process_command_hub_scripts(command_hub, &p_response_action));
    // printf("suc-4\n");
    if (!p_response_action) {
      MCcall(systems_process_command_hub_issues(command_hub, &p_response_action));
    }

    // printf("suc-5\n");

    // printf("suc-6\n");
    if (!p_response_action) {
      // Send control back to the user
      // MCcall(suggest_user_process_action(command_hub, suggestion));
      break;
    }
    process_action = (mc_process_action_v1 *)p_response_action;
  }
  // printf("suc-7\n");

  // Send control back to the user
  return 0;
}

int format_user_response(mc_command_hub_v1 *command_hub, char *command, mc_process_action_v1 **command_action)
{
  mc_process_action_v1 *process_action;

  if (command_hub->focused_issue_stack_count == 0) {
    MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, NULL, PROCESS_ACTION_USER_UNPROVOKED_COMMAND, command,
                                    NULL, &process_action));
  }
  else {
    // Pop the focused issue from the stack
    mc_process_action_v1 *focused_issue =
        (mc_process_action_v1 *)command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1];
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    switch (focused_issue->type) {
    case PROCESS_ACTION_PM_SEQUENCE_RESOLVED: {
      // Previous sequence concluded: Move back up a tier
      MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_RESOLVE, focused_issue,
                                      PROCESS_ACTION_USER_UNPROVOKED_COMMAND, command, NULL, &process_action));
    } break;
    case PROCESS_ACTION_PM_IDLE: {
      MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, focused_issue,
                                      PROCESS_ACTION_USER_UNPROVOKED_COMMAND, command, NULL, &process_action));
    } break;
    case PROCESS_ACTION_PM_DEMO_INITIATION: {
      MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, focused_issue,
                                      PROCESS_ACTION_USER_UNPROVOKED_COMMAND, command, NULL, &process_action));
      // printf("demo_focused_issue>next:%p\n", focused_issue->next_issue);
    } break;
    case PROCESS_ACTION_PM_UNRESOLVED_COMMAND: {

      MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_RESOLVE, focused_issue,
                                      PROCESS_ACTION_USER_UNPROVOKED_COMMAND, command, NULL, &process_action));
    } break;
    case PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME: {
      MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, focused_issue,
                                      PROCESS_ACTION_USER_CREATED_SCRIPT_NAME, command, NULL, &process_action));
    } break;
    case PROCESS_ACTION_SCRIPT_QUERY: {
      MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, focused_issue, PROCESS_ACTION_USER_SCRIPT_RESPONSE,
                                      command, NULL, &process_action));
    } break;
    default:
      MCerror(2562, "Unhandled PM-query-type:%i  '%s'", focused_issue->type, focused_issue->dialogue);
    }
  }
  *command_action = process_action;
  return 0;
}

int attach_process_unit_to_matrix_branch(mc_process_unit_v1 *branch_unit, mc_process_unit_v1 *process_unit);
int construct_process_action_detail(mc_process_action_v1 *process_action, mc_process_action_detail_v1 **action_detail);
int release_process_action_detail(mc_process_action_detail_v1 **action_detail);
int convert_from_demo_dialogue(char **dialogue);
int process_matrix_register_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 *action)
{
  // printf("pmra-0\n");

  // Remove demonstrations from data
  mc_process_unit_v1 *action_process_unit;
  if (action->next_issue->type == PROCESS_ACTION_PM_DEMO_INITIATION ||
      action->next_issue->type == PROCESS_ACTION_PM_UNRESOLVED_COMMAND || action->type == PROCESS_ACTION_PM_UNRESOLVED_COMMAND ||
      (action->next_issue->type == PROCESS_ACTION_USER_UNPROVOKED_COMMAND &&
       !strncmp(action->next_issue->dialogue, "enddemo", 7))) {
    // printf("### demo invocation registration to process matrix delayed!\n");
    return 0;
  }
  else if (action->type == PROCESS_ACTION_PM_DEMO_INITIATION) {
    // printf("pmra-1\n");
    // Add the trimmed demo invocation action, that was ignored above, instead
    if (action->contextual_issue->type != PROCESS_ACTION_USER_UNPROVOKED_COMMAND || !action->contextual_issue->dialogue ||
        strncmp(action->contextual_issue->dialogue, "demo ", 5)) {
      MCerror(2734, "Unexpected");
    }

    MCcall(construct_process_unit_from_action(command_hub, action->contextual_issue, &action_process_unit));

    construct_process_action_detail(action->next_issue, &action_process_unit->continuance);

    // Transfer the DEMO-INITIATION indentation
    action_process_unit->continuance->process_movement = PROCESS_MOVEMENT_INDENT;

    // printf("### demo command registered to process matrix!\n");
  }
  else if (action->type == PROCESS_ACTION_USER_UNPROVOKED_COMMAND && action->dialogue &&
           !strncmp(action->next_issue->dialogue, "enddemo", 7)) {

    // Add the previously delayed action
    // printf("pmra-1\n");
    // Add the trimmed demo invocation action, that was ignored above, instead
    if (action->contextual_issue->type != PROCESS_ACTION_USER_UNPROVOKED_COMMAND || !action->contextual_issue->dialogue ||
        strncmp(action->contextual_issue->dialogue, "demo ", 5)) {
      MCerror(2734, "Unexpected");
    }
    if (!action->previous_issue) {
      MCerror(2754, "Unexpected");
    }

    MCcall(construct_process_unit_from_action(command_hub, action->previous_issue, &action_process_unit));

    construct_process_action_detail(action->next_issue, &action_process_unit->continuance);

    // Transfer the DEMO-INITIATION indentation
    // action_process_unit->continuance->process_movement = PROCESS_MOVEMENT_INDENT;

    // printf("### demo command registered to process matrix!\n");
  }
  else if (action->previous_issue && action->previous_issue->type == PROCESS_ACTION_PM_DEMO_INITIATION) {
    // printf("pmra-2\n");
    if (action->previous_issue->previous_issue) {
      MCerror(2745, "Unexpected");
    }

    // Construct the process Unit
    MCcall(construct_process_unit_from_action(command_hub, action, &action_process_unit));

    // Skip the DEMO_INITIATION as the previous issue
    MCcall(release_process_action_detail(&action_process_unit->previous_issue));
    MCcall(construct_process_action_detail(NULL, &action_process_unit->previous_issue));

    // Transfer the DEMO-INITIATION indentation
    // action_process_unit->action->process_movement = PROCESS_MOVEMENT_INDENT;
  }
  else {
    // printf("pmra-3\n");
    // Construct and register the focused action inside the process matrix
    MCcall(construct_process_unit_from_action(command_hub, action, &action_process_unit));
  }

  // printf("pmra-4\n");
  // Trim all "demo ..." dialogues
  if (action_process_unit->previous_issue->dialogue && !strncmp(action_process_unit->previous_issue->dialogue, "demo ", 5)) {
    MCcall(convert_from_demo_dialogue(&action_process_unit->previous_issue->dialogue));
  }
  if (action_process_unit->action->dialogue && !strncmp(action_process_unit->action->dialogue, "demo ", 5)) {
    MCcall(convert_from_demo_dialogue(&action_process_unit->action->dialogue));
  }
  // printf("pmra-4a\n");
  if (action_process_unit->contextual_issue->dialogue && !strncmp(action_process_unit->contextual_issue->dialogue, "demo ", 5)) {
    MCcall(convert_from_demo_dialogue(&action_process_unit->contextual_issue->dialogue));
  }
  if (action_process_unit->sequence_root_issue->dialogue &&
      !strncmp(action_process_unit->sequence_root_issue->dialogue, "demo ", 5)) {
    MCcall(convert_from_demo_dialogue(&action_process_unit->sequence_root_issue->dialogue));
  }
  // printf("pmra-5:%p\n", action_process_unit->continuance);
  if (action_process_unit->continuance->dialogue && !strncmp(action_process_unit->continuance->dialogue, "demo ", 5)) {
    // printf("pmra-5a:%s\n", action_process_unit->continuance->dialogue);
    MCcall(convert_from_demo_dialogue(&action_process_unit->continuance->dialogue));
    // MCerror(2779, "TODO:%s", action_process_unit->continuance->dialogue);
  }

  if (action_process_unit->continuance && action_process_unit->continuance->type == PROCESS_ACTION_PM_DEMO_INITIATION) {
    MCerror(2020, "hopper");
  }

  if (action->type == PROCESS_ACTION_PM_DEMO_INITIATION) {
    // printf("\ndemo_process_unit:\n");
    // print_process_unit(action_process_unit, 4, 3, 0);
  }
  // printf("pmra-6\n");
  // Find the appropriate branch to place it, creating a new one if need be
  // Search first amongst action types
  MCcall(attach_process_unit_to_matrix_branch(command_hub->process_matrix, action_process_unit));

  if (action->type == PROCESS_ACTION_PM_DEMO_INITIATION) {
    // printf("\nprocess_matrix:\n");
    // print_process_unit(command_hub->process_matrix, 3, 3, 0);
  }

  return 0;

  // // Obtain the demo begin action
  // mc_process_action_v1 *root_demo_issue = action;
  // while (root_demo_issue->previous_issue) {
  //   root_demo_issue = root_demo_issue->previous_issue;
  // }
  // process_unit_type action_process_unit_type =
  //     root_demo_issue->type == PROCESS_ACTION_PM_DEMO_INITIATION ? PROCESS_TYPE_DEMONSTRATED : PROCESS_TYPE_EXHIBITED;

  // mc_process_unit_v1 *action_process_unit;
  // MCcall(construct_process_unit(command_hub, action_process_unit_type, action->dialogue,
  // get_process_originator(action->type),
  //                               action, &action_process_unit));

  // // Apply this action as a continuance of the previous action (focused_issue)
  // mc_process_action_v1 *focused_issue = NULL;
  // if (command_hub->focused_issue_stack_count == 0) {
  //   mc_process_action_v1 *focused_issue =
  //       (mc_process_action_v1 *)command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1];
  // }
  // if (focused_issue) {
  //   // Find the process unit for the focused issue
  //   for(int i = command_hub->process_matrix->count - 1; i >= 0; --i){
  //     mc_process_unit_v1 *process_unit = (mc_process_unit_v1 *) command_hub->process_matrix->items[i];

  //     if(process_unit->action->)
  //   }
  // }

  // // Insert this action into the process matrix too (with 0 continuances)

  // // -- Construct the process from the demo action sequence
  // mc_process_unit_v1 *demod_process;
  // MCcall(construct_process_unit(command_hub, PROCESS_TYPE_DEMONSTRATED, (char *)root_demo_issue->data,
  // PROCESS_ORIGINATOR_USER,
  //                               root_demo_issue, &demod_process));

  // printf("root_demo_issue>next:%p  type:%i\n", root_demo_issue->next_issue, root_demo_issue->next_issue->type);

  // // Add root to the process matrix
  // printf("procm>demo process added: type:%i dialogue:'%s'\n", demod_process->type, demod_process->dialogue);
  // MCcall(append_to_collection(&command_hub->process_matrix->items, &command_hub->process_matrix->allocated,
  //                             &command_hub->process_matrix->count, demod_process));

  // // Go through each action within the sequence
  // mc_process_action_v1 *action = (mc_process_action_v1 *)root_demo_issue->next_issue;
  // mc_process_unit_v1 *previous_unit = demod_process;
  // while (action) {
  //   if (action->object_uid == focused_issue->object_uid) {
  //     // Don't include demo end action
  //     break;
  //   }

  //   process_originator origin;
  //   printf("processLoopActionType:%i\n", action->type);
  //   switch (action->type) {
  //     // User Initiated
  //   case PROCESS_ACTION_USER_UNPROVOKED_COMMAND:
  //   case PROCESS_ACTION_USER_SCRIPT_ENTRY:
  //   case PROCESS_ACTION_USER_SCRIPT_RESPONSE:
  //   case PROCESS_ACTION_USER_CREATED_SCRIPT_NAME:
  //     origin = PROCESS_ORIGINATOR_USER;
  //     break;
  //     // Process Manager Initiated
  //   case PROCESS_ACTION_PM_IDLE:
  //   case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
  //   case PROCESS_ACTION_PM_DEMO_INITIATION:
  //   case PROCESS_ACTION_PM_SCRIPT_REQUEST:
  //   case PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME:
  //   case PROCESS_ACTION_PM_SEQUENCE_RESOLVED:
  //     origin = PROCESS_ORIGINATOR_PM;
  //     break;
  //     // Script
  //   case PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS:
  //   case PROCESS_ACTION_SCRIPT_QUERY:
  //     origin = PROCESS_ORIGINATOR_SCRIPT;
  //     break;
  //   default:
  //     MCerror(3152, "TODO for type:%i", action->type);
  //     break;
  //   }
  //   mc_process_unit_v1 *process_unit;
  //   MCcall(construct_process_unit(command_hub, PROCESS_TYPE_EXHIBITED, action->dialogue, origin, action, &process_unit));

  //   // Add it as a continuance of the previous process unit
  //   MCcall(append_to_collection((void ***)&previous_unit->continuances, &previous_unit->continuances_alloc,
  //                               &previous_unit->continuances_count, (void *)process_unit));

  //   // Add root to the process matrix
  //   printf("procm>demo process added: type:%i dialogue:'%s'\n", process_unit->type,
  //          process_unit->dialogue == NULL ? "(null)" : process_unit->dialogue);
  //   MCcall(append_to_collection(&command_hub->process_matrix->items, &command_hub->process_matrix->allocated,
  //                               &command_hub->process_matrix->count, (void *)process_unit));

  //   // Continue
  //   action = (mc_process_action_v1 *)action->next_issue;
  //   previous_unit = process_unit;
  // }

  return 0;
}

int command_hub_submit_process_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 *process_action)
{
  void **mc_dvp;

  // printf("submitting:%i\n", process_action->type);
  append_to_collection(&command_hub->focused_issue_stack, &command_hub->focused_issue_stack_alloc,
                       &command_hub->focused_issue_stack_count, process_action);
  command_hub->focused_issue_activated = false;

  // DEBUG
  int depth = 0;
  mc_process_action_v1 *action = process_action;
  while (action->contextual_issue) {
    ++depth;
    action = action->contextual_issue;
  }
  // printf("chspa>(%u:seq=%u) Depth:%i", process_action->object_uid, process_action->sequence_uid, depth);
  // printf(" Submitted:%s", get_action_type_string(process_action->type));
  // printf("\n");

  return 0;
}

int command_hub_process_outstanding_actions(mc_command_hub_v1 *command_hub)
{
  void **mc_dvp;
  if (command_hub->focused_issue_activated || command_hub->focused_issue_stack_count == 0)
    return 0;

  // Process the foremost focused issue
  mc_process_action_v1 *focused_issue =
      (mc_process_action_v1 *)command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1];

  switch (focused_issue->type) {
  case PROCESS_ACTION_USER_UNPROVOKED_COMMAND:
  case PROCESS_ACTION_USER_SCRIPT_ENTRY: {
    // Print to terminal
    printf(">>)%s\n", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;
  } break;
  case PROCESS_ACTION_USER_SCRIPT_RESPONSE:
  case PROCESS_ACTION_USER_CREATED_SCRIPT_NAME: {
    // Print to terminal
    printf("%s\n", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;
  } break;
  case PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS: {
    if (focused_issue->dialogue != NULL) {
      // Print to terminal
      printf("%s\n", focused_issue->dialogue);
      command_hub->focused_issue_activated = true;
    }
  } break;
  case PROCESS_ACTION_SCRIPT_QUERY:
  case PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME: {
    // Print to terminal
    printf("%s", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;
  } break;
  case PROCESS_ACTION_PM_DEMO_INITIATION: {
    // Print to terminal
    if (focused_issue->dialogue != NULL)
      printf("%s\n", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;

    // Indicate user response
    printf(":> ");
  } break;
  case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
  case PROCESS_ACTION_PM_IDLE:
  case PROCESS_ACTION_PM_SEQUENCE_RESOLVED: {
    // Print to terminal
    if (focused_issue->dialogue != NULL)
      printf("%s\n", focused_issue->dialogue);
    command_hub->focused_issue_activated = true;

    // Indicate user response
    printf(":> ");
  } break;
  default:
    MCerror(1511, "UnhandledType:%i", focused_issue->type)
  }

  return 0;
}

int systems_process_command_hub_scripts(mc_command_hub_v1 *command_hub, void **p_response_action)
{
  *p_response_action = NULL;

  if (command_hub->script_instances_count == 0)
    return 0;

  for (int i = 0; i < command_hub->script_instances_count; ++i) {
    mc_script_instance_v1 *script_instance = (mc_script_instance_v1 *)command_hub->script_instances[i];

    if (script_instance->awaiting_data_set_index >= 0)
      continue;

    // printf("script->sequence_uid=%u\n", script->sequence_uid);
    if (script_instance->segments_complete > script_instance->script->segment_count) {
      // Cleanup
      MCerror(5482, "TODO");
    }
    // Execute the script
    char buf[1024];
    // strcpy(buf, SCRIPT_NAME_PREFIX);

    // printf("scriptcont:%i\n", script_instance->contextual_action->contextual_data_count);

    char **output = NULL;
    // printf("script entered: %u: %i / %i\n", script_instance->sequence_uid, script_instance->segments_complete,
    //  script_instance->script->segment_count);
    int mc_script_res;
    sprintf(buf,
            "{\n"
            "  mc_script_instance_v1 *p_script = (mc_script_instance_v1 *)%p;\n"
            "  int *mc_script_res = (int *)%p;\n"
            "  *mc_script_res = %s(p_script);\n"
            "}",
            script_instance, &mc_script_res, script_instance->script->created_function_name);
    clint_process(buf);
    if (mc_script_res) {
      printf("--script '%s' error:%i\n", script_instance->script->name, mc_script_res);
      return mc_script_res;
    }
    // printf("script exited: %u: %i / %i\n", script_instance->sequence_uid, script_instance->segments_complete,
    //  script_instance->script->segment_count);

    // Pop the focused issue from the stack
    mc_process_action_v1 *focused_issue =
        (mc_process_action_v1 *)command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1];
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    // printf("spchs-1\n");
    // printf("focused_issue->seq:%u script_instance->seq:%u\n", focused_issue->sequence_uid, script_instance->sequence_uid);
    if (focused_issue->sequence_uid != script_instance->sequence_uid) {
      MCerror(2734, "incorrect script / process_action association : %u!=%u", script_instance->sequence_uid,
              focused_issue->sequence_uid);
    }

    // printf("spchs-2\n");

    if (script_instance->segments_complete >= script_instance->script->segment_count) {

      // printf("@@@ demo_issue(%u)->data='%s'\n", command_hub->demo_issue->sequence_uid, (char
      // *)command_hub->demo_issue->data); printf("spchs-4\n");
      // // -- Script Complete!
      // // Free script data
      // if (script_instance->contextual_command)
      //   free(script_instance->contextual_command);
      // printf("spchs-4b\n");

      // Locals should be freed at the end of script TODO
      // for (int j = 0; j < script_instance->script->local_count; ++j)
      //   if (script_instance->locals[j])
      //     free(script_instance->locals[j]);
      // printf("spchs-4c\n");
      free(script_instance->locals);

      // printf("@@@ demo_issue(%u)->data='%s'\n", command_hub->demo_issue->sequence_uid, (char
      // *)command_hub->demo_issue->data); printf("spchs-5\n");

      // printf("spchs-4d\n");
      if (script_instance->response)
        free(script_instance->response);

      // printf("spchs-4e\n");
      // Remove script
      remove_from_collection(&command_hub->script_instances, &command_hub->script_instances_alloc,
                             &command_hub->script_instances_count, i);
      --i;

      // printf("spchs-5\n");
      // printf("@@@ demo_issue(%u)->data='%s'\n", command_hub->demo_issue->sequence_uid, (char
      // *)command_hub->demo_issue->data);
      mc_process_action_v1 *script_completion;
      bool resolve_indented_workflow = (focused_issue->contextual_issue->next_issue->type != PROCESS_ACTION_PM_DEMO_INITIATION);
      construct_completion_action(command_hub, focused_issue, " - script completed", resolve_indented_workflow,
                                  &script_completion);

      // printf("spchs-6\n");

      // TODO -- setting response action while also iterating through all scripts isn't right. Fix it up. Find a better
      // way
      *p_response_action = script_completion;
      // printf("@@@ demo_issue(%u)->data='%s'\n", command_hub->demo_issue->sequence_uid, (char
      // *)command_hub->demo_issue->data);
      continue;
    }

    // printf("spchs-7\n");

    // -- The script has exited with a query
    if (!script_instance->response) {
      MCerror(5222, "TODO: response=%s", script_instance->response);
    }

    // printf("spchs-8\n");

    if (focused_issue->type != PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS) {
      MCerror(4245, "TODO"); // Cleanup
      remove_from_collection(&command_hub->script_instances, &command_hub->script_instances_alloc,
                             &command_hub->script_instances_count, i);
      --i;
      continue;
    }

    // printf("spchs-9\n");

    // Begin Query
    mc_process_action_v1 *script_query;
    MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, focused_issue, PROCESS_ACTION_SCRIPT_QUERY,
                                    script_instance->response, NULL, &script_query));
    free(script_instance->response);
    script_instance->response = NULL;

    // printf("spchs-10\n");

    // TODO -- setting response action while also iterating through all scripts isn't right. Fix it up. Find a better
    // way
    *p_response_action = script_query;
  }
  return 0;
}

int systems_process_command_hub_issues(mc_command_hub_v1 *command_hub, void **p_response_action)
{
  *p_response_action = NULL;

  if (command_hub->focused_issue_stack_count == 0)
    return 0;

  // Process the Focused Issue
  mc_process_action_v1 *focused_issue =
      (mc_process_action_v1 *)command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1];

  // Templates first
  switch (focused_issue->type) {
  case PROCESS_ACTION_SCRIPT_QUERY:
  case PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME:
    // case PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS:
    {
      //   // Do not process these commands
      //   // Send to other systems
      return 0;
    }
  case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
  case PROCESS_ACTION_PM_DEMO_INITIATION:
  case PROCESS_ACTION_PM_IDLE:
  case PROCESS_ACTION_PM_SEQUENCE_RESOLVED: {
    // Do not process these commands
    // Send back to user
    return 0;
  }
  case PROCESS_ACTION_USER_UNPROVOKED_COMMAND: {
    // Pop the focused issue from the stack
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    // Script Creation Request
    if (!strncmp(focused_issue->dialogue, ".script", 7) || !strncmp(focused_issue->dialogue, ".createScript", 13)) {
      bool is_creation_only = !strncmp(focused_issue->dialogue, ".createScript", 12);
      // Create the script
      mc_script_v1 *script = (mc_script_v1 *)malloc(sizeof(mc_script_v1));
      script->created_function_name = NULL;
      script->struct_id = NULL;
      ++command_hub->uid_counter;
      script->script_uid = command_hub->uid_counter;
      script->name = NULL;
      script->created_function_name = (char *)malloc(sizeof(char) * (strlen(SCRIPT_NAME_PREFIX) + 5 + 1));
      sprintf(script->created_function_name, "%s%u", SCRIPT_NAME_PREFIX, script->script_uid);

      // -- Parse statements
      MCcall(mcqck_translate_script_code(command_hub->nodespace, script, focused_issue->dialogue + (is_creation_only ? 13 : 7)));

      if (is_creation_only) {
        // Add the script to loaded scripts
        append_to_collection(&command_hub->scripts, &command_hub->scripts_alloc, &command_hub->scripts_count, script);

        // Save the script and do not invoke
        // Requires name
        // Set corresponding issue
        mc_process_action_v1 *script_issue;
        MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_INDENT, focused_issue,
                                        PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME, "Enter Script Name:", (void *)script,
                                        &script_issue));

        // Set as response action
        *p_response_action = (void *)script_issue;
        return 0;
      }

      MCerror(4928, "TODO -- quick invoke temp script");
      // script->created_function_name = NULL;
      // script->locals = (void **)calloc(sizeof(void *), script->local_count);

      // // Invoke the script straightaway as a quick temporary execution
      // declare_and_allocate_anon_struct(script_v1, script, sizeof_script_v1);
      // // script->execution_state = SCRIPT_EXECUTION_STATE_INITIAL;
      // // script->next_statement_index = -1;
      // script_instance_v1 *script_instance;

      // script->sequence_uid = focused_issue->sequence_uid;
      // script->segments_complete = 0;
      // script->awaiting_data_set_index = -1;
      // script->arguments = (void **)malloc(sizeof(void *) * 3);
      // script->response = NULL;
      // // Set as response action
      // *p_response_action = (void *)script_issue;
      // return 0;
    }

    // Script Execution Request
    if (!strncmp(focused_issue->dialogue, ".runScript ", 11)) {
      // Find the script
      char *script_identity;
      allocate_and_copy_cstr(script_identity, focused_issue->dialogue + 11);
      mc_script_v1 *script = NULL;
      for (int i = 0; i < command_hub->scripts_count; ++i) {
        if (!strcmp(script_identity, ((mc_script_v1 *)command_hub->scripts[i])->name)) {
          script = (mc_script_v1 *)command_hub->scripts[i];
          break;
        }
      }
      if (!script) {
        MCerror(7583, "TODO case where no script was found with that name");
      }

      // Invoke it
      // -- Instance
      mc_script_instance_v1 *script_instance = (mc_script_instance_v1 *)malloc(sizeof(mc_script_instance_v1));
      script_instance->script = script;
      script_instance->contextual_action = (mc_process_action_v1 *)focused_issue;
      // if(script_instance->contextual_action->contextual_data_count)
      // for(int m = 0; m < )
      // printf("beforesica:%p\n", script_instance->contextual_action);
      script_instance->struct_id = NULL;
      // printf("focused_issue_");
      script_instance->sequence_uid = focused_issue->sequence_uid;

      script_instance->command_hub = command_hub;
      script_instance->nodespace = command_hub->nodespace;
      // if (focused_issue->contextual_issue == NULL)
      //   script_instance->contextual_command = NULL;
      // else {
      //   mc_process_action_v1 *contextual_issue = (mc_process_action_v1 *)focused_issue->contextual_issue;
      //   switch (contextual_issue->type) {
      //   case PROCESS_ACTION_USER_UNPROVOKED_COMMAND: {
      //     if (!strncmp(contextual_issue->dialogue, "demo ", 5)) {
      //       // Obtain pattern from demo initiation child
      //       mc_process_action_v1 *demo_issue = contextual_issue->next_issue;
      //       if (demo_issue->type != PROCESS_ACTION_PM_DEMO_INITIATION) {
      //         MCerror(3006, "TODO :%i", demo_issue->type);
      //       }
      //       allocate_and_copy_cstr(script_instance->contextual_command, (char *)demo_issue->data);
      //     }
      //     else {
      //       MCerror(3012, "TODO");
      //     }
      //   } break;
      //   case PROCESS_ACTION_PM_DEMO_INITIATION:
      //     allocate_and_copy_cstr(script_instance->contextual_command, (char *)contextual_issue->data);
      //     break;
      //   default:
      //     MCerror(3014, "UNHANDLED type:%i", contextual_issue->type);
      //   }
      // }
      script_instance->locals = (void **)malloc(sizeof(void *) * script->local_count);
      script_instance->response = NULL;
      script_instance->segments_complete = 0;
      script_instance->awaiting_data_set_index = -1;

      // Add to active scripts
      append_to_collection(&command_hub->script_instances, &command_hub->script_instances_alloc,
                           &command_hub->script_instances_count, script_instance);

      // Set corresponding issue
      char *initiation_msg;
      cprintf(initiation_msg, " -- initiating script '%s'...", script->name);
      mc_process_action_v1 *script_initiation;
      MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_INDENT, focused_issue,
                                      PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS, initiation_msg, script_instance,
                                      &script_initiation));
      free(initiation_msg);
      // printf("spchi-14\n");

      // mc_process_action_v1 *root_demo_issue = focused_issue;
      // while (root_demo_issue->previous_issue) {
      //   root_demo_issue = root_demo_issue->previous_issue;
      // }
      // if (root_demo_issue->type != PROCESS_ACTION_PM_DEMO_INITIATION) {
      //   MCerror(2998, "unexpected:%i", root_demo_issue->type);
      // }
      // printf("root_demo_issue>next:%p  type:%i\n", root_demo_issue->next_issue, root_demo_issue->next_issue->type);
      // Set as response action
      *p_response_action = (void *)script_initiation;
      return 0;
    }
    // Attempt to find the action the user is commanding

    // Demo Initiation
    if (!strncmp(focused_issue->dialogue, "demo ", 5)) {
      // ++command_hub->uid_counter;
      // focused_issue->sequence_uid = command_hub->uid_counter;

      // Process the command
      char patbuf[256];
      int bi = 0;
      void *kvps[16];
      int kvps_index = 0;
      for (int i = 5; i < strlen(focused_issue->dialogue); ++i) {
        if (focused_issue->dialogue[i] != '@') {
          patbuf[bi++] = focused_issue->dialogue[i];
          continue;
        }

        // Pattern
        patbuf[bi++] = '@';
        ++i;
        parse_past(focused_issue->dialogue, &i, "{");

        // Parse past variable name
        char *var_name, *var_value;
        parse_past_identifier(focused_issue->dialogue, &i, &var_name, false, false);
        strcpy(patbuf + bi, var_name);
        bi += strlen(var_name);

        parse_past(focused_issue->dialogue, &i, "=");
        int s = i;
        for (; i < strlen(focused_issue->dialogue); ++i) {
          if (focused_issue->dialogue[i] != '}')
            continue;

          var_value = (char *)malloc(sizeof(char) * (i - s + 1));
          strncpy(var_value, focused_issue->dialogue + s, i - s);
          var_value[i - s] = '\0';
          break;
        }
        parse_past(focused_issue->dialogue, &i, "}");
        if (i != strlen(focused_issue->dialogue)) {
          MCerror(3029, "TODO");
        }

        // Add var_name & var_value to issue contextual data
        mc_key_value_pair_v1 *kvp = (mc_key_value_pair_v1 *)malloc(sizeof(mc_key_value_pair_v1));
        kvp->key = var_name;
        kvp->value = (void *)var_value;
        kvps[kvps_index++] = (void *)kvp;
        break;
      }

      // Set pattern
      char *pattern = (char *)malloc(sizeof(char) * (bi + 1));
      strncpy(pattern, patbuf, bi);
      pattern[bi] = '\0';

      // Begin demonstration
      char *demo_issue_dialogue;
      cprintf(demo_issue_dialogue, "Demonstrating '%s' (type 'end' to end).", pattern);
      unsigned int demo_issue_dialogue_alloc = strlen(demo_issue_dialogue) + 1;
      if (kvps_index > 0) {
        char buf[256];
        for (int k = 0; k < kvps_index; ++k) {
          sprintf(buf, "\n - using '$%s'='%s'", ((mc_key_value_pair_v1 *)kvps[k])->key, ((mc_key_value_pair_v1 *)kvps[k])->value);
          MCcall(append_to_cstr(&demo_issue_dialogue_alloc, &demo_issue_dialogue, buf));
        }
      }

      mc_process_action_v1 *demo_issue;
      mc_process_action_v1 *contextual_issue = NULL;
      if (focused_issue)
        contextual_issue = (mc_process_action_v1 *)focused_issue->contextual_issue;
      MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_INDENT, focused_issue, PROCESS_ACTION_PM_DEMO_INITIATION,
                                      demo_issue_dialogue, (void *)pattern, &demo_issue));
      focused_issue->next_issue = demo_issue;
      // printf("demo_issue sequid:%u\n", demo_issue->sequence_uid);

      for (int i = 0; i < kvps_index; ++i) {
        printf("setting contextual data to %u; key:%s value:%s\n", demo_issue->object_uid, ((mc_key_value_pair_v1 *)kvps[i])->key,
               ((mc_key_value_pair_v1 *)kvps[i])->value);
        append_to_collection(&demo_issue->contextual_data->items, &demo_issue->contextual_data->allocated,
                             &demo_issue->contextual_data->count, kvps[i]);
      }
      free(demo_issue_dialogue);
      // // Return the original command to the stack
      // append_to_collection(&command_hub->focused_issue_stack, &command_hub->focused_issue_stack_alloc,
      // &command_hub->focused_issue_stack_count,
      //                      historical_issue);

      // Add a demonstration process on top of the focused issue stack
      *p_response_action = (void *)demo_issue;
      command_hub->demo_issue = demo_issue;
      // printf("@@@ demo_issue(%u)->data='%s'\n", command_hub->demo_issue->sequence_uid, (char
      // *)command_hub->demo_issue->data);
      return 0;
    }

    // Demo End
    if (!strncmp(focused_issue->dialogue, "enddemo", 7)) {

      if (focused_issue == NULL || focused_issue->contextual_issue == NULL) {
        MCerror(9482, "TODO");
      }

      // Conclude the demo
      mc_process_action_v1 *demo_completed_issue;
      construct_completion_action(command_hub, focused_issue, " -- demo completed", true, &demo_completed_issue);

      *p_response_action = (void *)demo_completed_issue;
      return 0;
    }
    // -- Find a suggestion from the process matrix
    // printf("##########################################\n");
    // printf("Begin Template Process:%s\n", process[0]);
    // MCcall(handle_process(argc, argsv));

    // -- Couldn't find one
    // -- Send Unresolved command message
    mc_process_action_v1 *unresolved_issue;
    MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_INDENT, focused_issue, PROCESS_ACTION_PM_UNRESOLVED_COMMAND,
                                    "Unresolved Command.", NULL, &unresolved_issue));

    *p_response_action = (void *)unresolved_issue;

    return 0;
  }
  case PROCESS_ACTION_USER_SCRIPT_RESPONSE: {
    // User response to script provoked query
    // Pop the focused issue from the stack
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    // Obtain the script that queried the response
    mc_script_instance_v1 *script = NULL;
    for (int i = 0; i < command_hub->script_instances_count; ++i) {
      script = (mc_script_instance_v1 *)command_hub->script_instances[i];
      if (focused_issue->sequence_uid == script->sequence_uid)
        break;
      script = NULL;
    }
    if (script == NULL) {
      MCerror(3300, "");
    }

    // Set the requested data on the script
    if (focused_issue->dialogue != NULL) {
      allocate_from_cstringv(&script->locals[script->awaiting_data_set_index], focused_issue->dialogue);
    }
    else {
      *((char **)script->locals[script->awaiting_data_set_index]) = NULL;
    }

    script->awaiting_data_set_index = -1;

    // Set corresponding issue
    mc_process_action_v1 *script_issue;
    MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, focused_issue,
                                    PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS, NULL, NULL, &script_issue));

    *p_response_action = script_issue;

    return 0;
  }
  case PROCESS_ACTION_USER_CREATED_SCRIPT_NAME: {
    // User response to provocation to name created script
    // Pop the focused issue from the stack
    command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1] = NULL;
    --command_hub->focused_issue_stack_count;

    // Get the script from the query issue
    mc_script_v1 *script = (mc_script_v1 *)((mc_process_action_v1 *)focused_issue->previous_issue)->data;
    if (!script) {
      MCerror(9427, "aint supposed to be the case");
    }

    script->name = focused_issue->dialogue;

    if (!focused_issue->contextual_issue) {
      // MCerror(7248, "TODO -- handle the case where the stack would return to empty. How is this chain of actions stored?");//
      // No Storage of discarded actions yet
      return 0;
    }

    // Layer a Idle process action on and return to the loop/user
    mc_process_action_v1 *idle_action;
    char *resolution_message;
    cprintf(resolution_message, "<> script '%s' created!\n", script->name);
    construct_completion_action(command_hub, focused_issue, resolution_message, true, &idle_action);
    free(resolution_message);

    *p_response_action = idle_action;
    return 0;
  }
  default:
    MCerror(3286, "UnhandledType:%i", focused_issue->type)
  }

  MCerror(3289, "Unintended flow");
}

int does_process_unit_match_detail_consensus(mc_process_action_detail_v1 *process_unit,
                                             mc_process_action_detail_v1 *consensus_unit, bool *match_result);
int compare_process_unit_field(mc_process_unit_v1 *process_unit_a, mc_process_unit_v1 *process_unit_b, int field_index,
                               bool *result);
int search_process_matrix_for_best_match(mc_process_unit_v1 *process_unit, mc_process_unit_v1 *matrix_branch,
                                         unsigned int const *const field_search_priority, mc_process_unit_v1 **best_match,
                                         int *best_matched_prioritized_field_count)
{
  // Search for a specific match within the children
  if (matrix_branch->type == PROCESS_MATRIX_NODE) {

    if (matrix_branch->process_unit_field_differentiation_index < 1) {
      printf("process_unit:\n");
      MCcall(print_process_unit(process_unit, 5, 0, 1));
      printf("matrix_branch:\n");
      MCcall(print_process_unit(matrix_branch, 5, 1, 1));
      MCerror(3676, "TODO:%i", matrix_branch->process_unit_field_differentiation_index);
    }

    for (int i = 0; i < matrix_branch->children->count; ++i) {

      mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)matrix_branch->children->items[i];
      bool search_further_with_child = true;
      for (int f = 0; f < *best_matched_prioritized_field_count; ++f) {

        bool matches_field_of_differentation;
        if (matrix_branch->process_unit_field_differentiation_index > PROCESS_UNIT_FIELD_COUNT)
          matches_field_of_differentation = true;
        else {
          MCcall(compare_process_unit_field(process_unit, branch_child, field_search_priority[1 + f],
                                            &matches_field_of_differentation));
        }
        if (!matches_field_of_differentation) {
          search_further_with_child = false;
          break;
        }
      }

      if (!search_further_with_child) {
        continue;
      }

      MCcall(search_process_matrix_for_best_match(process_unit, branch_child, field_search_priority, best_match,
                                                  best_matched_prioritized_field_count));
    }
  }
  else {
    int match_count = *best_matched_prioritized_field_count;
    for (; match_count < field_search_priority[0]; ++match_count) {

      bool matches_field_of_differentation;
      MCcall(compare_process_unit_field(process_unit, matrix_branch, field_search_priority[1 + match_count],
                                        &matches_field_of_differentation));
      if (!matches_field_of_differentation) {
        break;
      }
    }

    if (best_match == NULL || match_count > *best_matched_prioritized_field_count) {
      *best_match = matrix_branch;
      *best_matched_prioritized_field_count = match_count;
    }
  }

  return 0;
}

int does_dialogue_have_pattern(const char *const text, bool *output)
{
  if (text == NULL) {
    *output = false;
    return 0;
  }

  for (int i = 0; i < strlen(text); ++i)
    if (text[i] == '@') {
      *output = true;
      return 0;
    }

  *output = false;
  return 0;
}

int does_dialogue_match_pattern(char const *const dialogue, char const *const pattern, mc_void_collection_v1 *variables,
                                bool *match)
{
  if (dialogue == NULL || pattern == NULL) {
    MCerror(3826, "TODO dialogue=%p pattern=%p", dialogue, pattern);
  }

  // Pattern Match
  mc_key_value_pair_v1 *kvps[256];
  int kvps_index = 0;

  // Determine how well the pattern matches
  char *pattern_args[256];
  char buf[256];

  int d = 0, dn = strlen(dialogue);
  int p = 0, pn = strlen(pattern);
  while (1) {
    // Find the next word in the pattern
    int pi;
    bool is_variable = pattern[p] == '@';
    bool string_end = false;
    for (pi = p; pi <= pn; ++pi) {
      if (pattern[pi] == ' ')
        break;
      if (pattern[pi] == '\0') {
        string_end = true;
        break;
      }
    }

    // Find the next word in the dialogue
    int di;
    for (di = d; di <= dn; ++di) {
      if (dialogue[di] == ' ')
        break;
      if (dialogue[di] == '\0') {
        break;
      }
    }
    if (string_end && di != dn) {
      if (variables) {
        for (int k = 0; k < kvps_index; ++k) {
          free(kvps[k]->key);
          free(kvps[k]->value);
          free(kvps[k]);
          kvps[k] = NULL;
        }
      }
      *match = false;
      return 0;
    }

    if (is_variable) {
      if (variables) {
        mc_key_value_pair_v1 *kvp = (mc_key_value_pair_v1 *)malloc(sizeof(mc_key_value_pair_v1));

        allocate_and_copy_cstrn(kvp->key, (pattern + p + 1), pi - p - 1);
        allocate_and_copy_cstrn(kvp->value, (dialogue + d), di - d);
        kvps[kvps_index++] = kvp;
      }
    }
    else {
      if (strncmp(dialogue + d, pattern + p, di - d)) {
        if (variables) {
          for (int k = 0; k < kvps_index; ++k) {
            free(kvps[k]->key);
            free(kvps[k]->value);
            free(kvps[k]);
            kvps[k] = NULL;
          }
        }
        *match = false;
        return 0;
      }
    }

    // Fitting word
    if (string_end) {
      // Statement match by pattern
      if (variables) {
        // -- Copy all variables to the given collection
        for (int k = 0; k < kvps_index; ++k) {
          MCcall(append_to_collection(&variables->items, &variables->allocated, &variables->count, kvps[k]));
          printf("--dialogue_var_added():%s=%s\n", kvps[k]->key, kvps[k]->value);
          kvps[k] = NULL;
        }
      }

      *match = true;
      return 0;
    }

    // Continue
    d = di + 1;
    p = pi + 1;
  }

  MCerror(3903, "Incorrect flow");
}

int attempt_to_resolve_command(mc_command_hub_v1 *command_hub, mc_process_action_v1 *intercepted_action,
                               void **p_response_action);

int suggest_user_process_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 **out_suggestion)
{
  if (command_hub->focused_issue_stack_count == 0)
    return 0;

  // Process the Focused Issue
  mc_process_action_v1 *focused_issue =
      (mc_process_action_v1 *)command_hub->focused_issue_stack[command_hub->focused_issue_stack_count - 1];

  // printf("aupi-1: focused_issue=%s\n", get_action_type_string(focused_issue->type));

  // Filter the types of actions that can be assisted with
  switch (focused_issue->type) {
    // User Initiated
  case PROCESS_ACTION_USER_UNPROVOKED_COMMAND:
  case PROCESS_ACTION_USER_SCRIPT_ENTRY:
  case PROCESS_ACTION_USER_SCRIPT_RESPONSE:
  case PROCESS_ACTION_USER_CREATED_SCRIPT_NAME:
    break;
    // Process Manager Initiated
  case PROCESS_ACTION_PM_IDLE:
  case PROCESS_ACTION_PM_DEMO_INITIATION:
  case PROCESS_ACTION_PM_SCRIPT_REQUEST:
  case PROCESS_ACTION_PM_QUERY_CREATED_SCRIPT_NAME:
  case PROCESS_ACTION_PM_UNRESOLVED_COMMAND:
    break;
  case PROCESS_ACTION_PM_SEQUENCE_RESOLVED: {
    if (!focused_issue->contextual_issue)
      break;

    // printf("aupi-2: sequence previous:%s:%s\n", get_action_type_string(focused_issue->previous_issue->type),
    //        focused_issue->previous_issue->dialogue == NULL ? "(null)" : focused_issue->previous_issue->dialogue);
    // printf("aupi-2: sequence contextual:%s:%s\n", get_action_type_string(focused_issue->contextual_issue->type),
    //        focused_issue->contextual_issue->dialogue == NULL ? "(null)" : focused_issue->contextual_issue->dialogue);

    mc_process_unit_v1 *process_unit;
    MCcall(construct_process_unit_from_action(command_hub, focused_issue, &process_unit));

    // printf("aupi-2: sequence contextual:%s : %s\n", get_action_type_string(process_unit->contextual_issue->type),
    //        process_unit->contextual_issue->dialogue);
    // printf("aupi-2: sequence root:%s : %s\n", get_action_type_string(process_unit->sequence_root_issue->type),
    //        process_unit->sequence_root_issue->dialogue);

    // printf("\nprocess_matrix:\n");
    // MCcall(print_process_unit(command_hub->process_matrix, 3, 3, 0));

    // Search the process matrix for a similar situation
    mc_process_unit_v1 *matrix = command_hub->process_matrix;

    // Find the best match via the field indices
    unsigned int sequence_resolved_field_priority[5] = {
        4,
        PROCESS_UNIT_FIELD_ACTION_TYPE,
        PROCESS_UNIT_FIELD_CONTEXTUAL_TYPE,
        PROCESS_UNIT_FIELD_CONTEXTUAL_DIALOGUE,
        PROCESS_UNIT_FIELD_PREVIOUS_TYPE,
    };
    mc_process_unit_v1 *best_match = NULL;
    int best_field_match_count = 0;
    MCcall(search_process_matrix_for_best_match(process_unit, matrix, sequence_resolved_field_priority, &best_match,
                                                &best_field_match_count));

    if (best_field_match_count < 3)
      return 0;

    printf("#### AUPI RESOLVED SEQUENCE ####\n");
    // printf("atrc-6a\n");
    // Replace the current unresolved action with the process action
    printf("-#AUPI# Beginning Process:%s-'%s': %i\n", get_action_type_string(best_match->continuance->type),
           best_match->continuance->dialogue, best_match->continuance->process_movement);
    printf("process_unit:\n");
    print_process_unit(process_unit, 6, 1, 0);
    printf("best_match:\n");
    print_process_unit(best_match, 6, 1, 0);

    // command_action->next_issue = NULL;
    MCcall(construct_process_action(command_hub, best_match->continuance->process_movement, focused_issue,
                                    best_match->continuance->type, best_match->continuance->dialogue, NULL, out_suggestion));

    // if (best_match->action->dialogue_has_pattern) {
    //   bool match;
    //   MCcall(does_dialogue_match_pattern(process_unit->action->dialogue, best_match->action->dialogue,
    //                                      replacement->contextual_data, &match));
    //   if (!match) {
    //     MCerror(3738, "TODO");
    //   }
    // }

    // //         // TODO - should dispose of intercepted action...

    // // printf("atrc-7\n");
    // // TODO -- delete the current response and its fields

    // // Replace the response with the resolution
    // *p_response_action = replacement;

    // MCerror(3805, "Unhandled");

  } break;
    // Script
  case PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS:
  case PROCESS_ACTION_SCRIPT_QUERY:
    break;
  default:
    MCerror(3813, "UnhandledType:%i '%s'", focused_issue->type, focused_issue->dialogue);
  }

  // printf("aupi-2\n");

  return 0;
}

int attempt_to_resolve_command(mc_command_hub_v1 *command_hub, mc_process_action_v1 *command_action, void **p_response_action)
{
  // printf("atrc-0\n");
  *p_response_action = NULL;
  // if (!command_action->dialogue || strncmp(command_action->dialogue, "invoke instantiate_", 19)) {
  //   return 0;
  // }

  mc_process_unit_v1 *process_unit;
  MCcall(construct_process_unit_from_action(command_hub, command_action, &process_unit));

  // MCcall(print_process_unit(process_unit, 3, 0, 0));
  // printf("\nprocess_matrix:\n");
  // MCcall(print_process_unit(command_hub->process_matrix, 3, 3, 0));

  // Search the process matrix for a similar situation
  mc_process_unit_v1 *matrix = command_hub->process_matrix;

  // Find the best match via the field indices
  unsigned int unresolved_command_field_priority[5] = {
      3,
      PROCESS_UNIT_FIELD_ACTION_TYPE,
      PROCESS_UNIT_FIELD_ACTION_DIALOGUE,
      PROCESS_UNIT_FIELD_PREVIOUS_TYPE,
      PROCESS_UNIT_FIELD_PREVIOUS_DIALOGUE,
  };
  mc_process_unit_v1 *best_match = NULL;
  int best_field_match_count = 0;
  MCcall(search_process_matrix_for_best_match(process_unit, matrix, unresolved_command_field_priority, &best_match,
                                              &best_field_match_count));

  if (best_field_match_count < 2)
    return 0;

  printf("#### ATRC ####\n");
  // printf("atrc-6a\n");
  // Replace the current unresolved action with the process action
  printf("-#ATRC# Beginning Process:%s-'%s'\n", get_action_type_string(best_match->action->type), best_match->action->dialogue);

  printf("continuance: %s : %s : %i\n", get_action_type_string(best_match->continuance->type), best_match->continuance->dialogue,
         best_match->continuance->process_movement);
  // ERROR
  // ERROR
  // ERROR
  // ERROR
  // ERROR
  mc_process_action_v1 *replacement;
  command_action->next_issue = NULL;
  MCcall(construct_process_action(command_hub, best_match->continuance->process_movement, command_action,
                                  best_match->continuance->type, best_match->continuance->dialogue, NULL, &replacement));

  if (best_match->action->dialogue_has_pattern) {
    bool match;
    MCcall(does_dialogue_match_pattern(process_unit->action->dialogue, best_match->action->dialogue, replacement->contextual_data,
                                       &match));
    if (!match) {
      MCerror(3738, "TODO");
    }
  }

  //         // TODO - should dispose of intercepted action...

  printf("atrc-7\n");
  *p_response_action = replacement;
  return 0;
}

int construct_process_action(mc_command_hub_v1 *command_hub, process_action_indent_movement process_movement,
                             mc_process_action_v1 *current_issue, process_action_type type, const char *const dialogue,
                             void *data, mc_process_action_v1 **output)
{
  // printf("cpa-0\n");
  *output = (mc_process_action_v1 *)malloc(sizeof(mc_process_action_v1));
  (*output)->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
  (*output)->struct_id->identifier = "process_action";
  (*output)->struct_id->version = 1U;

  ++command_hub->uid_counter;
  (*output)->object_uid = command_hub->uid_counter;

  if (current_issue && current_issue->type == PROCESS_ACTION_USER_UNPROVOKED_COMMAND &&
      type != PROCESS_ACTION_PM_SEQUENCE_RESOLVED && process_movement != PROCESS_MOVEMENT_INDENT &&
      (!dialogue || strncmp(dialogue, ".runScript ", 11))) {
    MCerror(3885, "Should you have indented this?");
  }

  // Process Positioning
  if (process_movement > 4 || process_movement < 1) {
    MCerror(3967, "processMOVEMENT:%i", process_movement);
  }
  (*output)->process_movement = process_movement;
  switch ((*output)->process_movement) {
  case PROCESS_MOVEMENT_CONTINUE: {
    // printf("CONTINUE: context %p<>%p\n", current_issue, (current_issue == NULL ? NULL : current_issue->contextual_issue));
    (*output)->contextual_issue = (current_issue == NULL ? NULL : current_issue->contextual_issue);
    (*output)->previous_issue = current_issue;
    if (current_issue) {
      if (current_issue->next_issue) {
        MCerror(3814, "NOT EXPECTED");
      }

      current_issue->next_issue = *output;
    }
  } break;
  case PROCESS_MOVEMENT_INDENT: {
    (*output)->contextual_issue = current_issue;
    (*output)->previous_issue = NULL;
    if (!current_issue || current_issue->next_issue) {
      MCerror(3835, "NOT EXPECTED");
    }

    current_issue->next_issue = *output;
  } break;
  case PROCESS_MOVEMENT_RESOLVE: {
    (*output)->contextual_issue = current_issue->contextual_issue->contextual_issue;
    (*output)->previous_issue = current_issue->contextual_issue;
    if (!current_issue || current_issue->next_issue) {
      MCerror(3913, "NOT EXPECTED");
    }

    current_issue->next_issue = *output;
  } break;
  default:
    MCerror(3825, "Unsupported");
  }
  (*output)->next_issue = NULL;

  // Sequence UID
  if (!(*output)->contextual_issue) {
    // Generate anew
    ++command_hub->uid_counter;
    (*output)->sequence_uid = command_hub->uid_counter;
    // printf("new seq uid:%u to action type:%s dialogue:%s | %p<>%p\n", (*output)->sequence_uid, get_action_type_string(type),
    //        dialogue, (*output)->previous_issue, (*output)->contextual_issue == NULL ? NULL : (*output)->contextual_issue);
  }
  else
    (*output)->sequence_uid = current_issue->sequence_uid;

  // Detail
  (*output)->type = type;
  if (dialogue) {
    allocate_and_copy_cstr((*output)->dialogue, dialogue);
  }
  else {
    (*output)->dialogue = NULL;
  }
  (*output)->data = data;

  // Initialize contextual data collection
  MCcall(init_void_collection_v1(&(*output)->contextual_data));

  // printf("cpa-10\n");
  return 0;
}

int construct_process_action_detail(mc_process_action_v1 *action, mc_process_action_detail_v1 **output)
{
  (*output) = (mc_process_action_detail_v1 *)malloc(sizeof(mc_process_action_detail_v1));
  if (action) {
    (*output)->process_movement = action->process_movement;
    (*output)->type = action->type;
    (*output)->dialogue_has_pattern = false;
    if (action->dialogue) {
      allocate_and_copy_cstr((*output)->dialogue, action->dialogue);
      for (int i = 0; i < strlen((*output)->dialogue); ++i)
        if ((*output)->dialogue[i] == '@') {
          (*output)->dialogue_has_pattern = true;
          break;
        }

      if (!strncmp((*output)->dialogue, "demo ", 5)) {
        MCcall(convert_from_demo_dialogue(&(*output)->dialogue));
      }
    }
    else {
      (*output)->dialogue = NULL;
    }
    get_process_originator_type(action->type, &(*output)->origin);
  }
  else {
    (*output)->process_movement = PROCESS_MOVEMENT_NULL;
    (*output)->type = PROCESS_ACTION_NONE;
    (*output)->dialogue = NULL;
    (*output)->dialogue_has_pattern = false;
    (*output)->origin = (process_originator_type)0;
  }

  return 0;
}

int init_command_hub_process_matrix(mc_command_hub_v1 *command_hub)
{
  // Process Matrix Node
  mc_process_unit_v1 *init_unit = (mc_process_unit_v1 *)malloc(sizeof(mc_process_unit_v1));
  init_unit->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
  init_unit->struct_id->identifier = "process_unit";
  init_unit->struct_id->version = 1U;

  init_unit->type = PROCESS_MATRIX_SAMPLE;
  init_unit->process_unit_field_differentiation_index = 0U;

  init_unit->action = (mc_process_action_detail_v1 *)malloc(sizeof(mc_process_action_detail_v1));
  init_unit->action->type = PROCESS_ACTION_USER_UNPROVOKED_COMMAND;
  allocate_and_copy_cstr(init_unit->action->dialogue, "hello midge");
  init_unit->action->dialogue_has_pattern = false;
  init_unit->action->origin = PROCESS_ORIGINATOR_USER;

  construct_process_action_detail(NULL, &init_unit->previous_issue);
  construct_process_action_detail(NULL, &init_unit->contextual_issue);
  construct_process_action_detail(NULL, &init_unit->sequence_root_issue);

  init_unit->continuance = (mc_process_action_detail_v1 *)malloc(sizeof(mc_process_action_detail_v1));
  init_unit->continuance->process_movement = PROCESS_MOVEMENT_INDENT;
  init_unit->continuance->type = PROCESS_ACTION_PM_IDLE;
  allocate_and_copy_cstr(init_unit->continuance->dialogue, "hello user!");
  init_unit->continuance->dialogue_has_pattern = false;
  init_unit->continuance->origin = PROCESS_ORIGINATOR_PM;

  init_unit->children = NULL;

  // init_unit->continuance_action_type = PROCESS_ACTION_PM_IDLE;
  // allocate_and_copy_cstr(init_unit->continuance_dialogue, "hello user!");
  // init_unit->continuance_dialogue_has_pattern = false;

  // Set
  command_hub->process_matrix = init_unit;

  return 0;
}

int construct_process_unit_from_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 *action, mc_process_unit_v1 **output)
{
  // printf("cpufa-0\n");
  *output = (mc_process_unit_v1 *)malloc(sizeof(mc_process_unit_v1));
  (*output)->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
  (*output)->struct_id->identifier = "process_unit";
  (*output)->struct_id->version = 1U;

  (*output)->type = PROCESS_MATRIX_SAMPLE;
  (*output)->process_unit_field_differentiation_index = 0U;

  // printf("cpufa-1\n");
  // Set Unit Data : Action
  // printf("cpufa-act:%p\n", action);
  MCcall(construct_process_action_detail(action, &(*output)->action));
  // printf("cpufa-1a\n");
  // if (action->previous_issue && action->previous_issue->next_issue->object_uid != action->object_uid) {
  //   MCerror(3860, "EXPECTED");
  // }
  // printf("cpufa-act:%p\n", action);
  MCcall(construct_process_action_detail(action->previous_issue, &(*output)->previous_issue));
  // printf("cpufa-1b\n");
  MCcall(construct_process_action_detail(action->contextual_issue, &(*output)->contextual_issue));

  // printf("cpufa-2\n");
  mc_process_action_v1 *sequence_root;
  if (action->contextual_issue == NULL) {
    sequence_root = NULL;
  }
  else {
    sequence_root = action;
    while (sequence_root->previous_issue) {
      if (sequence_root->previous_issue->type == PROCESS_ACTION_PM_DEMO_INITIATION ||
          sequence_root->previous_issue->type == PROCESS_ACTION_PM_IDLE ||
          sequence_root->previous_issue->type == PROCESS_ACTION_PM_SEQUENCE_RESOLVED)
        break;

      sequence_root = sequence_root->previous_issue;
    }
    if (sequence_root->object_uid == action->object_uid) {
      sequence_root = NULL;
    }
  }
  MCcall(construct_process_action_detail(sequence_root, &(*output)->sequence_root_issue));

  // printf("cpufa-3\n");
  // if (!action->next_issue) {
  //   MCerror(3876, "Should never enter process unit without a continuance result");
  // }

  MCcall(construct_process_action_detail(action->next_issue, &(*output)->continuance));
  // (*output)->continuance_action_type = action->next_issue->type;
  // (*output)->continuance_dialogue = action->next_issue->dialogue;
  // MCcall(does_dialogue_have_pattern((*output)->continuance_dialogue, &(*output)->continuance_dialogue_has_pattern));

  (*output)->children = NULL;

  // printf("cpufa-9\n");
  return 0;
}

int construct_completion_action(mc_command_hub_v1 *command_hub, mc_process_action_v1 *current_focused_issue,
                                char const *const dialogue, bool force_resolution, mc_process_action_v1 **output)
{
  // printf("cca-0\n");
  if (current_focused_issue->contextual_issue == NULL || current_focused_issue->contextual_issue->next_issue == NULL) {
    MCerror(3615, "shouldn't be");
  }

  if (force_resolution) {
    // Resolve
    // printf("cca-RESOLUTION\n");
    MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, current_focused_issue,
                                    PROCESS_ACTION_PM_SEQUENCE_RESOLVED, dialogue, NULL, output));
    return 0;
  }

  // Idle
  // printf("cca-IDLE\n");
  MCcall(construct_process_action(command_hub, PROCESS_MOVEMENT_CONTINUE, current_focused_issue, PROCESS_ACTION_PM_IDLE, dialogue,
                                  NULL, output));
  return 0;
}

int does_process_unit_detail_match(mc_process_action_detail_v1 *process_unit_detail,
                                   mc_process_action_detail_v1 *detail_comparable, bool *result)
{
  // It must be equal to all __specified__ fields of the detail
  if (detail_comparable->type != PROCESS_ACTION_NONE && process_unit_detail->type != detail_comparable->type) {
    *result = false;
    return 0;
  }
  // If the comparison has dialogue - then it must fit the pattern (if one exists, or equal if one does not).
  if (detail_comparable->dialogue) {
    if (!detail_comparable->dialogue_has_pattern) {
      if (strcmp(process_unit_detail->dialogue, detail_comparable->dialogue)) {
        *result = false;
        return 0;
      }
    }
    else {
      bool match;
      MCcall(does_dialogue_match_pattern(process_unit_detail->dialogue, detail_comparable->dialogue, NULL, &match));
      if (!match) {
        *result = false;
        return 0;
      }
    }
  }

  // Origin doesn't matter atm
  *result = true;
  return 0;
}

int does_process_unit_match_action_and_continuance(mc_process_unit_v1 *process_unit, mc_process_unit_v1 *comparable, bool *result)
{
  MCerror(4071, "TODO");
  //   // For a process unit to match:
  //   // The action types must equal
  //   if (process_unit->action->type != comparable->action->type) {
  //     *result = false;
  //     return 0;
  //   }

  //   // The continuance results must be equal
  //   if (comparable->continuance_action_type != process_unit->continuance_action_type) {
  //     *result = false;
  //     return 0;
  //   }
  //   if (!comparable->continuance_dialogue && process_unit->continuance_dialogue) {
  //     *result = false;
  //     return 0;
  //   }
  //   if (comparable->continuance_dialogue_has_pattern) {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(process_unit->continuance_dialogue, comparable->continuance_dialogue, &match));
  //     if (!match) {
  //       *result = false;
  //       return 0;
  //     }
  //   }
  //   else if (strcmp(process_unit->continuance_dialogue, comparable->continuance_dialogue)) {
  //     *result = false;
  //     return 0;
  //   }

  //   // If the comparison has dialogue - then it must fit the pattern (if one exists, or equal if one does not).
  //   if (comparable->action->dialogue) {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(process_unit->action->dialogue, comparable->action->dialogue, &match));
  //     if (!match) {
  //       *result = false;
  //       return 0;
  //     }
  //   }

  *result = true;
  return 0;
}

int does_process_unit_match(mc_process_unit_v1 *process_unit, mc_process_unit_v1 *comparable, bool *result)
{
  MCcall(does_process_unit_match_action_and_continuance(process_unit, comparable, result));
  if (!result) {
    return 0;
  }

  MCcall(does_process_unit_detail_match(process_unit->previous_issue, comparable->previous_issue, result));
  if (!result) {
    return 0;
  }

  MCcall(does_process_unit_detail_match(process_unit->contextual_issue, comparable->contextual_issue, result));
  if (!result) {
    return 0;
  }

  MCcall(does_process_unit_detail_match(process_unit->sequence_root_issue, comparable->sequence_root_issue, result));
  if (!result) {
    return 0;
  }

  *result = true;
  return 0;
}

int clone_process_action_detail(mc_process_action_detail_v1 *process_detail, mc_process_action_detail_v1 **cloned_unit)
{
  (*cloned_unit) = (mc_process_action_detail_v1 *)malloc(sizeof(mc_process_action_detail_v1));
  // TODO -- struct_id
  (*cloned_unit)->type = process_detail->type;
  allocate_and_copy_cstr((*cloned_unit)->dialogue, process_detail->dialogue);
  (*cloned_unit)->dialogue_has_pattern = process_detail->dialogue_has_pattern;
  (*cloned_unit)->origin = process_detail->origin;
  (*cloned_unit)->process_movement = process_detail->process_movement;

  return 0;
}

int clone_process_unit(mc_process_unit_v1 *process_unit, mc_process_unit_v1 **cloned_unit)
{
  // Check
  if (process_unit->children) {
    MCerror(4169, "Incorrect argument state:type=%i ptr_children=%p", process_unit->type, process_unit->children);
  }

  // Clone the unit
  mc_process_unit_v1 *cloned = (mc_process_unit_v1 *)malloc(sizeof(mc_process_unit_v1));
  cloned->struct_id = (mc_struct_id_v1 *)malloc(sizeof(mc_struct_id_v1));
  cloned->struct_id->identifier = "process_unit";
  cloned->struct_id->version = 1U;
  cloned->type = process_unit->type;
  cloned->process_unit_field_differentiation_index = process_unit->process_unit_field_differentiation_index;

  MCcall(clone_process_action_detail(process_unit->action, &cloned->action));
  MCcall(clone_process_action_detail(process_unit->previous_issue, &cloned->previous_issue));
  MCcall(clone_process_action_detail(process_unit->contextual_issue, &cloned->contextual_issue));
  MCcall(clone_process_action_detail(process_unit->sequence_root_issue, &cloned->sequence_root_issue));
  MCcall(clone_process_action_detail(process_unit->continuance, &cloned->continuance));

  cloned->children = NULL;

  *cloned_unit = cloned;

  return 0;
}

int does_process_unit_match_detail_consensus(mc_process_action_detail_v1 *process_unit,
                                             mc_process_action_detail_v1 *consensus_unit, bool *match_result)
{
  if (consensus_unit->type != PROCESS_ACTION_NULL && consensus_unit->type != process_unit->type) {
    *match_result = false;
    return 0;
  }

  if (consensus_unit->dialogue != NULL) {
    if (process_unit->dialogue == NULL) {
      *match_result = false;
      return 0;
    }

    MCcall(does_dialogue_match_pattern(process_unit->dialogue, consensus_unit->dialogue, NULL, match_result));
    if (!*match_result) {
      return 0;
    }
  }

  *match_result = true;

  return 0;
}

int does_process_unit_match_consensus(mc_process_unit_v1 *process_unit, mc_process_unit_v1 *consensus_unit, bool *match_result)
{
  MCcall(does_process_unit_match_detail_consensus(process_unit->action, consensus_unit->action, match_result));
  if (!*match_result) {
    return 0;
  }

  MCcall(does_process_unit_match_detail_consensus(process_unit->previous_issue, consensus_unit->previous_issue, match_result));
  if (!*match_result) {
    return 0;
  }

  MCcall(
      does_process_unit_match_detail_consensus(process_unit->contextual_issue, consensus_unit->contextual_issue, match_result));
  if (!*match_result) {
    return 0;
  }

  MCcall(does_process_unit_match_detail_consensus(process_unit->sequence_root_issue, consensus_unit->sequence_root_issue,
                                                  match_result));

  return 0;
}

int does_process_unit_match_continuance(mc_process_unit_v1 *process_unit, mc_process_unit_v1 *branch_unit, bool *match_result)
{
  MCerror(4266, "TODO");
  //   if (process_unit->continuance_action_type != branch_unit->continuance_action_type) {
  //     *match_result = false;
  //     return 0;
  //   }

  //   MCcall(does_dialogue_match_pattern(process_unit->continuance_dialogue, branch_unit->continuance_dialogue, match_result));

  //   return 0;
  // }

  // int calculate_process_unit_match_score(mc_process_unit_v1 *matrix_unit, bool include_utilization_strength,
  //                                        process_action_type action_type, char const *const action_dialogue,
  //                                        process_action_type previous_issue_type, char const *const previous_issue_dialogue,
  //                                        process_action_type contextual_issue_type, char const *const
  //                                        contextual_issue_dialogue, process_action_type sequence_root_issue_type, char const
  //                                        *const sequence_root_issue_dialogue, unsigned int *score)
  // {
  //   if (include_utilization_strength) {
  //     MCerror(2808, "TODO");
  //   }

  //   // Initialize
  //   *score = 0;

  //   // action
  //   if (matrix_unit->action->type == action_type)
  //     *score += 100;
  //   else if (matrix_unit->action->type == PROCESS_ACTION_NONE)
  //     *score += 20;
  //   if (matrix_unit->action->dialogue == NULL) {
  //     if (action_dialogue == NULL)
  //       *score += 200;
  //     else {
  //       *score += 20;
  //     }
  //   }
  //   else {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(matrix_unit->action->dialogue, action_dialogue, &match));
  //     if (match) {
  //       *score += 200;
  //     }
  //   }

  //   // previous_issue
  //   if (matrix_unit->previous_issue->type == previous_issue_type)
  //     *score += 100;
  //   else if (matrix_unit->previous_issue->type == PROCESS_ACTION_NONE)
  //     *score += 20;
  //   if (matrix_unit->previous_issue->dialogue == NULL) {
  //     if (previous_issue_dialogue == NULL)
  //       *score += 200;
  //     else {
  //       *score += 20;
  //     }
  //   }
  //   else {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(matrix_unit->previous_issue->dialogue, previous_issue_dialogue, &match));
  //     if (match) {
  //       *score += 200;
  //     }
  //   }

  //   // contextual_issue
  //   if (matrix_unit->contextual_issue->type == contextual_issue_type)
  //     *score += 100;
  //   else if (matrix_unit->contextual_issue->type == PROCESS_ACTION_NONE)
  //     *score += 20;
  //   if (matrix_unit->contextual_issue->dialogue == NULL) {
  //     if (contextual_issue_dialogue == NULL)
  //       *score += 200;
  //     else {
  //       *score += 20;
  //     }
  //   }
  //   else {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(matrix_unit->contextual_issue->dialogue, contextual_issue_dialogue, &match));
  //     if (match) {
  //       *score += 200;
  //     }
  //   }

  //   // sequence_root_issue
  //   if (matrix_unit->sequence_root_issue->type == sequence_root_issue_type)
  //     *score += 100;
  //   else if (matrix_unit->sequence_root_issue->type == PROCESS_ACTION_NONE)
  //     *score += 20;
  //   if (matrix_unit->sequence_root_issue->dialogue == NULL) {
  //     if (sequence_root_issue_dialogue == NULL)
  //       *score += 200;
  //     else {
  //       *score += 20;
  //     }
  //   }
  //   else {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(matrix_unit->sequence_root_issue->dialogue, sequence_root_issue_dialogue, &match));
  //     if (match) {
  //       *score += 200;
  //     }
  //   }

  return 0;
}

int release_process_action_detail(mc_process_action_detail_v1 **action_detail)
{
  if (!(*action_detail))
    return 0;

  if ((*action_detail)->dialogue)
    free((*action_detail)->dialogue);

  free(*action_detail);
  *action_detail = NULL;

  return 0;
}

int convert_from_demo_dialogue(char **dialogue)
{
  char buf[1024];
  int b = 0;
  // printf("'%s'\n", *dialogue);

  for (int i = 5;; ++i) {
    if ((*dialogue)[i] == '\0') {
      buf[b++] = '\0';
      break;
    }
    else if ((*dialogue)[i] == '@') {
      buf[b++] = '@';
      MCcall(parse_past(*dialogue, &i, "@{"));

      for (;; ++i) {
        if ((*dialogue)[i] == '=') {
          for (; (*dialogue)[i] != '}'; ++i)
            ;
          if ((*dialogue)[i] != '}') {
            MCerror(4473, "incorrect format");
          }
          break;
        }
        else {
          buf[b++] = (*dialogue)[i];
        }
      }
    }
    else {
      buf[b++] = (*dialogue)[i];
    }
  }

  free((void *)(*dialogue));

  char *cstr = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
  strcpy(cstr, buf);
  *dialogue = cstr;

  return 0;
}

int form_consensus_from_process_unit_detail(mc_process_action_detail_v1 *consensus_detail,
                                            mc_process_action_detail_v1 *unit_detail)
{
  // Type
  if (consensus_detail->type != unit_detail->type)
    consensus_detail->type = PROCESS_ACTION_NULL;

  // Dialogue & pattern
  if (consensus_detail->dialogue != NULL) {
    if (unit_detail->dialogue == NULL) {
      free(consensus_detail->dialogue);
      consensus_detail->dialogue = NULL;
      consensus_detail->dialogue_has_pattern = false;
    }
    else if (consensus_detail->dialogue_has_pattern) {
      bool pattern_match;
      MCcall(does_dialogue_match_pattern(unit_detail->dialogue, consensus_detail->dialogue, NULL, &pattern_match));
      if (!pattern_match) {
        free(consensus_detail->dialogue);
        consensus_detail->dialogue = NULL;
        consensus_detail->dialogue_has_pattern = false;
      }
    }
    else if (!consensus_detail->dialogue_has_pattern) {
      if (unit_detail->dialogue_has_pattern) {
        // Ensure match
        bool pattern_match;
        MCcall(does_dialogue_match_pattern(consensus_detail->dialogue, unit_detail->dialogue, NULL, &pattern_match));

        free(consensus_detail->dialogue);
        if (pattern_match) {
          // Replace
          allocate_and_copy_cstr(consensus_detail->dialogue, unit_detail->dialogue);
          consensus_detail->dialogue_has_pattern = true;
        }
        else {
          consensus_detail->dialogue = NULL;
          consensus_detail->dialogue_has_pattern = false;
        }
      }
      else {
        // consensus_detail exists without pattern, as does unit dialogue
        if (strcmp(consensus_detail->dialogue, unit_detail->dialogue)) {
          free(consensus_detail->dialogue);
          consensus_detail->dialogue = NULL;
          consensus_detail->dialogue_has_pattern = false;
        }
      }
    }
  }

  // Origin
  if (consensus_detail->origin != unit_detail->origin)
    consensus_detail->origin = PROCESS_ORIGINATOR_NULL;

  // Origin
  if (consensus_detail->process_movement != unit_detail->process_movement)
    consensus_detail->process_movement = PROCESS_MOVEMENT_NULL;

  return 0;
}

int form_consensus_from_process_unit_collection(mc_process_unit_v1 *consensus_unit, mc_void_collection_v1 *unit_collection)
{
  // Specify detail fields where they can be specified
  MCcall(release_process_action_detail(&consensus_unit->action));
  MCcall(release_process_action_detail(&consensus_unit->previous_issue));
  MCcall(release_process_action_detail(&consensus_unit->contextual_issue));
  MCcall(release_process_action_detail(&consensus_unit->sequence_root_issue));
  MCcall(release_process_action_detail(&consensus_unit->continuance));

  if (unit_collection->count < 1) {
    MCerror(4129, "TODO");
  }

  MCcall(clone_process_action_detail(((mc_process_unit_v1 *)unit_collection->items[0])->action, &consensus_unit->action));
  MCcall(clone_process_action_detail(((mc_process_unit_v1 *)unit_collection->items[0])->previous_issue,
                                     &consensus_unit->previous_issue));
  MCcall(clone_process_action_detail(((mc_process_unit_v1 *)unit_collection->items[0])->contextual_issue,
                                     &consensus_unit->contextual_issue));
  MCcall(clone_process_action_detail(((mc_process_unit_v1 *)unit_collection->items[0])->sequence_root_issue,
                                     &consensus_unit->sequence_root_issue));
  MCcall(
      clone_process_action_detail(((mc_process_unit_v1 *)unit_collection->items[0])->continuance, &consensus_unit->continuance));

  for (int i = 1; i < unit_collection->count; ++i) {
    mc_process_unit_v1 *collection_unit = (mc_process_unit_v1 *)unit_collection->items[i];

    // consensus
    MCcall(form_consensus_from_process_unit_detail(consensus_unit->action, collection_unit->action));
    MCcall(form_consensus_from_process_unit_detail(consensus_unit->previous_issue, collection_unit->previous_issue));
    MCcall(form_consensus_from_process_unit_detail(consensus_unit->contextual_issue, collection_unit->contextual_issue));
    MCcall(form_consensus_from_process_unit_detail(consensus_unit->sequence_root_issue, collection_unit->sequence_root_issue));
    MCcall(form_consensus_from_process_unit_detail(consensus_unit->continuance, collection_unit->continuance));
  }

  return 0;
}

int find_most_specific_branch_to_form_with(mc_process_unit_v1 *branch_unit, mc_process_unit_v1 *focused_process_unit,
                                           mc_process_unit_v1 **unit_to_form_with)
{
  MCerror(4465, "TODO");
  // // action
  // if (focused_process_unit->action->type == branch_unit->action->type) {
  //   *unit_to_form_with = branch_unit;
  //   return 0;
  // }
  // for (int i = 0; i < branch_unit->branches->count; ++i) {
  //   mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->branches->items[i];
  //   if (focused_process_unit->action->type == branch_child->action->type) {
  //     *unit_to_form_with = branch_child;
  //     return 0;
  //   }
  // }
  // if (focused_process_unit->action->dialogue) {
  //   if (branch_unit->action->dialogue) {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(focused_process_unit->action->dialogue, branch_unit->action->dialogue, &match));
  //     if (match) {
  //       *unit_to_form_with = branch_unit;
  //       return 0;
  //     }
  //   }
  //   for (int i = 0; i < branch_unit->branches->count; ++i) {
  //     mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->branches->items[i];
  //     if (branch_child->action->dialogue) {
  //       bool match;
  //       MCcall(does_dialogue_match_pattern(focused_process_unit->action->dialogue, branch_child->action->dialogue, &match));
  //       if (match) {
  //         *unit_to_form_with = branch_child;
  //         return 0;
  //       }
  //     }
  //   }
  // }

  // // previous_issue
  // if (focused_process_unit->previous_issue->type == branch_unit->previous_issue->type) {
  //   *unit_to_form_with = branch_unit;
  //   return 0;
  // }
  // for (int i = 0; i < branch_unit->branches->count; ++i) {
  //   mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->branches->items[i];
  //   if (focused_process_unit->previous_issue->type == branch_child->previous_issue->type) {
  //     *unit_to_form_with = branch_child;
  //     return 0;
  //   }
  // }
  // if (focused_process_unit->previous_issue->dialogue) {
  //   if (branch_unit->previous_issue->dialogue) {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(focused_process_unit->previous_issue->dialogue,
  //     branch_unit->previous_issue->dialogue,
  //                                        &match));
  //     if (match) {
  //       *unit_to_form_with = branch_unit;
  //       return 0;
  //     }
  //   }
  //   for (int i = 0; i < branch_unit->branches->count; ++i) {
  //     mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->branches->items[i];
  //     if (branch_child->previous_issue->dialogue) {
  //       bool match;
  //       MCcall(does_dialogue_match_pattern(focused_process_unit->previous_issue->dialogue,
  //       branch_child->previous_issue->dialogue,
  //                                          &match));
  //       if (match) {
  //         *unit_to_form_with = branch_child;
  //         return 0;
  //       }
  //     }
  //   }
  // }

  // // sequence_root_issue
  // if (focused_process_unit->sequence_root_issue->type == branch_unit->sequence_root_issue->type) {
  //   *unit_to_form_with = branch_unit;
  //   return 0;
  // }
  // for (int i = 0; i < branch_unit->branches->count; ++i) {
  //   mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->branches->items[i];
  //   if (focused_process_unit->sequence_root_issue->type == branch_child->sequence_root_issue->type) {
  //     *unit_to_form_with = branch_child;
  //     return 0;
  //   }
  // }
  // if (focused_process_unit->sequence_root_issue->dialogue) {
  //   if (branch_unit->sequence_root_issue->dialogue) {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(focused_process_unit->sequence_root_issue->dialogue,
  //                                        branch_unit->sequence_root_issue->dialogue, &match));
  //     if (match) {
  //       *unit_to_form_with = branch_unit;
  //       return 0;
  //     }
  //   }
  //   for (int i = 0; i < branch_unit->branches->count; ++i) {
  //     mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->branches->items[i];
  //     if (branch_child->sequence_root_issue->dialogue) {
  //       bool match;
  //       MCcall(does_dialogue_match_pattern(focused_process_unit->sequence_root_issue->dialogue,
  //                                          branch_child->sequence_root_issue->dialogue, &match));
  //       if (match) {
  //         *unit_to_form_with = branch_child;
  //         return 0;
  //       }
  //     }
  //   }
  // }

  // // sequence_root_issue
  // if (focused_process_unit->contextual_issue->type == branch_unit->contextual_issue->type) {
  //   *unit_to_form_with = branch_unit;
  //   return 0;
  // }
  // for (int i = 0; i < branch_unit->branches->count; ++i) {
  //   mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->branches->items[i];
  //   if (focused_process_unit->contextual_issue->type == branch_child->contextual_issue->type) {
  //     *unit_to_form_with = branch_child;
  //     return 0;
  //   }
  // }
  // if (focused_process_unit->contextual_issue->dialogue) {
  //   if (branch_unit->contextual_issue->dialogue) {
  //     bool match;
  //     MCcall(does_dialogue_match_pattern(focused_process_unit->contextual_issue->dialogue,
  //                                        branch_unit->contextual_issue->dialogue, &match));
  //     if (match) {
  //       *unit_to_form_with = branch_unit;
  //       return 0;
  //     }
  //   }
  //   for (int i = 0; i < branch_unit->branches->count; ++i) {
  //     mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->branches->items[i];
  //     if (branch_child->contextual_issue->dialogue) {
  //       bool match;
  //       MCcall(does_dialogue_match_pattern(focused_process_unit->contextual_issue->dialogue,
  //                                          branch_child->contextual_issue->dialogue, &match));
  //       if (match) {
  //         *unit_to_form_with = branch_child;
  //         return 0;
  //       }
  //     }
  //   }
  // }

  // // No specific match
  // *unit_to_form_with = NULL;
  return 0;
}

int compare_dialogue(mc_process_action_detail_v1 *action_detail_a, mc_process_action_detail_v1 *action_detail_b, bool *result)
{
  if (!action_detail_a->dialogue) {
    *result = !(action_detail_b->dialogue);
  }
  else if (!action_detail_b->dialogue) {
    *result = false;
  }
  else {
    // Compare the dialogue
    if (action_detail_b->dialogue_has_pattern) {
      MCcall(does_dialogue_match_pattern(action_detail_a->dialogue, action_detail_b->dialogue, NULL, result));
    }
    else {
      MCcall(does_dialogue_match_pattern(action_detail_b->dialogue, action_detail_a->dialogue, NULL, result));
    }
  }

  return 0;
}

int compare_process_unit_field(mc_process_unit_v1 *process_unit_a, mc_process_unit_v1 *process_unit_b, int field_index,
                               bool *result)
{
  switch (field_index) {
  case PROCESS_UNIT_FIELD_ACTION_TYPE: {
    *result = process_unit_a->action->type == process_unit_b->action->type;
    return 0;
  }
  case PROCESS_UNIT_FIELD_ACTION_DIALOGUE: {
    MCcall(compare_dialogue(process_unit_a->action, process_unit_b->action, result));
    return 0;
  }
  case PROCESS_UNIT_FIELD_PREVIOUS_TYPE: {
    *result = process_unit_a->previous_issue->type == process_unit_b->previous_issue->type;
    return 0;
  }
  case PROCESS_UNIT_FIELD_PREVIOUS_DIALOGUE: {
    // printf("PROCESS_UNIT_FIELD_PREVIOUS_DIALOGUE\n");
    // printf("process_unit_a->previous_issue='%s'\n", process_unit_a->previous_issue->dialogue);
    // printf("process_unit_b->previous_issue='%s'\n", process_unit_b->previous_issue->dialogue);
    MCcall(compare_dialogue(process_unit_a->previous_issue, process_unit_b->previous_issue, result));
    // printf("compare_dialogue='%s'\n", result ? "true" : "false");
    return 0;
  }
  case PROCESS_UNIT_FIELD_CONTEXTUAL_TYPE: {
    *result = process_unit_a->contextual_issue->type == process_unit_b->contextual_issue->type;
    return 0;
  }
  case PROCESS_UNIT_FIELD_CONTEXTUAL_DIALOGUE: {
    MCcall(compare_dialogue(process_unit_a->contextual_issue, process_unit_b->contextual_issue, result));
    return 0;
  }
  case PROCESS_UNIT_FIELD_SEQUENCE_ROOT_TYPE: {
    *result = process_unit_a->sequence_root_issue->type == process_unit_b->sequence_root_issue->type;
    return 0;
  }
  case PROCESS_UNIT_FIELD_SEQUENCE_ROOT_DIALOGUE: {
    MCcall(compare_dialogue(process_unit_a->sequence_root_issue, process_unit_b->sequence_root_issue, result));
    return 0;
  }

  default:
    MCerror(4795, "Unsupported:%i", field_index);
  }

  return 0;
}

int attach_process_unit_to_matrix_branch(mc_process_unit_v1 *branch_unit, mc_process_unit_v1 *focused_process_unit)
{
  // printf("aputmb-0  %p %p\n", branch_unit, focused_process_unit);
  // printf("\n##attach_process_unit_to_matrix_branch##\n");
  // printf("focused_process_unit:\n");
  // MCcall(print_process_unit(focused_process_unit, 5, 0, 1));
  // printf("branch_unit:\n");
  // MCcall(print_process_unit(branch_unit, 5, 0, 1));

  switch (branch_unit->type) {
  case PROCESS_MATRIX_SAMPLE: {
    int field_difference_index = 1;
    for (; field_difference_index <= PROCESS_UNIT_FIELD_COUNT; ++field_difference_index) {

      bool equivalent;
      MCcall(compare_process_unit_field(focused_process_unit, branch_unit, field_difference_index, &equivalent));
      if (!equivalent) {
        // Fields at this index don't match
        break;
      }
    }
    if (field_difference_index > PROCESS_UNIT_FIELD_COUNT) {
      // There is no difference...
      // printf("focused_process_unit:\n");
      // MCcall(print_process_unit(focused_process_unit, 5, 0, 1));
      // printf("branch_sample:\n");
      // MCcall(print_process_unit(branch_unit, 5, 0, 1));
      // MCerror(4856, "TODO:%i", field_difference_index);
    }

    // Form a branch from the current attached sample unit
    mc_process_unit_v1 *former_unit;
    MCcall(clone_process_unit(branch_unit, &former_unit));

    branch_unit->type = PROCESS_MATRIX_NODE;

    // if (field_difference_index > PROCESS_UNIT_FIELD_COUNT)
    //   field_difference_index = 0;
    branch_unit->process_unit_field_differentiation_index = field_difference_index;
    // printf("aputmb-2: field_difference_index:%i\n", field_difference_index);

    if (branch_unit->continuance->process_movement > 4 || branch_unit->continuance->process_movement < 1) {
      MCerror(4915, "addProblemMOVEMENT:%i", branch_unit->continuance->process_movement);
    }
    if (former_unit->continuance->process_movement > 4 || former_unit->continuance->process_movement < 1) {
      MCerror(4918, "addProblemMOVEMENT:%i", former_unit->continuance->process_movement);
    }
    if (focused_process_unit->continuance->process_movement > 4 || focused_process_unit->continuance->process_movement < 1) {
      MCerror(4921, "addProblemMOVEMENT:%i", focused_process_unit->continuance->process_movement);
    }

    // Add new units
    MCcall(init_void_collection_v1(&branch_unit->children));
    MCcall(append_to_collection((void ***)&branch_unit->children->items, &branch_unit->children->allocated,
                                &branch_unit->children->count, former_unit));
    MCcall(append_to_collection((void ***)&branch_unit->children->items, &branch_unit->children->allocated,
                                &branch_unit->children->count, focused_process_unit));
    MCcall(form_consensus_from_process_unit_collection(branch_unit, branch_unit->children));

    // printf("== 2 Samples combined to form a node:\n");
    // MCcall(print_process_unit(branch_unit, 5, 2, 1));
    return 0;
  }
  case PROCESS_MATRIX_NODE: {

    int field_difference_index = 1;
    for (; field_difference_index <= PROCESS_UNIT_FIELD_COUNT; ++field_difference_index) {

      bool equivalent;
      MCcall(compare_process_unit_field(focused_process_unit, branch_unit, field_difference_index, &equivalent));
      if (!equivalent) {
        // Fields at this index don't match
        // printf("broke at %i\n", field_difference_index);
        break;
      }
    }

    if (field_difference_index > PROCESS_UNIT_FIELD_COUNT) {
      // There is no difference...
      // printf("focused_process_unit:\n");
      // MCcall(print_process_unit(focused_process_unit, 5, 0, 1));
      // printf("branch_unit:\n");
      // MCcall(print_process_unit(branch_unit, 5, 0, 1));
      // MCerror(4806, "TODO");
    }

    if (field_difference_index < branch_unit->process_unit_field_differentiation_index) {
      // Create a node that houses both these nodes
      // Form a node from the current node
      mc_process_unit_v1 *former_unit;
      mc_void_collection_v1 *former_children = branch_unit->children;
      branch_unit->children = NULL;
      MCcall(clone_process_unit(branch_unit, &former_unit));
      former_unit->children = former_children;

      branch_unit->type = PROCESS_MATRIX_NODE;
      branch_unit->process_unit_field_differentiation_index = field_difference_index;

      // Add new units
      MCcall(init_void_collection_v1(&branch_unit->children));
      MCcall(append_to_collection((void ***)&branch_unit->children->items, &branch_unit->children->allocated,
                                  &branch_unit->children->count, former_unit));
      MCcall(append_to_collection((void ***)&branch_unit->children->items, &branch_unit->children->allocated,
                                  &branch_unit->children->count, focused_process_unit));
      MCcall(form_consensus_from_process_unit_collection(branch_unit, branch_unit->children));

      // printf("== The Sample attached with the node to a new node:\n");
      // MCcall(print_process_unit(branch_unit, 5, 2, 1));
      return 0;
    }
    else if (field_difference_index >= branch_unit->process_unit_field_differentiation_index) {

      // printf("field_difference_index=%i\n", field_difference_index);
      // Determine if any existing children match the unit at the difference index
      if (field_difference_index <= PROCESS_UNIT_FIELD_COUNT) {
        for (int i = 0; i < branch_unit->children->count; ++i) {
          mc_process_unit_v1 *branch_child = (mc_process_unit_v1 *)branch_unit->children->items[i];

          bool equivalent;
          MCcall(compare_process_unit_field(focused_process_unit, branch_child, field_difference_index, &equivalent));
          if (equivalent) {
            // A match -- combine it with this node/sample
            // printf("== Sample to be attached to child branch unit:\n");

            MCcall(attach_process_unit_to_matrix_branch(branch_child, focused_process_unit));

            return 0;
          }
        }
      }

      // Attach as the nodes child
      MCcall(append_to_collection((void ***)&branch_unit->children->items, &branch_unit->children->allocated,
                                  &branch_unit->children->count, focused_process_unit));
      MCcall(form_consensus_from_process_unit_collection(branch_unit, branch_unit->children));

      // printf("== The Sample was added as the nodes child:\n");
      // MCcall(print_process_unit(branch_unit, 5, 2, 1));

      return 0;
    }
    else {

      MCerror(4818, "TODO%i", field_difference_index);
    }

    MCerror(5048, "Invalid Path");
  }

  default:
    MCerror(4765, "unsupported for process_unit_type:%i", branch_unit->type);
  }

  MCerror(4768, "Invalid Path");
  // bool match;
  // MCcall(does_process_unit_match_action_and_continuance(focused_process_unit, branch_unit, &match));
  // if (match) {
  //   if (!branch_unit->consensus_process_units) {
  //     // 'Branch' is actually just an attached process action sample. Needs splitting
  //     // Clone it, add it as an consensus process actions

  //     mc_process_unit_v1 *cloned_unit;
  //     MCcall(clone_process_unit(branch_unit, &cloned_unit));

  //     // Keep the original as a branching unit and add the clone as a consensus action process unit
  //     branch_unit->type = PROCESS_UNIT_CONSENSUS_DETAIL;
  //     MCcall(init_void_collection_v1(&branch_unit->consensus_process_units));
  //     MCcall(append_to_collection((void ***)&branch_unit->consensus_process_units->items,
  //                                 &branch_unit->consensus_process_units->allocated,
  //                                 &branch_unit->consensus_process_units->count, cloned_unit));
  //   }

  //   // Add given unit as a consensus process action
  //   MCcall(append_to_collection((void ***)&branch_unit->consensus_process_units->items,
  //                               &branch_unit->consensus_process_units->allocated,
  //                               &branch_unit->consensus_process_units->count, focused_process_unit));
  //   branch_unit->utilization_count = branch_unit->consensus_process_units->count;

  //   // Modify the branch to specify its fields on all of its consensus process action matches
  //   MCcall(adjust_process_unit_to_consensus_details(branch_unit));

  //   return 0;
  // }
  // else {
  //   if (branch_unit->branch.first) {
  //   }
  //   else {
  //     // 'Branch' is actually just an attached process action sample. Needs splitting
  //     // Clone it, add it as the first branch
  //     mc_process_unit_v1 *cloned_unit;
  //     MCcall(clone_process_unit(branch_unit, &cloned_unit));

  //     branch_unit->continuance_action_type = 0;
  //     branch_unit->continuance_dialogue = NULL;
  //     branch_unit->continuance_dialogue_has_pattern = false;

  //     branch_unit->branch.first = cloned_unit;
  //     branch_unit->branch.second = focused_process_unit;

  //     //
  //     // MCcall(adjust_process_unit_to_consensus_details(branch_unit));
  //   }
  // }
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
//     printf("icontext:%i process_unit:%i:%s\n", *(int *)interaction_context[3], process_unit->type,
//     process_unit->debug);
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

int replace_init_file_with_v1_labels(mc_command_hub_v1 *command_hub, char *input, int input_len, char **output)
{
  int fsize = input_len;
  char *out = (char *)malloc(sizeof(char) * ((fsize * 12) / 10));

  char *command_hub_replace = (char *)malloc(sizeof(char) * (26 - 2 + 14 + 1));
  sprintf(command_hub_replace, "((mc_command_hub_v1 *)%p)", command_hub);
  command_hub_replace[38] = '\0';

  int n = 0;
  int s = 0;
  const char *func_marker = "/*mcfuncreplace*/\n";
  for (int i = 0; i < fsize; ++i) {
    if (input[i] == '/') {
      bool marker = true;
      for (int j = 0; j < strlen(func_marker); ++j) {
        if (input[i + j] != func_marker[j]) {
          marker = false;
          break;
        }
      }
      if (!marker)
        continue;

      // Output up to now
      strncpy(out + n, input + s, i - s);
      n += i - s;
      i += strlen(func_marker);

      // strcpy(out + n, insert);
      // n += strlen(insert);

      for (; i < fsize; ++i) {
        if (input[i] != '/')
          continue;

        marker = true;
        for (int j = 0; j < strlen(func_marker); ++j) {
          if (input[i + j] != func_marker[j]) {
            marker = false;
            break;
          }
        }
        if (marker) {
          break;
        }
      }
      if (!marker) {
        MCerror(-9928, "second marker wasn't found");
      }
      i += strlen(func_marker);
      s = i;
    }

    if (!strncmp(input + i, "command_hub", strlen("command_hub")) && strncmp(input + i - 3, "mc_", 3) &&
        strncmp(input + i - 1, "\"", 1)) {
      // Output up to now
      strncpy(out + n, input + s, i - s);
      n += i - s;
      i += strlen("command_hub");
      s = i;

      // Replace it
      strcpy(out + n, command_hub_replace);
      n += strlen(command_hub_replace);
    }

    // function_info >> mc_function_info_v1
    if (!strncmp(input + i, " function_info", strlen(" function_info"))) {
      // Output up to now
      strncpy(out + n, input + s, i - s);
      n += i - s;
      i += strlen(" function_info");
      s = i;

      // Replace it
      strcpy(out + n, " mc_function_info_v1");
      n += strlen(" mc_function_info_v1");
    }
    if (!strncmp(input + i, "(function_info", strlen("(function_info"))) {
      // Output up to now
      strncpy(out + n, input + s, i - s);
      n += i - s;
      i += strlen("(function_info");
      s = i;

      // Replace it
      strcpy(out + n, "(mc_function_info_v1");
      n += strlen("(mc_function_info_v1");
    }

    // struct_info >> mc_struct_info_v1
    if (!strncmp(input + i, " struct_info", strlen(" struct_info"))) {
      // Output up to now
      strncpy(out + n, input + s, i - s);
      n += i - s;
      i += strlen(" struct_info");
      s = i;

      // Replace it
      strcpy(out + n, " mc_struct_info_v1");
      n += strlen(" mc_struct_info_v1");
    }
    if (!strncmp(input + i, "(struct_info", strlen("(struct_info"))) {
      // Output up to now
      strncpy(out + n, input + s, i - s);
      n += i - s;
      i += strlen("(struct_info");
      s = i;

      // Replace it
      strcpy(out + n, "(mc_struct_info_v1");
      n += strlen("(mc_struct_info_v1");
    }

    // node >> mc_node_v1
    if (!strncmp(input + i, " node ", strlen(" node "))) {
      // Output up to now
      strncpy(out + n, input + s, i - s);
      n += i - s;
      i += strlen(" node ");
      s = i;

      // Replace it
      strcpy(out + n, " mc_node_v1 ");
      n += strlen(" mc_node_v1 ");
    }
    if (!strncmp(input + i, "(node ", strlen("(node "))) {
      // Output up to now
      strncpy(out + n, input + s, i - s);
      n += i - s;
      i += strlen("(node ");
      s = i;

      // Replace it
      strcpy(out + n, "(mc_node_v1 ");
      n += strlen("(mc_node_v1 ");
    }
  }
  strncpy(out + n, input + s, fsize - s);
  out[n + fsize - s] = '\0';

  *output = out;

  return 0;
}

int init_process_matrix(mc_command_hub_v1 *command_hub)
{
  // Parse
  FILE *f = fopen("/home/jason/midge/src/process_matrix.c", "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char *input = (char *)malloc(fsize + 1);
  fread(input, sizeof(char), fsize, f);
  fclose(f);

  char *output;
  MCcall(replace_init_file_with_v1_labels(command_hub, input, fsize, &output));

  // clint_process("int (*create_default_mc_struct)(int, void **);");

  clint_declare(output);

  free(input);
  free(output);

  return 0;
}

int init_core_functions(mc_command_hub_v1 *command_hub)
{
  // Declare
  mc_function_info_v1 **all_function_definitions = (mc_function_info_v1 **)malloc(sizeof(mc_function_info_v1) * 20);
  int all_function_definition_count = 0;

  mc_function_info_v1 *find_function_info_definition_v1 = (mc_function_info_v1 *)malloc(sizeof(mc_function_info_v1));
  all_function_definitions[all_function_definition_count++] = find_function_info_definition_v1;
  {
    find_function_info_definition_v1->struct_id = NULL;
    find_function_info_definition_v1->name = "find_function_info";
    find_function_info_definition_v1->latest_iteration = 1U;
    find_function_info_definition_v1->return_type = "function_info *";
    find_function_info_definition_v1->parameter_count = 2;
    find_function_info_definition_v1->parameters =
        (mc_parameter_info_v1 **)malloc(sizeof(void *) * find_function_info_definition_v1->parameter_count);
    find_function_info_definition_v1->variable_parameter_begin_index = -1;
    find_function_info_definition_v1->struct_usage_count = 0;
    find_function_info_definition_v1->struct_usage = NULL;

    mc_parameter_info_v1 *field;
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    find_function_info_definition_v1->parameters[0] = field;
    field->type_name = "node";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "nodespace";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    find_function_info_definition_v1->parameters[1] = field;
    field->type_name = "char";
    field->type_version = 0U;
    field->type_deref_count = 1;
    field->name = "function_name";
  }

  mc_function_info_v1 *declare_function_pointer_definition_v1 = (mc_function_info_v1 *)malloc(sizeof(mc_function_info_v1));
  all_function_definitions[all_function_definition_count++] = declare_function_pointer_definition_v1;
  {
    declare_function_pointer_definition_v1->struct_id = NULL;
    declare_function_pointer_definition_v1->name = "declare_function_pointer";
    declare_function_pointer_definition_v1->latest_iteration = 1U;
    declare_function_pointer_definition_v1->return_type = "void";
    declare_function_pointer_definition_v1->parameter_count = 4;
    declare_function_pointer_definition_v1->parameters =
        (mc_parameter_info_v1 **)malloc(sizeof(void *) * declare_function_pointer_definition_v1->parameter_count);
    declare_function_pointer_definition_v1->variable_parameter_begin_index = 2;
    declare_function_pointer_definition_v1->struct_usage_count = 0;
    declare_function_pointer_definition_v1->struct_usage = NULL;

    mc_parameter_info_v1 *field;
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    declare_function_pointer_definition_v1->parameters[0] = field;
    field->type_name = "char";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "function_name";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    declare_function_pointer_definition_v1->parameters[1] = field;
    field->type_name = "char";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "return_type";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    declare_function_pointer_definition_v1->parameters[2] = field;
    field->type_name = "char";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "parameter_type";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    declare_function_pointer_definition_v1->parameters[3] = field;
    field->type_name = "char";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "parameter_name";
  }

  mc_function_info_v1 *instantiate_function_definition_v1 = (mc_function_info_v1 *)malloc(sizeof(mc_function_info_v1));
  all_function_definitions[all_function_definition_count++] = instantiate_function_definition_v1;
  {
    instantiate_function_definition_v1->struct_id = NULL;
    instantiate_function_definition_v1->name = "instantiate_function";
    instantiate_function_definition_v1->latest_iteration = 1U;
    instantiate_function_definition_v1->return_type = "void";
    instantiate_function_definition_v1->parameter_count = 2;
    instantiate_function_definition_v1->parameters =
        (mc_parameter_info_v1 **)malloc(sizeof(void *) * instantiate_function_definition_v1->parameter_count);
    instantiate_function_definition_v1->variable_parameter_begin_index = -1;
    instantiate_function_definition_v1->struct_usage_count = 0;
    instantiate_function_definition_v1->struct_usage = NULL;

    mc_parameter_info_v1 *field;
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    instantiate_function_definition_v1->parameters[0] = field;
    field->type_name = "char";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "function_name";
    field = (mc_parameter_info_v1 *)malloc(sizeof(mc_parameter_info_v1));
    instantiate_function_definition_v1->parameters[1] = field;
    field->type_name = "char";
    field->type_version = 1U;
    field->type_deref_count = 1;
    field->name = "script";
  }

  // Parse
  FILE *f = fopen("/home/jason/midge/src/midge_core_functions.c", "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char *input = (char *)malloc(fsize + 1);
  fread(input, sizeof(char), fsize, f);
  fclose(f);

  char *output;
  MCcall(replace_init_file_with_v1_labels(command_hub, input, fsize, &output));

  clint_process("int (*declare_function_pointer)(int, void **);");
  clint_process("int (*instantiate_function)(int, void **);");
  clint_process("int (*parse_script_to_mc)(int, void **);");
  clint_process("int (*conform_type_identity)(int, void **);");
  clint_process("int (*create_default_mc_struct)(int, void **);");

  //  printf("processed_core_function:\n%s\n", output);
  clint_declare(output);

  // Declare with cling interpreter
  clint_process("find_function_info = &find_function_info_v1;");
  MCcall(append_to_collection((void ***)&command_hub->global_node->functions, &command_hub->global_node->functions_alloc,
                              &command_hub->global_node->function_count, (void *)find_function_info_definition_v1));

  clint_process("declare_function_pointer = &declare_function_pointer_v1;");
  MCcall(append_to_collection((void ***)&command_hub->global_node->functions, &command_hub->global_node->functions_alloc,
                              &command_hub->global_node->function_count, (void *)declare_function_pointer_definition_v1));

  clint_process("instantiate_function = &instantiate_function_v1;");
  MCcall(append_to_collection((void ***)&command_hub->global_node->functions, &command_hub->global_node->functions_alloc,
                              &command_hub->global_node->function_count, (void *)instantiate_function_definition_v1));

  clint_process("parse_script_to_mc = &parse_script_to_mc_v1;");
  // MCcall(append_to_collection((void ***)&command_hub->global_node->functions, &command_hub->global_node->functions_alloc,
  //             TODO                &command_hub->global_node->function_count, (void *)parse_script_to_mc_definition_v1));

  clint_process("conform_type_identity = &conform_type_identity_v1;");
  // MCcall(append_to_collection((void ***)&command_hub->global_node->functions, &command_hub->global_node->functions_alloc,
  //             TODO                &command_hub->global_node->function_count, (void *)conform_type_identity_definition_v1));

  clint_process("create_default_mc_struct = &create_default_mc_struct_v1;");
  // MCcall(append_to_collection((void ***)&command_hub->global_node->functions, &command_hub->global_node->functions_alloc,
  //             TODO                &command_hub->global_node->function_count, (void
  //             *)create_default_mc_struct_definition_v1));

  // printf("first:%s\n", ((mc_function_info_v1 *)command_hub->global_node->functions[0])->name);

  free(input);
  free(output);

  return 0;
}

// int kvr_count = 3;
// const char **kvr_collection = (char **)malloc(sizeof(char *) * kvr_count * 2);
// kvr_collection[0] = "function_info";
// kvr_collection[1] = "mc_function_info_v1";
// kvr_collection[2] = "struct_info";
// kvr_collection[3] = "mc_struct_info_v1";
// kvr_collection[4] = "node";
// kvr_collection[5] = "mc_node_v1";

// switch (input[i])
// {
// case ' ':
// case '(':
// case '\n':
// case '\t':
// case ';':
//   break;
// default:
//   continue;
// }
// if (!isalpha(input[i]))
//   continue;

// for (int k = 0; k < kvr_count; ++k)
// {
//   int klen = strlen(kvr_collection[k * 2]);
//   switch (input[i + 1 + klen])
//   {
//   case '*':
//   case ' ':
//   case ')':
//     break;
//   default:
//     continue;
//   }
//   if (!strncmp(input + i + 1, kvr_collection[k * 2], klen))
//   {
//     sprintf("replaced\n");
//     // Output up to now
//     strncpy(output + n, input + s, i - s);
//     n += i - s;
//     i += klen;
//     s = i;

//     // Replace it
//     strcpy(output + n, kvr_collection[k * 2 + 1]);
//     n += strlen(kvr_collection[k * 2 + 1]);
//   }
// }
