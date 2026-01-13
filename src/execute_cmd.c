#include "execute_cmd.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parse.h"

// 运行单条命令：使用 /bin/sh -c "cmd" 执行
// 返回 0 表示成功；返回 -1 表示失败（包括 fork/exec 失败、命令非 0
// 退出、被信号终止等）
static int run_single_command(const char *cmd)
{
    if (cmd == NULL || cmd[0] == '\0')
    {
        // 空命令视为成功
        return 0;
    }

    // 为了避免在 fork 后父子进程重复刷新缓冲区导致重复打印，在 fork
    // 前刷新用户态缓冲
    fflush(NULL);

    pid_t pid = fork();
    if (pid < 0)
    {
        // fork 失败
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        // 子进程路径：使用 /bin/sh -c 执行命令行字符串
        // argv[0] 通常为程序名，此处使用 "sh" 作为名称（不影响执行）
        // -c 告诉 shell：接下来的参数是一个要执行的命令字符串
        char *argv[] = {(char *)"sh", (char *)"-c", (char *)cmd, NULL};

        // 使用 execvp 执行 /bin/sh
        execvp("/bin/sh", argv);

        // exec 失败才会执行到这里
        int ec = (errno == ENOENT) ? 127 : 126;  // 127: 未找到; 126: 不能执行
        perror("execv");
        _exit(ec);  // 避免运行 atexit
    }

    // 父进程路径：等待子进程结束
    int status = 0;
    for (;;)
    {
        pid_t w = waitpid(pid, &status, 0);

        // 子进程出错或被信号中断
        if (w == -1)
        {
            if (errno == EINTR)
            {
                // 被中断（如收到信号），重试 waitpid
                continue;
            }
            perror("waitpid");
            return -1;
        }

        // 正常获得子进程状态
        break;
    }

    // 根据子进程结束状态进行判定
    if (WIFEXITED(status))
    {
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0)
        {
            fprintf(stderr, "Command exited with code %d, aborting this target\n", exit_code);
            return -1;
        }
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        fprintf(stderr, "Command terminated by signal %d (%s)\n", sig, strsignal(sig));
        return -1;
    }
    else
    {
        // 其他情况统一当作失败处理
        fprintf(stderr, "Command did not exit normally\n");
        return -1;
    }

    return 0;
}

int execute_target_commands(Target_block *tb_arr, int tb_index)
{
    for (int i = 0; i < tb_arr[tb_index].cmd_count; i++)
    {
        const char *cmd = tb_arr[tb_index].commands[i];
        printf("[CMD %d] %s\n", i + 1, cmd);

        // 执行单条命令，失败则立即中断构建
        if (run_single_command(cmd) != 0)
        {
            return -1;
        }
    }
    return 0;
}

// 自动选择并发度：优先读取 MINIMAKE_JOBS；否则取在线 CPU 数；至少为 1
static int determine_max_jobs(int requested)
{
    if (requested > 0) return requested;

    long n = sysconf(_SC_NPROCESSORS_ONLN);  // 在线 CPU 核数，至少为1
    if (n <= 0) n = 1;
    return (int)n;
}

// 为每个目标 fork 一个子进程：子进程内部顺序执行该目标的命令
int execute_targets_parallel(Target_block *tb_arr, const int *indices, int count, int max_parallel)
{
    if (!tb_arr || !indices || count <= 0) return 0;

    max_parallel = determine_max_jobs(max_parallel);
    if (max_parallel < 1) max_parallel = 1;

    int next     = 0;  // 下一个待启动的目标在 indices[] 的位置
    int active   = 0;  // 正在运行的子进程数量
    int finished = 0;  // 已完成的目标数量
    int failed   = 0;  // 任一失败即置位，停止继续启动新作业

    // 简单环路：启动到达并发上限；等待任意子进程结束；再继续启动
    while (finished < count)
    {
        // 启动新作业
        while (!failed && active < max_parallel && next < count)
        {
            int ti = indices[next];
            // 无命令的目标直接视为成功完成
            if (tb_arr[ti].cmd_count <= 0)
            {
                next++;
                finished++;
                continue;
            }

            printf("[PAR] start target: %s\n", tb_arr[ti].target);

            pid_t pid = fork();
            if (pid < 0)
            {
                perror("fork");
                failed = 1;
                break;
            }
            if (pid == 0)
            {
                // 子进程：串行执行本目标的命令
                int rc = execute_target_commands(tb_arr, ti);
                _exit(rc == 0 ? 0 : 1);
            }

            // 父进程：计数
            active++;
            next++;
        }

        if (active == 0)
        {
            // 没有在跑的子进程且无法再启动（可能因为失败或已全部跳过）
            break;
        }

        // 等待任意一个子进程结束
        int status = 0;
        pid_t w    = waitpid(-1, &status, 0);
        if (w < 0)
        {
            if (errno == EINTR) continue;
            perror("waitpid");
            failed = 1;
            break;
        }

        active--;
        finished++;

        if (WIFEXITED(status))
        {
            int ec = WEXITSTATUS(status);
            if (ec != 0)
            {
                fprintf(stderr, "[PAR] a target failed (exit=%d)\n", ec);
                failed = 1;
            }
        }
        else if (WIFSIGNALED(status))
        {
            fprintf(stderr, "[PAR] a target terminated by signal %d\n", WTERMSIG(status));
            failed = 1;
        }
        else
        {
            fprintf(stderr, "[PAR] a target ended abnormally\n");
            failed = 1;
        }
    }

    // 若失败，不再启动新的子进程；但已等待所有已启动的进程回收（循环条件保证）
    return failed ? -1 : 0;
}