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
#include <errno.h>
#endif

bool cstr_to_wchar(const char* str, wchar_t* wchar) {
#ifdef _WIN32
    size_t len = 0;
    errno_t err = mbstowcs_s(&len, NULL, 0, str, _TRUNCATE);
    if (err != 0 || len == 0) {
        return false;
    }

    size_t converted = 0;
    err = mbstowcs_s(&converted, wchar, len, str, _TRUNCATE);
    if (err != 0 || converted == 0) {
        return false;
    }
#else
    size_t len = mbstowcs(NULL, str, 0);
    if (len == (size_t)(-1)) return false;
    mbstowcs(wchar, str, len + 1);
#endif

    return true;
}

bool wchar_to_cstr(const wchar_t* wchar, char* str) {
#ifdef _WIN32
    size_t len = 0;
    errno_t err = wcstombs_s(&len, NULL, 0, wchar, _TRUNCATE);

    if (err != 0 || len == 0) {
        return false;
    }

    size_t converted = 0;
    err = wcstombs_s(&converted, str, len, wchar, _TRUNCATE);
    if (err != 0 || converted == 0) {
        return false;
    }
#else
    size_t len = wcstombs(NULL, wchar, 0);
    if (len == (size_t)(-1)) return false;
    wcstombs(str, wchar, len + 1);
#endif

    return true;
}

static void validate_path(vpath_t path) {
#ifdef _WIN32
    uint32_t len = (uint32_t)strlen(path);
    for (uint32_t i = 0; i < len; i++) {
        if (path[i] == UNIX_PATH_END_STD) {
            path[i] = WIN_PATH_END_STD;
        }
    }
    uint32_t parent_length = (uint32_t)strlen(path);
    char lastchar = path[parent_length - 1];
    if (lastchar != WIN_PATH_END_STD && lastchar != UNIX_PATH_END_STD) {
        strcat(path, (const char*)WIN_PATH_END_STD);
    }
#else
    uint32_t len = (uint32_t)strlen(path);
    for (uint32_t i = 0; i < len; i++) {
        if (path[i] == '\\') {
            path[i] = '/';
        }
    }
    uint32_t parent_length = (uint32_t)strlen(path);
    char lastchar = path[parent_length - 1];
    if (lastchar != '/') {
        strcat(path, "/");
    }
#endif
}

bool vfs_exist(const vpath_t path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES);
#else
    return access(path, F_OK) == 0;
#endif
}

bool vfs_is(const vpath_t path, int type) {
#ifdef _WIN32
    DWORD attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    switch (type) {
    case VFS_TYPE_FILE:
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
    case VFS_TYPE_DIR:
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    default:
        return false;
    }
#else
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }

    switch (type) {
    case VFS_TYPE_FILE:
        return S_ISREG(st.st_mode);
    case VFS_TYPE_DIR:
        return S_ISDIR(st.st_mode);
    default:
        return false;
    }
#endif 
}

bool vfs_create(const vpath_t parent, const char* name, int v) {
    vpath_t new_path = { 0 };
    strcpy(new_path, parent);

    validate_path(new_path);
    strcat(new_path, name);

    if (v == VFS_TYPE_DIR) {
        if (vfs_exist(new_path)) {
            return false;
        }

        char cmd[MAX_PATH + 9];
        snprintf(cmd, sizeof(cmd), "mkdir %s", new_path);
        return system(cmd) == 0;
    }
    else if (v == VFS_TYPE_FILE) {
        FILE* file = fopen(new_path, "w");
        if (file == NULL) {
            printf("failed to create file: %s\n", new_path);
            return false;
        }

        fclose(file);
    }

    return true;
}

bool vfs_path_new(const char* cstr_path, vpath_t out) {
    if (!vfs_exist(cstr_path)) {
        printf("path: %s doesn't exist! create it first\n", cstr_path);
        return false;
    }
    memset(out, 0, sizeof(vpath_t));
    strcpy(out, cstr_path);
    return true;
}

bool vfs_system_root(vpath_t out) {
    memset(out, 0, sizeof(vpath_t));

#ifdef _WIN32
    strcpy(out, "C:\\");
#else
    strcpy(out, "/");
#endif

    return true;
}

bool vfs_user_home(vpath_t out) {
    memset(out, 0, sizeof(vpath_t));

#ifdef _WIN32
    char win_app_data[MAX_PATH];
    if (GetEnvironmentVariableA("LOCALAPPDATA", win_app_data, MAX_PATH)) {
        strcpy(out, win_app_data);
        return true;
    }
    else {
        printf("error getting \"Local AppData\" path\n");
        return false;
    }
#else
    char* home = getenv("HOME");
    if (home != NULL) {
        strcpy(out, home);
        return true;
    }
    else {
        printf("error getting \"Home\" path\n");
        return false;
    }
#endif
}

bool vfs_depth_len(const vpath_t path, uint32_t* out_depth) {
    if (out_depth == NULL) {
        return false;
    }

    uint32_t found = 0;
    for (uint32_t i = 0; i < strlen(path); ++i) {
        if (path[i] == '\\' || path[i] == '/') {
            found++;
        }
    }

    *out_depth = found;
    return true;
}

bool vfs_cd(const vpath_t path) {
#ifdef _WIN32
    wchar_t buffer[260] = { 0 };
    cstr_to_wchar(path, buffer);
    if (SetCurrentDirectory(buffer)) {
        return true;
    }
    else {
        printf("failed to change directory to %s\n", path);
    }
#else
    if (chdir(path) == 0) {
        return true;
    }
    else {
        perror("failed to change directory");
    }
#endif
    return false;
}

uint32_t vfs_split_path(const vpath_t path, vpath_t* out) {
    uint32_t len = 0;
    vfs_depth_len(path, &len);
    if (out == NULL) {
        return len;
    }

    uint32_t index = 0;
    uint32_t cache_len = 0;
    vpath_t cache = { 0 };

    for (uint32_t i = 0; path[i] != '\0' && index < len; ++i) {
        if (path[i] == '\\' || path[i] == '/') {
            if (cache_len > 0) {
                cache[cache_len] = '\0';
                strcpy(out[index++], cache);
                cache_len = 0;
            }
        }
        else {
            if (cache_len < MAX_PATH - 1) {
                cache[cache_len++] = path[i];
            }
        }
    }

    if (cache_len > 0 && index < len) {
        cache[cache_len] = '\0';
        strcpy(out[index], cache);
    }

    return len;
}

bool vfs_find_path(const vpath_t path, bool create_missing) {
    bool found = false;
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path);
    found = (dwAttrib != INVALID_FILE_ATTRIBUTES);
#else
    DIR* dir = opendir(path);
    if (dir) {
        found = true;
        closedir(dir);
    }
#endif

    if (create_missing && !found) {
        return vfs_create(path, "", VFS_TYPE_DIR);
    }

    return found;
}

bool vfs_current_path(vpath_t out) {
#ifdef _WIN32
    WCHAR buff[MAX_PATH] = { 0 };
    GetCurrentDirectory(MAX_PATH, buff);
    wchar_to_cstr(buff, out);
#else
    if (getcwd(out, MAX_PATH) == NULL) {
        perror("getcwd");
        return false;
    }
#endif
    return true;
}

bool vfs_extend_path(const vpath_t parent, const char* child, vpath_t out) {
    strcpy(out, parent);
    strcat(out, "/");
    strcat(out, child);
    return true;
}
