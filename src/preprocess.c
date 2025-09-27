#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "preprocess.h"
#include "common.h"

// 去除行尾空格，将行尾所有空格替换为“\0”
void trim_trailing_whitespace(char *str) {
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

// 去除注释（#后的内容）
void remove_comments(char *str) {
    char *comment_pos = strchr(str, '#');
    // 将“#”替换为“\0”，删除后面的内容
    if (comment_pos != NULL) {
        *comment_pos = '\0';
    }
}

// 检查字符串是否为空行（仅包含空白字符），为空行则返回1
int is_empty_line(const char *str) {
    while (*str) {
        if (!isspace((unsigned char)*str)) {    // 字符不是空白字符
            return 0;
        }
        str++;
    }
    return 1;
}

// 读取和预处理Makefile，返回0表示处理正常，为1表示异常
int process_makefile(int verbose_mode, char (*line_arr_ptr)[MAX_LINE_LENGTH]) {
    // 打开文件
    FILE *fp = fopen("./Makefile", "r");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }

    // 分界与提示：预处理阶段开始
    puts("=========================================================");
    puts("== 预处理阶段: 读取并清理 Makefile (comments/尾部空白) ==");
    puts("=========================================================");

    FILE *output_fp = NULL;
    if (verbose_mode == 1) {
        output_fp = fopen("./Minimake_cleared.mk", "w");
        if (output_fp == NULL) {
            perror("Failed to create output file");
            fclose(fp);
            return 1;
        }
        printf("Verbose mode enabled. Cleaned content will be saved to Minimake_cleared.mk.\n\n");
    }

    char line[MAX_LINE_LENGTH];
    int line_number = 0;

    // 一行一行地读取文件
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_number++;
        
        // 去除注释
        remove_comments(line);
        
        // 去除行尾空格
        trim_trailing_whitespace(line);
        
        // 将预处理结果存到数组中
        strcpy(line_arr_ptr[line_number - 1], line);

        // 过滤空行
        if (!is_empty_line(line)) {     // 如果不是空行就打印
            if (verbose_mode && output_fp) {
                fprintf(output_fp, "%s\n", line);
            }
                printf("%s\n", line_arr_ptr[line_number - 1]);
        }else{
            printf("\n");
        }
    }

    fclose(fp);
    if (output_fp) {
        fclose(output_fp);
    }
    
    printf("\nMakefile processing completed.\n");
    // 分界与提示：预处理阶段结束
    puts("----------------------------------------------");
    puts("");
    return 0;
}