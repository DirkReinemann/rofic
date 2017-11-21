#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <locale.h>
#include <regex.h>

const char *BOOKDIR = "Books";
const char *LOCALE = "de_DE.UTF-8";
const char *CURDIRNODE = ".";
const char *PARDIRNODE = "..";

typedef struct {
    char *name;
} file;

typedef struct {
    int   size;
    file *data;
} files;

void listdir(char *path, size_t plength, files *fs)
{
    DIR *dir;
    struct dirent *entry;
    size_t length = 0;

    if (!(dir = opendir(path)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, CURDIRNODE) == 0 || strcmp(entry->d_name, PARDIRNODE) == 0)
                continue;
            length = plength + strlen(entry->d_name) + 2;
            char newpath[length];
            snprintf(newpath, length, "%s/%s", path, entry->d_name);
            listdir(newpath, length, fs);
        } else {
            fs->data = (file *)realloc(fs->data, (fs->size + 1) * sizeof(file));
            length = strlen(entry->d_name) + 1;
            file *f = fs->data + fs->size;
            f->name = (char *)malloc(sizeof(char) * length);
            strncpy(f->name, entry->d_name, length);
            f->name[length - 1] = '\0';
            fs->size++;
        }
    } while ((entry = readdir(dir)) != NULL);
    closedir(dir);
}

void find(char *path, size_t plenght, char *search, file *f, int found)
{
    if (found == 1)
        return;

    DIR *dir;
    struct dirent *entry;
    size_t length = 0;

    if (!(dir = opendir(path)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        if (entry->d_type == DT_DIR && found == 0) {
            if (strcmp(entry->d_name, CURDIRNODE) == 0 || strcmp(entry->d_name, PARDIRNODE) == 0)
                continue;
            length = plenght + strlen(entry->d_name) + 2;
            char newpath[length];
            snprintf(newpath, length, "%s/%s", path, entry->d_name);
            find(newpath, length, search, f, found);
        } else {
            regex_t regex;
            regcomp(&regex, search, 0);
            if (!regexec(&regex, entry->d_name, 0, NULL, 0)) {
                length = plenght + strlen(entry->d_name) + 2;
                f->name = (char *)malloc(sizeof(char) * length);
                snprintf(f->name, 1024, "%s/%s", path, entry->d_name);
                found = 1;
                return;
            }
        }
    } while ((entry = readdir(dir)) != NULL);
    closedir(dir);
}

int comp(const void *a, const void *b)
{
    file *fa = (file *)a;
    file *fb = (file *)b;

    return strcoll(fa->name, fb->name);
}

void list(char *path)
{
    files fs;

    fs.size = 0;
    fs.data = (file *)calloc(0, sizeof(file));
    listdir(path, strlen(path), &fs);
    qsort(fs.data, fs.size, sizeof(file), comp);
    for (int i = 0; i < fs.size; i++) {
        file *f = fs.data + i;
        printf("%s\n", f->name);
        free(f->name);
    }
    free(fs.data);
}

void open(char *path, char *search)
{
    file f;

    find(path, strlen(path), search, &f, 0);
    if (f.name == NULL || strlen(f.name) == 0) {
        printf("Error while finding the file '%s'.\n", search);
        return;
    }
    const char *cmdfmt = "nohup >/dev/null 2>&1 xdg-open '%s' &";
    size_t size = strlen(cmdfmt) + strlen(f.name);
    char cmd[size];
    snprintf(cmd, size, cmdfmt, f.name);
    system(cmd);
    free(f.name);
}

int main(int argc, char *argv[])
{
    setlocale(LC_COLLATE, LOCALE);

    char *home = getenv("HOME");
    if (home == NULL) {
        printf("Error while accessing home directory.\n");
        return 1;
    }
    size_t size = strlen(home) + strlen(BOOKDIR) + 2;
    char path[size];
    snprintf(path, size, "%s/%s", home, BOOKDIR);

    switch (argc) {
    case 2: {
        char *search = argv[1];
        if (search == NULL || strlen(search) == 0) {
            printf("Error while reading the parameter.\n");
            return 1;
        }
        open(path, search);
        break;
    }
    default:
        list(path);
        break;
    }
    return 0;
}
