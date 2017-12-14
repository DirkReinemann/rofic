#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

const char *LAYOUTDIR = ".screenlayout";
const char *FILEENDING = ".sh";

void open(char *path, char *layout)
{
    const char *cmdfmt = "%s/%s%s >/dev/null 2>&1";
    size_t lenght = strlen(cmdfmt) + strlen(path) + strlen(layout) + strlen(FILEENDING);
    char cmd[lenght];

    snprintf(cmd, lenght, cmdfmt, path, layout, FILEENDING);
    system(cmd);
}

void list(char *path)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(path)))
        return;
    if (!(entry = readdir(dir)))
        return;

    char *ptr = NULL;
    do {
        if (entry->d_type != DT_DIR) {
            char *ending = strrchr(entry->d_name, '.');
            if (ending && !strcmp(ending, FILEENDING)) {
                char *token = strtok_r(entry->d_name, ".", &ptr);
                printf("%s\n", token);
            }
        }
    } while ((entry = readdir(dir)) != NULL);
    closedir(dir);
}

int main(int argc, char *argv[])
{
    char *home = getenv("HOME");

    if (home == NULL) {
        printf("Error while accessing home directory.\n");
        return 1;
    }
    size_t size = strlen(home) + strlen(LAYOUTDIR) + 2;
    char path[size];
    snprintf(path, size, "%s/%s", home, LAYOUTDIR);

    switch (argc) {
    case 2: {
        char *layout = argv[1];
        if (layout == NULL || strlen(layout) == 0) {
            printf("Error while reading the parameter.\n");
            return 1;
        }
        open(path, layout);
        break;
    }
    default:
        list(path);
        break;
    }
}
