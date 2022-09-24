
/**
 * @brief cli shell module
 * @file cli_shell.c
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
#include "cli_cmd.h"
#include "cli_shell.h"

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define PROMPT_LOGIN    "Login:"
#define PROMPT_PASSWD   "Password:"

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/
typedef enum  login_status {
    LOGIN_USERNAME,
    LOGIN_PASSWORD,
    LOGIN_SUCCESS,
} login_status_t;

typedef struct cli_shell {
    const cli_api_t *api;

    cli_cmd_t  *obj_cmd;
    cli_line_t *obj_line;

    login_status_t login;
    const char *username;
    const char *password;
    char       *temp_user;
    const char *prompt;
} cli_shell_t;

static cli_shell_t s_shell = {0};

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

static void cmdHandle(void *ctx, char *line)
{
    switch (s_shell.login) {
    case LOGIN_USERNAME:
        s_shell.temp_user = (char *)s_shell.api->malloc_cb(strlen(line) + 1);
        if (!s_shell.temp_user) {
            break;
        }

        strcpy(s_shell.temp_user, line);
        CliLineSetEcho(s_shell.obj_line, ECHO_DISABLE);
        CliLineSetPrompt(s_shell.obj_line, PROMPT_PASSWD);
        s_shell.login = LOGIN_PASSWORD;
        break;

    case LOGIN_PASSWORD:
        if (!strcmp(s_shell.temp_user, s_shell.username) &&
            !strcmp(line, s_shell.password)) {
            s_shell.login = LOGIN_SUCCESS;
            CliLineSetPrompt(s_shell.obj_line, s_shell.prompt);
            s_shell.api->printf_cb("Login success\r\n");
        } else {
            s_shell.login = LOGIN_USERNAME;
            CliLineSetPrompt(s_shell.obj_line, PROMPT_LOGIN);
            s_shell.api->printf_cb("Login incorrect\r\n");
        }

        s_shell.api->free_cb(s_shell.temp_user);
        s_shell.temp_user = NULL;
        CliLineSetEcho(s_shell.obj_line, ECHO_ENABLE);
        break;

    case LOGIN_SUCCESS:
        CliCmdHandle(s_shell.obj_cmd, line);
        break;

    default:
        break;
    }
}

static void cmdLogout(int argc, char **argv)
{
    s_shell.api->printf_cb("Logout\r\n");
    s_shell.login = LOGIN_USERNAME;
    CliLineSetPrompt(s_shell.obj_line, PROMPT_LOGIN);
}

int32_t CliShellInit(const cli_api_t *api, cli_shell_cfg_t *cfg)
{
    if (s_shell.api) {
        return RET_ERROR;
    }

    if (!api || !api->malloc_cb || !api->free_cb ||
        !api->printf_cb || !cfg) {
        return RET_ERROR;
    }

    memset(&s_shell, 0, sizeof(cli_shell_t));

    s_shell.api = api;
    s_shell.username = cfg->username;
    s_shell.password = cfg->password;
    if (cfg->prompt) {
        s_shell.prompt = cfg->prompt;
    } else {
        s_shell.prompt = "hello> ";
    }

    cli_line_cfg_t line_cfg = {0};
    line_cfg.queue_size    = cfg->line_queue_size;
    line_cfg.line_buf_size = cfg->line_buf_size;
    line_cfg.history_max   = cfg->line_history_num;
    line_cfg.handle_cb     = cmdHandle;

    s_shell.obj_line = CliLineCreate(api, &line_cfg);
    if (!s_shell.obj_line) {
        return RET_ERROR;
    }

    s_shell.obj_cmd = CliCmdCreate(api);
    if (!s_shell.obj_cmd) {
        CliLineDestroy(s_shell.obj_line);
        return RET_ERROR;
    }

    if (s_shell.username && s_shell.password) {
        s_shell.login = LOGIN_USERNAME;
        CliLineSetPrompt(s_shell.obj_line, PROMPT_LOGIN);
        s_shell.api->printf_cb(PROMPT_LOGIN);
#if CLI_MCD_EN_LIST > 0
        CliCmdRegister(s_shell.obj_cmd, "logout", cmdLogout, "log out");
#else
        static const cli_cmd_entry_t cmd_logout = {
            "logout", cmdLogout, "log out"
        };

        CliCmdTableRegister(s_shell.obj_cmd, &cmd_logout, 1);
#endif
    } else {
        s_shell.login = LOGIN_SUCCESS;
        CliLineSetPrompt(s_shell.obj_line, s_shell.prompt);
        s_shell.api->printf_cb(s_shell.prompt);
    }

    return RET_OK;
}

void CliShellDeinit(void)
{
    CliLineDestroy(s_shell.obj_line);
    CliCmdDestroy(s_shell.obj_cmd);
    memset(&s_shell, 0, sizeof(cli_shell_t));
}

#if CLI_MCD_EN_LIST > 0
int32_t CliShellRegister(char *cmd,
                         void (*function)(int, char **),
                         char *describe)
{
    return CliCmdRegister(s_shell.obj_cmd, cmd, function, describe);
}
#else
int32_t CliShellTableRegister(const cli_cmd_entry_t *entry, int16_t num)
{
    return CliCmdTableRegister(s_shell.obj_cmd, entry, num);
}
#endif

int32_t CliShellUnegister(char *cmd)
{
    return CliCmdUnregister(s_shell.obj_cmd, cmd);
}

void CliShellTick(void)
{
    CliLineTick(s_shell.obj_line);
}

void CliShellInputBlock(uint8_t *pdata, uint32_t datalen)
{
    CliLineInputBlock(s_shell.obj_line, pdata, datalen);
}

void CliShellInputChar(uint8_t data)
{
    CliLineInputChar(s_shell.obj_line, data);
}

