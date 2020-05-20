/* midge_core.h */
#ifndef MIDGE_CORE_H
#define MIDGE_CORE_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#ifndef __cplusplus
typedef unsigned char bool;
static const bool false = 0;
static const bool true = 1;
#endif

typedef void **midgeo;
typedef void **midgeary;
typedef unsigned int uint;

enum process_unit_type
{
    PROCESS_UNIT_INTERACTION_INCR_DPTR = 1,
    PROCESS_UNIT_INTERACTION,
    PROCESS_UNIT_BRANCHING_INTERACTION,
    PROCESS_UNIT_INVOKE,
    PROCESS_UNIT_SET_CONTEXTUAL_DATA,
    PROCESS_UNIT_SET_NODESPACE_FUNCTION_INFO,
};

enum branching_interaction_type
{
    BRANCHING_INTERACTION_IGNORE_DATA = 1,
    BRANCHING_INTERACTION_INCR_DPTR,
    BRANCHING_INTERACTION,
    BRANCHING_INTERACTION_INVOKE,
};

enum interaction_process_state
{
    INTERACTION_PROCESS_STATE_INITIAL = 1,
    INTERACTION_PROCESS_STATE_POSTREPLY,
};

enum process_contextual_data
{
    PROCESS_CONTEXTUAL_DATA_NODESPACE = 1,
};

enum process_action_type
{
    PROCESS_ACTION_USER_UNPROVOKED_COMMAND = 1,
    PROCESS_ACTION_USER_UNRESOLVED_RESPONSE,
    PROCESS_ACTION_USER_SCRIPT_ENTRY,
    PROCESS_ACTION_USER_SCRIPT_RESPONSE,

    PROCESS_ACTION_PM_UNRESOLVED_COMMAND,
    PROCESS_ACTION_PM_DEMO_INITIATION,
    PROCESS_ACTION_PM_SCRIPT_REQUEST,

    PROCESS_ACTION_SCRIPT_EXECUTION_IN_PROGRESS,
    PROCESS_ACTION_SCRIPT_QUERY,
};

enum script_process_state
{
    SCRIPT_PROCESS_NOT_STARTED = 1,
    SCRIPT_PROCESS_EXECUTION,
    SCRIPT_PROCESS_QUERY_USER,
};

#define SCRIPT_NAME_PREFIX "mc_script_"

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

#define allocate_and_copy_cstr(dest, src)                    \
    dest = (char *)malloc(sizeof(char) * (strlen(src) + 1)); \
    strcpy(dest, src);                                       \
    dest[strlen(src)] = '\0';

#define MCcall(function)                     \
    res = function;                          \
    if (res)                                 \
    {                                        \
        printf("--" #function ":%i\n", res); \
        return res;                          \
    }

#define MCerror(error_code, error_message, ...)                            \
    printf("\n\nERR[%i]: " error_message "\n", error_code, ##__VA_ARGS__); \
    return error_code;

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
#define sizeof_branch_unit_v1 (sizeof(void *) * 6)
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

#define process_action_v1               \
    struct                              \
    {                                   \
        struct                          \
        {                               \
            const char *identifier;     \
            unsigned int version;       \
        } struct_id;                    \
        unsigned int sequence_uid;      \
        enum process_action_type type;  \
        char *dialogue;                 \
        void *history;                  \
        union {                         \
            char *demonstrated_command; \
        } data;                         \
    }
#define sizeof_process_action_v1 (sizeof(void *) * 7)

#define template_collection_v1        \
    struct                            \
    {                                 \
        struct                        \
        {                             \
            const char *identifier;   \
            uint version;             \
        } struct_id;                  \
        unsigned int templates_alloc; \
        unsigned int template_count;  \
        void **templates;             \
    }
#define sizeof_template_collection_v1 (sizeof(void *) * 5)

#define script_v1                       \
    struct                              \
    {                                   \
        struct                          \
        {                               \
            const char *identifier;     \
            uint version;               \
        } struct_id;                    \
        unsigned int sequence_uid;      \
        unsigned int script_uid;        \
        unsigned int arguments_alloc;   \
        unsigned int argument_count;    \
        void **arguments;               \
        unsigned int local_count;       \
        void **locals;                  \
        char *response;                 \
        unsigned int segment_count;     \
        unsigned int segments_complete; \
    }
#define sizeof_script_v1 (sizeof(void *) * 11)

#define command_hub_v1                          \
    struct                                      \
    {                                           \
        struct                                  \
        {                                       \
            const char *identifier;             \
            uint version;                       \
        } struct_id;                            \
        void *global_node;                      \
        void *nodespace;                        \
        void *template_collection;              \
        void *process_matrix;                   \
        unsigned int focused_issue_stack_alloc; \
        unsigned int focused_issue_stack_count; \
        void **focused_issue_stack;             \
        unsigned int active_scripts_alloc;      \
        unsigned int active_script_count;       \
        void **active_scripts;                  \
        bool focused_issue_activated;           \
        unsigned int uid_counter;               \
    }
#define sizeof_command_hub_v1 (sizeof(void *) * 14)

#define void_collection_v1          \
    struct                          \
    {                               \
        struct                      \
        {                           \
            const char *identifier; \
            uint version;           \
        } struct_id;                \
        unsigned int allocated;     \
        unsigned int count;         \
        void *items;                \
    }
#define sizeof_void_collection_v1 (sizeof(void *) * 5)

#define allocate_anon_struct(ptr_to_struct, size) \
    mc_dvp = (void **)&ptr_to_struct;             \
    *mc_dvp = malloc(size);

#define declare_and_allocate_anon_struct(struct, ptr_to_struct, size) \
    struct *ptr_to_struct;                                            \
    mc_dvp = (void **)&ptr_to_struct;                                 \
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

#endif // MIDGE_CORE_H