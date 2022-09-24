/**
 * @brief cli command module
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
 * macros                                       *
 *----------------------------------------------*/
#define CMD_ARGC_MAX    10

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/
typedef struct cli_cmd_list {
	struct cli_cmd_list *next;
    char *command;
    void (*function)(int, char **);
    char *describe;
} cli_cmd_list_t;

struct cli_cmd {
    const cli_api_t *api;
#if CLI_MCD_EN_LIST > 0
    uint16_t         list_num;
    cli_cmd_list_t  *list_head;
#else
    uint16_t         entry_size;
    uint16_t         entry_num;
    cli_cmd_entry_t **entry;
#endif
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
#if CLI_MCD_EN_LIST > 0
	cli_cmd->api->printf_cb("Command list[%d]:\r\n", cli_cmd->list_num);
	cli_cmd->api->printf_cb("==============================\r\n");

	cli_cmd_list_t *target = cli_cmd->list_head;
	while (target) {
      //cli_cmd->api->printf_cb("  [%12s] %s\r\n", target->command, target->describe);
		cli_cmd->api->printf_cb("  [%-12s] %s\r\n", target->command, target->describe);

		target = target->next;
	}
#else
	cli_cmd->api->printf_cb("Command list[%d]:\r\n", cli_cmd->entry_num);
	cli_cmd->api->printf_cb("==============================\r\n");

    for (int16_t i = 0; i < cli_cmd->entry_num; i++) {
		cli_cmd->api->printf_cb("  [%-12s] %s\r\n", cli_cmd->entry[i]->command, cli_cmd->entry[i]->describe);
    }
#endif
}

cli_cmd_t *CliCmdCreate(const cli_api_t *api)
{
    if (!api || !api->malloc_cb || !api->free_cb || !api->printf_cb)
    {
        return NULL;
    }

    cli_cmd_t *cli_cmd = (cli_cmd_t *)api->malloc_cb(sizeof(cli_cmd_t));
    if (!cli_cmd) {
        return NULL;
    }

    memset(cli_cmd, 0, sizeof(cli_cmd_t));
    cli_cmd->api = api;

    return cli_cmd;
}

void CliCmdDestroy(cli_cmd_t *cli_cmd)
{
    if (!cli_cmd) {
        return;
    }

#if CLI_MCD_EN_LIST > 0
    cli_cmd_list_t *target = cli_cmd->list_head;

	while (target) {
        cli_cmd_list_t *temp = target;
		target = target->next;
		cli_cmd->api->free_cb(temp);
	}
#else
    if (cli_cmd->entry) {
        cli_cmd->api->free_cb(cli_cmd->entry);
    }
#endif

    memset(cli_cmd, 0, sizeof(cli_cmd_t));
    cli_cmd->api->free_cb(cli_cmd);
}
#if CLI_MCD_EN_LIST > 0
int32_t CliCmdRegister(cli_cmd_t *cli_cmd,
                       char *cmd,
                       void (*function)(int, char **),
                       char *describe)
{
    if (!cli_cmd || !cmd || !function) {
        return RET_ERROR;
    }

    cli_cmd_list_t *target = cli_cmd->list_head;

	while (target) {
        if (!strcmp(cmd, target->command)) {
            target->function = function;
            target->describe = describe;
            return RET_OK;
        }
		target = target->next;
	}

    cli_cmd_list_t *node = (cli_cmd_list_t *)cli_cmd->api->malloc_cb(sizeof(cli_cmd_list_t));
    if (node == NULL) {
        return RET_ERROR;
    }

    node->command  = cmd;
    node->function = function;
    node->describe = describe;
    node->next     = NULL;

    if (!cli_cmd->list_head) {
        cli_cmd->list_head = node;
    } else {
        cli_cmd_list_t *tail = cli_cmd->list_head;
    	while (tail->next) {
    		tail = tail->next;
    	}
        tail->next = node;
    }

    cli_cmd->list_num++;

    return RET_OK;
}
#else
int32_t CliCmdTableRegister(cli_cmd_t *cli_cmd, const cli_cmd_entry_t *entry, int16_t num)
{
    if (!cli_cmd || !entry || num <= 0) {
        return RET_ERROR;
    }

    int16_t num_max = cli_cmd->entry_num + num;
    cli_cmd_entry_t **temp;

    temp = (cli_cmd_entry_t **)cli_cmd->api->malloc_cb(num_max * sizeof(cli_cmd_entry_t *));
    if (!temp) {
        return RET_ERROR;
    }

    if (cli_cmd->entry) {
        memcpy(temp, cli_cmd->entry, cli_cmd->entry_num * sizeof(cli_cmd_entry_t *));
        cli_cmd->api->free_cb(cli_cmd->entry);
    }

    cli_cmd->entry = temp;

    for (int16_t i = 0; i < num; i++, cli_cmd->entry_num++) {
        cli_cmd->entry[cli_cmd->entry_num] = (cli_cmd_entry_t *)&entry[i];
    }

    return RET_OK;
}
#endif

int32_t CliCmdUnregister(cli_cmd_t *cli_cmd, char *cmd)
{
    int32_t ret = RET_ERROR;

    if (!cli_cmd || !cmd) {
        return RET_ERROR;
    }

#if CLI_MCD_EN_LIST > 0
	cli_cmd_list_t **curr;

	for (curr = &cli_cmd->list_head; *curr; ) {
		cli_cmd_list_t *entry = *curr;

		if (!strcmp(cmd, entry->command)) {
			*curr = entry->next;
			cli_cmd->api->free_cb(entry);
            cli_cmd->list_num--;
            ret = RET_OK;
            break;
		} else {
			curr = &entry->next;
        }
	}
#else
    for (int16_t i = 0; i < cli_cmd->entry_num; i++) {
		if (!strcmp(cmd, cli_cmd->entry[i]->command)) {
            memmove(&cli_cmd->entry[i], &cli_cmd->entry[i+1],
                    (cli_cmd->entry_num - i - 1)*sizeof(cli_cmd_entry_t *));

            cli_cmd->entry_num--;
            ret = RET_OK;
            break;
		}
    }
#endif

    return ret;
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
#if CLI_MCD_EN_LIST > 0
    	cli_cmd_list_t *target = cli_cmd->list_head;
    	while (target) {
            if (strcmp(argv[0], target->command) == 0) {
                target->function(argc, argv);
                return;
            }
    		target = target->next;
    	}
#else
        for (int16_t i = 0; i < cli_cmd->entry_num; i++) {
    		if (!strcmp(argv[0], cli_cmd->entry[i]->command)) {
                cli_cmd->entry[i]->function(argc, argv);
                return;
    		}
        }
#endif

        cli_cmd->api->printf_cb("Unknown command:%s\r\n", argv[0]);
    } else {
        cli_cmd->api->printf_cb("Unknown data!!!\r\n");
    }
}

