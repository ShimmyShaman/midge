/* parse_past.c */
/* Dependencies: */
#define ERR(error_type, format, ...) 1
/* End-Dependencies */

void parse_past(char *text, int *index, char *sequence)
{
  for (int i = 0;; ++i) {
    if (sequence[i] == '\0') {
      *index += i;
      return;
    }
    else if (text[*index + i] == '\0') {
      ERR(PARSE_PAST_SEQUENCE_NOT_FOUND, "text ends before sequence completion.");
    }
    else if (sequence[i] != text[*index + i]) {
      ERR(PARSE_PAST_SEQUENCE_NOT_FOUND, "expected:'%c' was:'%c'", sequence[i], text[*index + i]);
    }
  }
}