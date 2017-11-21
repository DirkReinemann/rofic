#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <uuid/uuid.h>
#include <sys/stat.h>
#include <sys/types.h>

const char *OUTDIR = "/tmp/rofi-search";
const char *SEARCHURL = "https://www.startpage.com/do/search";

size_t wfile(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);

    return written;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        exit(1);

    char *search = argv[1];
    if (search == NULL || strlen(search) == 0)
        exit(1);

    mkdir(OUTDIR, 0755);

    CURL *curl = curl_easy_init();
    if (curl == NULL)
        exit(1);

    size_t size;
    uuid_t uuid;

    const char *datafmt = "cmd=process_search&language=english&enginecount=1&pl&abp=1&ff&theme&flag_ac=0&cat=web&ycc=0&t&nj=0&query=%s&pg=0";
    size = strlen(datafmt) + strlen(search);
    char data[size];
    snprintf(data, size, datafmt, search);

    curl_easy_setopt(curl, CURLOPT_URL, SEARCHURL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, wfile);

    char filename[37];
    uuid_generate(uuid);
    uuid_unparse(uuid, filename);

    size = strlen(OUTDIR) + strlen(filename) + 2;
    char outfile[size];
    snprintf(outfile, size, "%s/%s", OUTDIR, filename);

    FILE *file = fopen(outfile, "wb");
    if (file) {
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        curl_easy_perform(curl);
        fclose(file);

        const char *cmdfmt = "firefox %s && i3-msg \"workspace 2: browser\" >/dev/null 2>&1";
        size = strlen(cmdfmt) + strlen(outfile);
        char cmd[size];
        snprintf(cmd, BUFSIZ, cmdfmt, outfile);
        system(cmd);
    }
    curl_easy_cleanup(curl);
}
