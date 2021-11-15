/**
 * @brief cli command
 * @file cli_command.c
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

#include "cli_command.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/


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

struct cli_command {
    command_printf  printf_cb;
    uint16_t        cmd_max;
    uint16_t        cmd_num;
    cli_cmd_entry_t cmd_table[0];
};

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

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

static void dumpHelpInfo(cli_command_t *cli_cmd)
{
	int32_t i;

	cli_cmd->printf_cb("Command list[%d]:\r\n", cli_cmd->cmd_num);
	cli_cmd->printf_cb("==============================\r\n");

	for (i = 0; i < cli_cmd->cmd_num; i++) {
      //cli_cmd->printf_cb("  [%12s] %s\r\n", cli_cmd->cmd_table[i].command, cli_cmd->cmd_table[i].describe);  /* 左对齐，12位长度，不够补空格 */
		cli_cmd->printf_cb("  [%-12s] %s\r\n", cli_cmd->cmd_table[i].command, cli_cmd->cmd_table[i].describe); /* 右对齐，12位长度，不够补空格 */
    }
}

cli_command_t *CliCmdCreate(int32_t cmd_max, command_printf printf)
{
    cli_command_t *cli_cmd = (cli_command_t *)malloc(sizeof(cli_command_t) + cmd_max * sizeof(cli_cmd_entry_t));
    if (cli_cmd == NULL) {
        return NULL;
    }

    memset(cli_cmd, 0, sizeof(cli_command_t));
    cli_cmd->cmd_max   = cmd_max;
    cli_cmd->printf_cb = printf;

    return cli_cmd;
}

void CliCmdDestroy(cli_command_t *cli_cmd)
{
    free(cli_cmd);
}

int32_t CliCmdRegister(cli_command_t *cli_cmd,
                             char *cmd,
                             void (*function)(int, char **),
                             char *describe)
{
    if (cli_cmd == NULL || cli_cmd->cmd_num >= cli_cmd->cmd_max) {
        return -1;
    }

    cli_cmd->cmd_table[cli_cmd->cmd_num].command = cmd;
    cli_cmd->cmd_table[cli_cmd->cmd_num].function = function;
    cli_cmd->cmd_table[cli_cmd->cmd_num].describe = describe;
    cli_cmd->cmd_num++;

    return 0;
}

void CliCmdHandle(cli_command_t *cli_cmd, char *cmdline)
{
    int32_t i;
    int32_t argc;
    char   *argv[CMD_ARGC_MAX];

    if (cli_cmd == NULL) {
        return;
    }

    if ((argc = cmdParse(cmdline, argv)) > 0) {

        if (strcmp(argv[0], "help") == 0 ||
            strcmp(argv[0], "HELP") == 0) {
            dumpHelpInfo(cli_cmd);
            return;
        }

        for (i = 0; i < cli_cmd->cmd_num; i++) {

            if (strcmp(argv[0], cli_cmd->cmd_table[i].command) == 0) {
                cli_cmd->cmd_table[i].function(argc, argv);
                return;
            }
        }

        cli_cmd->printf_cb("Unknown command:%s\r\n", argv[0]);
    } else {
        cli_cmd->printf_cb("Unknown data!!!\r\n");
    }
}

