#include <sys/types.h>
#include <dirent.h>
#include <string.h>
// #include <linux/limits.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdbool.h>

#include "folder.h"
#include "../commons/commons.h"
/*
** function to create all folders recursively in give dir
*/
void recursivemkdir(const char *dir) {
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if (tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
        if (*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
}

/*
** function to count how many files are in a folder, recusively
*/
void countFiles(char* basePath, int* count) {
    char path[PATH_MAX];
    struct dirent* dp;
    DIR* dir = opendir(basePath);
    // Unable to open directory stream
    if (!dir)
        return;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // if it is a file
            if (dp->d_type == DT_REG && dp->d_type != DT_LNK) {
                (*count)++;
            }
            else {
                // Construct new path from our base path
                strcpy(path, basePath);
                strcat(path, "/");
                strcat(path, dp->d_name);
                countFiles(path, count);
            }
        }
    }
    closedir(dir);
}

/*
** Lists all files recursively at given path. Returns a list of files in files,
*/
void listFiles(char* basePath, int* current, char** files) {
    char path[PATH_MAX];
    struct dirent* dp;
    DIR* dir = opendir(basePath);
    // Unable to open directory stream
    if (!dir)
        return;
    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // if it is a file
            if (dp->d_type == DT_REG && dp->d_type != DT_LNK) {
                sprintf(files[*current], "%s/%s", basePath, dp->d_name);
                (*current)++;
            }
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, dp->d_name);
            listFiles(path, current, files);
        }
    }
    closedir(dir);
}

bool regularFile(const char *path){
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}
bool regularDirectory(const char *path){
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

bool fileExists(const char *path){
    struct stat   path_stat;   
    return (stat (path, &path_stat) == 0) && S_ISREG(path_stat.st_mode);
}
bool directoryExists(const char *path){
    struct stat   path_stat;   
    return (stat (path, &path_stat) == 0) && S_ISDIR(path_stat.st_mode);
}
int matchingCount(const char* str1, const char* str2){
    int count = 0;
    while ( (str1[count] == str2[count]) && (str1[count]!= '\0') && (str2[count]!= '\0')){
        count++;
    }
    return count;
}
void slashFolder(char * folder){
    int len = strlen(folder);
    if (folder[len-1]!='/'){
        folder[len] = '/';
        folder[len+1] = '\0';
    }
}
