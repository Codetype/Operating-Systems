/*
* Paweł Gędłek
* LAB2 - ZAD2
*
*/

#define __USE_XOPEN
#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <stdlib.h>
#include <ftw.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <dirent.h>
#include <string.h>


char* path;
char* operant;
char* date;

char* getAccess(const struct stat *buff) {
    char* arrayOfPerm = malloc(sizeof(char) * 11);
    //is directory
    arrayOfPerm[0] = S_ISDIR(buff->st_mode) ? 'd' : '-';
    //user permission
    arrayOfPerm[1] = (buff->st_mode & S_IRUSR) ? 'r' : '-';
    arrayOfPerm[2] = (buff->st_mode & S_IWUSR) ? 'w' : '-';
    arrayOfPerm[3] = (buff->st_mode & S_IXUSR) ? 'x' : '-';
    //group permission
    arrayOfPerm[4] = (buff->st_mode & S_IRGRP) ? 'r' : '-';
    arrayOfPerm[5] = (buff->st_mode & S_IWGRP) ? 'w' : '-';
    arrayOfPerm[6] = (buff->st_mode & S_IXGRP) ? 'x' : '-';
    //other permission
    arrayOfPerm[7] = (buff->st_mode & S_IROTH) ? 'r' : '-';
    arrayOfPerm[8] = (buff->st_mode & S_IWOTH) ? 'w' : '-';
    arrayOfPerm[9] = (buff->st_mode & S_IXOTH) ? 'x' : '-';
    arrayOfPerm[10] = '\0';
    return arrayOfPerm;
}

void dir_way(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *), int fd_limit, int flags){
  DIR* trigger = opendir(path);

  struct dirent* current = readdir(trigger);
  while(current!=NULL){
       int pthLength = 0;
       pthLength = pthLength + strlen(path);
       pthLength = pthLength + strlen(current->d_name);
       char * pathTmp = calloc(pthLength+2,sizeof(char));
       /*give pointer from one string to other*/
       strcat(strcat (strcat(pathTmp,path),"/"), current->d_name);

       if(strcmp(current->d_name, "..") == 0 || strcmp(current->d_name, ".") == 0) {
               current = readdir(trigger);
               continue;
           }
      /*recursion call while we meet directory*/
       if(current->d_type == DT_DIR) dir_way(pathTmp,*fn,fd_limit,flags);
       if(current->d_type == DT_REG){
           struct stat* stat_buffer = malloc(sizeof(struct stat));
           /**save data from given pathname to buffor*/
           stat(pathTmp, stat_buffer);
          /*nftw function to work with FTW structure */
           fn(pathTmp,stat_buffer,FTW_F,NULL);
       }
       current = readdir(trigger);

     }
}

void printResults(const struct stat* buff, char* abs_path) {
    char* arrayOfPerm =  getAccess(buff);
    printf("Absolute path:\t%s\nSize of element:\t%lld B\nPermission:\t%s\nLast modified: %s \n",
    abs_path, (long long int)buff->st_size, arrayOfPerm, ctime(&(buff->st_mtime)));
    free(arrayOfPerm);
}

int compareDates(char* user_date, char* operant, time_t file_date){
    struct tm* dttm = malloc(sizeof(struct tm));

    /**structure to convert string to tm structure.*/
    strptime(user_date, "%Y-%m-%d %H:%M:%S", dttm);

    time_t mctime = mktime(dttm);
    if(operant[0]=='='){
        return difftime(mctime,file_date) == 0 ? 1 : 0;
    } else if(operant[0]=='>'){
        return difftime(mctime,file_date) > 0 ? 1 : 0;
    } else if(operant[0]=='<'){
        return difftime(mctime,file_date) < 0 ? 1 :0;
    } else {
        printf("It is incorrect operant!\n");
        return 1;
    }
}

int elim(const char *fpath,const struct stat *sb, int typeflag, struct FTW *ftwbuf){

    char * abspath = calloc(256,sizeof(char));
    /**return cannonicalized absolute pathname*/
    realpath(fpath,abspath);

    if(typeflag == FTW_F && compareDates(date,operant,sb->st_mtime)){
        printResults(sb,abspath);
    }
    free(abspath);
    return 0;
}

/*
*
* Main function
*
*/
int main(int argc,char* argv[]){
    if(argc < 4){
      printf("Too few arguments!\nYou have to set out 3 arguments: dirpath, operator of comparison, date");
      return 1;
    }
    path = argv[1]; operant = argv[2]; date = argv[3];
    DIR* handle = opendir(path);
    if(handle==NULL){
        printf("Wrong directory path!\n");
        return 1;
    } if(!(strcmp(argv[2],"=")==0 || strcmp(argv[2],">") || strcmp(argv[2],"<"))){
        printf("Wrong operant of comparison.\n");
        return 1;
    }
    if(argc == 5){
      if((strcmp(argv[4],"nftw")==0))
        printf("NFTW version:\n");
        nftw(path,&elim,5,FTW_F);
    }
    else{
      printf("stat_ version:\n");
      dir_way(path, &elim, 5, FTW_F);
    }
    return 0;
}
