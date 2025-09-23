#ifndef BUILD_H
#define BUILD_H

#include "graph.h"
#include "parse.h"

static int find_target_block(Target_block *tb_arr, int tb_count,
                             const char *name);
// 针对一个目标判断：
// 返回  1 -> 需要构建
// 返回  0 -> 已是最新
// 返回 -1 -> 依赖缺失或 stat 出错
int target_need_build(Target_block *tb_arr, int tb_index);

// 根据拓扑序执行所有目标的按需构建
// 返回 0 正常；非 0 表示过程中出现错误并中止
int perform_builds(Target_block *tb_arr, int tb_count, const DepGraph *g,
                   const int order[], int order_len);

#endif