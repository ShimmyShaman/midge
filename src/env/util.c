// #include "core/core_definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include "core/c_parser_lexer.h"
#include "env/environment_definitions.h"

/*
 * Obtains the latest index of an array which doubles in allocation every time it reaches a count equal to a power of
 * two.
 * @ptr_data will return the pointer to the location in the array of the count-1 item
 */
int mc_grow_array(void **ary, unsigned int *ptr_count, size_t item_allocation_size, void **ptr_data)
{
  unsigned int count = *ptr_count, alloc;
  void *pa = *ary;

  /* every power of two we double array size */
  if ((count & (count - 1)) == 0) {
    if (!count) {
      pa = NULL;
      alloc = 1;
    }
    else
      alloc = count * 2;
    pa = realloc(pa, alloc * item_allocation_size);
    if (!pa) {
      MCerror(9727, "reallocation error");
    }
    *ary = pa;
  }

  if (ptr_data)
    *ptr_data = (void *)((unsigned char *)pa + item_allocation_size * count);
  *ptr_count = count + 1;

  return 0;
}

// @desired_allocation may be zero indicating the reallocate amount will be expanded by a 'reasonable' amount.
// @optional_item_allocation_size must be non-zero, the size of each item to allocate.
int reallocate_array(void **array, unsigned int *current_allocation, unsigned int desired_allocation,
                     size_t item_allocation_size)
{
  unsigned int realloc_amount;
  if (desired_allocation) {
    // Check
    if (desired_allocation < *current_allocation) {
      MCerror(1423, "TODO NotYetImplemented");
    }
    realloc_amount = desired_allocation;
  }
  else
    realloc_amount = *current_allocation + 4 + *current_allocation / 3;

  printf("reallocate array size %i->%i\n", *current_allocation, realloc_amount);
  void *new_array = (void **)malloc(item_allocation_size * realloc_amount);
  if (new_array == NULL) {
    MCerror(32, "apaend_to_array malloc error");
  }

  if (*current_allocation) {
    memcpy(new_array, *array, *current_allocation * item_allocation_size);
    free(*array);
  }

  printf("Expanded array capacity from %u to %u items, each with size=%lu\n", *current_allocation, realloc_amount,
         item_allocation_size);

  *array = new_array;
  *current_allocation = realloc_amount;
  return 0;
}

// @desired_allocation may be zero indicating the reallocate amount will be expanded by a 'reasonable' amount.
// @optional_item_allocation_size may be zero indicating no memory shall be assigned to the later allocation sizes.
int reallocate_collection(void ***collection, unsigned int *current_allocation, unsigned int desired_allocation,
                          size_t optional_item_allocation_size)
{
  unsigned int realloc_amount;
  if (desired_allocation) {
    // Check
    if (desired_allocation < *current_allocation) {
      MCerror(1489, "TODO NotYetImplemented");
    }
    realloc_amount = desired_allocation;
  }
  else
    realloc_amount = *current_allocation + 4 + *current_allocation / 3;

  // printf("reallocate collection size %i->%i\n", *current_allocation, realloc_amount);
  void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
  if (new_collection == NULL) {
    MCerror(32, "apaend_to_collection malloc error");
  }

  if (*current_allocation) {
    memcpy(new_collection, *collection, *current_allocation * sizeof(void *));
    free(*collection);
  }

  // printf("Expanded collection capacity from %i to %i items.", *current_allocation, realloc_amount);
  if (optional_item_allocation_size) {
    for (int i = *current_allocation; i < realloc_amount; ++i) {
      new_collection[i] = (void *)malloc(optional_item_allocation_size);
    }

    printf("Expanded collection capacity from %i to %i items.", *current_allocation, realloc_amount);
    printf(" Also allocated %i items with size=%lu.", realloc_amount - *current_allocation,
           optional_item_allocation_size);
    printf("\n");
  }
  // printf("\n");

  *collection = new_collection;
  *current_allocation = realloc_amount;
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
      print_parse_error(text, *index + i, "see_below", "");
      printf("!parse_past() expected:'%c' was:'%c'\n", sequence[i], text[*index + i]);
      return 1 + i;
    }
  }
}

int mce_parse_past_integer(char *text, int *text_index, int *result)
{
  if (!isdigit(text[*text_index])) {
    MCerror(303, "Not an integer");
  }

  int n = 0;
  while (isdigit(text[*text_index])) {
    n *= 10;
    n += text[*text_index] - '0';
    ++*text_index;
  }

  *result = n;

  return 0;
}
