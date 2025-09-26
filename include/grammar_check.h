#ifndef GRAMMAR_CHECK_H
#define GRAMMAR_CHECK_H

#include "common.h"

typedef enum {
  LINE_EMPTY,    // 空行
  LINE_TARGET,   // 目标行 (含冒号)
  LINE_COMMAND,  // 命令行 (以制表符开头)
  LINE_MESSAGE,  // 注释行
  LINE_VARIABLE, // 变量定义行
  LINE_INVALID   // 无效行(有内容但既不是目标行也不是命令行也不是注释行)
} LineType;

// 函数声明
LineType line_type_judge(const char *str);
int target_line_check(const char *str);
int grammar_check(char (*line_arr_ptr)[MAX_LINE_LENGTH]);

#endif // GRAMMAR_CHECK_H