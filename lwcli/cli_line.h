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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* parameter config */
#define LINE_QUEUE_BUF_SIZE	 128   /* 循环队列缓冲区大小 */
#define LINE_LEN_MAX         100   /* 支持行最大长度 */
#define LINE_HISTORY_MAX	 4     /* 历史命令最大记录条数 */

#define DEFAULT_PROMPT       "hello>"

typedef enum {
    ECHO_ENABLE = 0,
    ECHO_DISABLE,
} line_echo_t;

typedef struct cli_line cli_line_t;

typedef int32_t (*line_printf)(const char *format, ...);

typedef void (*line_handle)(void *ctx, char *line);

cli_line_t *CliLineCreate(const char *prompt, line_printf printf, line_handle handle, void *ctx);

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


