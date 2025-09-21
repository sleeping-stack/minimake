#ifndef PARSE_H
#define PARSE_H

#include "common.h"
#include <stdio.h>

typedef struct {
  char target[MAX_WORD_NUMBERS];
  char dep_arr[MAX_DEP_NUMBERS][MAX_WORD_NUMBERS];
  int dep_count;
  char commands[MAX_LINE_NUMBERS][MAX_LINE_LENGTH];
  int cmd_count;
} Target_block;

static void trim(char *str);
static void safe_copy(char *dst, size_t dst_size, const char *src);
void parse_target_line(const char *str, Target_block *tb_arr, int tb_count);
void parse_cmd_line(const char *str, Target_block *tb_arr, int tb_count,
                    int cmd_count);
int parse_makefile(char (*line_arr_ptr)[MAX_LINE_LENGTH], Target_block *tb_arr);
void print_target_blocks(Target_block *tb_arr, int tb_count);
int has_duplicate_target(Target_block *tb_arr, int tb_count);
int is_dep_invaild(Target_block *tb_arr, int tb_count, const char *dep);
int parse_check(Target_block *tb_arr, int tb_count);

#endif // PARSE_H