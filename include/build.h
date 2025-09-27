#ifndef BUILD_H
#define BUILD_H

// #include "graph.h"
#include "parse.h"

static int find_target_index(Target_block *tb_arr, int tb_count,
                             const char *name);
// 针对一个目标判断：
// 返回  1 -> 需要构建
// 返回  0 -> 已是最新
// 返回 -1 -> 依赖缺失或 stat 出错
int target_need_build(Target_block *tb_arr, int tb_index);

static void mark_needed_dfs(Target_block *tb_arr, int tb_count, int idx,
                            char *needed);

int build_parallel(Target_block *tb_arr, int tb_count, int jobs,
                   const char *target);

#endif