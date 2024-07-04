clang -c vfs.c -o vfs.o
ar rcs vfs.a vfs.o
clang main.c -o vfs -L. -lvfs

# win = llvm-ar rcs libvfs.a vfs.o