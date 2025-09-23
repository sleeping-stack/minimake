// ...existing code...
#include "execute_cmd.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_target_commands(Target_block *tb_arr, int tb_index) {
  for (int i = 0; i < tb_arr[tb_index].cmd_count; i++) {
    printf("[CMD %d] %s\n", i + 1, tb_arr[tb_index].commands[i]);

    int status = system(tb_arr[tb_index].commands[i]);
    if (status == -1) {
      perror("system");
      return -1;
    }

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
      fprintf(stderr, "Command exited with code %d, aborting this target\n",
              WEXITSTATUS(status));
      return -1;
    } else if (WIFSIGNALED(status)) {
      fprintf(stderr, "Command terminated by signal %d\n", WTERMSIG(status));
      return -1;
    }
  }
  return 0;
}
