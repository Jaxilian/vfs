#ifndef VELIGHT_VFS_H
#define VELIGHT_VFS_H

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#include <inttypes.h>
#include <stdbool.h>

typedef char vpath_t[MAX_PATH];

bool vfs_path_new(const char* cstr_path, vpath_t out);
bool vfs_change_directory(const vpath_t path);
bool vfs_extend_path(const vpath_t parent, const char* child, vpath_t out);
bool vfs_system_root(vpath_t out);
bool vfs_user_root(vpath_t out);
bool vfs_virtual_path(vpath_t out);
bool vfs_path_depth(const vpath_t path, uint32_t* out_depth);
bool vfs_split_path(const vpath_t path, uint32_t depth, vpath_t* out);
bool vfs_create_directory(const vpath_t full_path);
bool vfs_find_path(const vpath_t path, bool create_missing);

#endif