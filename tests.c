#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    // int ret = check_archive(fd);
    // printf("check_archive returned %d\n", ret);

    char *test = "dossier/symb.txt";

    int ret0 = exists(fd, test);
    printf("exists returned %d\n", ret0);

    int ret1 = is_dir(fd, test);
    printf("DIR returned %d\n", ret1);

    int ret2 = is_file(fd, test);
    printf("file returned %d\n", ret2);

    int ret3 = is_symlink(fd, test);
    printf("is_symlink returned %d\n", ret3);


    char *entries[10];
    for (int i = 0; i < 10; i++) {
        entries[i] = malloc(64);
    }

    size_t no_entries = 10;
    int ret4 = list(fd, test, entries, &no_entries);
    printf("list returned %d\n", ret4);

    printf("==========\n");

    uint8_t buffer[256];
    size_t len = sizeof(buffer);
    int ret5 = read_file(fd, test,0, buffer, &len);
    printf("readfile returned %d\n", ret5);
    if(ret5>=0){
        printf("buffer content : %s\n-----\n", buffer);
    }
    return 0;
}