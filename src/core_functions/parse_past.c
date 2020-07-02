/* parse_past.c */
/* Dependencies: */
/* End-Dependencies */

int parse_past(char *text, int *index, char *sequence)
{
  for (int i = 0;; ++i) {
    if (sequence[i] == '\0') {
      *index += i;
      return 0;
    }
    else if (text[*index + i] == '\0') {
      return -1;
    }
    else if (sequence[i] != text[*index + i]) {
      printf("!parse_past() expected:'%c' was:'%c'\n", sequence[i], text[*index + i]);
      return 1 + i;
    }
  }
}