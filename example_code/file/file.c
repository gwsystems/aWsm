#include <assert.h>
#include <stdio.h>
#include <wasi/api.h>

int main(int argc, char **argv)
{
    /* Create a file */
    FILE *handle = fopen("/home/letters.txt", "w+");
    for (char c = 'a'; c <= 'z'; c++)
        fputc(c, handle);
    fclose(handle);

    /* Preopened file descriptors in WASI montonically increment from 3 */
    __wasi_errno_t rc;
    __wasi_fd_t fd = 3;
    __wasi_prestat_t prestat;

    rc = __wasi_fd_prestat_get(fd, &prestat);
    assert(rc == __WASI_ERRNO_SUCCESS);

    /* Directories are the only preopen type currently */
    assert(prestat.tag == __WASI_PREOPENTYPE_DIR);
    size_t path_len = prestat.u.dir.pr_name_len;
    char path[path_len];

    /* 
     * I suspect WASI strings might clobber the terminator
     * However, the uvwasi implementation will not overwrite the last character 
     * in the provided buffer.
     */
    path[path_len - 1] = '\0';

    __wasi_fd_prestat_dir_name(fd, path, path_len);

    printf("Preopened path is %s\n", path);
}
