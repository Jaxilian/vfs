#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

bool vfs_path_new(const char* cstr_path, vpath_t out){
    memset(out, 0, sizeof(vpath_t));
    strcpy(out, cstr_path);
    return true;
}

bool vfs_system_root(vpath_t out){
    memset(out, 0, sizeof(vpath_t));

    #ifdef _WIN32
        strcpy(out, "C:");
    #else
        strcpy(out, "");
    #endif

    return true;
}

bool vfs_user_root(vpath_t out){
    memset(out, 0, sizeof(vpath_t));

    #ifdef _WIN32
        char win_app_data[MAX_PATH];
        if (
            GetEnvironmentVariableA(
                    "LOCALAPPDATA",
                    win_app_data,
                    MAX_PATH
                )
            ) 
            {
            strcpy(out->data, win_app_data);
            return true,
        } else {
            printf("Error getting \"Local AppData\" path\n");
            return false;
        }
    #else
        strcpy(out, "~");
        return true;
    #endif
}

bool vfs_path_depth(const vpath_t path, uint32_t* out_depth){
    if(out_depth == NULL){
        return false;
    }

    uint32_t found = 0;
    for(uint32_t i = 0; i < strlen(path); ++i){
        if(path[i] == '\\' || path[i] == '/'){
            found++;    
        }
    }

    *out_depth = found;
    return true;
}

bool vfs_change_directory(const vpath_t path){
    #ifdef _WIN32
    wchar_t buffer[260] = { 0 };
    cstr_to_wchar(path->data, buffer);
    if (SetCurrentDirectory(buffer)) {
        return true;
    }
    else {
        printf("Failed to change directory to %s\n", path->data);
    }

    #else

    if (chdir(path) == 0) {
        return true;
    }
    else {
        perror("Failed to change directory");
    }
    #endif
    return true;
}

bool vfs_split_path(const vpath_t path, uint32_t depth, vpath_t* out){
      if (out == NULL || depth == 0) {
        printf("vfs_split_path -> \"out\" argument was NULL or depth is 0\n");
        return false;
    }

    uint32_t index = 0;
    uint32_t cache_len = 0;
    vpath_t cache = {0};

    for (uint32_t i = 0; path[i] != '\0' && index < depth; ++i) {
        if (path[i] == '\\' || path[i] == '/') {
            if (cache_len > 0) { 
                cache[cache_len] = '\0'; 
                strcpy(out[index++], cache); 
                cache_len = 0; 
            }
        } else {
            if (cache_len < MAX_PATH - 1) {
                cache[cache_len++] = path[i];
            }
        }
    }

    if (cache_len > 0 && index < depth) { 
        cache[cache_len] = '\0'; 
        strcpy(out[index], cache);
    }

    return true;
}

bool vfs_create_directory(const vpath_t full_path){
#ifdef _WIN32
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "mkdir %s", full_path);
    return system(cmd) == 0;
#else
    char cmd[MAX_PATH];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", full_path);
    return system(cmd) == 0;
#endif
}

bool vfs_find_path(const vpath_t path, bool create_missing){
      if (path == NULL) {
        printf("path was null!\n");
        return false;
    }

    bool found = false;
    #ifdef _WIN32
        DWORD dwAttrib = GetFileAttributesA(path->data);

        found = (dwAttrib != INVALID_FILE_ATTRIBUTES);
    #else
        DIR* dir = opendir(path);
        if(dir){
            found = true;
            closedir(dir);
        }
    #endif

    if(create_missing && !found){
        return vfs_create_directory(path);
    }

    return found;
}

bool vfs_virtual_path(vpath_t out){
    #ifdef _WIN32
        WCHAR buff[MAX_PATH] = { 0 };
        GetCurrentDirectory(MAX_PATH, buff);
        wchar_to_cstr(buff, out);
    #else
        getcwd(out, MAX_PATH);
    #endif
    return true;
}

bool vfs_extend_path(const vpath_t parent, const char* child, vpath_t out){
    strcpy(out, parent);
    strcat(out, "/");
    strcat(out, child);
    return true;
}