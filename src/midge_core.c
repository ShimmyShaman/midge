/*
typedef struct midge_str
{
    int length;
    char *text;
} midge_str;

typedef struct midge_struct_id
{
    midge_str name;
    unsigned int version;
} midge_struct_id;

typedef struct midge_field_info
{
    char *type;
    int pointer_count;
    char *name;
} midge_field_info;

struct _struct
{
    int field_count;
    midge_field_info *fields;
};

struct node
{
    char *name;
    int *child_count;
    struct node *parent;
    struct node *children;
};*/

typedef void **struct_definition;
typedef void **struct_allocation;
typedef void **midge_array;
typedef unsigned int uint;

int allocate_from_int(void **out_field, int value)
{
    int *alloc = (int *)malloc(sizeof(int) * 1);
    *alloc = value;
    *out_field = (void *)alloc;
    return 0;
}

int allocate_from_uint(void **out_field, unsigned int value)
{
    unsigned int *alloc = (unsigned int *)malloc(sizeof(unsigned int) * 1);
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

int allocate_midge_field_info(void **out_field, const char *type, int pointer_count, const char *name)
{
    void **field_data = (void **)malloc(sizeof(void *) * 3);
    allocate_from_cstring(&field_data[0], type);
    allocate_from_int(&field_data[1], pointer_count);
    allocate_from_cstring(&field_data[2], name);
    *out_field = (void *)field_data;
    return 0;
}

int define_struct(struct_definition *stdef, const char *name, uint version, midge_array fields, int field_count)
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
    allocate_struct_id(&alloc[0], *(char **)struct_definition[0], **(uint **)struct_definition[0]);

    *allocation = alloc;
    return 0;
}

// int free_void_array(void **)

int mc_main(int argc, const char *const *argv)
{
    // Define the node structure
    struct_definition node_v0;
    midge_array node_v0_fields = (midge_array)malloc(sizeof(void *) * (4));
    allocate_midge_field_info(&node_v0_fields[0], "char", 1, "name");
    allocate_midge_field_info(&node_v0_fields[1], "int", 0, "child_count");
    allocate_midge_field_info(&node_v0_fields[2], "node", 1, "parent");
    allocate_midge_field_info(&node_v0_fields[3], "node", 1, "children");
    define_struct(&node_v0, "node", 0U, node_v0_fields, 4);
    free(node_v0_fields);

    // node global;
    void **global;
    allocate_from_definition(&global, node_v0);
    allocate_from_cstring(&global[1], "global");
    allocate_from_int(&global[2], 0);
    global[3] = &global;
    midge_array children = (midge_array)malloc(sizeof(void *) * (0));
    global[4] = &children;

    return 0;
}