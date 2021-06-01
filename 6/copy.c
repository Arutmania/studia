/*
 * Operations on files, mmap() and getopt()
 *
 *
 * Write a program which copies one file to another. Syntax:
 *
 * copy [-m] <file_name> <new_file_name>
 * copy [-h]
 *
 *
 * Without option -m use read() and write() functions to copy file contents. If
 * the option -m is given, do not use neither read() nor write() but map files
 * to memory regions with mmap() and copy the file with memcpy() instead.
 *
 * If the option -h is given or the program is called without arguments print
 * out some help information.
 *
 * Important remarks:
 *
 * - use getopt() function to process command line options and arguments,
 *
 * - the skeleton of the code for both versions (with read/write and with mmap)
 * should be the same, but in some place either copy_read_write(int fd_from,
 * int fd_to) or copy_mmap(int fd_from, int fd_to) should be called,
 *
 * - check error codes after each system call.
 *
 *
 * Manuals (mostly from section 2, i.e. "man -s 2 open"):
 *
 * man -s 3C getopt
 * man open
 * man close
 * man read
 * man write
 * man lseek
 * man fstat
 * man mmap
 * man memcpy
 *
 * List of necessary include files:

 * #include <sys/types.h>
 * #include <sys/stat.h>
 * #include <sys/mman.h>
 * #include <fcntl.h>
 * #include <unistd.h>
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static inline void copy_read_write(int fd_from, int fd_to) {
    char buffer[1024];
    ssize_t r, w;
    while ((r = read(fd_from, buffer, sizeof(buffer))) != 0) {
       if (r < 0)
           err(EXIT_FAILURE, "read failed");

       w = write(fd_to, buffer, r);
       if (w < 0)
           err(EXIT_FAILURE, "write failed");

       // https://stackoverflow.com/a/47372864
       // Technically, you should be checking the return value from write().
       // Just like read(), it's not guaranteed to complete a full write every
       // time it's called. (In practice, when working with regular files,
       // it will always either complete the write or return an error, but
       // there are some esoteric situations where this may not be the case.)
       //if (r != w) {
       //    fprintf(stderr, "no space to write\n");
       //    return;
       //}
    }
}

static inline void copy_mmap(int fd_from, int fd_to) {
    /* obtain source file size */
    struct stat stat;
    if (fstat(fd_from, &stat) < 0)
        err(EXIT_FAILURE, "failed to fstat source file");
    /* set the target file size to the source file size */
    if (ftruncate(fd_to, stat.st_size) != 0)
        err(EXIT_FAILURE, "failed to ftruncate target file");

    /*
     * mmap source file
     *
     * first argument is a hint to influence the return value,
     * NULL means ignore the hint
     *
     * mmap stat.st_size length bytes
     * PROT_READ for read permission
     * MAP_PRIVATE means changes are visible only
     * for this process and discarded afterwards
     * fd_from is the file descriptor of the source file
     * last argument is the file offset - we need to start from 0
     */
    char* from = mmap(NULL,
                      stat.st_size,
                      PROT_READ,
                      MAP_PRIVATE,
                      fd_from,
                      0);
    if (from == MAP_FAILED)
        err(EXIT_FAILURE, "failed to mmap source file");

    /*
     * mmap target file similarly to the source file
     * PROT_WRITE used for writing permission
     * MAP_SHARED for persistant change
     */
    char* to = mmap(NULL,
                    stat.st_size,
                    PROT_WRITE,
                    MAP_SHARED,
                    fd_to,
                    0);
    if (to == MAP_FAILED)
        err(EXIT_FAILURE, "failed to mmap target file");

    /* copy to to from from stat.st_size bytes */
    memcpy(to, from, stat.st_size);

    /*
     * technicall you don't need to munmap because
     * it will happen when process exits
     */
    if (munmap(from, stat.st_size) != 0)
        err(EXIT_FAILURE, "failed to munmap source file");
    if (munmap(to, stat.st_size) != 0)
        err(EXIT_FAILURE, "failed to munmap target file");
}

static inline __attribute__((noreturn)) void usage(char const* argv0) {
    printf("%s [-m] <file_name> <new_file_name>\n"
           "%s [-h]\n"
           " -m    if specified use read() and write() syscalls,\n"
           "       nmap and memcpy otherwise\n"
           " -h    display this message\n", argv0, argv0);
    exit(0);
}

int main(int argc, char* argv[]) {
    int opt;
    bool use_mmap = true;

    /* two flag arguments
     * -h for usage and
     * -m to specify if mmap or read/write is used
     */
    while ((opt = getopt(argc, argv, "hm")) != -1) {
        switch (opt) {
        case 'm':
            use_mmap = false;
            break;
        case 'h':
        default:
            usage(argv[0]);
        }
    }


    /* if supplied any other than 2 positional argument print usage and exit */
    if (argc - optind != 2)
        usage(argv[0]);

    /* open source file specified by first positional argument for reading */
    int from = open(argv[optind], O_RDONLY);
    if (from < 0)
        err(EXIT_FAILURE, "failed to open source file");

    /*
     * open target file specified by second positional argument
     * mmap needs read permission even for just writing (?)
     * O_CREAT | O_EXCL fails if file exists
     * 0666 for rw-rw-rw- permissions
     */
    int to = open(argv[optind + 1], O_RDWR | O_CREAT | O_EXCL, 0666);
    if (to < 0)
        err(EXIT_FAILURE, "failed to create target file");

    if (use_mmap)
        copy_mmap(from, to);
    else
        copy_read_write(from, to);

    close(from);
    close(to);
}
