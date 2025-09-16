#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "grammar_check.h"

LineType line_type_judge(const char *str){
    if (str == NULL) 
        return 0;  // 添加空指针检查

    // 跳过前导空格
    while (*str == ' ') {
        str++;
    }
    
    // 如果以制表符开头，是命令行
    if (*str == '\t') {
        return LINE_COMMAND;
    }else if (*str == '\0' || *str == '\n') {    // 空行
        return LINE_EMPTY;
    }else if (*str == '#') {    // 注释行
        return LINE_MESSAGE;
    }

    return LINE_TARGET;
}


// 检查冒号(该行应该不为命令行，空行，注释行)
LineType check_colon(const char *str) {
    char *colon_pos = strchr(str, ':');
    if (colon_pos == NULL) {
        return LINE_INVALID;
    }

    return LINE_TARGET;
}

// 主要的语法检查函数
int grammar_check(char (*line_arr_ptr)[MAX_LINE_LENGTH]) {
    if (line_arr_ptr == NULL) {
        printf("Error: NULL pointer passed to grammar_check\n");
        return 1;
    }

    int has_error = 0;

    // 从第二行开始检查
    for (int i = 0; i < MAX_LINE_NUMBERS && line_arr_ptr[i][0] != '\0'; i++) {

    }

    return has_error;
}