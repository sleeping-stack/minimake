#include "graph.h"

#include <stdio.h>
#include <string.h>

static int add_node(DepGraph *g, const char *name, int is_target)
{
    // 如果之前是普通文件，后来发现它其实是目标，就升级 is_target
    for (int i = 0; i < g->node_count; i++)
    {
        if (strcmp(g->nodes[i].name, name) == 0)
        {
            if (is_target == 1 && g->nodes[i].is_target == 0) g->nodes[i].is_target = 1;
            return i;
        }
    }

    if (g->node_count >= MAX_GRAPH_NODES) return -1;

    int idx = g->node_count++;

    // 填充新节点
    strncpy(g->nodes[idx].name, name, sizeof(g->nodes[idx].name) - 1);
    g->nodes[idx].name[sizeof(g->nodes[idx].name) - 1] = '\0';
    g->nodes[idx].is_target                            = is_target;
    return idx;
}

int graph_find_node(const DepGraph *g, const char *name)
{
    for (int i = 0; i < g->node_count; i++)
    {
        if (strcmp(g->nodes[i].name, name) == 0) return i;
    }
    return -1;
}

int build_dep_graph(DepGraph *g, Target_block *tb_arr, int tb_count)
{
    // 把指针 g 所指向的结构体（类型是 DepGraph）占用的整块内存全部填充为
    // 0；效果：node_count 变为 0；nodes[] 每个元素的 name[] 全部置
    // '\0'，is_target 变 0；adj 矩阵全部清零（无边）；indegree[] 全清零
    memset(g, 0, sizeof(*g));

    // 先添加所有目标节点
    for (int i = 0; i < tb_count; i++)
    {
        if (add_node(g, tb_arr[i].target, 1) < 0)
        {
            fprintf(stderr, "The number of vertex is overload!\n");
            return -1;
        }
    }

    // 添加依赖并建立边 dep -> target
    for (int i = 0; i < tb_count; i++)
    {
        int target_idx = graph_find_node(g, tb_arr[i].target);
        for (int d = 0; d < tb_arr[i].dep_count; d++)
        {
            int dep_idx = add_node(g, tb_arr[i].dep_arr[d], 0);
            if (dep_idx < 0)
            {
                fprintf(stderr, "The number of vertex is overload!\n");
                return -1;
            }
            if (g->adj[dep_idx][target_idx] == 0)
            {
                g->adj[dep_idx][target_idx] = 1;
                g->indegree[target_idx]++;
            }
        }
    }
    return 0;
}

int topo_sort_graph(const DepGraph *g, int order[], int *order_len)
{
    int indeg[MAX_GRAPH_NODES];
    memcpy(indeg, g->indegree, sizeof(int) * g->node_count);

    int queue[MAX_GRAPH_NODES];
    int qh = 0, qt = 0;  // 使用数组模拟队列，qh 为队头索引，指向第一个元素；qt
                         // 为队尾索引，指向最后一个元素的下一个位置

    // 初始化：所有入度为 0 的节点入队
    for (int i = 0; i < g->node_count; i++)
    {
        if (indeg[i] == 0) queue[qt++] = i;
    }

    int count = 0;
    while (qh < qt)
    {  // 队列非空
        int u          = queue[qh++];
        order[count++] = u;
        for (int v = 0; v < g->node_count; v++)
        {
            if (g->adj[u][v] == 1)
            {
                indeg[v]--;
                if (indeg[v] == 0) queue[qt++] = v;
            }
        }
    }

    *order_len = count;
    if (count != g->node_count)
    {
        // 存在环
        printf("The depgraph has circles!\n");
        return -1;
    }

    // 打印排序后结果
    puts("==============================================");
    puts("== 拓扑排序结果 ==");
    puts("==============================================");
    for (int i = 0; i < *order_len; i++)
    {
        printf("%s%s", g->nodes[order[i]].name, (i == *order_len - 1) ? "\n" : " -> ");
    }
    puts("----------------------------------------------");
    puts("");

    return 0;
}

void print_graph(const DepGraph *g)
{
    puts("==============================================");
    puts("== 依赖图 (dep -> target) ==");
    puts("==============================================");
    printf("Graph nodes (%d):\n", g->node_count);
    for (int i = 0; i < g->node_count; i++)
    {
        printf("  [%d] %s %s\n", i, g->nodes[i].name, g->nodes[i].is_target ? "(target)" : "");
    }
    puts("Edges (dep -> target):");
    for (int u = 0; u < g->node_count; u++)
    {
        for (int v = 0; v < g->node_count; v++)
        {
            if (g->adj[u][v])
            {
                printf("  %s -> %s\n", g->nodes[u].name, g->nodes[v].name);
            }
        }
    }
    puts("----------------------------------------------");
    puts("");
}