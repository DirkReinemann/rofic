#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static const char BROWSER_CMD[] = "firefox";
static const char BROWSER_ARGS[] = "new-tab";
static const char SEARCH_FMT[] = "https://duckduckgo.com/?q=%s";

void execute_websearch(const char *browser_cmd, const char *browser_args,
                       const char *search_fmt, const char *search_term)
{
    pid_t child = fork();
    if (child == 0) {
        char search_cmd[100];
        snprintf(search_cmd, 100, search_fmt, search_term);
        char *argv[3] = {
            (char *) browser_args, search_cmd, NULL
        };
        if (execvp(browser_cmd, argv) < 0) {
            fprintf(stderr, "Command execution failed: %s\n", strerror(errno));
            exit(1);
        }
    } else if (child > 0) {
        int wstatus;
        if (wait(&wstatus) < 0) {
            fprintf(stderr, "Wait failed: %s\n", strerror(errno));
            exit(1);
        }

    } else {
        fprintf(stderr, "Fork failed: %s\n", strerror(errno));
        exit(1);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        exit(0);
    }

    const char *browser_cmd = getenv("BROWSER_CMD");
    if (browser_cmd == NULL) {
        browser_cmd = BROWSER_CMD;
    }
    const char *browser_args = getenv("BROWSER_ARGS");
    if (browser_args == NULL) {
        browser_args = BROWSER_ARGS;
    }
    const char *search_fmt = getenv("SEARCH_FMT");
    if (search_fmt == NULL) {
        search_fmt = SEARCH_FMT;
    }

    char *search_term = argv[1];
    if (search_term == NULL || strlen(search_term) == 0) {
        fprintf(stderr, "Error while reading argument\n");
        exit(1);
    }

    execute_websearch(browser_cmd, browser_args, search_fmt, search_term);

    return 0;
}
