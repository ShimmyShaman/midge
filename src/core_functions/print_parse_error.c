/* parse_past.c */
/* Dependencies: */
#define ERR(error_type, format, ...) 1
/* End-Dependencies */

void print_parse_error(char *text, int index, char *function_name, char *section_id)
{
  char buf[30];
  int off = 6 - index;
  for (int i = 0; i < 13; ++i) {
    if (index - 13 + i < 0)
      buf[i] = ' ';
    else
      buf[i] = text[index - 13 + i];
  }
  buf[13] = '|';
  buf[14] = text[index];
  buf[15] = '|';
  char eof = text[index] == '\0';
  for (int i = 0; i < 13; ++i) {
    if (eof)
      buf[16 + i] = ' ';
    else {
      eof = text[index + 1 + i] == '\0';
      buf[16 + i] = text[index + 1 + i];
    }
  }
  buf[29] = '\0';

  printf("\n%s>%s#unhandled-char:'%s'\n", function_name, section_id, buf);
}