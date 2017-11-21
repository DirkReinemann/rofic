#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>

static const char *DBFILE = ".mozilla/firefox/fth9gwv2.default/places.sqlite";
static const char *LISTQUERY = "SELECT f.title, b.title FROM moz_bookmarks b INNER JOIN moz_bookmarks f ON b.parent=f.id INNER JOIN moz_places p ON b.fk=p.id WHERE p.rev_host IS NOT NULL ORDER BY f.title";
static const char *FINDQUERY = "SELECT p.url FROM moz_bookmarks b INNER JOIN moz_bookmarks f ON b.parent=f.id INNER JOIN moz_places p ON b.fk=p.id WHERE p.rev_host IS NOT NULL AND b.title=?";

typedef struct {
    char folder[BUFSIZ];
    char name[BUFSIZ];
} Bookmark;

typedef struct {
    Bookmark *data;
    int       len;
    int       flength;
} Bookmarks;

int list()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    char filename[BUFSIZ];

    snprintf(filename, BUFSIZ, "%s/%s", getenv("HOME"), DBFILE);

    rc = sqlite3_open(filename, &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        exit(1);
    }

    rc = sqlite3_prepare_v2(db, LISTQUERY, strlen(LISTQUERY), &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        exit(1);
    }

    Bookmarks bookmarks;
    bookmarks.len = 0;
    bookmarks.flength = 0;
    bookmarks.data = (Bookmark *)malloc(0 * sizeof(Bookmark));

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        bookmarks.data = (Bookmark *)realloc(bookmarks.data, (bookmarks.len + 1) * sizeof(Bookmark));

        Bookmark bookmark;
        snprintf(bookmark.folder, BUFSIZ, (char *)sqlite3_column_text(stmt, 0));
        snprintf(bookmark.name, BUFSIZ, (char *)sqlite3_column_text(stmt, 1));

        int l = strlen(bookmark.folder);
        if (l > bookmarks.flength)
            bookmarks.flength = l;

        *(bookmarks.data + bookmarks.len) = bookmark;
        bookmarks.len++;
    }

    for (int i = 0; i < bookmarks.len; i++) {
        Bookmark *bookmark = (bookmarks.data + i);
        printf("%-*s : %s\n", bookmarks.flength, bookmark->folder, bookmark->name);
    }

    free(bookmarks.data);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int open(char *str)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    str = strchr(str, ':') + 2;

    char filename[BUFSIZ];
    snprintf(filename, BUFSIZ, "%s/%s", getenv("HOME"), DBFILE);

    rc = sqlite3_open(filename, &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    rc = sqlite3_prepare_v2(db, FINDQUERY, strlen(FINDQUERY), &stmt, 0);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    rc = sqlite3_bind_text(stmt, 1, str, strlen(str), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char url[BUFSIZ];
        snprintf(url, BUFSIZ, (char *)sqlite3_column_text(stmt, 0));
        char cmd[BUFSIZ];
        snprintf(cmd, BUFSIZ, "firefox %s && i3-msg \"workspace 2: browser\" >/dev/null 2>&1", url);
        system(cmd);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int main(int argc, char *argv[])
{
    char *home = getenv("HOME");

    if (home == NULL) {
        printf("Error while accessing home directory.\n");
        return 1;
    }

    if (argc == 1)
        return list();
    else
        return open(argv[1]);
}
