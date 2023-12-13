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

    int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);


    int fd2 = open(argv[1] , O_RDONLY);
    int ret0 = exists(fd2, "dossier/");
    printf("exists returned %d\n", ret0);

    int fd3 = open(argv[1] , O_RDONLY);
    int ret1 = is_dir(fd3, "dossier/ok.txt");
    printf("DIR returned %d\n", ret1);

    int fd4 = open(argv[1] , O_RDONLY);
    int ret2 = is_file(fd4, "dossier/");
    printf("file returned %d\n", ret2);


    return 0;
}