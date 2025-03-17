#include <stdio.h>
#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    int long_index = 0;

    // 定义长选项
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},          // --help 对应短选项 -h
        {0, 0, 0, 0}                           // 结束标志
    };

    // 解析命令行参数
    while ((opt = getopt_long(argc, argv, "h", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'h': // 支持 -h 和 --help
                printf("帮助信息: \n");
                break;

            case '?':
                // 处理未知选项的情况
                fprintf(stderr, "错误: 未知选项。\n");
                return 1;

            default:
                fprintf(stderr, "用法: %s [-h]\n", argv[0]);
                return 1;
        }
    }

    // 打印剩余的非选项参数
    for (int i = optind; i < argc; i++) {
        printf("非选项参数: %s\n", argv[i]);
    }

    return 0;
}
