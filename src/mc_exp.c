
#include <stdio.h>

int set_num(int** var, int value) {
  *var = (int*)malloc(sizeof(int));
  **var = value;
}

int add_to_num(int* v, int e) { *v = *v + e; }

struct complex {
  int b;
  int c;
};

int mcmain() {
  int* v;
  set_num(&v, 6);
  add_to_num(&v, 2);
  printf("out:%i\n", v);
}