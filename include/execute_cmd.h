#ifndef EXECUTE_CMD_H
#define EXECUTE_CMD_H

#include "parse.h"

int execute_target_commands(Target_block *tb_arr, int tb_index);
static int run_single_command(const char *cmd);
static int determine_max_jobs(int requested);
int execute_targets_parallel(Target_block *tb_arr, const int *indices,
                             int count, int max_parallel);

#endif // EXECUTE_CMD_H
