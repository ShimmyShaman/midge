/* special_debug.c */

#include "core/midge_core.h"

// [_mc_version=3]
struct special_data {
  int num;
  int add;
};

struct only_int_struct {
  int a;
};

// [_mc_iteration=3]
// void special_modification(only_int_struct ais, only_int_struct *bis, int dint, int lint, int *pint)
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

void special_syntax()
{
  int a = 4 + 3 + 7;
  int *b = &a;
  bool c;
  if (a == *b && 3 * 4 + 5 < *b || 6 - 7 / 1 < a % 8) {
    return;
  }
  // operational: 1 * 2 + 3 * 4 - 5;
  // 5
  // - 4
}

// [_mc_iteration=12]
void special_update(frame_time *elapsed)
{

  // char b = (char)e + 'a';
  return;
  // printf("special_update! @ %li appsecs\n", elapsed->app_secs);
  // special_data s;
  // s.num = 8;
  // s.add = 14;

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