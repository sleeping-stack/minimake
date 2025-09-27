#include "grammar_check.h"
#include "common.h"
#include <ctype.h>
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
      const char *p = str;
      while (*p == ' ')
        p++;

      if (*p == '\0' || *p == '\n') { // 空行
        return LINE_EMPTY;
      } else if (*p == '#') { // 注释行
        return LINE_MESSAGE;
      }

      // 变量定义行：形如  NAME = VALUE  （前导空格允许）
      // 条件：含有 '=' 且 '=' 左侧至少有一个非空白字符
      const char *eq = strchr(p, '=');
      if (eq != NULL) {
        // 检查 '=' 左侧是否存在非空白字符
        const char *q = eq - 1;
        int has_name_char = 0;
        while (q >= p) {
          if (!isspace((unsigned char)*q)) {
            has_name_char = 1;
            break;
          }
          q--;
        }
        if (has_name_char) {
          return LINE_VARIABLE;
        }
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
  puts("==============================================");
  puts("== 静态语法检查阶段 ==");
  puts("==============================================");
  if (line_arr_ptr == NULL) {
    printf("Error: NULL pointer passed to grammar_check\n");
    return 1;
  }

  int has_error = 0;
  int line_count = 0;

  // 跳过第一个目标行前的空行，注释行，变量定义行
  for (int i = 0; i < MAX_LINE_NUMBERS; i++) {
    LineType t = line_type_judge(line_arr_ptr[i]);
    if (t == LINE_MESSAGE || t == LINE_EMPTY || t == LINE_VARIABLE) {
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
    LineType t = line_type_judge(line_arr_ptr[i]);
    if (t == LINE_MESSAGE || t == LINE_EMPTY || t == LINE_VARIABLE) {
      continue;
    }

    if (t == LINE_TARGET) {
      continue;
    }

    if (t == LINE_INVALID) {
      printf("Line%d: Command line should start with a tab\n", i + 1);
      has_error = 1;
    }
  }

  if (has_error == 0)
    puts("-- 静态语法检查通过 --");
  puts("----------------------------------------------");
  puts("");
  return has_error;
}