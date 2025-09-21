#ifndef EXECUTE_CMD_H
#define EXECUTE_CMD_H

#include "parse.h"

int execute_target_commands(Target_block *tb_arr, int tb_index);
int execute_all_targets(Target_block *tb_arr, int tb_count);

#endif // EXECUTE_CMD_H