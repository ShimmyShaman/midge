/* midge_core.c */

#include <stdlib.h>

typedef void **struct_definition;
typedef void **struct_allocation;
typedef void **midgeo;
typedef unsigned int uint;

int allocate_from_int(void **out_field, int value)
{
    int *alloc = (int *)malloc(sizeof(int) * 1);
    *alloc = value;
    *out_field = (void *)alloc;
    return 0;
}

int allocate_from_uint(void **out_field, uint value)
{
    uint *alloc = (uint *)malloc(sizeof(uint) * 1);
    *alloc = value;
    *out_field = (void *)alloc;
    return 0;
}

int allocate_from_cstring(void **out_field, const char *cstr)
{
    char *alloc = (char *)malloc(sizeof(char) * strlen(cstr));
    strcpy(alloc, cstr);
    *out_field = (void *)alloc;
    return 0;
}

int allocate_struct_id(void **out_field, const char *struct_name, uint version)
{
    void **field_data = (void **)malloc(sizeof(void *) * 2);
    allocate_from_cstring(&field_data[0], struct_name);
    allocate_from_uint(&field_data[1], version);
    *out_field = (void *)field_data;
    return 0;
}

#define allocate_from_intv(field, val)       \
    *field = (int *)malloc(sizeof(int) * 1); \
    **(int **)field = val;

#define allocate_from_cstringv(field, cstr)               \
    *field = (char *)malloc(sizeof(char) * strlen(cstr)); \
    strcpy((char *)*field, cstr);

int (*allocate_midge_field_infof)(int, void **);

int allocate_midge_field_infov(int argc, void **argv)
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

int allocate_midge_field_info(void **out_field, const char *type, int pointer_count, const char *name)
{
    void **field_data = (void **)malloc(sizeof(void *) * 3);
    allocate_from_cstring(&field_data[0], type);
    allocate_from_int(&field_data[1], pointer_count);
    allocate_from_cstring(&field_data[2], name);
    *out_field = (void *)field_data;
    return 0;
}

int define_struct(struct_definition *stdef, const char *name, uint version, midgeo fields, int field_count)
{
    struct_definition field_data = (struct_definition)malloc(sizeof(void *) * (2 + field_count));
    allocate_struct_id(&field_data[0], name, version);
    allocate_from_uint(&field_data[1], field_count);
    for (int i = 0; i < field_count; ++i)
        field_data[2 + i] = fields[i];

    *stdef = field_data;
    return 0;
}

int allocate_from_definition(struct_allocation *allocation, struct_definition struct_definition)
{
    struct_allocation alloc = (struct_allocation)malloc(sizeof(void *) * *(int *)struct_definition[1]);
    allocate_struct_id(&alloc[0], (char *)((void **)struct_definition[0])[0], *(uint *)((void **)struct_definition[0])[1]);

    *allocation = alloc;
    return 0;
}

int mc_main(int argc, const char *const *argv)
{
    // Define the node structure
    struct_definition node_v0;
    midgeo node_v0_fields = (midgeo)malloc(sizeof(void *) * (4));

    allocate_midge_field_infof = &allocate_midge_field_infov;

    void *vargs[5];
    vargs[0] = &node_v0_fields[0];
    const char *_mtl_0 = "char";
    vargs[1] = (void *)_mtl_0;
    int _mtl_1 = 1;
    vargs[2] = (void *)&_mtl_1;
    const char *_mtl_2 = "name";
    vargs[3] = (void *)_mtl_2;
    allocate_midge_field_infof(4, vargs);

    allocate_midge_field_info(&node_v0_fields[1], "int", 0, "child_count");
    allocate_midge_field_info(&node_v0_fields[2], "node", 1, "parent");
    allocate_midge_field_info(&node_v0_fields[3], "node", 1, "children");
    define_struct(&node_v0, "node", 0U, node_v0_fields, 4);
    free(node_v0_fields);

    printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v0[2])[0], *(uint *)((void **)node_v0[2])[1], (char *)((void **)node_v0[2])[2]);
    printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v0[3])[0], *(uint *)((void **)node_v0[3])[1], (char *)((void **)node_v0[3])[2]);
    printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v0[4])[0], *(uint *)((void **)node_v0[4])[1], (char *)((void **)node_v0[4])[2]);
    printf("type:%s deref:%u ident:%s\n", (char *)((void **)node_v0[5])[0], *(uint *)((void **)node_v0[5])[1], (char *)((void **)node_v0[5])[2]);

    // node global
    void **global;
    allocate_from_definition(&global, node_v0);
    allocate_from_cstring(&global[1], "global");
    allocate_from_int(&global[2], 0);
    global[3] = &global;
    midgeo children = (midgeo)malloc(sizeof(void *) * (0));
    global[4] = NULL;

    printf("global:%p\n", global);
    printf("struct_id:%s - %u\n", (char *)((void **)global[0])[0], *(uint *)((void **)global[0])[1]);
    printf("global:%s\n", (char *)global[1]);
    printf("global:%i\n", *(int *)global[2]);
    printf("global:%p\n", global[3]);
    printf("global:%p\n", global[4]);

    return 0;
}
// int mc_main(int argc, const char *const *argv)
// {
//     // Define the node structure
//     struct_definition node_v0;
//     midge_array node_v0_fields = (midge_array)malloc(sizeof(void *) * (4));

//     allocate_midge_field_info(&node_v0_fields[0], "char", 1, "name");
//     allocate_midge_field_info(&node_v0_fields[1], "int", 0, "child_count");
//     allocate_midge_field_info(&node_v0_fields[2], "node", 1, "parent");
//     allocate_midge_field_info(&node_v0_fields[3], "node", 1, "children");
//     define_struct(&node_v0, "node", 0U, node_v0_fields, 4);
//     free(node_v0_fields);

//     // node global
//     void **global;
//     allocate_from_definition(&global, node_v0);
//     allocate_from_cstring(&global[1], "global");
//     allocate_from_int(&global[2], 0);
//     global[3] = &global;
//     midge_array children = (midge_array)malloc(sizeof(void *) * (0));
//     global[4] = &children;

//     return 0;
// }