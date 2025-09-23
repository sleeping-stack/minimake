/* main.c */
#include "libutil.h"
#include <stdio.h>

int main() {
  printf("main: sum(2,3)=%d\n", sum(2, 3));
  hello("main");
  return 0;
}
