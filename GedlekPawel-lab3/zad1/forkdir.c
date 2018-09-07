/*
*
* PAWEŁ GĘDŁEK
* LAB 3, ZAD 1
*
*/
#include <sys/wait.h>
#include <sys/stat.h>
#define __USE_XOPEN_EXTENDED
#include <ftw.h>
#define __USE_XOPEN
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>

time_t usertime = 0;
typedef struct dirent dirent;
char comp = 0;

bool isAbsolute(const char *dirpth) {
    return dirpth[0] == '/';
}
char *printPermission(mode_t st_mode) {
    char *result = calloc(10, sizeof(char));
    //user permission access
    result[0] = (st_mode & S_IRUSR) ? 'r' : '-';
    result[1] = (st_mode & S_IWUSR) ? 'w' : '-';
    result[2] = (st_mode & S_IXUSR) ? 'x' : '-';
    //group permission access
    result[3] = (st_mode & S_IRGRP) ? 'r' : '-';
    result[4] = (st_mode & S_IWGRP) ? 'w' : '-';
    result[5] = (st_mode & S_IXGRP) ? 'x' : '-';
    //other permission access
    result[6] = (st_mode & S_IROTH) ? 'r' : '-';
    result[7] = (st_mode & S_IWOTH) ? 'w' : '-';
    result[8] = (st_mode & S_IXOTH) ? 'x' : '-';
    result[9] = 0;
    return result;
}
char *joinPaths(const char *pth1, const char *pth2) {
    char *result;
    size_t l1 = strlen(pth1), l2 = strlen(pth2);

    //if is absolute path, we concatenate path2 to our result buffor
    if (isAbsolute(pth2)) {
        result = malloc(l2 + 1);
        strcpy(result, pth2);
    }
    //in other case we concatenate '/'
    else {
        char *slash = "/";
        result = malloc(l1 + l2 + 2);
        if (pth1[l1 - 1] == '/') {
            slash = "";
        }
        //save to result string given arguments
        sprintf(result, "%s%s%s", pth1, slash, pth2);
    }
    return result;
}
void showFileDetails(const char *path, const struct stat *stats) {
    char *absPth = calloc(PATH_MAX + 1, sizeof(char));
    //expands all symbolic links and resolves refernces to /./, /../ and extra /
    if (realpath(path, absPth) == NULL) {
        printf("Error conected with absolute path\n");
    }
    struct timespec mtime = stats->st_mtim;
    time_t time = mtime.tv_sec;
    printf("Absolute path:\t| %s \nPermission: \t| %s\nSize in bytes:\t| %ld B\nModife time:\t| %.*s\n\n",
    absPth, printPermission(stats->st_mode), stats->st_size, (int) strlen(ctime(&time)) - 1, ctime(&time));

    free(absPth);
}
bool timeCompare(const struct stat *stats) {
    time_t fileDays = stats->st_mtim.tv_sec / (3600 * 24);
    time_t userDays = usertime / (3600 * 24);
    long int diff = fileDays - userDays;
    return (comp == '<' && diff < 0) || (comp == '=' && diff == 0) || (comp == '>' && diff > 0);
}
void recurDirPath(const char *path) {
    DIR *dir = opendir(path); dirent *entry = NULL; char *filepth = NULL;
    struct stat *stats = calloc(1, sizeof(struct stat));
    if (dir == NULL) {
        printf("Error during opening dir %s\n", path);
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        filepth = joinPaths(path, entry->d_name);
        //lstat return file status to buffor and for symlinks return symlinks to this file
        lstat(filepth, stats);
        //in case of regular file
        if (S_ISREG(stats->st_mode) && timeCompare(stats)) {
            //show path, permission, size and last modify time
            showFileDetails(filepth, stats);
        }
        free(filepth);
    }
    //reset the position of directory to which stream refers to
    rewinddir(dir);

    while ((entry = readdir(dir)) != NULL) {
        filepth = joinPaths(path, entry->d_name);
        //lstat return file status to buffor and for symlinks return symlinks to this fi
        lstat(filepth, stats);

        //in case of directory, we call recursion
        if (S_ISDIR(stats->st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            //start new process before call recursion
            pid_t pid;
            pid = fork();
            if (pid == 0) {
                recurDirPath(filepth);
                exit(0);
            }
            int status;
            //check children proces status
            wait(&status);
            if(status){
                printf("Error while running new proces.\n");
            }
        }
        free(filepth);
    }
    free(stats); closedir(dir);
}
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Too few arguments! You have to set up 3 arguments.\n");
        printf("<path to file/directory> <comparison operator> <user date>\n");
        return 1;
    }
    const char *path = argv[1];
    comp = argv[2][0];
    if (!(strcmp(argv[2],">") == 0 || strcmp(argv[2],"=") == 0 || strcmp(argv[2],"<") == 0)){
        printf("Argument should be compare operator: '<', '=' or '>'\n");
        return 1;
    }
    struct tm time;
    //strptime which let to check format, it cnverts to broken-down time
    if (strptime(argv[3], "%Y-%m-%d %H:%M:%S", &time) == NULL) {
        printf("Wrong time format!\nExpected: 'YY-mm-dd HH:MM:SS'\n");
        return 1;
    }
    //convert to structure time format
    usertime = mktime(&time);
    if (usertime == -1) exit(EXIT_FAILURE);

    printf("\n------------------------------\nThe list of searching files:\n------------------------------\n");

    recurDirPath(path);

    return 0;
}
