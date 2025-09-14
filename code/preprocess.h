#ifndef PREPROCESS_H
#define PREPROCESS_H

// 函数声明
void trim_trailing_whitespace(char *str);
void remove_comments(char *str);
int is_empty_line(const char *str);
int process_makefile(int verbose_mode);

// 常量定义
#define MAX_LINE_LENGTH 1024

#endif // PREPROCESS_H