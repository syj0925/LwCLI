/**
 * @brief cli command
 * @file cli_cmd.c
 * @version 1.0
 * @author Su YouJiang
 * @date 2020-3-17 17:42:5
 * @par Copyright:
 * Copyright (c) LwCLI 2018-2020. All rights reserved.
 *
 * @history
 * 1.Date        : 2020-3-17 17:42:5
 *   Author      : Su YouJiang
 *   Modification: Created file
 */

#include "cli_cmd.h"

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/
struct cli_cmd {
    const cli_inf_t *inf;
    uint16_t         cmd_max;
    uint16_t         cmd_num;
    cli_cmd_entry_t *list_head;
};

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
static int32_t cmdParse(char *buf, char **argv)
{
    int32_t argc = 0;

    memset(argv, 0, sizeof(*argv)*CMD_ARGC_MAX);
    while ((argc < CMD_ARGC_MAX) && (*buf != '\0')) {
        argv[argc] = buf;
        argc++;
        buf++;

        while ((*buf != ' ') && (*buf != '\0')) {
            buf++;
        }

        while (*buf == ' ') {
            *buf = '\0';
            buf++;
        }
    }

    return argc;
}

static void dumpHelpInfo(cli_cmd_t *cli_cmd)
{
	cli_cmd->inf->printf_cb("Command list[%d]:\r\n", cli_cmd->cmd_num);
	cli_cmd->inf->printf_cb("==============================\r\n");

	cli_cmd_entry_t *target = cli_cmd->list_head;
	while (target) {
      //cli_cmd->inf->printf_cb("  [%12s] %s\r\n", target->command, target->describe);  /* 左对齐，12位长度，不够补空格 */
		cli_cmd->inf->printf_cb("  [%-12s] %s\r\n", target->command, target->describe); /* 右对齐，12位长度，不够补空格 */

		target = target->next;
	}
}

cli_cmd_t *CliCmdCreate(const cli_inf_t *inf, int16_t cmd_max)
{
    if (!inf || !inf->malloc_cb || !inf->free_cb || !inf->printf_cb)
    {
        return NULL;
    }

    cli_cmd_t *cli_cmd = (cli_cmd_t *)inf->malloc_cb(sizeof(cli_cmd_t));
    if (!cli_cmd) {
        return NULL;
    }

    memset(cli_cmd, 0, sizeof(cli_cmd_t));
    cli_cmd->inf = inf;
    cli_cmd->cmd_max = cmd_max;

    return cli_cmd;
}

void CliCmdDestroy(cli_cmd_t *cli_cmd)
{
    if (!cli_cmd) {
        return;
    }

    cli_cmd_entry_t *target = cli_cmd->list_head;

	while (target) {
        cli_cmd_entry_t *temp = target;
		target = target->next;
		cli_cmd->inf->free_cb(temp);
	}

    memset(cli_cmd, 0, sizeof(cli_cmd_t));
    cli_cmd->inf->free_cb(cli_cmd);
}

int32_t CliCmdRegister(cli_cmd_t *cli_cmd,
                       char *cmd,
                       void (*function)(int, char **),
                       char *describe)
{
    if (!cli_cmd || !cmd || !function) {
        return RET_ERROR;
    }

    cli_cmd_entry_t *target = cli_cmd->list_head;

	while (target) {
        if (!strcmp(cmd, target->command)) {
            target->function = function;
            target->describe = describe;
            return RET_OK;
        }
		target = target->next;
	}

    if (cli_cmd->cmd_num >= cli_cmd->cmd_max) {
        return RET_ERROR;
    }

    cli_cmd_entry_t *node = (cli_cmd_entry_t *)cli_cmd->inf->malloc_cb(sizeof(cli_cmd_entry_t));
    if (node == NULL) {
        return RET_ERROR;
    }

    node->command  = cmd;
    node->function = function;
    node->describe = describe;
    node->next = cli_cmd->list_head;
    cli_cmd->list_head = node;
    cli_cmd->cmd_num++;

    return RET_OK;
}

int32_t CliCmdUnregister(cli_cmd_t *cli_cmd, char *cmd)
{
    if (!cli_cmd || !cmd) {
        return RET_ERROR;
    }

	cli_cmd_entry_t **curr;

	for (curr = &cli_cmd->list_head; *curr; ) {
		cli_cmd_entry_t *entry = *curr;

		if (!strcmp(cmd, entry->command)) {
			*curr = entry->next;
			cli_cmd->inf->free_cb(entry);
            cli_cmd->cmd_num--;
            return RET_OK;
		} else {
			curr = &entry->next;
        }
	}

    return RET_ERROR;
}

void CliCmdHandle(cli_cmd_t *cli_cmd, char *cmdline)
{
    int32_t argc;
    char *argv[CMD_ARGC_MAX];

    if (!cli_cmd || !cmdline) {
        return;
    }

    if ((argc = cmdParse(cmdline, argv)) > 0) {

        if (strcmp(argv[0], "help") == 0 ||
            strcmp(argv[0], "HELP") == 0) {
            dumpHelpInfo(cli_cmd);
            return;
        }

    	cli_cmd_entry_t *target = cli_cmd->list_head;
    	while (target) {
            if (strcmp(argv[0], target->command) == 0) {
                target->function(argc, argv);
                return;
            }
    		target = target->next;
    	}

        cli_cmd->inf->printf_cb("Unknown command:%s\r\n", argv[0]);
    } else {
        cli_cmd->inf->printf_cb("Unknown data!!!\r\n");
    }
}

