#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

// /home/adr01/Downloads
char *dirpath = "/home/oni/Downloads";
// /home/adr01/SinSeiFS.log.txt
char *logpath = "/home/oni/SinSeiFS.log.txt";
// /home/adr01/Documents/SesiLab4/Soal1/log.txt
char *logpath1 = "/media/sf_soal-shift-sisop-modul-4-E05-2021/log.txt";

char *FileExtension(char *filename)
{
    char *temp = strrchr(filename, '/');
    return temp + 1;
}

// Dapatkan directory dan nama file
void getDirAndFile(char *dir, char *file, char *path)
{
    char buff[1000];
    memset(dir, 0, 1000);
    memset(file, 0, 1000);
    strcpy(buff, path);
    char *token = strtok(buff, "/");
    while (token != NULL)
    {
        sprintf(file, "%s", token);
        token = strtok(NULL, "/");
        if (token != NULL)
        {
            strcat(dir, "/");
            strcat(dir, file);
        }
    }
}

// Untuk enkripsi dan dekripsi
void decrypt(char *path, int isEncrypt)
{
    char *str = path;
    int i = 0;
    while (str[i] != '\0')
    {
        if (!((str[i] >= 0 && str[i] < 65) || (str[i] > 90 && str[i] < 97) || (str[i] > 122 && str[i] <= 127)))
        {
            if (str[i] >= 'A' && str[i] <= 'Z')
                str[i] = 'Z' + 'A' - str[i];
            if (str[i] >= 'a' && str[i] <= 'z')
                str[i] = 'z' + 'a' - str[i];
        }

        if (((str[i] >= 0 && str[i] < 65) || (str[i] > 90 && str[i] < 97) || (str[i] > 122 && str[i] <= 127)))
            str[i] = str[i];

        i++;
    }
}

void changePath(char *fpath, const char *path, int isWriteOper, int isFileAsked)
{
    char *ptr = strstr(path, "/AtoZ_");
    int state = 0;
    if (ptr != NULL)
    {
        if (strstr(ptr + 1, "/") != NULL)
            state = 1;
    }
    char fixPath[1000];
    memset(fixPath, 0, sizeof(fixPath));

    if (ptr != NULL && state)
    {
        ptr = strstr(ptr + 1, "/");
        char pathEncvDirBuff[1000];
        char pathEncryptedBuff[1000];
        strcpy(pathEncryptedBuff, ptr);
        strncpy(pathEncvDirBuff, path, ptr - path);

        if (isWriteOper)
        {
            char pathFileBuff[1000];
            char pathDirBuff[1000];
            getDirAndFile(pathDirBuff, pathFileBuff, pathEncryptedBuff);
            decrypt(pathDirBuff, 0);

            strcat(fixPath, pathEncvDirBuff);
            strcat(fixPath, pathDirBuff);
            strcat(fixPath, "/");
            strcat(fixPath, pathFileBuff);
        }
        else if (isFileAsked)
        {
            char pathFileBuff[1000];
            char pathDirBuff[1000];
            char pathExtBuff[1000];
            getDirAndFile(pathDirBuff, pathFileBuff, pathEncryptedBuff);
            char *whereIsExtension = strrchr(pathFileBuff, '.');

            if (whereIsExtension - pathFileBuff < 1)
            {
                decrypt(pathDirBuff, 0);
                decrypt(pathFileBuff, 0);
                strcat(fixPath, pathEncvDirBuff);
                strcat(fixPath, pathDirBuff);
                strcat(fixPath, "/");
                strcat(fixPath, pathFileBuff);
            }
            else
            {
                char pathJustFileBuff[1000];
                memset(pathJustFileBuff, 0, sizeof(pathJustFileBuff));
                strcpy(pathExtBuff, whereIsExtension);
                snprintf(pathJustFileBuff, whereIsExtension - pathFileBuff + 1, "%s", pathFileBuff);
                decrypt(pathDirBuff, 0);
                decrypt(pathJustFileBuff, 0);

                strcat(fixPath, pathEncvDirBuff);
                strcat(fixPath, pathDirBuff);
                strcat(fixPath, "/");
                strcat(fixPath, pathJustFileBuff);
                strcat(fixPath, pathExtBuff);
            }
        }
        else
        {
            decrypt(pathEncryptedBuff, 0);
            strcat(fixPath, pathEncvDirBuff);
            strcat(fixPath, pathEncryptedBuff);
        }
    }
    else
    {
        strcpy(fixPath, path);
    }
    if (strcmp(path, "/") == 0)
    {
        sprintf(fpath, "%s", dirpath);
    }
    else
    {
        sprintf(fpath, "%s%s", dirpath, fixPath);
    }
}

void logFile(char *level, char *cmd, int res, int lenDesc, const char *desc[])
{
    FILE *f = fopen(logpath, "a");
    time_t t;
    struct tm *tmp;
    char timeBuff[100];

    time(&t);
    tmp = localtime(&t);
    strftime(timeBuff, sizeof(timeBuff), "%d%m%Y-%H:%M:%S", tmp);

    fprintf(f, "%s::%s::%s::%d", level, timeBuff, cmd, res);
    for (int i = 0; i < lenDesc; i++)
    {
        fprintf(f, "::%s", desc[i]);
    }
    fprintf(f, "\n");

    fclose(f);
}

void logFile1(char *pathasal, char *pathakhir)
{
    char fileExt[300], fileExt1[300];
    memset(fileExt, 0, sizeof(fileExt));

    strcpy(fileExt, FileExtension(pathasal));
    strcpy(fileExt1, FileExtension(pathakhir));

    FILE *f = fopen(logpath1, "a");
    // fprintf(f, "/home/adr01/Downloads/%s -> /home/adr01/Downloads/%s\n", fileExt, fileExt1);
    fprintf(f, "/home/oni/Downloads/%s -> /home/oni/Downloads/%s\n", fileExt, fileExt1);
    fclose(f);
}

static int _getattr(const char *path, struct stat *stbuf)
{
    char fpath[1000];
    changePath(fpath, path, 0, 1);

    if (access(fpath, F_OK) == -1)
    {
        memset(fpath, 0, sizeof(fpath));
        changePath(fpath, path, 0, 0);
    }

    int res;

    res = lstat(fpath, stbuf);

    const char *desc[] = {path};
    logFile("INFO", "GETATTR", res, 1, desc);

    if (res == -1)
        return -errno;

    return 0;
}

static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    changePath(fpath, path, 0, 0);

    DIR *dp;
    struct dirent *de;

    (void)offset;
    (void)fi;

    dp = opendir(fpath);
    if (dp == NULL)
    {
        const char *desc[] = {path};
        logFile("INFO", "READDIR", -1, 1, desc);
        return -errno;
    }

    while ((de = readdir(dp)) != NULL)
    {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
            continue;

        if (strstr(path, "/AtoZ_") != NULL)
        {
            char encryptThis[1000];
            strcpy(encryptThis, de->d_name);
            if (de->d_type == DT_REG)
            {
                char *whereIsExtension = strrchr(encryptThis, '.');
                if (whereIsExtension - encryptThis < 1)
                {
                    decrypt(encryptThis, 1);
                }
                else
                {
                    char pathFileBuff[1000];
                    char pathExtBuff[1000];
                    strcpy(pathExtBuff, whereIsExtension);
                    snprintf(pathFileBuff, whereIsExtension - encryptThis + 1, "%s", encryptThis);
                    decrypt(pathFileBuff, 1);
                    memset(encryptThis, 0, sizeof(encryptThis));
                    strcat(encryptThis, pathFileBuff);
                    strcat(encryptThis, pathExtBuff);
                }
            }
            else if (de->d_type == DT_DIR)
            {
                decrypt(encryptThis, 1);
            }

            if (filler(buf, encryptThis, &st, 0))
                break;
        }

        else
        {
            if (filler(buf, de->d_name, &st, 0))
                break;
        }
    }

    const char *desc[] = {path};
    logFile("INFO", "READDIR", 0, 1, desc);

    closedir(dp);
    return 0;
}

static int _mkdir(const char *path, mode_t mode)
{
    char fpath[1000];
    changePath(fpath, path, 1, 0);

    char *ptr = strrchr(path, '/');
    char *filePtr = strstr(ptr, "/AtoZ_");
    if (filePtr != NULL)
    {
        if (filePtr - ptr == 0)
        {
            const char *desc[] = {path};
            logFile("INFO", "MKDIR", 0, 1, desc);
        }
    }

    int res;

    res = mkdir(fpath, mode);

    char *temp = strstr(fpath, "/AtoZ_");
    if (temp != NULL)
    {
        logFile1(fpath, fpath);
    }

    if (res == -1)
        return -errno;

    return 0;
}

static int _unlink(const char *path)
{
    char fpath[1000];
    changePath(fpath, path, 0, 1);

    int res;

    res = unlink(fpath);

    const char *desc[] = {path};
    logFile("WARNING", "UNLINK", res, 1, desc);

    if (res == -1)
        return -errno;

    return 0;
}

static int _rmdir(const char *path)
{
    char fpath[1000];
    changePath(fpath, path, 0, 0);
    int res;

    res = rmdir(fpath);

    const char *desc[] = {path};
    logFile("WARNING", "RMDIR", res, 1, desc);

    if (res == -1)
        return -errno;

    return 0;
}

static int _rename(const char *from, const char *to)
{
    char ffrom[1000];
    char fto[1000];
    changePath(ffrom, from, 0, 1);
    changePath(fto, to, 0, 1);

    if (access(ffrom, F_OK) == -1)
    {
        memset(ffrom, 0, sizeof(ffrom));
        changePath(ffrom, from, 0, 0);
    }

    if (access(fto, F_OK) == -1)
    {
        memset(fto, 0, sizeof(fto));
        changePath(fto, to, 0, 0);
    }

    char *toPtr = strrchr(to, '/');
    char *toStartPtr = strstr(toPtr, "/AtoZ_");

    if (toStartPtr != NULL)
    {
        if (toStartPtr - toPtr == 0)
        {
            const char *desc[] = {fto};
            logFile("INFO", "RENAME", 0, 1, desc);
        }
    }

    int res;

    res = rename(ffrom, fto);

    const char *desc[] = {from, to};
    logFile("INFO", "RENAME", res, 2, desc);

    char *temp = strstr(fto, "/AtoZ_");
    if (temp != NULL)
    {
        logFile1(ffrom, fto);
    }

    if (res == -1)
        return -errno;

    return 0;
}

static int _utimens(const char *path, const struct timespec ts[2])
{
    char fpath[1000];
    changePath(fpath, path, 0, 1);

    if (access(fpath, F_OK) == -1)
    {
        memset(fpath, 0, sizeof(fpath));
        changePath(fpath, path, 0, 0);
    }

    int res;

    /* don't use utime/utimes since they follow symlinks */
    res = utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW);

    const char *desc[] = {path};
    logFile("INFO", "UTIMENSAT", res, 1, desc);

    if (res == -1)
        return -errno;

    return 0;
}

static int _read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    changePath(fpath, path, 0, 1);

    int fd;
    int res;

    if (fi == NULL)
        fd = open(fpath, O_RDONLY);
    else
        fd = fi->fh;

    if (fd == -1)
        return -errno;

    res = pread(fd, buf, size, offset);

    const char *desc[] = {path};
    logFile("INFO", "READ", res, 1, desc);

    if (res == -1)
        res = -errno;

    if (fi == NULL)
        close(fd);
    return res;
}

static int _statfs(const char *path, struct statvfs *stbuf)
{
    char fpath[1000];
    changePath(fpath, path, 0, 1);
    int res;

    res = statvfs(fpath, stbuf);

    const char *desc[] = {path};
    logFile("INFO", "STATFS", res, 1, desc);

    if (res == -1)
        return -errno;

    return 0;
}

static int _open(const char *path, struct fuse_file_info *fi)
{
    char fpath[1000];
    changePath(fpath, path, 0, 1);

    int res;

    res = open(fpath, fi->flags);

    const char *desc[] = {path};
    logFile("INFO", "OPEN", res, 1, desc);

    if (res == -1)
        return -errno;

    fi->fh = res;
    return 0;
}

static int _create(const char *path, mode_t mode,
                   struct fuse_file_info *fi)
{
    char fpath[1000];
    changePath(fpath, path, 1, 0);

    int res;

    res = open(fpath, fi->flags, mode);

    const char *desc[] = {path};
    logFile("INFO", "CREATE", res, 1, desc);

    if (res == -1)
        return -errno;

    fi->fh = res;
    return 0;
}

static int _write(const char *path, const char *buf, size_t size,
                  off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    changePath(fpath, path, 1, 0);

    int fd;
    int res;

    (void)fi;
    if (fi == NULL)
        fd = open(fpath, O_WRONLY);
    else
        fd = fi->fh;

    if (fd == -1)
        return -errno;

    res = pwrite(fd, buf, size, offset);

    const char *desc[] = {path};
    logFile("INFO", "WRITE", res, 1, desc);

    if (res == -1)
        res = -errno;

    if (fi == NULL)
        close(fd);
    return res;
}

static int _truncate(const char *path, off_t size)
{
    char fpath[1000];
    changePath(fpath, path, 0, 1);
    if (access(fpath, F_OK) == -1)
    {
        memset(fpath, 0, sizeof(fpath));
        changePath(fpath, path, 0, 0);
    }

    int res;

    res = truncate(fpath, size);

    const char *desc[] = {path};
    logFile("INFO", "TRUNCATE", res, 1, desc);

    if (res == -1)
        return -errno;

    return 0;
}

static const struct fuse_operations _oper = {
    .getattr = _getattr,
    .readdir = _readdir,
    .mkdir = _mkdir,
    .unlink = _unlink,
    .rmdir = _rmdir,
    .rename = _rename,
    .utimens = _utimens,
    .read = _read,
    .write = _write,
    .statfs = _statfs,
    .open = _open,
    .create = _create,
    .truncate = _truncate,
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &_oper, NULL);
}