#include "build.h"
#include "clioptions_parse.h"
#include "common.h"
#include "grammar_check.h"
#include "graph.h"
#include "parse.h"
#include "parse_var_replace.h"
#include "preprocess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // 初始化变量系统
  var_init();

  // 解析命令行
  CliOptions opts;
  if (parse_args(argc, argv, &opts) != 0) {
    print_usage(argv[0]);
    printf("Please input options correctly!\n");
    return 1;
  }
  if (opts.show_help)
    print_usage(argv[0]);

  int verbose_mode = opts.verbose_mode;

  char line_arr[MAX_LINE_NUMBERS][MAX_LINE_LENGTH] = {{'\0'}};
  if (process_makefile(verbose_mode, line_arr) == 1)
    return 1;

  int has_error = grammar_check(line_arr);

  if (has_error == 1) {
    printf("Please make sure the grammer is right so that the program can "
           "parse the makefile.\n");
    return 1;
  }

  Target_block *tb_arr = malloc(MAX_BLOCK_NUMBERS * sizeof(Target_block));
  if (tb_arr == NULL) {
    perror("malloc");
    return 1;
  }
  int tb_count = parse_makefile(line_arr, tb_arr);

  if (parse_check(tb_arr, tb_count) != 0) {
    free(tb_arr);
    return 1;
  }

  // 将 Makefile 变量导出到环境（便于命令中用 $NAME）
  var_export_to_env();

  // 打印依赖图
  DepGraph g;
  if (build_dep_graph(&g, tb_arr, tb_count) != 0) {
    fprintf(stderr, "DepGraph build failed.\n");
    free(tb_arr);
    return 1;
  }
  puts("==== 依赖图 (dep -> target) ====");
  print_graph(&g);

  int order[MAX_GRAPH_NODES];
  int order_len = 0;
  int *order_len_ptr = &order_len;

  if (topo_sort_graph(&g, order, order_len_ptr) != 0) {
    fprintf(stderr, "Topo soft faild.");
    return 1;
  }

  // 读取并发度参数
  int jobs = opts.jobs;

  // 并行构建目标（默认为第一个目标）
  if (opts.n_targets == 0) {
    if (build_parallel(tb_arr, tb_count, jobs, NULL) != 0) {
      free(tb_arr);
      return 1;
    }
  } else {
    for (int i = 0; i < opts.n_targets; i++) {
      if (build_parallel(tb_arr, tb_count, jobs, opts.targets_arr[i]) != 0) {
        free(tb_arr);
        return 1;
      }
    }
  }

  free(tb_arr);

  return 0;
}