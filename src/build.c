#include "build.h"
#include "execute_cmd.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

// 查找目标在 Target_block 数组中的索引
static int find_target_block(Target_block *tb_arr, int tb_count,
                             const char *name) {
  for (int i = 0; i < tb_count; i++) {
    if (strcmp(tb_arr[i].target, name) == 0)
      return i;
  }
  return -1;
}

int target_need_build(Target_block *tb_arr, int tb_index) {
  struct stat st_target;
  int target_exists =
      (stat(tb_arr[tb_index].target, &st_target) == 0); // 文件存在则等于1

  // 目标不存在：检查依赖是否都存在
  if (!target_exists) {
    for (int i = 0; i < tb_arr[tb_index].dep_count; i++) {
      struct stat st_dep;
      if (stat(tb_arr[tb_index].dep_arr[i], &st_dep) != 0) {
        fprintf(stderr, "缺失依赖: '%s' (目标 '%s' 无法构建)\n",
                tb_arr[tb_index].dep_arr[i], tb_arr[tb_index].target);
        return -1;
      }
    }

    return 1; // 目标缺失且依赖齐全 -> 需要构建
  }

  // 目标存在：若任一依赖不存在 -> 错误（理论上前面已保证存在）
  time_t tgt_mtime = st_target.st_mtime; // 最后修改时间
  int need = 0;                          // 等于表示需要构建

  for (int i = 0; i < tb_arr[tb_index].dep_count; i++) {
    struct stat st_dep;
    if (stat(tb_arr[tb_index].dep_arr[i], &st_dep) != 0) {
      fprintf(stderr, "依赖缺失: '%s' (目标 '%s')\n",
              tb_arr[tb_index].dep_arr[i], tb_arr[tb_index].target);
      return -1;
    }

    if (st_dep.st_mtime > tgt_mtime) { // 依赖比目标新，需要重新构建
      need = 1;
    }
  }
  return need;
}

int perform_builds(Target_block *tb_arr, int tb_count, const DepGraph *g,
                   const int order[], int order_len) {
  printf("Start compile.\n");

  for (int i = 0; i < order_len; i++) { // 按照拓扑排序的顺序进行构建
    int node_idx = order[i];
    if (!g->nodes[node_idx].is_target)
      continue; // 普通文件节点跳过

    const char *tgt_name = g->nodes[node_idx].name;
    int tb_index = find_target_block(tb_arr, tb_count, tgt_name);
    if (tb_index < 0) {
      fprintf(stderr, "内部错误: 找不到 '%s' 所在目标块\n", tgt_name);
      return -1;
    }

    int need = target_need_build(tb_arr, tb_index);
    if (need < 0) {
      fprintf(stderr, "停止：无法处理目标 '%s'\n", tgt_name);
      return -1;
    } else if (need == 0) {
      printf("[UP-TO-DATE] %s\n", tgt_name);
      continue;
    } else {
      printf("[BUILD] %s\n", tgt_name);
      if (execute_target_commands(tb_arr, tb_index) != 0) {
        fprintf(stderr, "构建目标 '%s' 失败，停止。\n", tgt_name);
        return -1;
      }
    }
  }
  return 0;
}