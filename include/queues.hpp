#ifndef QUEUES_H
#define QUEUES_H

#include "structures.hpp"
using namespace std;


//file d'attente pour les vidéos

struct VideoQueue {
    VideoFrame frames[MAX_QUEUE_SIZE];
    int size;
    pthread_mutex_t mutex;
    sem_t notEmpty;
    sem_t notFull;
    bool finished;

    void init();
    void push(const VideoFrame& frame);
    bool pop(VideoFrame& frame);
    void setFinished() ;
    void destroy() ;

};

struct AudioQueue {
    AudioFrame frames[MAX_QUEUE_SIZE];
    int size;
    pthread_mutex_t mutex;
    sem_t notEmpty;
    sem_t notFull;
    bool finished;

    void init();
    void push(const AudioFrame& frame) ;
    bool pop(AudioFrame& frame) ;
    void setFinished() ;
    void destroy() ;

};

// buffer pour paires vidéo/audio synchronisées
struct SyncBuffer {
    struct FramePair {
        VideoFrame video;
        AudioFrame audio;
        bool valid;
    };
    FramePair pairs[NUM_FRAMES];
    int writeIndex;
    int readIndex;
    pthread_mutex_t mutex;
    sem_t dataAvailable;
    bool finished;
    
    void init();
    void pushPair(const VideoFrame& vFrame , const AudioFrame& aFrame);
    bool popPair(VideoFrame& vFrame , AudioFrame& aFrame);
    void setFinished();
    void destroy();
};
//structure de données partagées
struct SharedData {
    VideoQueue videoQueue;
    AudioQueue audioQueue;
    SyncBuffer syncBuffer;
    MediaConfig config;
    
    void init() ;
    void destroy();
};

#endif