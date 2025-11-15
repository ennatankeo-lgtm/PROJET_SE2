#include <iostream>
#include <unistd.h>

#include "process.hpp"

using namespace std;

// PROCESSUS 2: Récepteur
void processRecepteur(SharedData* shared) {
    cout << "\n[PID " << getpid() << "] === PROCESSUS RÉCEPTEUR DÉMARRÉ ===\n\n"
        << flush;
    
    int frameCount = 0;
    
    while (frameCount< NUM_FRAMES) {
        VideoFrame vFrame;
        AudioFrame aFrame;
        
        bool hasVideo = shared->videoQueue.pop(vFrame);
        if (!hasVideo) break;
        
        bool hasAudio = shared->audioQueue.pop(aFrame);
        if (!hasAudio) break;
        
        frameCount++;
        
        cout << "[RÉCEPTEUR PID " << getpid() << "] Frame " << frameCount
            << " récupérée:\n";
        cout << "  - Vidéo: ID=" << vFrame.id << ", Fichier=" << vFrame.filename 
            << ", Taille=" << vFrame.dataSize << " bytes\n";
        cout << "  - Audio: ID=" << aFrame.id << ", Taille=" << aFrame.dataSize 
            << " bytes\n";
        cout << flush;
        
        shared->syncBuffer.pushPair(vFrame, aFrame);
        
        cout << "[RÉCEPTEUR PID " << getpid() 
            << "] Signal envoyé à l'afficheur\n\n" << flush;
        
        usleep(50000);  // 50ms
    }
    shared->syncBuffer.setFinished();
    cout << "[RÉCEPTEUR PID " << getpid() << "] Terminé ("
        << frameCount << " frames)\n\n" << flush;
}