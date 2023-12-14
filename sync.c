#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_PATH_LENGTH 4096
#define LOG_FILE "sync_log.txt"

void synchronizeDirectories(const char *sourcePath, const char *replicaPath);
void copyFiles(const char *sourcePath, const char *replicaPath);
void logOperation(const char *operation, const char *filePath);

void synchronizeDirectories(const char *sourcePath, const char *replicaPath){
    DIR  *sourceDir, *replicaDir;
    struct dirent *sourceEntry, *replicaEntry;

    sourceDir = opendir(sourcePath);
    replicaDir = opendir(replicaPath);

    if(sourceDir == NULL || replicaDir == NULL){
        printf("Error opening directories\n");
        return;
    }
    
    while ((sourceEntry = readdir(sourceDir)) != NULL)
    {
        if (strcmp(sourceEntry->d_name, ".") != 0 && strcmp(sourceEntry->d_name, "..") != 0){
            char sourceFilePath[MAX_PATH_LENGTH];
            char replicaFilePath[MAX_PATH_LENGTH];

            snprintf(sourceFilePath, MAX_PATH_LENGTH, "%s/%s", sourcePath, sourceEntry->d_name);
            snprintf(replicaFilePath, MAX_PATH_LENGTH, "%s/%s", replicaPath, sourceEntry->d_name);

            struct stat sourceStat, replicaStat;

            if (stat(sourceFilePath, &sourceStat) == 0) {
                if (S_ISDIR(sourceStat.st_mode)) {
                    // If it's a directory, create it in the replica if it doesn't exist
                    if (stat(replicaFilePath, &replicaStat) != 0) {
                        mkdir(replicaFilePath, sourceStat.st_mode);
                    }

                    // Recursively synchronize subdirectories
                    synchronizeDirectories(sourceFilePath, replicaFilePath);
                } else {
                    // If the file in the source directory doesnt exist in the replica directory
                    if (stat(replicaFilePath, &replicaStat) != 0) {
                        FILE *replicaFile = fopen(replicaFilePath, "w");
                        if (replicaFile != NULL) {
                            logOperation("File Created", replicaFilePath);
                            fclose(replicaFile);

                        } else {
                            perror("Error creating replica file");
                            exit(EXIT_FAILURE);
                        }
                    }
                    logOperation("File Copied", replicaFilePath);
                    //copy the file to the replica
                    copyFiles(sourceFilePath, replicaFilePath);
                }
            }
            else{
               printf("n");
            }   
        }
    }
    closedir(sourceDir);
    closedir(replicaDir);
}



void copyFiles(const char *sourcePath, const char *replicaPath) {
    FILE *sourceFile, *replicaFile;
    char data;

    sourceFile = fopen(sourcePath, "r");
    replicaFile = fopen(replicaPath, "w");

    if (sourceFile == NULL || replicaFile == NULL) {
        printf("Error opening files\n");
        return;
    }
    while ((data = fgetc(sourceFile)) != EOF) {
        fputc(data, replicaFile);
    }
    fclose(sourceFile);
    fclose(replicaFile);
}

void logOperation(const char *operation, const char *filePath) {
    time_t currentTime;
    time(&currentTime);
    struct tm *localTime = localtime(&currentTime);

    FILE *logFile = fopen(LOG_FILE, "a");
    if (logFile != NULL) {
        fprintf(logFile, "[%04d-%02d-%02d %02d:%02d:%02d] %s: %s\n",
                localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
                localTime->tm_hour, localTime->tm_min, localTime->tm_sec,
                operation, filePath);
        printf("[%s] %s\n", operation, filePath);
        fclose(logFile);
    } else {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Provide both directories paths and the time interval you want the directories to be synchronized\n");
        exit(EXIT_FAILURE);
    }

    const char *sourcePath = argv[1];
    const char *replicaPath = argv[2];
    int syncInterval = atoi(argv[3]);

    while (1) {
        logOperation("Program Start", "N/A");

        synchronizeDirectories(sourcePath, replicaPath);

        logOperation("Program End", "N/A");

        sleep(syncInterval);
    }

    return EXIT_SUCCESS;
}                   


// gcc sync.c -o sync_folders
//  ./sync_folders /Users/rafaellourenco/Downloads/Uni/VS/Test1 /Users/rafaellourenco/Downloads/Uni/VS/Test2 100
