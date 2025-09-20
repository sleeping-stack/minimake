#include "parse.h"
#include "common.h"
#include "grammar_check.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 去掉首尾空白
static void trim(char *str) {
  char *p = str;
  while (*p && isspace((unsigned char)*p))
    p++;

  if (p != str)
    memmove(str, p, strlen(p) + 1);

  size_t len = strlen(str);
  while (len > 0 && isspace((unsigned char)str[len - 1])) {
    str[len - 1] = '\0';
    len--;
  }
}

void parse_target_line(const char *str, Target_block tb_arr[MAX_BLOCK_NUMBERS],
                       int tb_count) {
  if (tb_count == -1)	// 防止第一次读取目标行出现错误
    tb_count = 0;

  tb_arr[tb_count].dep_count = 0;
  // 解析目标行
  char s[MAX_LINE_LENGTH] = {'\0'};
  strncpy(s, str, MAX_LINE_LENGTH - 1);
  s[MAX_LINE_LENGTH - 1] = '\0';

  char *token;
  token = strtok(s, ":");
  if (token != NULL) {
    // 提取目标
    strncpy(tb_arr[tb_count].target, token, MAX_WORD_NUMBERS);
    tb_arr[tb_count].target[MAX_WORD_NUMBERS - 1] = '\0'; // 确保字符串终止

    // 提取依赖项
    token = strtok(NULL, " ");
    while (token != NULL && tb_arr[tb_count].dep_count < MAX_DEP_NUMBERS) {
      strncpy(tb_arr[tb_count].dep_arr[tb_arr[tb_count].dep_count], token,
              MAX_WORD_NUMBERS);
      tb_arr[tb_count]
          .dep_arr[tb_arr[tb_count].dep_count][MAX_WORD_NUMBERS - 1] =
          '\0'; // 确保字符串终止
      tb_arr[tb_count].dep_count++;
      token = strtok(NULL, " ");
    }
  }
}

void parse_cmd_line(const char *str, Target_block tb_arr[MAX_BLOCK_NUMBERS],
                    int tb_count, int cmd_count) {
  strcpy(tb_arr[tb_count].commands[cmd_count], str);
}

int parse_makefile(char (*line_arr_ptr)[MAX_LINE_LENGTH],
                   Target_block tb_arr[MAX_BLOCK_NUMBERS]) {
  int tb_count = -1;
  int line_count = 0;
  int cmd_count = 0;

  // 解析每个目标块的目标行
  for (; line_count < MAX_LINE_NUMBERS; line_count++) {
    if (line_type_judge(line_arr_ptr[line_count]) == LINE_TARGET) {
      // 重置cmd_count
      if (tb_count >= 0) {
        tb_arr[tb_count].cmd_count = cmd_count;
        cmd_count = 0;
      }
      tb_count++;
      parse_target_line(line_arr_ptr[line_count], tb_arr, tb_count);
    } else if (line_type_judge(line_arr_ptr[line_count]) == LINE_COMMAND) {
      parse_cmd_line(line_arr_ptr[line_count], tb_arr, tb_count, cmd_count);
      cmd_count++;
    } else
      continue;
  }

  return tb_count;
}
