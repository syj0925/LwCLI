/**
 * @brief cli line module
 * @file cli_line.c
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

#include "cli_line.h"
#include "list.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define DEFAULT_PROMPT  "hello>"
#define STR_BACKSPACE   "\b \b"
#define STR_ENTER       "\r\n"

enum KEY_ACTION {
    KEY_NULL = 0,	    /* NULL */
    KEY_TAB = 9,        /* Tab */
    KEY_NEWLINE = 10,   /* NewLine '\n' */
    KEY_ENTER = 13,     /* Enter '\r' */
    KEY_ESC = 27,       /* Escape */
    KEY_LBRKT = 91,
    KEY_BACKSPACE = 127,/* Backspace */

    CTRL_A = 1,         /* Ctrl+a */
    CTRL_B = 2,         /* Ctrl-b */
    CTRL_C = 3,         /* Ctrl-c */
    CTRL_D = 4,         /* Ctrl-d */
    CTRL_E = 5,         /* Ctrl-e */
    CTRL_F = 6,         /* Ctrl-f */
    CTRL_H = 8,         /* Ctrl-h */  /* '\b' */
    CTRL_K = 11,        /* Ctrl+k */
    CTRL_L = 12,        /* Ctrl+l */
    CTRL_N = 14,        /* Ctrl-n */
    CTRL_P = 16,        /* Ctrl-p */
    CTRL_T = 20,        /* Ctrl-t */
    CTRL_U = 21,        /* Ctrl+u */
    CTRL_W = 23,        /* Ctrl+w */
};

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/
typedef struct hostory {
    dlist_t node;
	char data[0];
} history_t;

typedef struct queue {
	uint16_t   r_cursor;
	uint16_t   w_cursor;
	uint16_t   queue_size;
	uint8_t    data[0];
} queue_t;

struct cli_line {
    const cli_api_t *api;
    cli_line_cfg_t cfg;                  /* Configuration parameters */

    const char *prompt;
    uint8_t     echo_opt;                /* Echo enable or disable */
    uint8_t     key_enter;               /* Key enter or newline */
    uint8_t     key_combo;               /* Key combination */

    uint8_t     history_num;             /* History number */
    dlist_t     history_list;            /* History list */
    dlist_t    *history_index;           /* History check index */

    uint16_t    line_cursor;             /* Line cursor position */
    uint16_t    line_end;                /* Line end position */
    char       *line_buf;                /* Line buffer */

    queue_t     queue;                   /* Input cache queue */
	uint8_t     data[0];
};

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#if 0
static int8_t queueIsEmpty(queue_t *queue)
{
	return (queue->w_cursor == queue->r_cursor);
}

static int8_t queueIsFull(queue_t *queue)
{
	return (((queue->w_cursor + 1) % sizeof(queue->data)) == queue->r_cursor);
}
#endif /* if 0. 2021-3-17 10:19:45 syj0925 */

static int32_t queuePop(queue_t *queue)
{
    int16_t ret = RET_ERROR;

	if (queue->r_cursor != queue->w_cursor) {
		ret = queue->data[queue->r_cursor];
		queue->r_cursor = (queue->r_cursor + 1) % queue->queue_size;
	}

	return ret;
}

static int32_t queuePush(queue_t *queue, uint8_t data)
{
	int8_t ret = RET_ERROR;
	uint16_t w_cursor = 0;

	w_cursor = (queue->w_cursor + 1) % queue->queue_size;

	if (w_cursor != queue->r_cursor) {
		queue->data[queue->w_cursor] = data;
		queue->w_cursor = w_cursor;
		ret = RET_OK;
	}

	return ret;
}

static int32_t queuePushBlock(queue_t *queue, uint8_t *pdata, uint32_t datalen)
{
    uint32_t i;
	uint16_t w_cursor = 0;

    for (i = 0; i < datalen; i++) {
    	w_cursor = (queue->w_cursor + 1) % queue->queue_size;

    	if (w_cursor != queue->r_cursor) {
    		queue->data[queue->w_cursor] = *pdata++;
    		queue->w_cursor = w_cursor;
    	} else {
            return i;
        }
    }

	return i;
}

static void keyUpDownHandle(cli_line_t *line, uint8_t readbyte)
{
    if (DlistIsEmpty(&line->history_list)) {
        return;
    }

    if (line->history_index == NULL) {
        line->history_index = line->history_list.next;
    } else if (readbyte == 'B') {                                              /* UP or Down */
        if (line->history_index->prev != &line->history_list) {
            line->history_index = line->history_index->prev;
        }
    } else if (readbyte == 'A') {
        if (line->history_index->next != &line->history_list) {
            line->history_index = line->history_index->next;
        }
    }

    while (line->line_end > 0) {
        line->api->printf_cb(STR_BACKSPACE);
        line->line_end--;
    }
    line->line_cursor = 0;
    history_t *history = GetContainerOf(line->history_index, history_t, node);
    line->api->printf_cb("%s", history->data);

    strncpy(line->line_buf, history->data, line->cfg.line_buf_size);
    line->line_buf[line->cfg.line_buf_size - 1] = '\0';
    line->line_end = strlen(line->line_buf);
    line->line_cursor = line->line_end;
}

static void keyRightLeftHandle(cli_line_t *line, uint8_t readbyte)
{
    if (readbyte == 'C') {
        if (line->line_cursor < line->line_end) {
            if (line->echo_opt == ECHO_ENABLE) {
                line->api->printf_cb("%c", line->line_buf[line->line_cursor]);
            }
            line->line_cursor++;
        }
    } else if (readbyte == 'D') {
        if (line->line_cursor) {
            if (line->echo_opt == ECHO_ENABLE) {
                line->api->printf_cb("\b");
            }
            line->line_cursor--;
        }
    }
}

static void saveHistory(cli_line_t *line)
{
    line->history_index = NULL;
   if ((line->echo_opt == ECHO_DISABLE) || (line->line_end <= 0)) {
        return;
    }

    history_t *history = NULL;

    /* Query whether the command exists in the history command,
       if so, move to the head of the list */
    DlistForEachEntry(&line->history_list, history, history_t, node)
    {
        if (strcmp(history->data, line->line_buf) == 0) {
            DlistDel(&history->node);
            DlistAdd(&history->node, &line->history_list);
            return;
        }
    }

    /* Number of historical commands >= maximum value, >= delete list tail command */
    uint8_t history_num = DlistEntryNumber(&line->history_list);

    if (history_num >= line->cfg.history_max) {
        history_t *history = DlistLastEntry(&line->history_list, history_t, node);
        DlistDel(&history->node);
        line->api->free_cb(history);
    }

    history = (history_t *)line->api->malloc_cb(sizeof(history_t) + line->line_end + 1);
    if (!history) {
        return;
    }

    strncpy(history->data, line->line_buf, line->line_end + 1);
    line->line_buf[line->line_end] = '\0';

    DlistAdd(&history->node, &line->history_list);
}

static void keyEnterHandle(cli_line_t *line, uint8_t readbyte)
{
    line->api->printf_cb(STR_ENTER);

    /* save history command */
    saveHistory(line);

    line->cfg.handle_cb(line->cfg.ctx, line->line_buf);

    memset(line->line_buf, '\0', line->cfg.line_buf_size);
    line->line_end = 0;
    line->line_cursor = 0;
    line->api->printf_cb("%s", line->prompt);
}

static void keyBackspaceHandle(cli_line_t *line, uint8_t readbyte)
{
    if (line->line_cursor > 0) {

        line->line_cursor--;
        line->line_end--;

        if (line->line_end > line->line_cursor) {

            memmove(&line->line_buf[line->line_cursor],
                    &line->line_buf[line->line_cursor + 1],
                    line->line_end - line->line_cursor);
            line->line_buf[line->line_end] = '\0';

            if (line->echo_opt == ECHO_ENABLE) {
                line->api->printf_cb("\b%s  \b", &line->line_buf[line->line_cursor]);
                /* Move the cursor to the origin position */
                for (int32_t i = line->line_cursor; i <= line->line_end; i++) {
                    line->api->printf_cb("\b");
                }
            }
        } else {
            if (line->echo_opt == ECHO_ENABLE) {
                line->api->printf_cb(STR_BACKSPACE);
            }
            line->line_buf[line->line_end] = '\0';
        }
    }
}

static void keyNormalCharacterHandle(cli_line_t *line, uint8_t readbyte)
{
        if (line->line_cursor < line->line_end) {

            memmove(&line->line_buf[line->line_cursor + 1],
                    &line->line_buf[line->line_cursor],
                    line->line_end - line->line_cursor);

            line->line_buf[line->line_cursor] = readbyte;

            if (line->echo_opt == ECHO_ENABLE) {
                line->api->printf_cb("%s", &line->line_buf[line->line_cursor]);
                /* Move the cursor to new position */
                for (int32_t i = line->line_cursor; i < line->line_end; i++) {
                    line->api->printf_cb("\b");
                }
            }

        } else {
            if (line->echo_opt == ECHO_ENABLE) {
                line->api->printf_cb("%c", readbyte);
            }

            line->line_buf[line->line_end] = readbyte;
        }

        line->line_end++;
        line->line_cursor++;
}

static void lineParse(cli_line_t *line, uint8_t readbyte)
{
    if (((line->key_enter == KEY_ENTER) && (readbyte == KEY_NEWLINE)) ||
        ((line->key_enter == KEY_NEWLINE) && (readbyte == KEY_ENTER))) {
        return;
    }

    if (readbyte == KEY_ESC) {
        line->key_combo = KEY_ESC;
    }
    else if (line->key_combo == KEY_ESC) {
        if (readbyte == KEY_LBRKT) {
            line->key_combo = KEY_LBRKT;
        } else {
            line->key_combo = 0;
        }
    }
    else if (line->key_combo == KEY_LBRKT) {
        /* exit combo */
        line->key_combo = 0;

        /* up key down key */
        if (readbyte == 'A' || readbyte == 'B') {
            if (line->echo_opt == ECHO_DISABLE) {
                return;
            }
            keyUpDownHandle(line, readbyte);
        }
        /* right key left key */
        else if (readbyte == 'C' || readbyte == 'D') {
            keyRightLeftHandle(line, readbyte);
        }
    }
    else if (readbyte == KEY_ENTER || readbyte == KEY_NEWLINE) {
        line->key_enter = readbyte;
        keyEnterHandle(line, readbyte);
    }
    else if (readbyte == KEY_BACKSPACE || readbyte == CTRL_H) {
        keyBackspaceHandle(line, readbyte);
    }
    else {
        /* cache input characters */
        line->key_enter = 0;

        if (line->line_end < (line->cfg.line_buf_size - 1)) {
            keyNormalCharacterHandle(line, readbyte);
        }  else if (line->line_end == (line->cfg.line_buf_size - 1)) {
            line->line_buf[line->line_end] = '\0';
            line->api->printf_cb("%s", STR_ENTER"Size limit exceeded!");
        }
    }
}

cli_line_t *CliLineCreate(const cli_api_t *api, cli_line_cfg_t *cfg)
{
    if (!api || !api->malloc_cb || !api->free_cb ||
        !api->printf_cb  || !cfg || !cfg->handle_cb) {
        return NULL;
    }

	cli_line_t *line = (cli_line_t *)api->malloc_cb(sizeof(cli_line_t) +
	                                                cfg->queue_size +
	                                                cfg->line_buf_size);

    if (line == NULL) {
        return NULL;
    }

    memset(line, 0, sizeof(cli_line_t) + cfg->queue_size + cfg->line_buf_size);
    line->prompt = DEFAULT_PROMPT;
    line->echo_opt = ECHO_ENABLE;
    line->api = api;
    line->cfg = *cfg;
    line->queue.queue_size = cfg->queue_size;
    line->line_buf = (char *)&line->data[cfg->queue_size];
    DlistInit(&line->history_list);

    return line;
}

void CliLineDestroy(cli_line_t *line)
{
    if (line == NULL) {
        return;
    }

    history_t *history = NULL;
    while (!DlistIsEmpty(&line->history_list)) {
        history = DlistFirstEntry(&line->history_list, history_t, node);
        DlistDel(&history->node);
        line->api->free_cb(history);
    }

    line->api->free_cb(line);
}

void CliLineSetPrompt(cli_line_t *line, const char *prompt)
{
    if (line == NULL) {
        return;
    }

    line->prompt = prompt;
}

void CliLineSetEcho(cli_line_t *line, line_echo_t echo)
{
    if (line == NULL) {
        return;
    }

    line->echo_opt = echo;
}

void CliLineInputChar(cli_line_t *line, uint8_t data)
{
    if (line == NULL) {
        return;
    }

    queuePush(&line->queue, data);
}

void CliLineInputBlock(cli_line_t *line, uint8_t *pdata, uint32_t datalen)
{
    if (line == NULL) {
        return;
    }

    queuePushBlock(&line->queue, pdata, datalen);
}

void CliLineTick(cli_line_t *line)
{
    if (line == NULL) {
        return;
    }

    while (1) {
        int16_t readbyte = queuePop(&line->queue);

        if (readbyte >= 0) {
            lineParse(line, readbyte);
        } else {
            return;
        }
    }
}

