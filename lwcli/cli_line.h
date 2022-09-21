/**
 * @brief cli line
 * @file cli_line.h
 * @version 1.0
 * @author Su YouJiang
 * @date 2020-3-17 9:34:57
 * @par Copyright:
 * Copyright (c) LwCLI 2018-2020. All rights reserved.
 *
 * @history
 * 1.Date        : 2020-3-17 9:34:57
 *   Author      : Su YouJiang
 *   Modification: Created file
 */

#ifndef __CLI_LINE_H__
#define __CLI_LINE_H__

#ifdef __cplusplus
extern "C"{
#endif  // __cplusplus

#include "cli_def.h"

/* parameter config */
#define DEFAULT_PROMPT       "hello>"

typedef enum {
    ECHO_ENABLE = 0,
    ECHO_DISABLE,
} line_echo_t;

typedef void (*line_handle_fn_t)(void *ctx, char *line);

typedef struct cli_line_cfg {
    int16_t queue_size;
    int16_t line_buf_size;
    int16_t history_max;
    line_handle_fn_t handle_cb;  /* 命令行处理回调函数 */
    void *ctx;
} cli_line_cfg_t;

typedef struct cli_line cli_line_t;

cli_line_t *CliLineCreate(const cli_inf_t *inf, const cli_line_cfg_t *cfg);

void CliLineDestory(cli_line_t *line);

void CliLineSetPrompt(cli_line_t *line, const char *prompt);

void CliLineSetEcho(cli_line_t *line, line_echo_t echo);

void CliLineInputChar(cli_line_t *line, uint8_t data);

void CliLineInputBlock(cli_line_t *line, uint8_t *pdata, uint32_t datalen);

void CliLineTick(cli_line_t *line);

#ifdef __cplusplus
}
#endif  // __cplusplus
#endif  // __CLI_LINE_H__


