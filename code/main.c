#include "common.h"
#include "grammar_check.h"
#include "parse.h"
#include "preprocess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // 创建动态flag数组用来表示输入的每个参数的状态，1表示该参数已匹配
  int *flag = malloc(argc * sizeof(int));
  if (flag == NULL) {
    perror("malloc failed");
    return 1;
  }

  for (int i = 0; i < argc; i++) {
    flag[i] = 0;
  }

  int verbose_mode = 0; // 为1表示使用详细模式

  for (int i = 0; i < argc; i++) {
    if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
      printf("usage: --help or -h          get usage\n");
      printf("       --verbose_mode or -v  open verbose mode\n");
      puts("");
      flag[i] = 1;
    } else if (strcmp("-v", argv[i]) == 0 ||
               strcmp("--verbose_mode", argv[i]) == 0) {
      verbose_mode = 1;
      flag[i] = 1;
    }
  }

  if (argc == 1)
    printf("No arguments input!\n");
  else {
    for (int i = 1; i < argc; i++) {
      if (flag[i] == 0) {
        printf("No.%d argument is an uncorrect argument!\n", i);
      }
    }
  }

  // 释放内存
  if (flag != NULL) {
    free(flag);
  }

  char line_arr[MAX_LINE_NUMBERS][MAX_LINE_LENGTH] = {{'\0'}};
  process_makefile(verbose_mode, line_arr);

  int has_error = grammar_check(line_arr);

  if (has_error == 1)
    printf("Please make sure the grammer is right so that the program can "
           "parse the makefile.\n");
  else {
    Target_block *tb_arr = malloc(MAX_BLOCK_NUMBERS * sizeof(Target_block));
    if (tb_arr == NULL) {
      perror("malloc");
      return 1;
    }
    int tb_count = parse_makefile(line_arr, tb_arr);
    free(tb_arr);
  }

  return 0;
}