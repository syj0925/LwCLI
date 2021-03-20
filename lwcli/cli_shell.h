/**
 * @brief cli shell
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

#define LOGIN_ENABLE    1
#define PROMPT_LOGIN    "Login:"
#define PROMPT_PASSWD   "Password:"
#define PROMPT_CMD      "hello>"

#define CLI_USERNAME    "admin"
#define CLI_PASSWORD    "admin"

typedef int32_t (*shell_printf)(const char *format, ...);

void CliShellInit(int32_t cmd_max, shell_printf printf);

int32_t CliShellRegister(char *cmd, 
                             void (*function)(int, char **), 
                             char *describe);
void CliShellTick(void);

void CliShellInputBlock(uint8_t *pdata, uint32_t datalen);

void CliShellInputChar(uint8_t data);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __CLI_SHELL_H__

