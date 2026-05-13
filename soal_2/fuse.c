#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

static const char *rootdir = "encrypted_storage";
static const unsigned char XOR_KEY = 0x76;

void fullpath(char fpath[1024], const char *path)
{
    sprintf(fpath, "%s%s", rootdir, path);
}

void enc_name(char out[1024], const char *path)
{
    sprintf(out, "%s%s.enc", rootdir, path);
}

void xor_buffer(char *buf, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        buf[i] ^= XOR_KEY;
    }
}

static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1024];

    fullpath(fpath, path);

    res = lstat(fpath, stbuf);

    if (res == -1)
    {
        enc_name(fpath, path);

        res = lstat(fpath, stbuf);

        if (res == -1)
            return -errno;
    }

    return 0;
}

static int xmp_access(const char *path, int mask)
{
    char fpath[1024];

    fullpath(fpath, path);

    if (access(fpath, mask) == -1)
    {
        enc_name(fpath, path);

        if (access(fpath, mask) == -1)
            return -errno;
    }

    return 0;
}

static int xmp_readdir(
    const char *path,
    void *buf,
    fuse_fill_dir_t filler,
    off_t offset,
    struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;
    char fpath[1024];

    (void) offset;
    (void) fi;

    fullpath(fpath, path);

    dp = opendir(fpath);

    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL)
    {
        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        char name[1024];
        strcpy(name, de->d_name);

        // hilangkan .enc
        char *ext = strstr(name, ".enc");

        if (ext && strcmp(ext, ".enc") == 0)
        {
            *ext = '\0';
        }

        filler(buf, name, &st, 0);
    }

    closedir(dp);

    return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    char fpath[1024];

    fullpath(fpath, path);

    int res = mkdir(fpath, mode);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_rmdir(const char *path)
{
    char fpath[1024];

    fullpath(fpath, path);

    int res = rmdir(fpath);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
    char fpath[1024];

    enc_name(fpath, path);

    int fd = creat(fpath, mode);

    if (fd == -1)
        return -errno;

    close(fd);

    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
    int fd;
    char fpath[1024];

    fullpath(fpath, path);

    fd = open(fpath, fi->flags);

    if (fd == -1)
    {
        enc_name(fpath, path);

        fd = open(fpath, fi->flags);

        if (fd == -1)
            return -errno;
    }

    close(fd);

    return 0;
}

static int xmp_read(
    const char *path,
    char *buf,
    size_t size,
    off_t offset,
    struct fuse_file_info *fi)
{
    int fd;
    int res;
    char fpath[1024];

    (void) fi;

    fullpath(fpath, path);

    fd = open(fpath, O_RDONLY);

    if (fd == -1)
    {
        enc_name(fpath, path);

        fd = open(fpath, O_RDONLY);

        if (fd == -1)
            return -errno;
    }

    res = pread(fd, buf, size, offset);

    if (res == -1)
        res = -errno;
    else
        xor_buffer(buf, res);

    close(fd);

    return res;
}

static int xmp_write(
    const char *path,
    const char *buf,
    size_t size,
    off_t offset,
    struct fuse_file_info *fi)
{
    int fd;
    int res;
    char fpath[1024];

    (void) fi;

    enc_name(fpath, path);

    fd = open(fpath, O_WRONLY);

    if (fd == -1)
        return -errno;

    char *tmp = malloc(size);

    memcpy(tmp, buf, size);

    xor_buffer(tmp, size);

    res = pwrite(fd, tmp, size, offset);

    free(tmp);

    if (res == -1)
        res = -errno;

    close(fd);

    return res;
}

static int xmp_truncate(const char *path, off_t size)
{
    char fpath[1024];

    enc_name(fpath, path);

    int res = truncate(fpath, size);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_unlink(const char *path)
{
    char fpath[1024];

    enc_name(fpath, path);

    int res = unlink(fpath);

    if (res == -1)
        return -errno;

    return 0;
}

static int xmp_utimens(const char *path,
                       const struct timespec ts[2])
{
    char fpath[1024];

    enc_name(fpath, path);

    int res = utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW);

    if (res == -1)
        return -errno;

    return 0;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .mkdir = xmp_mkdir,
    .rmdir = xmp_rmdir,
    .create = xmp_create,
    .open = xmp_open,
    .read = xmp_read,
    .write = xmp_write,
    .truncate = xmp_truncate,
    .unlink = xmp_unlink,
    .access = xmp_access,
    .utimens = xmp_utimens,
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
