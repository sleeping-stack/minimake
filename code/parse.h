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

void parse_target_line(const char *str, Target_block tb_arr[MAX_BLOCK_NUMBERS],
                       int tb_count);
void parse_cmd_line(const char *str, Target_block tb_arr[MAX_BLOCK_NUMBERS],
                    int tb_count, int cmd_count);
int parse_makefile(char (*line_arr_ptr)[MAX_LINE_LENGTH],
                   Target_block tb_arr[MAX_BLOCK_NUMBERS]);

#endif // PARSE_H