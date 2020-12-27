#ifndef mc_source_extensions_H
#define mc_source_extensions_H

#include "core/core_definitions.h"

typedef struct source_entity_info {
  mc_source_file_code_segment_type type;
  union {
    struct_info *st_info;
    function_info *fu_info;
    enumeration_info *en_info;
  };
} source_entity_info;

/* Searches the interpreter for declared/defined functions/structs enums with the given name. Fills the destination
 * structure. Will be set to 0 if no such entity found.
 */
int find_source_entity_info(source_entity_info *dest, const char *name);
int mcs_obtain_source_file_info(const char *path, bool create_if_not_exists, mc_source_file_info **source_file);
int mc_insert_segment_judiciously_in_source_file(mc_source_file_info *source_file,
                                                 mc_source_file_code_segment_type type, void *data);
int mcs_insert_struct_declaration(mc_source_file_info *source_file, const char *name);

#endif // mc_source_extensions_H