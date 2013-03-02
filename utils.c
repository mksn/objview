#include <utils.h>
#include <ctype.h>

void lowerstring(char *s)
{
  while (*s)
  {
    *s = tolower(*s);
    s++;
  }
}

