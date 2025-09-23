#include "grammar_check.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LineType line_type_judge(const char *str) {
  if (str == NULL)
    exit(1); // 添加空指针检查

  if (target_line_check(str) == 1) {
    return LINE_TARGET;
  } else {
    if (*str == '\t')
      return LINE_COMMAND;
    else {
      // 跳过前导空格
      while (*str == ' ') {
        str++;
      }

      if (*str == '\0' || *str == '\n') { // 空行
        return LINE_EMPTY;
      } else if (*str == '#') { // 注释行
        return LINE_MESSAGE;
      }

      return LINE_INVALID;
    }
  }
}

// 检查是否是目标行
int target_line_check(const char *str) {
  if (str[0] == ' ' || str[0] == '\t' || str[0] == ':') // 不是目标行
    return 0;
  if (strchr(str, ':') != NULL)
    return 1;

  return 0;
}

// 主要的语法检查函数
int grammar_check(char (*line_arr_ptr)[MAX_LINE_LENGTH]) {
  if (line_arr_ptr == NULL) {
    printf("Error: NULL pointer passed to grammar_check\n");
    return 1;
  }

  int has_error = 0;
  int line_count = 0;

  // 跳过第一个目标行前的空行和注释行
  for (int i = 0; i < MAX_LINE_NUMBERS; i++) {
    if (line_type_judge(line_arr_ptr[i]) == LINE_MESSAGE ||
        line_type_judge(line_arr_ptr[i]) == LINE_EMPTY) {
      line_count++;
      continue;
    } else {
      break;
    }
  }

  // 检查目标行是否有问题
  if (line_type_judge(line_arr_ptr[line_count]) == LINE_COMMAND) {
    printf("Line%d: Command found before rule\n", line_count + 1);
    has_error = 1;
  } else if (line_type_judge(line_arr_ptr[line_count]) == LINE_INVALID) {
    printf("Line%d: Missing colon in target definition\n", line_count + 1);
    has_error = 1;
  }

  line_count++;

  // 开始检查命令行是否有问题
  for (int i = line_count; i < MAX_LINE_NUMBERS; i++) {
    if (line_type_judge(line_arr_ptr[i]) == LINE_MESSAGE ||
        line_type_judge(line_arr_ptr[i]) == LINE_EMPTY) {
      continue;
    }

    if (line_type_judge(line_arr_ptr[i]) == LINE_TARGET) {
      continue;
    }

    if (line_type_judge(line_arr_ptr[i]) == LINE_INVALID) {
      printf("Line%d: Command line should start with a tab\n", i + 1);
      has_error = 1;
    }
  }

  return has_error;
}