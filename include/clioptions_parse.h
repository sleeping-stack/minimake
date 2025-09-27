#ifndef CLIOPTIONS_PARSE_H
#define CLIOPTIONS_PARSE_H

#include "common.h"
// 解析全部 CLI 参数：区分 -h/-v/-j/目标名/无效选项
typedef struct {
  int show_help;
  int verbose_mode;
  int jobs;
  char targets_arr[MAX_BLOCK_NUMBERS][MAX_WORD_NUMBERS];
  int n_targets; // 需要构建的目标数量
} CliOptions;

void print_usage(const char *prog);
int parse_args(int argc, char **argv, CliOptions *out);

#endif