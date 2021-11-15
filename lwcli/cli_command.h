/**
 * @brief cli command
 * @file cli_command.h
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

#ifndef __CLI_COMMAND_H__
#define __CLI_COMMAND_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define CMD_ARGC_MAX    6

typedef struct cli_cmd_entry {
    char *command;
    void (*function)(int, char **);
    char *describe;
} cli_cmd_entry_t;

typedef struct cli_command cli_command_t;

typedef int32_t (*command_printf)(const char *format, ...);

cli_command_t *CliCmdCreate(int32_t cmd_max, command_printf printf);

void CliCmdDestroy(cli_command_t *cli_cmd);

int32_t CliCmdRegister(cli_command_t *cli_cmd,
                             char *cmd,
                             void (*function)(int, char **),
                             char *describe);

void CliCmdHandle(cli_command_t *cli_cmd, char *cmdline);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __CLI_COMMAND_H__

