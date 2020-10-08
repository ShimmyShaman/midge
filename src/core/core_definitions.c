
#include "core/core_definitions.h"
#include <stdio.h>

int read_file_text(const char *filepath, char **output)
{
  // printf("rft-0\n");
  // printf("rft-0 '%s' '%p'\n", filepath, output);
  // Parse
  FILE *f = fopen(filepath, "rb");
  // printf("rft-1\n");
  if (f == NULL) {
    // printf("rft-2\n");
    MCerror(2263, "File '%s' not found!", filepath);
  }
  // printf("rft-3\n");
  fseek(f, 0, SEEK_END);
  // printf("rft-4\n");
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */
  // printf("rft-5\n");

  char *input = (char *)malloc(fsize + 1);
  // printf("rft-6\n");
  fread(input, sizeof(char), fsize, f);
  // printf("rft-7\n");
  input[fsize] = '\0';
  fclose(f);
  // printf("rft-8\n");

  *output = input;
  // printf("rft-9\n");
  return 0;
}

int append_to_collection(void ***collection, unsigned int *collection_capacity, unsigned int *collection_count,
                         void *item)
{
  if (*collection_count + 1 > *collection_capacity) {
    unsigned int recapacity_amount = *collection_capacity + 8 + *collection_capacity / 3;
    // printf("recapacity collection size %i->%i\n", *collection_capacity, recapacity_amount);
    void **new_collection = (void **)malloc(sizeof(void *) * recapacity_amount);
    if (new_collection == NULL) {
      MCerror(32, "append_to_collection malloc error");
    }

    if (*collection_capacity) {
      memcpy(new_collection, *collection, *collection_count * sizeof(void *));
      free(*collection);
    }

    *collection = new_collection;
    *collection_capacity = recapacity_amount;
  }

  (*collection)[*collection_count] = item;
  ++*collection_count;
  return 0;
}

int insert_in_collection(void ***collection, unsigned int *collection_capacity, unsigned int *collection_count,
                         int insertion_index, void *item)
{
  if (*collection_count + 1 > *collection_capacity) {
    unsigned int recapacity_amount = *collection_capacity + 8 + *collection_capacity / 3;
    // printf("recapacity collection size %i->%i\n", *collection_capacity, recapacity_amount);
    void **new_collection = (void **)malloc(sizeof(void *) * recapacity_amount);
    if (new_collection == NULL) {
      MCerror(57, "append_to_collection malloc error");
    }

    if (*collection_capacity) {
      memcpy(new_collection, *collection, *collection_count * sizeof(void *));
      free(*collection);
    }

    *collection = new_collection;
    *collection_capacity = recapacity_amount;
  }

  for (int i = *collection_count; i > insertion_index; --i) {
    (*collection)[i] = (*collection)[i - 1];
  }
  (*collection)[insertion_index] = item;
  ++*collection_count;
  return 0;
}

int remove_from_collection(void ***collection, unsigned int *collection_count, int index)
{
  register_midge_error_tag("remove_from_collection( %p : %u : %i )", *collection, *collection_count, index);
  (*collection)[index] = NULL;
  for (int i = index + 1; i < *collection_count; ++i)
    (*collection)[i - 1] = (*collection)[i];

  --*collection_count;
  if (index > 0)
    (*collection)[*collection_count] = NULL;

  return 0;
}

int remove_ptr_from_collection(void ***collection, unsigned int *collection_count, bool return_error_on_failure,
                               void *ptr)
{
  bool found = false;
  for (int a = 0; a < *collection_count; ++a) {
    if ((*collection)[a] == ptr) {
      remove_from_collection(collection, collection_count, a);
      found = true;
      break;
    }
  }
  if (!found && return_error_on_failure) {
    MCerror(103, "Could not find given ptr(=%p) in collection", ptr);
  }

  return 0;
}

int find_function_info(char *name, function_info **result)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  for (int i = 0; i < global_data->functions.count; ++i) {
    if (!strcmp(name, global_data->functions.items[i]->name)) {
      *result = global_data->functions.items[i];
      return 0;
    }
  }

  for (int i = 0; i < global_data->function_declarations.count; ++i) {
    if (!strcmp(name, global_data->function_declarations.items[i]->name)) {
      *result = global_data->function_declarations.items[i];
      return 0;
    }
  }

  *result = NULL;

  // TODO -- ?? Search in children
  return 0;
}

int find_function_info_by_ptr(void *function_ptr, function_info **result)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  for (int i = 0; i < global_data->functions.count; ++i) {
    // printf("compare-fffbyptr! %p <> %p  '%s'\n", function_ptr, global_data->functions.items[i]->ptr_declaration,
    //  global_data->functions.items[i]->name);
    if (function_ptr == global_data->functions.items[i]->ptr_declaration) {
      // printf("found! %p <> %p  '%s' %p\n", function_ptr, global_data->functions.items[i]->ptr_declaration,
      //  global_data->functions.items[i]->name, result);
      *result = global_data->functions.items[i];
      return 0;
    }
  }

  *result = NULL;

  // TODO -- ?? Search in children
  return 0;
}

int find_struct_info(char *name, struct_info **result)
{
  // printf("fsi-0\n");
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);
  // printf("fsi-1 %s\n", name);

  for (int i = 0; i < global_data->structs.count; ++i) {
    if (!strcmp(name, global_data->structs.items[i]->name)) {
      *result = global_data->structs.items[i];
      return 0;
    }
  }
  // printf("fsi-2\n");

  *result = NULL;

  // TODO -- ?? Search in children
  return 0;
}

int find_enumeration_info(char *name, enumeration_info **result)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // printf("find_enum(%s)=", name);
  for (int i = 0; i < global_data->enumerations.count; ++i) {
    // if (strcmp(name, "int") && strcmp(name, "char"))
    if (!strcmp(name, global_data->enumerations.items[i]->name)) {
      *result = global_data->enumerations.items[i];
      // printf("found\n");
      return 0;
    }
  }

  *result = NULL;
  // printf("(null)\n");

  // TODO -- ?? Search in children
  return 0;
}

int find_enum_member_info(char *name, enumeration_info **result_type, enum_member_info **result)
{
  global_root_data *global_data;
  obtain_midge_global_root(&global_data);

  // printf("find_enum(%s)=", name);
  for (int i = 0; i < global_data->enumerations.count; ++i) {
    enumeration_info *enum_info = global_data->enumerations.items[i];
    for (int m = 0; m < enum_info->members.count; ++m) {
      // printf("femi- %s<>%s\n", name, enum_info->members.items[m]->identity);
      if (!strcmp(name, enum_info->members.items[m]->identity)) {
        *result = enum_info->members.items[m];
        *result_type = enum_info;
        return 0;
      }
    }
  }

  *result = NULL;
  *result_type = NULL;
  // printf("(null)\n");

  // TODO -- ?? Search in children
  return 0;
}

int release_struct_id(struct_id *ptr)
{
  if (!ptr)
    return 0;
  if (ptr->identifier) {
    free(ptr->identifier);
  }
  free(ptr);

  return 0;
}

// typedef struct field_info {
//   struct_id *type_id;
//   field_kind field_type;

//   union {
//     struct {
//       char *type_name;
//       struct_info *type_info;
//       field_declarator_info_list *declarators;
//     } field;
//     struct {
//       bool is_union;
//       char *type_name;
//       field_info_list *fields;
//       field_declarator_info_list *declarators;
//     } sub_type;
//   };
// } field_info;

int release_field_declarator_info(field_declarator_info *declarator)
{
  if (!declarator) {
    return 0;
  }

  MCerror(210, "TODO");

  return 0;
}

int release_field_declarator_info_list(field_declarator_info_list *declarator_list)
{
  if (declarator_list) {
    if (declarator_list->items && declarator_list->alloc) {
      for (int i = 0; i < declarator_list->count; ++i) {
        release_field_declarator_info(declarator_list->items[i]);
      }
      free(declarator_list->items);
    }
    free(declarator_list);
  }

  return 0;
}

int release_field_info(field_info *field)
{
  if (!field)
    return 0;

  release_struct_id(field->type_id);
  switch (field->field_type) {
  case FIELD_KIND_STANDARD: {
    if (field->field.type_name)
      free(field->field.type_name);
    if (field->field.declarators)
      release_field_declarator_info_list(field->field.declarators);
  } break;
  // case FIELD_KIND_NESTED_UNION: {

  // } break;
  default: {
    MCerror(214, "release_field_info:Unsupported:%i", field->field_type);
  }
  }

  return 0;
}

int release_field_info_list(field_info_list *field_list)
{
  if (field_list) {
    if (field_list->items && field_list->alloc) {
      for (int i = 0; i < field_list->count; ++i) {
        release_field_info(field_list->items[i]);
      }
      free(field_list->items);
    }
    free(field_list);
  }

  return 0;
}

int release_parameter_info(parameter_info *ptr)
{
  if (!ptr)
    return 0;

  switch (ptr->parameter_type) {
  case PARAMETER_KIND_STANDARD: {
    if (ptr->type_name) {
      free(ptr->type_name);
    }
  } break;
  case PARAMETER_KIND_VARIABLE_ARGS:
    break;
  case PARAMETER_KIND_FUNCTION_POINTER: {
    if (ptr->return_type) {
      free(ptr->return_type);
    }
  } break;
  default:
    MCerror(341, "Update for type:%i", ptr->parameter_type);
  }

  if (ptr->name) {
    free(ptr->name);
  }
  if (ptr->type_id) {
    release_struct_id(ptr->type_id);
  }
  free(ptr);

  return 0;
}