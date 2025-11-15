#include <iostream>
#include <unistd.h>
#include <ctime>
#include <cstring>

#include "process.hpp"
#include "medias.hpp"
using namespace std;

// PROCESSUS 1: Émetteur
void processEmetteur(SharedData* shared) {
    cout << "\n[PID " << getpid() << "] === PROCESSUS ÉMETTEUR DÉMARRÉ ===\n\n"
        << flush;
    
    // Lire les fichiers images
    vector<string> imageFiles = getImageFiles(shared->config.videoDir);
    
    if (imageFiles.empty()) {
        cerr << "[ÉMETTEUR] Aucune image trouvée dans " << shared->config.videoDir << endl;
        shared->videoQueue.setFinished();
        shared->audioQueue.setFinished();
        return;
    }
    
    cout << "[ÉMETTEUR] " << imageFiles.size() << " images trouvées\n" << flush;
    
    // Limiter à NUM_FRAMES images
    int numImagesToProcess = min((int)imageFiles.size(), NUM_FRAMES);
    for (int i = 0; i < numImagesToProcess; i++) {
        // Créer frame vidéo avec vraie image
        VideoFrame vFrame;
        vFrame.id = i + 1;
        vFrame.timestamp = time(nullptr);
        
        string imagePath = string(shared->config.videoDir) + "/" + imageFiles[i];
        strncpy(vFrame.filename, imageFiles[i].c_str(), sizeof(vFrame.filename) - 1);
        
        if (readImageFile(imagePath.c_str(), vFrame)) {
            cout << "[ÉMETTEUR] Image chargée: " << imageFiles[i] 
                << " (" << vFrame.dataSize << " bytes)\n" << flush;
            shared->videoQueue.push(vFrame);
        } else {
            cerr << "[ÉMETTEUR] Erreur lecture image: " << imageFiles[i] << endl;
        }
        
        // Créer frame audio (chunk de l'audio)
        AudioFrame aFrame;
        aFrame.id = i + 1;
        aFrame.timestamp = time(nullptr);
        
        if (readAudioChunk(shared->config.audioFile, i, numImagesToProcess, aFrame)) {
            cout << "[ÉMETTEUR] Audio chunk " << (i+1) << " chargé (" 
                << aFrame.dataSize << " bytes)\n" << flush;
            shared->audioQueue.push(aFrame);
        } else {
            cerr << "[ÉMETTEUR] Erreur lecture audio chunk " << (i+1) << endl;
        }
        
        cout << "[ÉMETTEUR PID " << getpid() << "] Envoi frame "
            << (i+1) << "/" << numImagesToProcess << " complété\n\n" << flush;
        usleep(100000);  // 100ms
    }
    
    shared->videoQueue.setFinished();
    shared->audioQueue.setFinished();
    
    cout << "\n[ÉMETTEUR PID " << getpid() << "] Terminé\n\n" << flush;
}