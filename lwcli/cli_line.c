/**
 * @brief cli line
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

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

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

enum {
    RET_OK = 0,
    RET_ERROR = -1,
};

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/
typedef struct queue {
	uint16_t   r_cursor;
	uint16_t   w_cursor;
	uint8_t    data[LINE_QUEUE_BUF_SIZE];
} queue_t;

struct cli_line {
    const char *prompt;                  /* ????????? */
    uint8_t     echo_opt;                /* ???????????? */
    uint8_t     key_enter;               /* ??????????????? */
    uint8_t     key_combo;               /* ??????????????? */
    uint8_t     history_index;           /* ???????????????????????? */
    uint8_t     history_count;           /* ??????????????? */
    uint8_t     line_cursor;             /* ???????????? */
    uint8_t     line_position;           /* ??????????????? */
    char        history[LINE_HISTORY_MAX][LINE_LEN_MAX];
    char        cmdline[LINE_LEN_MAX];   /* ??????????????? */
    line_printf printf_cb;               /* ???????????????????????? */
    line_handle handle_cb;               /* ??????????????????????????? */
    void       *ctx;
    queue_t     queue_r;                 /* ?????????????????? */
};

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

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
		queue->r_cursor = (queue->r_cursor + 1) % sizeof(queue->data);
	}

	return ret;
}

static int32_t queuePush(queue_t *queue, uint8_t data)
{
	int8_t ret = RET_ERROR;
	uint16_t w_cursor = 0;

	w_cursor = (queue->w_cursor + 1) % sizeof(queue->data);
	
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
    	w_cursor = (queue->w_cursor + 1) % sizeof(queue->data);
    	
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
    if (line->history_count <= 0) {
        return;
    }

    if (line->history_index == 0xFF) {
        line->history_index = 0;
    } else if (readbyte == 'A') {                                              /* UP or Down */
        if (line->history_index <= 0) {
            line->history_index = line->history_count - 1;
        } else {
            line->history_index--;
        }
    } else if (readbyte == 'B') {
        if (++line->history_index >= line->history_count) {
            line->history_index = 0;
        }
    }
    
    while (line->line_position > 0) {
        line->printf_cb(STR_BACKSPACE);
        line->line_position--;
    }
    line->line_cursor = 0;
    line->printf_cb("%s", line->history[line->history_index]);
    
    strncpy(line->cmdline, line->history[line->history_index], LINE_LEN_MAX);
    line->line_position = strlen(line->cmdline);
    line->line_cursor = line->line_position;
}

static void keyRightLeftHandle(cli_line_t *line, uint8_t readbyte)
{
    if (readbyte == 'C') {
        if (line->line_cursor < line->line_position) {
            if (line->echo_opt == ECHO_ENABLE) {
                line->printf_cb("%c", line->cmdline[line->line_cursor]);
            }
            line->line_cursor++;
        }
    } else if (readbyte == 'D') {
        if (line->line_cursor) {
            if (line->echo_opt == ECHO_ENABLE) {
                line->printf_cb("\b");
            }
            line->line_cursor--;
        }
    }
}

static int32_t checkHistory(cli_line_t *line)
{
    for (int32_t i = 0; i < line->history_count; i++) {
        if (strcmp(line->history[i], line->cmdline) == 0) {
            return i;
        }
    }

    return -1;
}

static void keyEnterHandle(cli_line_t *line, uint8_t readbyte)
{    
    line->printf_cb(STR_ENTER);

    /* save command */
    if ((line->echo_opt == ECHO_ENABLE) && (line->line_position > 0)) {
        int32_t i;
        
        if ((i = checkHistory(line)) >= 0) {
            memmove(line->history[1], line->history[0], i * LINE_LEN_MAX);
        } else {
            if (line->history_count < LINE_HISTORY_MAX) {
                line->history_count++;
            }
            memmove(line->history[1], line->history[0], (line->history_count-1) * LINE_LEN_MAX);
        }

        strncpy(line->history[0], line->cmdline, LINE_LEN_MAX);
        line->history_index = 0xFF;                                            /* FF????????????????????????????????? */
    }

    line->handle_cb(line->ctx, line->cmdline);

    memset(line->cmdline,'\0', sizeof(line->cmdline));
    line->line_position = 0;
    line->line_cursor = 0;
    line->printf_cb("%s", line->prompt);
}

static void keyBackspaceHandle(cli_line_t *line, uint8_t readbyte)
{
    if (line->line_cursor > 0) {
        
        line->line_cursor--;
        line->line_position--;

        if (line->line_position > line->line_cursor) {

            memmove(&line->cmdline[line->line_cursor],
                    &line->cmdline[line->line_cursor + 1],
                    line->line_position - line->line_cursor);
            line->cmdline[line->line_position] = '\0';

            if (line->echo_opt == ECHO_ENABLE) {
                line->printf_cb("\b%s  \b", &line->cmdline[line->line_cursor]);
                /* Move the cursor to the origin position */
                for (int32_t i = line->line_cursor; i <= line->line_position; i++) {
                    line->printf_cb("\b");
                }
            }
        } else {
            if (line->echo_opt == ECHO_ENABLE) {
                line->printf_cb(STR_BACKSPACE);
            }
            line->cmdline[line->line_position] = '\0';
        }
    }
}

static void keyNormalCharacterHandle(cli_line_t *line, uint8_t readbyte)
{
        if (line->line_cursor < line->line_position) {

            memmove(&line->cmdline[line->line_cursor + 1],
                    &line->cmdline[line->line_cursor],
                    line->line_position - line->line_cursor);

            line->cmdline[line->line_cursor] = readbyte;

            if (line->echo_opt == ECHO_ENABLE) {
                line->printf_cb("%s", &line->cmdline[line->line_cursor]);
                /* Move the cursor to new position */
                for (int32_t i = line->line_cursor; i < line->line_position; i++) {
                    line->printf_cb("\b");
                }
            }

        } else {
            if (line->echo_opt == ECHO_ENABLE) {
                line->printf_cb("%c", readbyte);
            }
        
            line->cmdline[line->line_position] = readbyte;
        }

        line->line_position++;
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
        
        if (line->line_position < (LINE_LEN_MAX - 1)) {
            keyNormalCharacterHandle(line, readbyte);
        }  else if (line->line_position == (LINE_LEN_MAX - 1)) {
            line->cmdline[line->line_position] = '\0';
            line->printf_cb("%s", STR_ENTER"Size limit exceeded!");
        }
    }
}

cli_line_t *CliLineCreate(const char *prompt, line_printf printf, line_handle handle, void *ctx)
{
    if (printf == NULL || handle == NULL) {
        return NULL;
    }

	cli_line_t *line = (cli_line_t *)malloc(sizeof(cli_line_t));

    if (line == NULL) {
        return NULL;
    }

    memset(line, 0, sizeof(cli_line_t));
    if (prompt == NULL) {
        line->prompt = DEFAULT_PROMPT;
    } else {
        line->prompt = prompt;
    }
    
    line->echo_opt  = ECHO_ENABLE;
    line->printf_cb = printf;
    line->handle_cb = handle;
    line->ctx       = ctx;

    return line;
}

void CliLineDestory(cli_line_t *line)
{
    free(line);
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

    queuePush(&line->queue_r, data);
}

void CliLineInputBlock(cli_line_t *line, uint8_t *pdata, uint32_t datalen)
{
    if (line == NULL) {
        return;
    }

    queuePushBlock(&line->queue_r, pdata, datalen);
}

void CliLineTick(cli_line_t *line)
{
    if (line == NULL) {
        return;
    }

    while (1) {
        int16_t readbyte = queuePop(&line->queue_r);

        if (readbyte >= 0) {
            lineParse(line, readbyte);
        } else {
            return;
        }
    }
}

