#ifndef __FOLDER__
#define __FOLDER__

/*
** function to create all folders recursively in give dir
*/
void recursivemkdir(const char* dir);

/*
** function to count how many files are in a folder, recusively
*/
void countFiles(char* basePath, int* count);

/*
** Lists all files recursively at given path. Returns a list of files in files,
*/
void listFiles(char* basePath, int* current, char** files);

/*
** check if is a file
*/
bool regularFile(const char* path);

/*
** check if is a directory
*/
bool regularDirectory(const char* path);

bool fileExists(const char* path);
bool directoryExists(const char* path);

/*
** return the number of characters that match at the beginning of two strings
*/
int matchingCount(const char* str1, const char* str2);

/*
** adds a '/' to a folder if not already present
*/
void slashFolder(char* folder);

#endif