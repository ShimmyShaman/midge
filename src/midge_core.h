/* midge_core.h */
#ifndef MIDGE_CORE_H
#define MIDGE_CORE_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef void **midgeo;
typedef void **midgeary;
typedef unsigned int uint;

/*
 * @field a (void **) variable to store the created value in.
 */
#define allocate_from_intv(field, val)                                                          \
    *field = (void *)malloc(sizeof(int) * 1);                                                   \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    **(int **)field = val;

#define allocate_from_uintv(field, val)                                                         \
    *field = (void *)malloc(sizeof(uint) * 1);                                                  \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    **(uint **)field = val;

#define allocate_from_cstringv(field, cstr)                                                     \
    *field = (void *)malloc(sizeof(char) * (strlen(cstr) + 1));                                 \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    strcpy((char *)(*field), cstr);

#define MCcall(function)                     \
    res = function;                          \
    if (res)                                 \
    {                                        \
        printf("--" #function ":%i\n", res); \
        return res;                          \
    }

#define process_unit_v1              \
    struct                           \
    {                                \
        struct                       \
        {                            \
            const char *identifier;  \
            uint version;            \
        } struct_id;                 \
        enum process_unit_type type; \
        void *data;                  \
        void *data2;                 \
        void *next;                  \
        const char *debug;           \
    }
#define sizeof_process_unit_v1 (sizeof(void *) * 7)
#define branch_unit_v1                        \
    struct                                    \
    {                                         \
        struct                                \
        {                                     \
            const char *identifier;           \
            unsigned int version;             \
        } struct_id;                          \
        enum branching_interaction_type type; \
        const char *match;                    \
        void *data;                           \
        void *next;                           \
    }
#define sizeof_branch_unit_v1 (sizeof(void *) * 13)
#define node_v1                       \
    struct                            \
    {                                 \
        struct                        \
        {                             \
            const char *identifier;   \
            unsigned int version;     \
        } struct_id;                  \
        const char *name;             \
        void *parent;                 \
        unsigned int functions_alloc; \
        unsigned int function_count;  \
        void **functions;             \
        unsigned int structs_alloc;   \
        unsigned int struct_count;    \
        void **structs;               \
        unsigned int children_alloc;  \
        unsigned int child_count;     \
        void **children;              \
    }
#define sizeof_node_v1 (sizeof(void *) * 13)
#define struct_info_v1              \
    struct                          \
    {                               \
        struct                      \
        {                           \
            const char *identifier; \
            unsigned int version;   \
        } struct_id;                \
        const char *name;           \
        unsigned int version;       \
        unsigned int field_count;   \
        void **fields;              \
    }
#define sizeof_struct_info_v1 (sizeof(void *) * 6)
#define function_info_v1                             \
    struct                                           \
    {                                                \
        struct                                       \
        {                                            \
            const char *identifier;                  \
            unsigned int version;                    \
        } struct_id;                                 \
        const char *name;                            \
        unsigned int latest_iteration;               \
        const char *return_type;                     \
        unsigned int parameter_count;                \
        void **parameters;                           \
        unsigned int variable_parameter_begin_index; \
        unsigned int struct_usage_count;             \
        void **struct_usage;                         \
    }
#define sizeof_function_info_v1 (sizeof(void *) * 10)
#define parameter_info_v1           \
    struct                          \
    {                               \
        struct                      \
        {                           \
            const char *identifier; \
            unsigned int version;   \
        } struct_id;                \
        struct                      \
        {                           \
            const char *identifier; \
            unsigned int version;   \
        } type;                     \
        const char *name;           \
    }
#define sizeof_parameter_info_v1 (sizeof(void *) * 5)

#define allocate_anon_struct(struct, ptr_to_struct, size) \
    struct *ptr_to_struct;                                \
    mc_dvp = (void **)&ptr_to_struct;                     \
    *mc_dvp = malloc(size);
#define declare_and_assign_anon_struct(struct, ptr_to_struct, voidassignee) \
    struct *ptr_to_struct;                                                  \
    mc_dvp = (void **)&ptr_to_struct;                                       \
    *mc_dvp = (void *)voidassignee;
#define assign_anon_struct(ptr_to_struct, voidassignee) \
    mc_dvp = (void **)&ptr_to_struct;                   \
    *mc_dvp = (void *)voidassignee;

int clint_process(const char *str);
int clint_declare(const char *str);
int clint_loadfile(const char *path);
int clint_loadheader(const char *path);

#define command_hub_v1               \
    struct                           \
    {                                \
        struct                       \
        {                            \
            const char *identifier;  \
            uint version;            \
        } struct_id;                 \
        void **;                  \
        void *data2;                 \
        void *next;                  \
        const char *debug;           \
    }
#define sizeof_command_hub_v1 (sizeof(void *) * 7)

#endif MIDGE_CORE_H