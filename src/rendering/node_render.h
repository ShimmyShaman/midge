/* node_render.h */
#ifndef NODE_RENDER_H
#define NODE_RENDER_H

#include "midge_common.h"

// typedef struct mc_struct_id_v1 {
//   const char *identifier;
//   unsigned short version;
// } mc_struct_id_v1;

// typedef struct mc_void_collection_v1 {
//   mc_struct_id_v1 *struct_id;
//   unsigned int allocated;
//   unsigned int count;
//   void **items;
// } mc_void_collection_v1;

// int append_to_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count, void
// *item)
// {
//   if (*collection_count + 1 > *collection_alloc) {
//     unsigned int realloc_amount = *collection_alloc + 8 + *collection_alloc / 3;
//     // printf("reallocate collection size %i->%i\n", *collection_alloc, realloc_amount);
//     void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
//     if (new_collection == NULL) {
//       MCerror(304, "append_to_collection malloc error");
//     }

//     if (*collection_alloc) {
//       memcpy(new_collection, *collection, *collection_count * sizeof(void *));
//       free(*collection);
//     }

//     *collection = new_collection;
//     *collection_alloc = realloc_amount;
//   }

//   (*collection)[*collection_count] = item;
//   ++*collection_count;
//   return 0;
// }

// int insert_in_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
//                          int insertion_index, void *item)
// {
//   if (*collection_count + 1 > *collection_alloc) {
//     unsigned int realloc_amount = *collection_alloc + 8 + *collection_alloc / 3;
//     // printf("reallocate collection size %i->%i\n", *collection_alloc, realloc_amount);
//     void **new_collection = (void **)malloc(sizeof(void *) * realloc_amount);
//     if (new_collection == NULL) {
//       MCerror(304, "append_to_collection malloc error");
//     }

//     if (*collection_alloc) {
//       memcpy(new_collection, *collection, *collection_count * sizeof(void *));
//       free(*collection);
//     }

//     *collection = new_collection;
//     *collection_alloc = realloc_amount;
//   }

//   for (int i = *collection_count; i > insertion_index; --i) {
//     (*collection)[i] = (*collection)[i - 1];
//   }
//   (*collection)[insertion_index] = item;
//   ++*collection_count;
//   return 0;
// }

// int remove_from_collection(void ***collection, unsigned int *collection_alloc, unsigned int *collection_count,
//                            int index)
// {
//   *collection[index] = NULL;
//   for (int i = index + 1; i < *collection_count; ++i)
//     *collection[i - 1] = *collection[i];

//   --*collection_count;
//   if (index > 0)
//     *collection[*collection_count] = NULL;

//   return 0;
// }


#endif // NODE_RENDER_H