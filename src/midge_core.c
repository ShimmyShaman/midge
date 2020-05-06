/* midge_core.c */

#include <stdlib.h>

typedef void **struct_definition;
typedef void **struct_allocation;
typedef void **midgeo;
typedef unsigned int uint;

/*
 * @field a (void **) variable to store the created value in.
 */
#define allocate_from_intv(field, val)                                                          \
    *field = (int *)malloc(sizeof(int) * 1);                                                    \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    **(int **)field = val;

#define allocate_from_uintv(field, val)                                                         \
    *field = (uint *)malloc(sizeof(uint) * 1);                                                  \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    **(uint **)field = val;

#define allocate_from_cstringv(field, cstr)                                                     \
    *field = (char *)malloc(sizeof(char) * strlen(cstr));                                       \
    if (!*field)                                                                                \
    {                                                                                           \
        printf("allocate_from_intv(): failed to allocate memory for " #field " field_data.\n"); \
        return -1;                                                                              \
    }                                                                                           \
    strcpy((char *)*field, cstr);

#define MCcall(function)                   \
    res = function;                        \
    if (res)                               \
    {                                      \
        printf("--" #function ":%i", res); \
        return res;                        \
    }

int (*allocate_struct_id)(int, void **);
int (*allocate_midge_field_info)(int, void **);
int (*define_struct)(int, void **);
int (*allocate_from_definition)(int, void **);

int allocate_struct_id_v0(int argc, void **argv)
{
    midgeo out_field = (midgeo)argv[0];
    const char *struct_name = (char *)argv[1];
    uint version = *(uint *)argv[2];

    midgeo field_data = (midgeo)malloc(sizeof(void *) * 2);
    if (!field_data)
    {
        printf("allocate_struct_id(): failed to allocate memory for field_data.\n");
        return -1;
    }

    allocate_from_cstringv(&field_data[0], struct_name);
    allocate_from_uintv(&field_data[1], version);
    *out_field = (void *)field_data;
    return 0;
}

int allocate_midge_field_info_v0(int argc, void **argv)
{
    midgeo out_field = (midgeo)argv[0];
    const char *type = (char *)argv[1];
    int pointer_count = *(int *)argv[2];
    const char *name = (char *)argv[3];

    midgeo field_data = (midgeo)malloc(sizeof(void *) * 3);
    allocate_from_cstringv(&field_data[0], type);
    allocate_from_intv(&field_data[1], pointer_count);
    allocate_from_cstringv(&field_data[2], name);
    *out_field = (void *)field_data;
    return 0;
}

int define_struct_v0(int argc, void **argv)
{
    struct_definition *stdef = (struct_definition *)argv[0];
    const char *name = (char *)argv[1];
    uint version = *(uint *)argv[2];
    midgeo fields = (midgeo)argv[3];
    int field_count = *(int *)argv[4];

    int res;

    struct_definition field_data = (struct_definition)malloc(sizeof(void *) * (2 + field_count));
    if (!field_data)
    {
        printf("define_struct(): failed to allocate memory for field_data.\n");
        return -1;
    }

    void *vargs[5];
    vargs[0] = (void *)&field_data[0];
    vargs[1] = (void *)name;
    vargs[2] = (void *)&version;
    MCcall(allocate_struct_id(3, vargs));

    allocate_from_uintv(&field_data[1], field_count);
    for (int i = 0; i < field_count; ++i)
        field_data[2 + i] = fields[i];

    *stdef = field_data;
    return 0;
}

int allocate_from_definition_v0(int argc, void **argv)
{
    struct_allocation *allocation = (struct_allocation *)argv[0];
    struct_definition definition = (struct_definition)argv[1];

    int res;

    struct_allocation alloc = (struct_allocation)malloc(sizeof(void *) * *(int *)definition[1]);
    if (!alloc)
    {
        printf("allocate_from_definition(): failed to allocate memory for alloc.\n");
        return -1;
    }

    void *vargs[3];
    vargs[0] = &alloc[0];
    vargs[1] = (char *)((void **)definition[0])[0];
    vargs[2] = (uint *)((void **)definition[0])[1];
    MCcall(allocate_struct_id(3, vargs));

    *allocation = alloc;
    return 0;
}

int mc_main(int argc, const char *const *argv)
{
    int res;

    // Define the node structure
    struct_definition node_v0;
    midgeo node_v0_fields = (midgeo)malloc(sizeof(void *) * (4));

    allocate_struct_id = &allocate_struct_id_v0;
    allocate_midge_field_info = &allocate_midge_field_info_v0;
    define_struct = &define_struct_v0;
    allocate_from_definition = &allocate_from_definition_v0;

    void *vargs[5];
    vargs[0] = &node_v0_fields[0];
    const char *_mtl_0 = "char";
    vargs[1] = (void *)_mtl_0;
    int _mtl_1 = 1;
    vargs[2] = (void *)&_mtl_1;
    const char *_mtl_2 = "name";
    vargs[3] = (void *)_mtl_2;
    MCcall(allocate_midge_field_info(4, vargs));

    vargs[0] = &node_v0_fields[1];
    const char *_mtl_3 = "int";
    vargs[1] = (void *)_mtl_3;
    int _mtl_4 = 0;
    vargs[2] = (void *)&_mtl_4;
    const char *_mtl_5 = "child_count";
    vargs[3] = (void *)_mtl_5;
    MCcall(allocate_midge_field_info(4, vargs));

    vargs[0] = &node_v0_fields[2];
    const char *_mtl_6 = "node";
    vargs[1] = (void *)_mtl_6;
    int _mtl_7 = 1;
    vargs[2] = (void *)&_mtl_7;
    const char *_mtl_8 = "parent";
    vargs[3] = (void *)_mtl_8;
    MCcall(allocate_midge_field_info(4, vargs));

    vargs[0] = &node_v0_fields[3];
    const char *_mtl_9 = "node";
    vargs[1] = (void *)_mtl_9;
    int _mtl_10 = 1;
    vargs[2] = (void *)&_mtl_10;
    const char *_mtl_11 = "children";
    vargs[3] = (void *)_mtl_11;
    MCcall(allocate_midge_field_info(4, vargs));

    vargs[0] = &node_v0;
    const char *_mtl_12 = "node";
    vargs[1] = (void *)_mtl_12;
    uint _mtl_13 = 1U;
    vargs[2] = (void *)&_mtl_13;
    vargs[3] = node_v0_fields;
    int _mtl_14 = 4;
    vargs[4] = (void *)&_mtl_14;
    MCcall(define_struct(5, vargs));
    free(node_v0_fields);

    // printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v0[2])[0], *(uint *)((void **)node_v0[2])[1], (char *)((void **)node_v0[2])[2]);
    // printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v0[3])[0], *(uint *)((void **)node_v0[3])[1], (char *)((void **)node_v0[3])[2]);
    // printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v0[4])[0], *(uint *)((void **)node_v0[4])[1], (char *)((void **)node_v0[4])[2]);
    // printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v0[5])[0], *(uint *)((void **)node_v0[5])[1], (char *)((void **)node_v0[5])[2]);

    // node global
    midgeo global;
    vargs[0] = &global;
    vargs[1] = node_v0;
    MCcall(allocate_from_definition(2, vargs));
    allocate_from_cstringv(&global[1], "global");
    allocate_from_intv(&global[2], 0);
    global[3] = &global;
    midgeo children = (midgeo)malloc(sizeof(void *) * (0));
    global[4] = NULL;

    // printf("global:%p\n", global);
    // printf("struct_id:%s - %u\n", (char *)((void **)global[0])[0], *(uint *)((void **)global[0])[1]);
    // printf("global:%s\n", (char *)global[1]);
    // printf("global:%i\n", *(int *)global[2]);
    // printf("global:%p\n", global[3]);
    // printf("global:%p\n", global[4]);

    return 0;
}