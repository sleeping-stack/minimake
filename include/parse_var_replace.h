#ifndef PARSE_VAR_REPLACE_H
#define PARSE_VAR_REPLACE_H

#include <stddef.h>

#include "common.h"

typedef struct
{
    char name[MAX_WORD_NUMBERS];
    char value[MAX_LINE_LENGTH];
    int used;  // 1表示已占用
} VarEntry;

// 变量系统初始化/清理
void var_init(void);

// 设置或覆盖变量
// 返回0成功，非0表示失败（满了）
int var_set(const char *name, const char *value);

// 获取变量到缓冲区（若未定义则尝试 getenv，仍无则返回空串）
// 返回写入长度
int var_get(const char *name, char *out, size_t out_size);

static void safe_append(char *dst, size_t dst_size, const char *src);
static void safe_append_ch(char *dst, size_t dst_size, char ch);

// 将 src 中的 $(NAME) 或 ${NAME} 替换后写入 dst
// 支持嵌套与递归展开；未定义变量替换为空串
static int expand_recursive(const char *src, char *dst, size_t dst_size, int depth);
void var_expand_into(const char *src, char *dst, size_t dst_size);

// 将变量表导出为环境变量（setenv），便于在命令中用 $NAME
void var_export_to_env(void);

// 打印当前已定义的变量（仅内部表，不含未覆盖的环境变量）
void var_print_all(void);

#endif