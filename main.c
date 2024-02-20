#include <stdio.h>
#include "vfs.h"
#include <string.h>
#include <stdlib.h>
#include "curl/curl.h"
#include <sqlite3.h>

struct string {
    char *ptr;
    size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}


static size_t callback(void *contents, size_t size, size_t nmemb, void *userp) {
    printf("%.*s\n", (int)(size * nmemb), (char *)contents);
    return size * nmemb;
}


int main(int argc, char* argv[]){

    vpath_t cache = {0};
    vfs_user_root(cache);
    printf("Current root: %s\n", cache);
    
    vpath_t vdir = {0};
    vfs_extend_path(cache, ".velight", vdir);
    vfs_find_path(vdir, false);
    printf("found path: %s\n", vdir);


    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://project-0-8d076-default-rtdb.europe-west1.firebasedatabase.app/vfs.json");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

/*
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        struct string s;
        init_string(&s);

        const char* google = "https://project-0-8d076-default-rtdb.europe-west1.firebasedatabase.app/";
        const char* github = "https://raw.githubusercontent.com/Jaxilian/package_repo/main/repo";
        curl_easy_setopt(curl, CURLOPT_URL, google);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        res = curl_easy_perform(curl);

        printf("%s\n", s.ptr);
        free(s.ptr);
        curl_easy_cleanup(curl);
    }
*/

    return 0;
}