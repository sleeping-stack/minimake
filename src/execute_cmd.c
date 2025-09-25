// ...existing code...
#include "execute_cmd.h"
#include "parse.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

// 运行单条命令：使用 /bin/sh -c "cmd" 执行
// 返回 0 表示成功；返回 -1 表示失败（包括 fork/exec 失败、命令非 0
// 退出、被信号终止等）
static int run_single_command(const char *cmd) {
  if (cmd == NULL || cmd[0] == '\0') {
    // 空命令视为成功
    return 0;
  }

  // 为了避免在 fork 后父子进程重复刷新缓冲区导致重复打印，在 fork
  // 前刷新用户态缓冲
  fflush(NULL);

  pid_t pid = fork();
  if (pid < 0) {
    // fork 失败
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    // 子进程路径：使用 /bin/sh -c 执行命令行字符串
    // argv[0] 通常为程序名，此处使用 "sh" 作为名称（不影响执行）
    // -c 告诉 shell：接下来的参数是一个要执行的命令字符串
    char *argv[] = {(char *)"sh", (char *)"-c", (char *)cmd, NULL};

    // 使用 execvp 执行 /bin/sh
    execvp("/bin/sh", argv);

    // exec 失败才会执行到这里
    perror("execvp");
    _exit(127); // 按惯例，无法执行命令用 127 退出，避免调用父进程的 atexit 处理
  }

  // 父进程路径：等待子进程结束
  int status = 0;
  for (;;) {
    pid_t w = waitpid(pid, &status, 0);

    // 子进程出错或被信号中断
    if (w == -1) {
      if (errno == EINTR) {
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
  if (WIFEXITED(status)) {
    int exit_code = WEXITSTATUS(status);
    if (exit_code != 0) {
      fprintf(stderr, "Command exited with code %d, aborting this target\n",
              exit_code);
      return -1;
    }
  } else if (WIFSIGNALED(status)) {
    int sig = WTERMSIG(status);
    fprintf(stderr, "Command terminated by signal %d (%s)\n", sig,
            strsignal(sig));
    return -1;
  } else {
    // 其他少见情况（如被停止/继续），这里统一当作失败处理
    fprintf(stderr, "Command did not exit normally\n");
    return -1;
  }

  return 0;
}

int execute_target_commands(Target_block *tb_arr, int tb_index) {
  for (int i = 0; i < tb_arr[tb_index].cmd_count; i++) {
    const char *cmd = tb_arr[tb_index].commands[i];
    printf("[CMD %d] %s\n", i + 1, cmd);

    // 执行单条命令，失败则立即中断构建
    if (run_single_command(cmd) != 0) {
      return -1;
    }
  }
  return 0;
}