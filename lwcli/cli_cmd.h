/**
 * @brief cli command
 * @file cli_cmd.h
 * @version 1.0
 * @author Su YouJiang
 * @date 2020-3-17 19:5:59
 * @par Copyright:
 * Copyright (c) LwCLI 2018-2020. All rights reserved.
 *
 * @history
 * 1.Date        : 2020-3-17 19:5:59
 *   Author      : Su YouJiang
 *   Modification: Created file
 */

#ifndef __CLI_CMD_H__
#define __CLI_CMD_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#include "cli_def.h"

#define CMD_ARGC_MAX    10

typedef struct cli_cmd_entry {
	struct cli_cmd_entry *next;
    char *command;
    void (*function)(int, char **);
    char *describe;
} cli_cmd_entry_t;

typedef struct cli_cmd cli_cmd_t;

cli_cmd_t *CliCmdCreate(const cli_inf_t *inf, int16_t cmd_max);

void CliCmdDestroy(cli_cmd_t *cli_cmd);

int32_t CliCmdRegister(cli_cmd_t *cli_cmd,
                       char *cmd,
                       void (*function)(int, char **),
                       char *describe);

int32_t CliCmdUnregister(cli_cmd_t *cli_cmd, char *cmd);

void CliCmdHandle(cli_cmd_t *cli_cmd, char *cmdline);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __CLI_CMD_H__

