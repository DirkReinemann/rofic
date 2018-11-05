#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <regex.h>
#include <dirent.h>

const char *FIREFOXDIR = ".mozilla/firefox";
const char *FIREFOXDBFILE = "places.sqlite";
const char *FIREFOXDIRREGEX = "default$";
const char *LISTQUERY = "SELECT f.title, b.title FROM moz_bookmarks b INNER JOIN moz_bookmarks f ON b.parent=f.id INNER JOIN moz_places p ON b.fk=p.id WHERE p.rev_host IS NOT NULL ORDER BY f.title";
const char *FINDQUERY = "SELECT p.url FROM moz_bookmarks b INNER JOIN moz_bookmarks f ON b.parent=f.id INNER JOIN moz_places p ON b.fk=p.id WHERE p.rev_host IS NOT NULL AND b.title=?";

typedef struct {
    char *folder;
    char *name;
} bookmark;

typedef struct {
    int          size;
    unsigned int flength;
    bookmark *   data;
} bookmarks;

int list(const char *dbfile)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = 0;

    rc = sqlite3_open(dbfile, &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        exit(1);
    }

    rc = sqlite3_prepare_v2(db, LISTQUERY, strlen(LISTQUERY), &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        exit(1);
    }

    bookmarks bs;
    bs.size = 0;
    bs.flength = 0;
    bs.data = (bookmark *)malloc(0 * sizeof(bookmark));

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        bs.data = (bookmark *)realloc(bs.data, (bs.size + 1) * sizeof(bookmark));
        bookmark *b = bs.data + bs.size;

        char *folder = (char *)sqlite3_column_text(stmt, 0);
        char *name = (char *)sqlite3_column_text(stmt, 1);

        size_t size = strlen(folder) + 1;
        b->folder = (char *)malloc(size * sizeof(char));
        strncpy(b->folder, folder, size);
        b->folder[size - 1] = '\0';

        if (size > bs.flength)
            bs.flength = size;

        size = strlen(name) + 1;
        b->name = (char *)malloc(size * sizeof(char));
        strncpy(b->name, name, size);
        b->name[size - 1] = '\0';

        bs.size++;
    }

    for (int i = 0; i < bs.size; i++) {
        bookmark *b = bs.data + i;
        printf("%-*s : %s\n", bs.flength, b->folder, b->name);

        if (b->folder != NULL)
            free(b->folder);
        if (b->name != NULL)
            free(b->name);
    }
    free(bs.data);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int open(const char *dbfile, const char *name)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    name = strchr(name, ':') + 2;

    rc = sqlite3_open(dbfile, &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    rc = sqlite3_prepare_v2(db, FINDQUERY, strlen(FINDQUERY), &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    rc = sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char *url = (char *)sqlite3_column_text(stmt, 0);
        const char *cmdfmt = "firefox %s && i3-msg \"workspace 2: browser\" >/dev/null 2>&1";
        size_t size = strlen(url) + strlen(cmdfmt);
        char cmd[size];
        snprintf(cmd, size, cmdfmt, url);
        system(cmd);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

char *read_dbfile(const char *home)
{
    DIR *dir;
    struct dirent *entry;
    regex_t regex;
    char *dbfile = NULL;
    size_t size = 0;

    size = strlen(home) + strlen(FIREFOXDIR) + 2;
    char path[size];
    snprintf(path, size, "%s/%s", home, FIREFOXDIR);

    if (!(dir = opendir(path)))
        return dbfile;
    if (!(entry = readdir(dir)))
        return dbfile;

    int found = 0;
    regcomp(&regex, FIREFOXDIRREGEX, 0);
    do {
        if (entry->d_type == DT_DIR) {
            if (!regexec(&regex, entry->d_name, 0, NULL, 0)) {
                size = size + strlen(entry->d_name) + strlen(FIREFOXDBFILE) + 3;
                dbfile = (char *)malloc(size * sizeof(char));
                snprintf(dbfile, size, "%s/%s/%s", path, entry->d_name, FIREFOXDBFILE);
                found = 1;
            }
        }
    } while ((entry = readdir(dir)) != NULL && found == 0);
    closedir(dir);
    return dbfile;
}

void copy_file(char *src, char *dst)
{
    FILE *f_src = fopen(src, "r");
    FILE *f_dst = fopen(dst, "w");

    if (f_src != NULL && f_dst != NULL) {
        int ch = 0;
        while ((ch = fgetc(f_src)) != EOF)
            fputc(ch, f_dst);
    }

    if (f_src != NULL)
        fclose(f_src);

    if (f_dst != NULL)
        fclose(f_dst);
}

int main(int argc, char **argv)
{
    char *home = getenv("HOME");

    if (home == NULL)
        return 1;

    char *dbfile = read_dbfile(home);
    size_t size = strlen(FIREFOXDBFILE) + 6;
    char tmpfile[size];

    snprintf(tmpfile, size, "/tmp/%s", FIREFOXDBFILE);
    copy_file(dbfile, tmpfile);

    int ret = 0;
    if (argc == 1) {
        ret = list(tmpfile);
    } else {
        char *name = argv[1];
        if (name == NULL || strlen(name) == 0)
            ret = 1;
        else
            ret = open(dbfile, name);
    }

    if (dbfile != NULL)
        free(dbfile);

    remove(tmpfile);

    return ret;
}
