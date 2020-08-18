
#include "core/core_definitions.h"
#include <stdio.h>

int read_file_text(char *filepath, char **output)
{
  // Parse
  FILE *f = fopen(filepath, "rb");
  if (f == NULL) {
    MCerror(2263, "File '%s' not found!", filepath);
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char *input = (char *)malloc(fsize + 1);
  fread(input, sizeof(char), fsize, f);
  input[fsize] = '\0';
  fclose(f);

  *output = input;
  return 0;
}

int append_to_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count, void *item)
{
  if (*collection_count + 1 > *collection_alloc) {
    unsigned int realloc_amount = *collection_alloc + 8 + *collection_alloc / 3;
    // printf("reallocate collection size %i->%i\n", *collection_alloc, realloc_amount);
    void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
    if (new_collection == NULL) {
      MCerror(32, "append_to_collection malloc error");
    }

    if (*collection_alloc) {
      memcpy(new_collection, *collection, *collection_count * sizeof(void *));
      free(*collection);
    }

    *collection = new_collection;
    *collection_alloc = realloc_amount;
  }

  (*collection)[*collection_count] = item;
  ++*collection_count;
  return 0;
}

int insert_in_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
                         int insertion_index, void *item)
{
  if (*collection_count + 1 > *collection_alloc) {
    unsigned int realloc_amount = *collection_alloc + 8 + *collection_alloc / 3;
    // printf("reallocate collection size %i->%i\n", *collection_alloc, realloc_amount);
    void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
    if (new_collection == NULL) {
      MCerror(57, "append_to_collection malloc error");
    }

    if (*collection_alloc) {
      memcpy(new_collection, *collection, *collection_count * sizeof(void *));
      free(*collection);
    }

    *collection = new_collection;
    *collection_alloc = realloc_amount;
  }

  for (int i = *collection_count; i > insertion_index; --i) {
    (*collection)[i] = (*collection)[i - 1];
  }
  (*collection)[insertion_index] = item;
  ++*collection_count;
  return 0;
}

int remove_from_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
                           int index)
{
  *collection[index] = NULL;
  for (int i = index + 1; i < *collection_count; ++i)
    *collection[i - 1] = *collection[i];

  --*collection_count;
  if (index > 0)
    *collection[*collection_count] = NULL;

  return 0;
}

int find_function_info(char *name, function_info **result)
{
  global_root_data *global_data;
  MCcall(obtain_midge_global_root(&global_data));

  for (int i = 0; i < global_data->functions.count; ++i) {
    if (!strcmp(name, global_data->functions.items[i]->name)) {
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
  global_root_data *global_data;
  MCcall(obtain_midge_global_root(&global_data));

  for (int i = 0; i < global_data->structs.count; ++i) {
    if (!strcmp(name, global_data->structs.items[i]->name)) {
      *result = global_data->structs.items[i];
      return 0;
    }
  }

  *result = NULL;

  // TODO -- ?? Search in children
  return 0;
}

int find_enumeration_info(char *name, enumeration_info **result)
{
  global_root_data *global_data;
  MCcall(obtain_midge_global_root(&global_data));

  for (int i = 0; i < global_data->enumerations.count; ++i) {
    if (!strcmp(name, global_data->enumerations.items[i]->name)) {
      *result = global_data->enumerations.items[i];
      return 0;
    }
  }

  *result = NULL;

  // TODO -- ?? Search in children
  return 0;
}