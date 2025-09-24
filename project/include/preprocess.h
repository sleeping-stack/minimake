#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "common.h"

// 函数声明
void trim_trailing_whitespace(char *str);
void remove_comments(char *str);
int is_empty_line(const char *str);
int process_makefile(int verbose_mode, char (*line_arr_ptr)[MAX_LINE_LENGTH]);

#endif // PREPROCESS_H