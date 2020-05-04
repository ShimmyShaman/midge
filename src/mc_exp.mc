

bind print(cstr format, ...vargs)
{
    #include <stdio.h>
    printf(format, ...${vargs});
}

func set_num(int v)
{
    v = 17;
}

func add_to_num(int v)
{
    $v = $v + 7;
}

struct complex {
    int b;
    int c;
};

func mcmain()
{
    int a;
    set_num(a);

    int v = 4;
    add_to_num(v);
    print("a=%i v=%i", a, v);
}


int (*set_num)(int);
int set_num(int *v)
{
    free(v);
    v = malloc()
    a = malloc
}