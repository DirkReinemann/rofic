#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <ctype.h>

const char *CONFIGFILE = ".ssh/config";
const char *REGEX_HOST = "^hostname";
const char *REGEX_PORT = "^port";
const char *REGEX_NAME = "^host";
const char *REGEX_USER = "^user";
const char *SEPARATOR = ";";

typedef struct {
    char *name;
    char *host;
    char *user;
    char *port;
} config;

typedef struct {
    config *data;
    int     size;
    int     nlen;
    int     hlen;
    int     ulen;
    int     plen;
} configs;

void change_length(char *c, int *i)
{
    if (c != NULL) {
        int l = strlen(c);
        if (l > *i)
            *i = l;
    }
}

char *remove_whitespace(char *str)
{
    while (isspace((unsigned char)*str))
        str++;
    return str;
}

char *read_value(const char *key, char *str, char *ret)
{
    regex_t regex;
    regmatch_t matches[1];
    regcomp(&regex, key, REG_ICASE);
    int s = strlen(str);
    char tmp[s + 1];
    strncpy(tmp, str, s);
    tmp[s] = '\0';
    char *ptr;
    char *token = strtok_r(tmp, SEPARATOR, &ptr);
    int found = 0;
    while (token != NULL && found != 1) {
        if (!regexec(&regex, token, 1, matches, 0)) {
            int len = strlen(token) - matches[0].rm_eo;
            ret = (char *)malloc(len * sizeof(char));
            strncpy(ret, token + matches[0].rm_eo + 1, len - 1);
            ret[len - 1] = '\0';
            found = 1;
        }
        token = strtok_r(NULL, SEPARATOR, &ptr);
    }
    if (found == -1)
        ret = NULL;
    return ret;
}

int compare_config(const void *a, const void *b)
{
    config *ca = (config *)a;
    config *cb = (config *)b;
    return strcoll(ca->name, cb->name);
}

void init_configs(configs *configs)
{
    configs->size = 0;
    configs->nlen = 0;
    configs->hlen = 0;
    configs->ulen = 0;
    configs->plen = 0;
    configs->data = (config *)malloc(0 * sizeof(config));
}

void free_config(config *c)
{
    if (c->host != NULL)
        free(c->host);
    if (c->name != NULL)
        free(c->name);
    if (c->port != NULL)
        free(c->port);
    if (c->user != NULL)
        free(c->user);
}

void list()
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
    char *home = getenv("HOME");
    int flength = strlen(home) + strlen(CONFIGFILE) + 2;
    char filename[flength];

    snprintf(filename, flength, "%s/%s", home, CONFIGFILE);
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("The file '%s' was not found.\n", filename);
        exit(1);
    }

    configs cs;
    init_configs(&cs);

    int l = 0;
    int r = 0;
    char *lines = (char *)calloc(1024, sizeof(char));
    while ((read = getline(&line, &len, file)) != -1) {
        if (read == 1) {
            cs.data = (config *)realloc(cs.data, (cs.size + 1) * sizeof(config));
            config *c = cs.data + cs.size;

            c->name = read_value(REGEX_NAME, lines, c->name);
            c->host = read_value(REGEX_HOST, lines, c->host);
            c->user = read_value(REGEX_USER, lines, c->user);
            c->port = read_value(REGEX_PORT, lines, c->port);

            change_length(c->name, &(cs.nlen));
            change_length(c->host, &(cs.hlen));
            change_length(c->user, &(cs.ulen));
            change_length(c->port, &(cs.plen));
            cs.size++;

            memset(lines, 0, 1024);
            l = 0;
        } else {
            r = strlen(line);
            line[r - 1] = '\0';
            l += snprintf(lines + l, r, "%s", remove_whitespace(line));
            l += snprintf(lines + l, 2, "%s", SEPARATOR);
        }
        if (line != NULL) {
            free(line);
            line = NULL;
        }
    }
    free(lines);

    qsort(cs.data, cs.size, sizeof(config), compare_config);
    for (int i = 0; i < cs.size; i++) {
        config *c = cs.data + i;
        printf("%-*s : %-*s %-*s %-*s\n", cs.nlen, c->name, cs.ulen, c->user, cs.hlen, c->host, cs.plen, c->port);
        free_config(c);
    }
    free(cs.data);
    fclose(file);
}

void open(char *str)
{
    char *token = strtok(str, " ");
    const char *cmdfmt = "i3-msg 'workspace 1: terminal' >/dev/null 2>&1 && terminator --new-tab -x 'ssh %s'";
    char length = strlen(cmdfmt) + strlen(token);
    char cmd[length];

    snprintf(cmd, length, cmdfmt, token);
    system(cmd);
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        list();
    else
        open(argv[1]);
    return 0;
}
