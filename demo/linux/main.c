#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>


#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include "cli_shell.h"

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

static int rawmode = 0; /* For atexit() function to check if restore is needed*/
static struct termios orig_termios; /* In order to restore at exit.*/
static int atexit_registered = 0; /* Register atexit just 1 time. */

static void disableRawMode(int fd) {
    /* Don't even check the return value as it's too late. */
    if (rawmode && tcsetattr(fd,TCSAFLUSH,&orig_termios) != -1)
        rawmode = 0;
}

/* At exit we'll try to fix the terminal to the initial conditions. */
static void linenoiseAtExit(void) {
    disableRawMode(STDIN_FILENO);
}

/* Raw mode: 1960 magic shit. */
static int enableRawMode(int fd) {
    struct termios raw;

    if (!isatty(STDIN_FILENO)) goto fatal;
    if (!atexit_registered) {
        atexit(linenoiseAtExit);
        atexit_registered = 1;
    }
    if (tcgetattr(fd,&orig_termios) == -1) goto fatal;

    raw = orig_termios;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - choing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer.
     * We want read to return every single byte, without timeout. */
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(fd,TCSAFLUSH,&raw) < 0) goto fatal;
    rawmode = 1;
    return 0;

fatal:
    errno = ENOTTY;
    return -1;
}

static void cmdReboot(int argc, char **argv)
{
    printf("reboot:ok\r\n");
}

static void cmdReset(int argc, char **argv)
{
    printf("reset:ok\r\n");
}

static void cmdFactory(int argc, char **argv)
{
    printf("factory:ok\r\n");
}

static void cmdExit(int argc, char **argv)
{
    printf("exit\r\n");
    disableRawMode(STDIN_FILENO);
    exit(0);
}
static void cmdVersion(int argc, char **argv)
{
    printf("app_ver:%s\r\n", "1.0");
}

int Xprintf(const char *fmt, ...)
{
    int ret = 0;
    va_list ap;

    va_start(ap, fmt);
    ret = vprintf(fmt, ap);
    va_end(ap);

    fflush(NULL);

    return ret;
}

void CliCmdInit(void)
{
    CliShellInit(40, Xprintf);

    CliShellRegister("reboot",    cmdReboot,    "reboot system");
    CliShellRegister("reset",     cmdReset,     "clear user data and reboot"); 
    CliShellRegister("factory",   cmdFactory,   "factory reset and reboot");
    CliShellRegister("exit",      cmdExit,      "exit the app");   /* 结束本应用，适用linux平台 */
    CliShellRegister("ver",       cmdVersion,   "query version");  /* 读版本 */
}

int main(int argc, char **argv)
{
    if (enableRawMode(STDIN_FILENO) == -1) return -1;

    Xprintf("%s\r\n", getenv("TERM"));  //xterm-256color

    CliCmdInit();

	while (1) {
		char c;

		if (read(STDIN_FILENO, &c, 1) > 0) {
			if (c == 3) {
				exit(0);
			}

			CliShellInputChar(c);	
		}

		CliShellTick();
	}
}

