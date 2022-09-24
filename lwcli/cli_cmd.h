/**
 * @brief cli command APIs
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

#define CLI_MCD_EN_LIST 0

typedef struct cli_cmd_entry {
    char *command;
    void (*function)(int, char **);
    char *describe;
} cli_cmd_entry_t;

typedef struct cli_cmd cli_cmd_t;

cli_cmd_t *CliCmdCreate(const cli_api_t *api);

void CliCmdDestroy(cli_cmd_t *cli_cmd);

#if CLI_MCD_EN_LIST > 0
int32_t CliCmdRegister(cli_cmd_t *cli_cmd,
                       char *cmd,
                       void (*function)(int, char **),
                       char *describe);
#else
int32_t CliCmdTableRegister(cli_cmd_t *cli_cmd, const cli_cmd_entry_t *entry, int16_t num);
#endif

int32_t CliCmdUnregister(cli_cmd_t *cli_cmd, char *cmd);

void CliCmdHandle(cli_cmd_t *cli_cmd, char *cmdline);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __CLI_CMD_H__

