#ifndef EXECUTE_CMD_H
#define EXECUTE_CMD_H

#include "parse.h"

int execute_target_commands(Target_block *tb_arr, int tb_index);
static int run_single_command(const char *cmd);


#endif // EXECUTE_CMD_H
     