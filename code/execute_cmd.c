#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int execute_target_commands(Target_block *tb_arr, int tb_index) {
  for (int i = 0; i < tb_arr[tb_index].cmd_count; i++) {
    printf("[CMD %d] %s\n", i + 1, tb_arr[tb_index].commands[i]);

    int status = system(tb_arr[tb_index].commands[i]);
    if (status == -1) {
      perror("system");
      return -1;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
      fprintf(stderr, "命令退出码 %d, 中止该目标\n", WEXITSTATUS(status));
      return -1;
    } else if (WIFSIGNALED(status)) {
      fprintf(stderr, "命令被信号 %d 终止\n", WTERMSIG(status));
      return -1;
    }
  }
  return 0;
}

int execute_all_targets(Target_block *tb_arr, int tb_count) {
  for (int i = 0; i < tb_count; i++) {
    if (execute_target_commands(tb_arr, i) != 0)
      return -1;
  }
  return 0;
}