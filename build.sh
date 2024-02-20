clang -c vfs.c -o vfs.o
ar rcs libvfs.a vfs.o
clang main.c -o vfs -L. -lvfs -lcurl
