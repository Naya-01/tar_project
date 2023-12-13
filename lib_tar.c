#include "lib_tar.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#define MAX_BLOCK 512

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    char buffer[MAX_BLOCK];
    tar_header_t *header = (tar_header_t*) buffer;
    int nb_headers = 0;

    while (read(tar_fd, header, MAX_BLOCK) != -1) {

        if (header->name[0] == '\0') { 
            break;
        }

        //printf("header name :::: %s \n", header->name);

        if (strncmp(header->magic, TMAGIC, TMAGLEN) != 0) {
            return -1;
        }

        if (strncmp(header->version, TVERSION, TVERSLEN) != 0) {
            return -2;
        }

        int chksum = 0;
        char *bytes = (char *)header;
        for (int i = 0; i < MAX_BLOCK; i++) {
            if (i >= 148 && i < 156) {
                chksum += ' '; 
            } else {
                chksum += bytes[i];
            }
        }

        if (chksum != TAR_INT(header->chksum)) {
            return -3;  
        }

        int file_size = TAR_INT(header->size);
        int file_blocks = (file_size + MAX_BLOCK - 1) / MAX_BLOCK;
        lseek(tar_fd, file_blocks * MAX_BLOCK, SEEK_CUR);

        nb_headers++;
    }


    return nb_headers;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    char buffer[MAX_BLOCK];
    tar_header_t *header = (tar_header_t*) buffer;
    
    while (read(tar_fd, header, MAX_BLOCK) != -1) {

        if (header->name[0] == '\0') {
            break;
        }

        // printf("header name :::: %s \n", header->name);
        // printf("path name :::: %s \n", path);
        if (strcmp(header->name, path) == 0) {
            return 1;
        }

        int file_size = TAR_INT(header->size);
        int file_blocks = (file_size + MAX_BLOCK - 1) / MAX_BLOCK;
        lseek(tar_fd, file_blocks * MAX_BLOCK, SEEK_CUR);

    }

    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    char buffer[MAX_BLOCK];
    tar_header_t *header = (tar_header_t*) buffer;
    
    while (read(tar_fd, header, MAX_BLOCK) != -1) {

        if (header->name[0] == '\0') {
            break;
        }

        //printf("header name :::: %s \n", header->name);
        //printf("flag type :::: %c \n", header->typeflag);

        if (strcmp(header->name, path) == 0) {
            return header->typeflag == DIRTYPE ? 1 : 0;
        }

        int file_size = TAR_INT(header->size);
        int file_blocks = (file_size + MAX_BLOCK - 1) / MAX_BLOCK;
        lseek(tar_fd, file_blocks * MAX_BLOCK, SEEK_CUR);

    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    char buffer[MAX_BLOCK];
    tar_header_t *header = (tar_header_t*) buffer;
    
    while (read(tar_fd, header, MAX_BLOCK) != -1) {

        if (header->name[0] == '\0') {
            break;
        }

        if (strcmp(header->name, path) == 0) {
            return (header->typeflag == REGTYPE || header->typeflag == AREGTYPE ) ? 1 : 0;
        }

        int file_size = TAR_INT(header->size);
        int file_blocks = (file_size + MAX_BLOCK - 1) / MAX_BLOCK;
        lseek(tar_fd, file_blocks * MAX_BLOCK, SEEK_CUR);

    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    char buffer[MAX_BLOCK];
    tar_header_t *header = (tar_header_t*) buffer;
    
    while (read(tar_fd, header, MAX_BLOCK) != -1) {

        if (header->name[0] == '\0') {
            break;
        }

        if (strcmp(header->name, path) == 0 && header->typeflag == LNKTYPE) {
            return 1;
        }

        int file_size = TAR_INT(header->size);
        int file_blocks = (file_size + MAX_BLOCK - 1) / MAX_BLOCK;
        lseek(tar_fd, file_blocks * MAX_BLOCK, SEEK_CUR);

    }
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    return 0;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    return 0;
}