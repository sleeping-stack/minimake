#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"
#include "parse.h"

typedef struct {
  char name[MAX_WORD_NUMBERS];
  int is_target; // 1: 该节点对应一个目标; 0: 仅普通文件
} GraphNode;

typedef struct {
  int node_count;
  GraphNode nodes[MAX_GRAPH_NODES];
  // 使用邻接矩阵表示有向图
  unsigned char adj[MAX_GRAPH_NODES]
                   [MAX_GRAPH_NODES]; // adj[u][v] = 1 表示 u -> v
  int indegree[MAX_GRAPH_NODES];      // 每个节点的入度
} DepGraph;

// 返回添加节点在邻接矩阵中的index, 返回-1表示异常
static int add_node(DepGraph *g, const char *name, int is_target);

// 查找节点
int graph_find_node(const DepGraph *g, const char *name);

// 构建依赖图（目标与其依赖，方向: dep -> target），返回-1表示异常
int build_dep_graph(DepGraph *g, Target_block *tb_arr, int tb_count);

// 拓扑排序（Kahn），order[] 返回节点索引序列
// 返回 0 正常；返回 -1 表示检测到环
int topo_sort_graph(const DepGraph *g, int order[], int *order_len);

// 调试输出
void print_graph(const DepGraph *g);

#endif