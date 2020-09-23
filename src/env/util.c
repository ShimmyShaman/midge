#include "core/core_definitions.h"

#include "env/environment_definitions.h"

// @desired_allocation may be zero indicating the reallocate amount will be expanded by a 'reasonable' amount.
// @optional_item_allocation_size may be zero indicating no memory shall be assigned to the later allocation sizes.
int reallocate_collection(void ***collection, unsigned int *current_allocation, unsigned int desired_allocation,
                          size_t optional_item_allocation_size)
{
  unsigned int realloc_amount;
  if (desired_allocation) {
    // Check
    if (desired_allocation < *current_allocation) {
      MCerror(1414, "TODO NotYetImplemented");
    }
    realloc_amount = desired_allocation;
  }
  else
    realloc_amount = *current_allocation + 4 + *current_allocation / 3;

  // printf("reallocate collection size %i->%i\n", *current_allocation, realloc_amount);
  void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
  if (new_collection == NULL) {
    MCerror(32, "append_to_collection malloc error");
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
  }
  printf("\n");

  *collection = new_collection;
  *current_allocation = realloc_amount;
}