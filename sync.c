#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_PATH_LENGTH 4096

void synchronizeDirectories(const char *sourcePath, const char *replicaPath);
void copyFiles(const char *sourcePath, const char *replicaPath);

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
                            fclose(replicaFile);
                        } else {
                            perror("Error creating replica file");
                            exit(EXIT_FAILURE);
                        }
                    }
                    //copy the file to the replica
                    copyFiles(sourceFilePath, replicaFilePath);
                }
            }
            else{
               // Need to work in possible errors 
               printf("Erro no stat\n");

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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("No directories provided");
        exit(EXIT_FAILURE);
    }

    const char *sourcePath = argv[1];
    const char *replicaPath = argv[2];

    synchronizeDirectories(sourcePath, replicaPath);

    printf("Folders synchronized successfully!\n");

    return EXIT_SUCCESS;
}



// gcc folder.c -o sync_folders
//  ./sync_folders
