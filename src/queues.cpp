#include "queues.hpp"
#include <iostream>
#include <string>
using namespace std;

void VideoQueue::init() {
    
    size = 0;
    finished = false;
        
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    sem_init(&notEmpty, 1, 0);
    sem_init(&notFull, 1, MAX_QUEUE_SIZE);
}
    
bool VideoQueue::pop(VideoFrame& frame) {
    sem_wait(&notEmpty);
    pthread_mutex_lock(&mutex);
    
    if (finished && size == 0) {
        pthread_mutex_unlock(&mutex);
        sem_post(&notEmpty);
        return false;
    }
        
    frame = frames[0];
        
    for (int i = 0; i < size - 1; i++) {
        frames[i] = frames[i + 1];
    }
    size--;
        
    cout << "[VIDEO QUEUE] Frame " << frame.id 
        << " retirée (size=" << size << ")\n" << flush;
        
    pthread_mutex_unlock(&mutex);
    sem_post(&notFull);
    return true;
}
void VideoQueue::push(const VideoFrame& frame){
    sem_wait(&notFull);
    pthread_mutex_lock(&mutex);
    
    frames[size] = frame;
    size++;
    
    cout << "[VIDEO QUEUE] Frame " << frame.id 
        << " (" << frame.filename << ") ajoutée (size=" << size << ")\n" << flush;
    
    pthread_mutex_unlock(&mutex);
    sem_post(&notEmpty);   
}

void VideoQueue::setFinished() {
    pthread_mutex_lock(&mutex);
    finished = true;
    pthread_mutex_unlock(&mutex);
    sem_post(&notEmpty);
}
    
void VideoQueue::destroy() {
    pthread_mutex_destroy(&mutex);
    sem_destroy(&notEmpty);
    sem_destroy(&notFull);
}

void AudioQueue::init() {
    
    size = 0;
    finished = false;
        
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mutex, &attr);
    pthread_mutexattr_destroy(&attr);
        
    sem_init(&notEmpty, 1, 0);
    sem_init(&notFull, 1, MAX_QUEUE_SIZE);
}
    
void AudioQueue::push(const AudioFrame& frame) {
    sem_wait(&notFull);
    pthread_mutex_lock(&mutex);
        
    frames[size] = frame;
    size++;
    
    cout << "[AUDIO QUEUE] Frame " << frame.id 
    << " ajoutée (size=" << size << ")\n" << flush;
        
    pthread_mutex_unlock(&mutex);
    sem_post(&notEmpty);
}
    
bool AudioQueue::pop(AudioFrame& frame) {
    sem_wait(&notEmpty);
    pthread_mutex_lock(&mutex);
        
    if (finished && size == 0) {
        pthread_mutex_unlock(&mutex);
        sem_post(&notEmpty);
        return false;
    }
        
    frame = frames[0];
        
    for (int i = 0; i < size - 1; i++) {
        frames[i] = frames[i + 1];
    }
    size--;
        
    cout << "[AUDIO QUEUE] Frame " << frame.id 
        << " retirée (size=" << size << ")\n" << flush;
        
    pthread_mutex_unlock(&mutex);
    sem_post(&notFull);
    return true;
}
    
void AudioQueue::setFinished() {
    pthread_mutex_lock(&mutex);
    finished = true;
    pthread_mutex_unlock(&mutex);
    sem_post(&notEmpty);
}
    
void AudioQueue::destroy() {
    pthread_mutex_destroy(&mutex);
    sem_destroy(&notEmpty);
    sem_destroy(&notFull);
    }

void SyncBuffer::init() {
    writeIndex = 0;
    readIndex = 0;
    finished = false;
        
    for (int i = 0; i < NUM_FRAMES; i++) {
        pairs[i].valid = false;
    }
        
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mutex, &attr);
    pthread_mutexattr_destroy(&attr);
        
    sem_init(&dataAvailable, 1, 0);
}

void SyncBuffer::pushPair(const VideoFrame& vFrame, const AudioFrame& aFrame) {
    pthread_mutex_lock(&mutex);
        
    if (writeIndex < NUM_FRAMES) {
        pairs[writeIndex].video = vFrame;
        pairs[writeIndex].audio = aFrame;
        pairs[writeIndex].valid = true;
        writeIndex++;
            
        cout << "[SYNC BUFFER] Paire " << vFrame.id << " synchronisée et prête pour l'affichage\n" << flush;
            
        sem_post(&dataAvailable);
    }
        
    pthread_mutex_unlock(&mutex);
}

bool SyncBuffer::popPair(VideoFrame& vFrame, AudioFrame& aFrame) {
    sem_wait(&dataAvailable);
    pthread_mutex_lock(&mutex);
        
    if (readIndex >= NUM_FRAMES || !pairs[readIndex].valid) {
        pthread_mutex_unlock(&mutex);
        return false;
    }
        
    vFrame = pairs[readIndex].video;
    aFrame = pairs[readIndex].audio;
    pairs[readIndex].valid = false;
        
    cout << "[SYNC BUFFER] Paire " << vFrame.id << " récupérée pour affichage\n" << flush;
        
    readIndex++;
        
    pthread_mutex_unlock(&mutex);
    return true;
}

void SyncBuffer::setFinished() {
    pthread_mutex_lock(&mutex);
    finished = true;
    pthread_mutex_unlock(&mutex);
    sem_post(&dataAvailable);
}
    
void SyncBuffer::destroy() {
    pthread_mutex_destroy(&mutex);
    sem_destroy(&dataAvailable);
}

void SharedData::init() {
    videoQueue.init();
    audioQueue.init();
    syncBuffer.init();
}
    
void SharedData::destroy() {
    videoQueue.destroy();
    audioQueue.destroy();
    syncBuffer.destroy();
}