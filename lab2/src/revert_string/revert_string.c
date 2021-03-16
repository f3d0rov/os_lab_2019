#include "revert_string.h"
#include <string.h>

void RevertString(char *str)
{
  int len, i;
  char temp;
  
  if (str == 0) return;
  
  len = strlen(str);
  for (i = 0; i < len / 2; ++i) {
    temp = str[i];
    str[i] = str[len - 1 - i];
    str[len - 1 - i] = temp;
  }
}

