/**
 * @brief cmd cli
 * @file cmd_cli.c
 * @version 1.0
 * @author suyoujiang
 * @date 2020-3-18 9:42:13
 * @par Copyright:
 * Copyright (c) LwCLI 2018-2020. All rights reserved.
 *
 * @history
 * 1.Date        : 2020-3-18 9:42:13
 *   Author      : suyoujiang
 *   Modification: Created file
 */

#include "cli_line.h"
#include "cli_command.h"
#include "cli_shell.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define XASSERT(p)  //assert(p)

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/
typedef enum  login_status {
    LOGIN_USERNAME,
    LOGIN_PASSWORD,
    LOGIN_SUCCESS,
} login_status_t;

typedef struct cli_shell {
    cli_line_t    *obj_line;
    cli_command_t *obj_cmd;
    shell_printf   printf_cb;

#if LOGIN_ENABLE > 0
    login_status_t login;
    const char *username;
    const char *password;
    char        temp_user[LINE_LEN_MAX];
#endif
} cli_shell_t;

static cli_shell_t sg_shell;

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

static void cmdHandle(void *ctx, char *line)
{
#if LOGIN_ENABLE > 0
    switch (sg_shell.login) {
        case LOGIN_USERNAME:
            if (strlen(line) <= 0) {
                break;
            }

            sg_shell.login = LOGIN_PASSWORD;
            strncpy(sg_shell.temp_user, line, sizeof(sg_shell.temp_user)-1);

            CliLineSetEcho(sg_shell.obj_line, ECHO_DISABLE);
            CliLineSetPrompt(sg_shell.obj_line, PROMPT_PASSWD);
            break;

        case LOGIN_PASSWORD:
            if (!strcmp(sg_shell.temp_user, sg_shell.username) &&
                !strcmp(line, sg_shell.password)) {
                sg_shell.login = LOGIN_SUCCESS;
                CliLineSetPrompt(sg_shell.obj_line, PROMPT_CMD);
                sg_shell.printf_cb("Login success\r\n");
            } else {
                sg_shell.login = LOGIN_USERNAME;
                CliLineSetPrompt(sg_shell.obj_line, PROMPT_LOGIN);
                sg_shell.printf_cb("Login incorrect\r\n");
            }
            CliLineSetEcho(sg_shell.obj_line, ECHO_ENABLE);
            break;

        case LOGIN_SUCCESS:
            if (strlen(line) <= 0) {
                break;
            }

            CliCmdHandle(sg_shell.obj_cmd, line);
            break;
        default:
            break;
    }
#else
    if (strlen(line) > 0) {
        CliCmdHandle(sg_shell.obj_cmd, line);
    }
#endif
}

#if LOGIN_ENABLE > 0
static void cmdLogout(int argc, char **argv)
{
    sg_shell.printf_cb("Logout\r\n");
    sg_shell.login = LOGIN_USERNAME;
    CliLineSetPrompt(sg_shell.obj_line, PROMPT_LOGIN);
}
#endif

void CliShellInit(int32_t cmd_max, shell_printf printf)
{
    memset(&sg_shell, 0, sizeof(sg_shell));

    sg_shell.obj_cmd = CliCmdCreate(cmd_max, printf);
    sg_shell.obj_line = CliLineCreate(NULL, printf, cmdHandle, sg_shell.obj_cmd);
    XASSERT(sg_shell.obj_cmd != NULL && sg_shell.obj_line != NULL);

    sg_shell.printf_cb = printf;

#if LOGIN_ENABLE > 0
    sg_shell.username = CLI_USERNAME;
    sg_shell.password = CLI_PASSWORD;

    CliLineSetPrompt(sg_shell.obj_line, PROMPT_LOGIN);
    sg_shell.printf_cb(PROMPT_LOGIN);
    CliCmdRegister(sg_shell.obj_cmd, "logout", cmdLogout, "log out");
#else
    CliLineSetPrompt(sg_shell.obj_line, PROMPT_CMD);
    sg_shell.printf_cb(PROMPT_CMD);
#endif
}

int32_t CliShellRegister(char *cmd,
                             void (*function)(int, char **),
                             char *describe)
{
    return CliCmdRegister(sg_shell.obj_cmd, cmd, function, describe);
}

void CliShellTick(void)
{
    CliLineTick(sg_shell.obj_line);
}

void CliShellInputBlock(uint8_t *pdata, uint32_t datalen)
{
    CliLineInputBlock(sg_shell.obj_line, pdata, datalen);
}

void CliShellInputChar(uint8_t data)
{
    CliLineInputChar(sg_shell.obj_line, data);
}

