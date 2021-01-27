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

// Source Files
int mc_save_source_file_info_to_disk(mc_source_file_info *source_file);
int mcs_obtain_source_file_info(const char *path, bool create_if_not_exists, mc_source_file_info **source_file);
int mcs_insert_segment_judiciously_in_source_file(mc_source_file_info *source_file,
                                                  mc_source_file_code_segment_type type, void *data);

// Struct Manipulation
int mcs_construct_struct_declaration(mc_source_file_info *source_file, const char *name);
int mcs_append_field_to_struct(struct_info *si, const char *type_name, unsigned int type_deref_count,
                               const char *field_name);

int mcs_append_field_to_struct_and_remap(struct_info *si, const char *type_name, unsigned int type_deref_count,
                                         const char *field_name, void **data, void **p_field);

// Function Manipulation
int mcs_construct_function_definition(mc_source_file_info *source_file, const char *name, const char *return_type_name,
                                      unsigned int return_type_deref, int parameter_count, const char **parameters,
                                      const char *code);
int mcs_attach_code_to_function(function_info *fi, const char *code);

// Redefinition
int mc_redefine_function(function_info *function);
int mc_redefine_structure(struct_info *structure);
#endif // mc_source_extensions_H
