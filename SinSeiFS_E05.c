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

static const char *dirpath = "/home/adr01/Documents";
static const char *key = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
static const char *logpath = "/home/adr01/Documents/fuse_log.txt";

// Dapatkan directory dan nama file
void getDirAndFile(char *dir, char *file, char *path) {
  char buff[1000];
  memset(dir, 0, 1000);
  memset(file, 0, 1000);
  strcpy(buff, path);
  char *token = strtok(buff, "/");
  while(token != NULL) {
    sprintf(file, "%s", token);
    token = strtok(NULL, "/");
    if (token != NULL) {
    	printf("ini token : %s\n", token);
    	strcat(dir,"/");
    	strcat(dir,file);
    }
  }
}

// Untuk enkripsi dan dekripsi
void decrypt(char *path, int isEncrypt) {
  char *cursor = path;
  while (cursor-path < strlen(path)) {
    char *ptr = strchr(key, *cursor);
    if (ptr != NULL) {
      int index = (ptr-key+strlen(key)-10)%strlen(key);
      if (isEncrypt) index = (ptr-key+strlen(key)+10)%strlen(key);
      *cursor = *(key+index);
    }
    cursor++;
  }
}

void nextSync(char *syncDirPath) {
  char buff[1000];
  char construct[1000];
  memset(construct, 0, sizeof(construct));
  strcpy(buff, syncDirPath);
  int state = 0;
  char *token = strtok(buff, "/");
  while (token != NULL) {
    char tBuff[1000];
    char get[1000];
    strcpy(tBuff, token);
    if (!state) {
      if (strstr(tBuff, "sync_")-tBuff == 0) {
        strcpy(get, tBuff+5);
      } else {
        strcat(get, "sync_");
        strcat(get, tBuff);
        state = 1;
      }
    } else {
      strcpy(get, tBuff);
    }
    strcat(construct, get);

    token = strtok(NULL, "/");
  }
  memset(syncDirPath, 0, 1000);
  sprintf(syncDirPath, "%s", construct);
}

// Untuk 
void changePath(char *fpath, const char *path, int isWriteOper, int isFileAsked) {
  char *ptr = strstr(path, "/encv1_");
  int state = 0;
  if (ptr != NULL) {
    if (strstr(ptr+1, "/") != NULL) state = 1;
  }
  char fixPath[1000];
  memset(fixPath, 0, sizeof(fixPath));
  if (ptr != NULL && state) {
    ptr = strstr(ptr+1, "/");
    char pathEncvDirBuff[1000];
    char pathEncryptedBuff[1000];
    strcpy(pathEncryptedBuff, ptr);
    strncpy(pathEncvDirBuff, path, ptr-path);
    if (isWriteOper) {
      char pathFileBuff[1000];
      char pathDirBuff[1000];
      getDirAndFile(pathDirBuff, pathFileBuff, pathEncryptedBuff);
      decrypt(pathDirBuff, 0);
      
      strcat(fixPath, pathEncvDirBuff);
      strcat(fixPath,pathDirBuff);
      strcat(fixPath, "/");
      strcat(fixPath, pathFileBuff);
    } else if (isFileAsked) {
      char pathFileBuff[1000];
      char pathDirBuff[1000];
      char pathExtBuff[1000];
      getDirAndFile(pathDirBuff, pathFileBuff, pathEncryptedBuff);
      char *whereIsExtension = strrchr(pathFileBuff, '.');
      if (whereIsExtension-pathFileBuff < 1) {
        decrypt(pathDirBuff, 0);
        decrypt(pathFileBuff, 0);
        strcat(fixPath, pathEncvDirBuff);
        strcat(fixPath,pathDirBuff);
        strcat(fixPath, "/");
        strcat(fixPath, pathFileBuff);
      } else {
        char pathJustFileBuff[1000];
        memset(pathJustFileBuff, 0, sizeof(pathJustFileBuff));
        strcpy(pathExtBuff, whereIsExtension);
        snprintf(pathJustFileBuff, whereIsExtension-pathFileBuff+1, "%s", pathFileBuff);
        decrypt(pathDirBuff, 0);
        decrypt(pathJustFileBuff, 0);
        
        strcat(fixPath, pathEncvDirBuff);
        strcat(fixPath,pathDirBuff);
        strcat(fixPath, "/");
        strcat(fixPath, pathJustFileBuff);
        strcat(fixPath, pathExtBuff);
      }
    } else {
      decrypt(pathEncryptedBuff, 0);
      strcat(fixPath, pathEncvDirBuff);
      strcat(fixPath,pathEncryptedBuff);
    }
  } else {
    strcpy(fixPath, path);
  }
  if (strcmp(path, "/") == 0) {
    sprintf(fpath, "%s", dirpath);
  } else {
    sprintf(fpath, "%s%s", dirpath, fixPath);
  }
}

void splitter(char *path) {
  DIR *dp;
  struct dirent *de;
  dp = opendir(path);
  while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    char newPath[1000];
    sprintf(newPath, "%s/%s", path, de->d_name);
    if (de->d_type == DT_DIR) {
      splitter(newPath);
    }
    if (de->d_type == DT_REG) {
      FILE *pathFilePointer = fopen(newPath, "rb");
      fseek(pathFilePointer, 0, SEEK_END);
      long pathFileSize = ftell(pathFilePointer);
      rewind(pathFilePointer);
      struct stat st;
      lstat(newPath, &st);
      int index = 0;
      while(pathFileSize > 0) {
        char newFileSplit[1000];
        char buff[1024];
        //sprintf(newFileSplit, "%s.%03d", newPath, index++);
        
        sprintf(newFileSplit, "%s", newPath);
        sprintf(newFileSplit, ".%03d", index++);
        close(creat(newFileSplit, st.st_mode));
        int size;
        if (pathFileSize >= 1024) {
          size = 1024;
          pathFileSize-=1024;
        } else {
          size = pathFileSize;
          pathFileSize = 0;
        }
        memset(buff, 0, sizeof(buff));
        fread(buff, 1, size, pathFilePointer);

        FILE *pathFileOut = fopen(newFileSplit, "wb");
        fwrite(buff, 1, size, pathFileOut);
        fclose(pathFileOut);
      }
      fclose(pathFilePointer);
      unlink(newPath);
    }
  }
}

void unsplitter(char *path) {
  DIR *dp;
  struct dirent *de;
  dp = opendir(path);
  while ((de = readdir(dp)) != NULL) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    if (de->d_type == DT_DIR) {
      char newPath[1000];
      sprintf(newPath, "%s/%s", path, de->d_name);
      unsplitter(newPath);
    }
    if (de->d_type == DT_REG) {
      char *ptr = strrchr(de->d_name, '.');
      if (strcmp(ptr, ".000") != 0) continue;
      char pathFileName[1000];
      char pathToFile[1000];
      snprintf(pathFileName, ptr-(de->d_name)+1, "%s", de->d_name);

      strcat(pathToFile, path);
      strcat(pathToFile, "/");
      strcat(pathToFile, pathFileName);

      char temp[1000];
      
      sprintf(temp, "%s", pathToFile);
      strcat(temp, ".000");
        
      struct stat st;
      lstat(temp, &st);
      close(creat(pathToFile, st.st_mode));

      int index = 0;
      FILE *pathFilePointer = fopen(pathToFile, "wb");
      while (1) {
        char partName[1000];
        
        sprintf(partName, "%s", pathToFile);
        sprintf(partName, ".%03d", index++);
        
        if (access(partName, F_OK) == -1) break;
        FILE *pathToPart = fopen(partName, "rb");
        fseek(pathToPart, 0, SEEK_END);
        long pathFileSize = ftell(pathToPart);
        rewind(pathToPart);
        char buff[1024];
        fread(buff, 1, pathFileSize, pathToPart);
        fwrite(buff, 1, pathFileSize, pathFilePointer);
        fclose(pathToPart);
        unlink(partName);
      }
      fclose(pathFilePointer);
    }
  }
}

void logFile(char *level, char *cmd, int res, int lenDesc, const char *desc[]) {
  FILE *f = fopen(logpath, "a");
  time_t t;
  struct tm *tmp;
  char timeBuff[100];

  time(&t);
  tmp = localtime(&t);
  strftime(timeBuff, sizeof(timeBuff), "%y%m%d-%H:%M:%S", tmp);

  fprintf(f, "%s::%s::%s::%d", level, timeBuff, cmd, res);
  for (int i = 0; i < lenDesc; i++) {
    fprintf(f, "::%s", desc[i]);
  }
  fprintf(f, "\n");

  fclose(f);
}

static int _getattr(const char *path, struct stat *stbuf)
{

	char fpath[1000];
  changePath(fpath, path, 0, 1);
  if (access(fpath, F_OK) == -1) {
    memset(fpath, 0, sizeof(fpath));
    changePath(fpath, path, 0, 0);
  }

	int res;

	res = lstat(fpath, stbuf);

  const char *desc[] = {path};
  logFile("INFO", "GETATTR", res, 1, desc);

	if (res == -1) return -errno;


	return 0;
}

static int _access(const char *path, int mask)
{
printf("proses _access\n");
	char fpath[1000];
	changePath(fpath, path, 0, 1);
  if (access(fpath, F_OK) == -1) {
    memset(fpath, 0, 1000);
    changePath(fpath, path, 0, 0);
  }

	int res;

	res = access(fpath, mask);

  const char *desc[] = {path};
  logFile("INFO", "ACCESS", res, 1, desc);

	if (res == -1) return -errno;


	return 0;
}

static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	changePath(fpath, path, 0, 0);

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL) {
    const char *desc[] = {path};
    logFile("INFO", "READDIR", -1, 1, desc);
    return -errno;
  }

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    if (strstr(path, "/encv1_") != NULL) {
      char encryptThis[1000];
      strcpy(encryptThis, de->d_name);
      if (de->d_type == DT_REG) {
        char *whereIsExtension = strrchr(encryptThis, '.');
        if (whereIsExtension-encryptThis < 1) {
          decrypt(encryptThis, 1);
        } else {
          char pathFileBuff[1000];
          char pathExtBuff[1000];
          strcpy(pathExtBuff, whereIsExtension);
          snprintf(pathFileBuff, whereIsExtension-encryptThis+1, "%s", encryptThis);
          decrypt(pathFileBuff, 1);
          memset(encryptThis, 0, sizeof(encryptThis));
          strcat(encryptThis, pathFileBuff);
          strcat(encryptThis, pathExtBuff);
        }
      } else if (de->d_type == DT_DIR) {
        decrypt(encryptThis, 1);
      }

  		if (filler(buf, encryptThis, &st, 0)) break;
    } else {
      if (filler(buf, de->d_name, &st, 0)) break;
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
  char *filePtr = strstr(ptr, "/encv1_");
  if (filePtr != NULL) {
    if (filePtr - ptr == 0) {
      const char *desc[] = {path};
      logFile("SPECIAL", "ENCV1", 0, 1, desc);
    }
  }

	int res;

	res = mkdir(fpath, mode);

  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, sizeof(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, sizeof(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    memset(syncPath, 0, sizeof(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath, 1, 0);
    if (access(syncPath, F_OK) == -1) continue;
    strcat(syncPath, syncFilePath);
    mkdir(syncPath, mode);
  } while (1);


  const char *desc[] = {path};
  logFile("INFO", "MKDIR", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _rmdir(const char *path)
{
printf("proses _rmdir\n");
	char fpath[1000];
	changePath(fpath, path, 0, 0);
	int res;

	res = rmdir(fpath);

  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, sizeof(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, sizeof(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    memset(syncPath, 0, sizeof(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath, 0, 0);
    if (access(syncPath, F_OK) == -1) continue;
    strcat(syncPath, syncFilePath);
    rmdir(syncPath);
  } while (1);

  const char *desc[] = {path};
  logFile("WARNING", "RMDIR", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static int _rename(const char *from, const char *to)
{
printf("proses _rename\n");
	char ffrom[1000];
	char fto[1000];
	changePath(ffrom, from, 0, 1);
	changePath(fto, to, 0, 1);
  if (access(ffrom, F_OK) == -1) {
    memset(ffrom, 0, sizeof(ffrom));
    changePath(ffrom, from, 0, 0);
  }
  if (access(fto, F_OK) == -1) {
    memset(fto, 0, sizeof(fto));
    changePath(fto, to, 0, 0);
  }

  char *toPtr = strrchr(to, '/');
  char *fromPtr = strrchr(from ,'/');
  char *toStartPtr = strstr(toPtr, "/encv2_");
  char *fromStartPtr = strstr(fromPtr, "/encv2_");
  if (toStartPtr != NULL) {
    if (toStartPtr - toPtr == 0) {
      DIR *d = opendir(ffrom);
      if (d) {
        closedir(d);
        splitter(ffrom);
        const char *desc[] = {fto};
        logFile("SPECIAL", "ENCV2", 0, 1, desc);
      }
    }
  }
  if (fromStartPtr != NULL) {
    if (fromStartPtr - fromPtr == 0) {
      DIR *d = opendir(ffrom);
      if (d) {
        closedir(d);
        unsplitter(ffrom);
      }
    }
  }
  toStartPtr = strstr(toPtr, "/encv1_");
  if (toStartPtr != NULL) {
    if (toStartPtr - toPtr == 0) {
      const char *desc[] = {fto};
      logFile("SPECIAL", "ENCV1", 0, 1, desc);
    }
  }

	int res;

	res = rename(ffrom, fto);

  const char *desc[] = {from, to};
  logFile("INFO", "RENAME", res, 2, desc);

	if (res == -1) return -errno;

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

	if (res == -1) return -errno;

	fi->fh = res;
	return 0;
}

static int _read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	changePath(fpath, path, 0, 1);

	int fd;
	int res;

	if(fi == NULL) fd = open(fpath, O_RDONLY);
	else fd = fi->fh;

	if (fd == -1) return -errno;

	res = pread(fd, buf, size, offset);

  const char *desc[] = {path};
  logFile("INFO", "READ", res, 1, desc);

	if (res == -1) res = -errno;

	if(fi == NULL) close(fd);
	return res;
}

static int _write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
printf("proses _write\n");
	char fpath[1000];
	changePath(fpath, path, 1, 0);

	int fd;
	int res;

	(void) fi;
	if(fi == NULL) fd = open(fpath, O_WRONLY);
	else fd = fi->fh;

	if (fd == -1) return -errno;

	res = pwrite(fd, buf, size, offset);

  char syncOrigPath[1000];
  char syncDirPath[1000];
  char syncFilePath[1000];
  memset(syncOrigPath, 0, sizeof(syncOrigPath));
  strcpy(syncOrigPath, path);
  getDirAndFile(syncDirPath, syncFilePath, syncOrigPath);
  memset(syncOrigPath, 0, sizeof(syncOrigPath));
  strcpy(syncOrigPath, syncDirPath);
  do {
    char syncPath[1000];
    int syncFd;
    memset(syncPath, 0, sizeof(syncPath));
    nextSync(syncDirPath);
    if (strcmp(syncDirPath, syncOrigPath) == 0) break;
    changePath(syncPath, syncDirPath, 0, 1);
    if (access(syncPath, F_OK) == -1) continue;
    strcat(syncPath, syncFilePath);
    syncFd = open(syncPath, O_WRONLY);
    if (syncFd == -1) return -errno;
    pwrite(syncFd, buf, size, offset);
    close(syncFd);
  } while (1);

  const char *desc[] = {path};
  logFile("INFO", "WRITE", res, 1, desc);

	if (res == -1) res = -errno;

	if(fi == NULL) close(fd);
	return res;
}

static int _statfs(const char *path, struct statvfs *stbuf)
{
printf("proses _statfs\n");
	char fpath[1000];
	changePath(fpath, path, 0, 1);
	int res;

	res = statvfs(fpath, stbuf);

  const char *desc[] = {path};
  logFile("INFO", "STATFS", res, 1, desc);

	if (res == -1) return -errno;

	return 0;
}

static const struct fuse_operations _oper = {
	.getattr	= _getattr,
	.access		= _access,
	.readdir	= _readdir,
	.mkdir		= _mkdir,
	.rmdir		= _rmdir,
	.rename		= _rename,
	.open		  = _open,
	.read	    = _read,
	.write		= _write,
	.statfs		= _statfs,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &_oper, NULL);
}
