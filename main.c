#include <stdio.h>
#include "vfs.h"

int main(int argc, char* argv[]){

    vpath_t cache = {0};
    vfs_user_root(cache);
    printf("Current root: %s\n", cache);
    
    vpath_t vdir = {0};
    vfs_extend_path(cache, ".velight", vdir);
    vfs_find_path(vdir, false);
    printf("found path: %s\n", vdir);




    return 0;
}