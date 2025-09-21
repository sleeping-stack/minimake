#include "parse.h"
#include "common.h"
#include "grammar_check.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

// 安全复制
static void safe_copy(char *dst, size_t dst_size, const char *src) {
  if (dst_size == 0)
    return;
  strncpy(dst, src, dst_size - 1);
  dst[dst_size - 1] = '\0';
}

void parse_target_line(const char *str, Target_block *tb_arr, int tb_count) {
  if (tb_count < 0 || tb_count >= MAX_BLOCK_NUMBERS) {
    printf("Error!\n");
    return;
  }

  tb_arr[tb_count].dep_count = 0;
  // 解析目标行
  char s[MAX_LINE_LENGTH] = {'\0'};
  safe_copy(s, sizeof(s), str);

  char *colon = strchr(s, ':');
  if (!colon)
    return; // 非法格式
  *colon = '\0';
  // 提取目标
  trim(s);
  safe_copy(tb_arr[tb_count].target, sizeof(tb_arr[tb_count].target), s);

  // 提取依赖项
  char *current = colon + 1;
  while (*current && tb_arr[tb_count].dep_count < MAX_DEP_NUMBERS) {
    // 跳过前导空格
    while (*current && isspace((unsigned char)*current)) {
      current++;
    }

    if (*current == '\0') {
      break; // 到达字符串末尾
    }

    // 标记 token 的开始
    char *token_start = current;

    // 寻找 token 的结束
    while (*current && !isspace((unsigned char)*current)) {
      current++;
    }

    // 提取 token
    size_t token_len = current - token_start;
    if (token_len > 0) {
      char token[MAX_WORD_NUMBERS];
      safe_copy(token, sizeof(token), token_start);
      token[token_len] = '\0'; // 确保 token 正确终止

      safe_copy(tb_arr[tb_count].dep_arr[tb_arr[tb_count].dep_count],
                sizeof(tb_arr[tb_count].dep_arr[tb_arr[tb_count].dep_count]),
                token);
      tb_arr[tb_count].dep_count++;
    }
  }
}

void parse_cmd_line(const char *str, Target_block *tb_arr, int tb_count,
                    int cmd_count) {
  if (tb_count < 0 || tb_count >= MAX_BLOCK_NUMBERS)
    return;
  if (cmd_count < 0 || cmd_count >= MAX_LINE_NUMBERS)
    return;
  // 去掉行尾换行
  char buf[MAX_LINE_LENGTH];
  safe_copy(buf, sizeof(buf), str);
  trim(buf);
  safe_copy(tb_arr[tb_count].commands[cmd_count],
            sizeof(tb_arr[tb_count].commands[cmd_count]), buf);
}

int parse_makefile(char (*line_arr_ptr)[MAX_LINE_LENGTH],
                   Target_block *tb_arr) {
  int tb_count = -1;
  int cmd_count = 0;

  // 解析每个目标块的目标行
  for (int line_count = 0; line_count < MAX_LINE_NUMBERS; line_count++) {
    if (line_type_judge(line_arr_ptr[line_count]) == LINE_TARGET) {
      // 结束前一个代码块并重置cmd_count
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

  // 提交最后一个目标块
  if (tb_count >= 0) {
    tb_arr[tb_count].cmd_count = cmd_count;
  }
  tb_count++;

  print_target_blocks(tb_arr, tb_count);
  parse_check(tb_arr, tb_count);

  return tb_count;
}

void print_target_blocks(Target_block *tb_arr, int tb_count) {
  for (int i = 0; i < tb_count; i++) {
    printf("目标块 #%d:\n", i + 1);
    // 打印目标名
    printf("  目标: %s\n", tb_arr[i].target);
    // 打印依赖
    for (int j = 0; j < tb_arr[i].dep_count; j++) {
      printf("  依赖 #%d: %s\n", j + 1, tb_arr[i].dep_arr[j]);
    }
    // 打印命令
    for (int j = 0; j < tb_arr[i].cmd_count; j++) {
      printf("  命令 #%d:\n    %s\n", j + 1, tb_arr[i].commands[j]);
    }
  }
}

// 返回1表示有错误
int has_duplicate_target(Target_block *tb_arr, int tb_count) {
  for (int i = 0; i < tb_count; i++) {
    for (int j = i + 1; j < tb_count; j++) {
      if (strcmp(tb_arr[i].target, tb_arr[j].target) == 0) {
        printf("Duplicate target definition '%s'\n", tb_arr[i].target);
        return 1;
      }
    }
  }
}

// 返回1表示有错误
int is_dep_invaild(Target_block *tb_arr, int tb_count, const char *dep) {
  int is_invaild = 0;

  // 比较依赖项是否是makefile中的其他目标
  for (int i = 0; i < tb_count; i++) {
    if (strcmp(tb_arr[i].target, dep) == 0) {
      return 0;
    }
  }

  // 判断依赖项在当前目录下是否存在
  if (access(dep, F_OK) == 0) {
    return 0;
  }

  printf("Invalid dependency '%s'\n", dep);
  return 1;
}

void parse_check(Target_block *tb_arr, int tb_count) {
  int has_error = 0;

  if (has_duplicate_target(tb_arr, tb_count) == 1)
    has_error = 1;

  // 逐个检查依赖项是否无效
  for (int i = 0; i < tb_count; i++) {
    for (int j = 0; j < tb_arr[i].dep_count; j++) {
      if (is_dep_invaild(tb_arr, tb_count, tb_arr[i].dep_arr[j]) == 1)
        has_error = 1;
    }
  }

  if (has_error == 0)
    printf("There is no mistakes in the makefile.\n");
}