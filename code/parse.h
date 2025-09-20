#ifndef PARSE_H
#define PARSE_H

#include "common.h"

typedef struct {
  char target[MAX_WORD_NUMBERS];
  char dep_arr[MAX_DEP_NUMBERS][MAX_WORD_NUMBERS];
  int dep_count;
  char commands[MAX_LINE_NUMBERS][MAX_LINE_LENGTH];
  int cmd_count;
} Target_block;

int parse(char (*line_arr_ptr)[MAX_LINE_LENGTH]);

#endif // PARSE_H