#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preprocess.h"

int main(int arvc, char *argv[]) {
    // 创建动态flag数组用来表示输入的每个参数的状态，1表示该参数已匹配
    int *flag = malloc(arvc * sizeof(int));
    if (flag == NULL) {
        perror("malloc failed");
        return 1;
    }

    // 初始化
    for (int i = 0; i < arvc; i++) {
        flag[i] = 0;
    }

    int verbose_mode = 0; // 为1表示使用详细模式

    for (int i = 0; i < arvc; i++) {
        if (strcmp("--help", argv[i]) == 0 || strcmp("-h", argv[i]) == 0) {
            printf("usage: --help or -h          get usage\n");
            printf("       --verbose_mode or -v  open verbose mode\n");
            puts("");
            flag[i] = 1;
        } else if (strcmp("-v", argv[i]) == 0 || strcmp("--verbose_mode", argv[i])== 0) {
            verbose_mode = 1;
            flag[i] = 1;
        }
    }

    if (arvc == 1)
        printf("No arguments input!\n");
    else{
        for(int i = 1; i < arvc; i++){
            if (flag[i] == 0){
                printf("No.%d argument is an uncorrect argument!\n", i);
            }
        }
    }

    // 释放内存
    if (flag != NULL) {
        free(flag);
    }

    process_makefile(verbose_mode);

    return 0;
}