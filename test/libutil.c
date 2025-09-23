/* libutil.c */
#include "libutil.h"
#include <stdio.h>
int sum(int a, int b) { return a + b; }
void hello(const char *tag) { printf("hello from %s\n", tag); }