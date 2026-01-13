#include "build.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "execute_cmd.h"

// 查找目标在 Target_block 数组中的索引
static int find_target_index(Target_block *tb_arr, int tb_count, const char *name)
{
    for (int i = 0; i < tb_count; i++)
    {
        if (strcmp(tb_arr[i].target, name) == 0) return i;
    }
    return -1;
}

int target_need_build(Target_block *tb_arr, int tb_index)
{
    struct stat st_target;
    int target_exists = (stat(tb_arr[tb_index].target, &st_target) == 0);  // 文件存在则等于1

    // 目标不存在：检查依赖是否都存在
    if (!target_exists)
    {
        for (int i = 0; i < tb_arr[tb_index].dep_count; i++)
        {
            struct stat st_dep;
            if (stat(tb_arr[tb_index].dep_arr[i], &st_dep) != 0)
            {
                return -1;
            }
        }

        return 1;  // 目标缺失且依赖齐全 -> 需要构建
    }

    // 目标存在：若任一依赖不存在 -> 错误（理论上前面已保证存在）
    time_t tgt_mtime = st_target.st_mtime;  // 最后修改时间
    int need         = 0;                   // 等于表示需要构建

    for (int i = 0; i < tb_arr[tb_index].dep_count; i++)
    {
        struct stat st_dep;
        if (stat(tb_arr[tb_index].dep_arr[i], &st_dep) != 0)
        {
            return -1;
        }

        if (st_dep.st_mtime > tgt_mtime)
        {  // 依赖比目标新，需要重新构建
            need = 1;
        }
    }
    return need;
}

// 使用深度优先遍历查找构建一个目标所需的依赖
static void mark_needed_dfs(Target_block *tb_arr, int tb_count, int idx, char *needed)
{
    if (idx < 0) return;

    if (needed[idx] == 1) return;

    needed[idx] = 1;

    for (int d = 0; d < tb_arr[idx].dep_count; ++d)
    {
        const char *dep = tb_arr[idx].dep_arr[d];
        int j           = find_target_index(tb_arr, tb_count, dep);
        if (j >= 0)  // dep[j]为makefile中的其他目标，需要构建
            mark_needed_dfs(tb_arr, tb_count, j, needed);
    }
}

// 返回0表示正常，1为异常
int build_parallel(Target_block *tb_arr, int tb_count, int jobs, const char *target)
{
    if (!tb_arr || tb_count <= 0) return 0;

    puts("==============================================");
    puts("== 构建阶段 ==");
    puts("==============================================");

    int root = 0;
    if (!target)
        root = 0;
    else
    {
        root = find_target_index(tb_arr, tb_count, target);
        if (root < 0)
        {
            printf("Invaild target!\n");
            return 1;
        }
    }

    // 计算“需要构建的子图”：从 root 出发的可达目标集合
    char *needed = (char *)calloc((size_t)tb_count, 1);  // 需要构建的目标的下标数组
    char *built  = (char *)calloc((size_t)tb_count, 1);  // 表示目标是否构建完成
    if (!needed || !built)
    {
        free(needed);
        free(built);
        fprintf(stderr, "Calloc failed.\n");
        return 1;
    }
    mark_needed_dfs(tb_arr, tb_count, root, needed);

    // 拓扑分批：每一批挑选“依赖都已准备好”的目标并行执行
    int remaining = 0;  // 剩余需要构建的目标数
    for (int i = 0; i < tb_count; ++i)
        if (needed[i]) remaining++;

    // 每个循环代表一批
    while (remaining > 0)
    {
        int indices[MAX_BLOCK_NUMBERS];
        int n_ready = 0;  // 本批可执行目标数量
        int failed  = 0;  // 本批是否出现错误（如缺失依赖）

        // 收集当前可执行的目标
        for (int i = 0; i < tb_count; ++i)
        {
            if (!needed[i] || built[i])  // 不在子图或已将构建完成，跳过
                continue;

            int ready = 1;  // 1表示依赖准备好可以构建

            for (int d = 0; d < tb_arr[i].dep_count; ++d)
            {
                int j = find_target_index(tb_arr, tb_count, tb_arr[i].dep_arr[d]);
                // 依赖若是目标且属于 needed，则必须先构建完成
                if (j >= 0 && needed[j] && !built[j])
                {
                    ready = 0;  // 该目标不可构建
                    break;
                }
            }

            if (ready == 0) continue;

            // 依赖已满足构建条件，判断是否需要构建：
            // 返回  1 -> 需要构建
            // 返回  0 -> 已是最新
            // 返回 -1 -> 依赖缺失（理论上不会出现）
            int need = target_need_build(tb_arr, i);
            if (need < 0)
            {
                failed = 1;
                break;
            }
            if (need == 0)
            {
                built[i] = 1;
                remaining--;
                printf("[UP-TO-DATE] %s\n", tb_arr[i].target);
                continue;
            }

            // 需要构建：加入并行队列
            indices[n_ready++] = i;
        }

        if (failed)
        {
            free(needed);
            free(built);
            return 1;
        }

        // 并行执行本批所有可执行的目标
        if (execute_targets_parallel(tb_arr, indices, n_ready, jobs) != 0)
        {
            free(needed);
            free(built);
            return 1;
        }

        // 标记本批已完成
        for (int k = 0; k < n_ready; ++k)
        {
            int i = indices[k];
            if (!built[i])
            {
                built[i] = 1;
                remaining--;
            }
        }
    }

    free(needed);
    free(built);
    puts("----------------------------------------------");
    puts("");
    return 0;
}