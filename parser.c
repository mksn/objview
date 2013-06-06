#include "parser.h"

void parser_main(const char *s) 
{
  if (strcmp(s, "test") == 0) {
    terminal_puts("1 2 3 testing");
  } 
  else if (strcmp(s, "quit") == 0 ||
      strcmp(s, "exit") == 0 ||
      strncmp(s, "q", 1) == 0 ||
      strncmp(s, "x", 1) == 0) {
    exit(1);
  }
  else if (strncmp(s, "say", 3) == 0) {
    char t[256]; 
    sprintf(t, "objview:%s", s+3);
    terminal_puts(t);
  }
}
