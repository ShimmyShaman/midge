/* src/core/special_debug.c
   Copyright 2020, Adam Rasburn, All Rights Reserved.
*/

#include "core/midge_core.h"


typedef struct special_data {
  int num;
  int add;
} special_data;

typedef struct only_int_struct {
  int a;
} only_int_struct;

void special_modification(special_data *s)
{
  s->num += 11;
  // This stuff
  // printf("SM: ais.a:%i bis.a:%i dint:%i lint:%i pint:%i\n", ais.a, bis->a, dint, lint, *pint);

  // ais.a += 2;
  // bis->a += 2;
  // dint += 2;
  // lint += 2;
  // *pint += 2;
  // printf("SM: ais.a:%i bis.a:%i dint:%i lint:%i pint:%i\n", ais.a, bis->a, dint, lint, *pint);
}

void special_suggester()
{
  int a = 5; // this number
  // and here

  // special_data s;
  // s.num = 8;
  // s.add = 14;

  // TODO Enter another special data 't' with 4 & 14
  // special_data t;
  // t.num = 4;
  // t.add = 14;

  // // TODO Enter another special data 'm' with any value
  // special_data m;
  // m.num = 7;
  // m.add = 14;
}

void special_update(frame_time *elapsed)
{

  // char b = (char)e + 'a';
  // printf("special_update! @ %li appsecs\n", elapsed->app_secs);
  // only_int_struct ais;
  // ais.a = 7;

  // only_int_struct bis;
  // bis.a = 7;
  // only_int_struct *pbis = &bis;

  // int lint = 7;

  // int b = 7;
  // int *pb = &b;

  // special_modification(ais, pbis, 7, lint, pb);

  // printf("SU-after: ais.a:%i bis.a:%i lint:%i pint:%i\n", ais.a, bis->a, lint, *pb);
  // printf("special_update! s.num:%i\n", s.num);

  // printf("global_node_name:%s\n", command_hub->global_node->name);
}

