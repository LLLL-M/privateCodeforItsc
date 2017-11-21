#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#ifndef READLINE_LIBRARY
#define READLINE_LIBRARY
#endif
#include "readline.h"
#include "history.h"
#include "io_ioctl.h"
#include "debug.h"

struct arg {
	int port;
	char *info;
} args[] = {
	{XRIO_0, "XRIO_0 input test"},
	{XRIO_1, "XRIO_1 input test"},
	{XRIO_2, "XRIO_2 input test"},
	{XRIO_3, "XRIO_3 input test"},
	{XRIO_4, "XRIO_4 input test"},
	{XRIO_5, "XRIO_5 input test"},
	{XRIO_6, "XRIO_6 input test"},
	{XRIO_7, "XRIO_7 input test"},
	{PeriphKey_0, "PeriphKey_0 input test"},
	{PeriphKey_1, "PeriphKey_1 input test"},
	{PeriphKey_2, "PeriphKey_2 input test"},
	{PeriphKey_3, "PeriphKey_3 input test"},
	{PeriphKey_4, "PeriphKey_4 input test"},
	{PeriphLED_0, "PeriphLED_0 on test"},
	{PeriphLED_1, "PeriphLED_1 on test"},
	{PeriphLED_2, "PeriphLED_2 on test"},
	{PeriphLED_3, "PeriphLED_3 on test"},
	{PeriphLED_4, "PeriphLED_4 on test"},
	{LED_0, "LED_0 on test"},
	{LED_1, "LED_1 on test"},
	{PeriphLED_0 + 7, "PeriphLED_0 off test"},
	{PeriphLED_1 + 7, "PeriphLED_1 off test"},
	{PeriphLED_2 + 7, "PeriphLED_2 off test"},
	{PeriphLED_3 + 7, "PeriphLED_3 off test"},
	{PeriphLED_4 + 7, "PeriphLED_4 off test"},
	{LED_0 + 7, "LED_0 off test"},
	{LED_1 + 7, "LED_1 off test"}
};

typedef void cmd_func(struct arg*);

typedef struct {
	char *name;
    cmd_func *func;
    struct arg *arg;
}COMMAND;

static int fd;

void io_input_test(struct arg *a)
{
	int arg = SET_ARG(a->port, 0);
	unsigned int cmd;

	puts(a->info);
	cmd = IO_GET_PIN_STATUS;
	if (ioctl(fd, cmd, &arg)) {
		perror("ioctl called fail");
	} else {
		//printf("value = %d\n\n", arg);
		INFO("value = %d\n\n", arg);
	}
}

void io_output_test(struct arg *a)
{
	int arg;
	if (a->port >= 14 && a->port <= 18) {	
		// PeriphLED0-4 on test, on:0, off:1
		arg = SET_ARG(a->port, 0);
	} else if (a->port >= 19 && a->port <= 20) {	
		// LED_0-1 on test, on:1, off:0
		arg = SET_ARG(a->port, 1);
	} else if (a->port >= 21 && a->port <= 25) {	
		// PeriphLED0-4 off test, on:0, off:1
		arg = SET_ARG(a->port - 7, 1);
	} else if (a->port >= 26) {
		// LED_0-1 off test, on:1, off:0
		arg = SET_ARG(a->port - 7, 0);
	}

	puts(a->info);
	if (ioctl(fd, IO_SET_PIN_STATUS, &arg)) {
		perror("ioctl called fail");
	}
	putchar('\n');
}

void print_help(struct arg *arg)
{
	int n = sizeof(args) / sizeof(struct arg);
	int i;
	for (i = 0; i < n; i++) {
		printf("%02d: %s\n", i + 1, args[i].info);
	}
	printf("q: exit the program!\n");
}

void exit_readline(struct arg *arg)
{
    exit(0);
}

COMMAND cmds[] = {{"01", io_input_test, &args[0]},
				  {"02", io_input_test, &args[1]},
				  {"03", io_input_test, &args[2]},
				  {"04", io_input_test, &args[3]},
				  {"05", io_input_test, &args[4]},
				  {"06", io_input_test, &args[5]},
				  {"07", io_input_test, &args[6]},
				  {"08", io_input_test, &args[7]},
				  {"09", io_input_test, &args[8]},
				  {"10", io_input_test, &args[9]},
				  {"11", io_input_test, &args[10]},
				  {"12", io_input_test, &args[11]},
				  {"13", io_input_test, &args[12]},
				  {"14", io_output_test, &args[13]},
				  {"15", io_output_test, &args[14]},
				  {"16", io_output_test, &args[15]},
				  {"17", io_output_test, &args[16]},
				  {"18", io_output_test, &args[17]},
				  {"19", io_output_test, &args[18]},
				  {"20", io_output_test, &args[19]},
				  {"21", io_output_test, &args[20]},
				  {"22", io_output_test, &args[21]},
				  {"23", io_output_test, &args[22]},
				  {"24", io_output_test, &args[23]},
				  {"25", io_output_test, &args[24]},
				  {"26", io_output_test, &args[25]},
				  {"27", io_output_test, &args[26]},
                  {"h", print_help, NULL},
                  {"q", exit_readline, NULL},
                  {NULL, NULL, NULL}
};

static char* command_generator(const char *text, int state)
{
    const char *name;
    static int list_index, len;

    if (!state)
    {
        list_index = 0;
    }
        len = strlen(text);

    while (name = cmds[list_index].name)
    {
        list_index++;

        if (strncmp (name, text, len) == 0)
            return strdup(name);
    }

    return ((char *)NULL);
}

char **command_completion(const char *text, int start, int end)
{
    char **matches = NULL;
    if (start == 0) {
        matches = rl_completion_matches(text, command_generator);
    }

    return matches;
}

void execute_command(char *text)
{
    int i = 0, len = strlen(text);

    while (cmds[i].name) {
        if (strncmp(text, cmds[i].name, len) == 0)
            break;
        i++;
    }

    if (cmds[i].name)
        cmds[i].func(cmds[i].arg);
    else
        printf("no this command : %s\n", text);
}

void readline_init(void)
{
    rl_readline_name = "myshell";
    rl_attempted_completion_function = command_completion;
}

void clear_blank(char *text)
{
    char *p = text + strlen(text) - 1;
    while(isspace(*p))
        *p-- = '\0';
}

int main(void)
{
	fd = open("/dev/gpio", O_RDWR);
	if (fd == -1) {
		perror("open /dev/gpio fail");
		return -1;
	}
	char *line = NULL;
    readline_init();

	while(1) {
		line = readline("input: ");
		if (!line)
		break;

		if (*line)  {
            clear_blank(line);
            add_history(line);
            execute_command(line);
		}

		free(line);
	}
	return 0;
}
