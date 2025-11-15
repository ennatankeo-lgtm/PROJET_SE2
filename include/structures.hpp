#ifndef STRUCTURES_H
#define STRUCTURES_H

#include<ctime>
#include <pthread.h>
#include <semaphore.h>

#include "const.hpp"

#define MAX_FILE_SIZE 2*1024*1024

// Structure pour une frame vidéo
struct VideoFrame {
    int id;
    char filename[256];
    char data[MAX_FILE_SIZE];
    size_t dataSize;
    time_t timestamp;
};

// Structure pour une frame audio
struct AudioFrame {
    int id;
    char data[MAX_FILE_SIZE];
    size_t dataSize; 
    time_t timestamp;
};

// Configuration des chemins
struct MediaConfig {
    char videoDir[512];
    char audioFile[512];
    void init(const char* vDir, const char* aFile) ;
};

#endif