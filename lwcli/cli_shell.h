/**
 * @brief cli shell APIs
 * @file cli_shell.h
 * @version 1.0
 * @author Su YouJiang
 * @date 2020-3-18 9:3:20
 * @par Copyright:
 * Copyright (c) LwCLI 2018-2020. All rights reserved.
 *
 * @history
 * 1.Date        : 2020-3-18 9:3:20
 *   Author      : Su YouJiang
 *   Modification: Created file
 */

#ifndef __CLI_SHELL_H__
#define __CLI_SHELL_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "cli_def.h"
#include "cli_cmd.h"

typedef struct cli_shell_cfg {
    int16_t line_queue_size;
    int16_t line_buf_size;
    int16_t line_history_num;
    const char *username;
    const char *password;
    const char *prompt;
} cli_shell_cfg_t;

int32_t CliShellInit(const cli_api_t *api, cli_shell_cfg_t *cfg);

void CliShellDeinit(void);

int32_t CliShellRegister(char *cmd,
                         void (*function)(int, char **),
                         char *describe);

int32_t CliShellTableRegister(const cli_cmd_entry_t *entry, int16_t num);

int32_t CliShellUnegister(char *cmd);

void CliShellTick(void);

void CliShellInputBlock(uint8_t *pdata, uint32_t datalen);

void CliShellInputChar(uint8_t data);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __CLI_SHELL_H__

