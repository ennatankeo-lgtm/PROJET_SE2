#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "process.hpp"
#include "medias.hpp"

using namespace std;

void processAfficheur(SharedData* shared) {
    cout << "\n[PID " << getpid() << "] === PROCESSUS AFFICHEUR DÉMARRÉ ===\n\n"
        << flush;
    
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║          🎬 LECTURE VIDÉO EN TEMPS RÉEL                      ║\n";
    cout << "║  Synchronisation audio/vidéo - 1 frame par seconde           ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    cout << "Préparation de la lecture..." << flush;
    
    sleep(2);
    cout << "\n\n[AFFICHEUR] Démarrage de la lecture...\n\n" << flush;
    
    int frameCount = 0;
    struct timespec frameStart, frameEnd;
    
    for (int i = 0; i < NUM_FRAMES; i++) {
        clock_gettime(CLOCK_MONOTONIC, &frameStart);
        
        VideoFrame vFrame;
        AudioFrame aFrame;
        
        cout << "[AFFICHEUR PID " << getpid() << "] En attente de la frame " << (i+1) << "...\n" << flush;
        
        if (!shared->syncBuffer.popPair(vFrame, aFrame)) {
            cout << "[AFFICHEUR] Impossible de récupérer la frame " << (i+1) << endl;
            break;
        }
        
        frameCount++;
        
        cout << "[AFFICHEUR PID " << getpid() << "] Affichage frame " << frameCount << ":\n";
        cout << "  - Vidéo: " << vFrame.filename << " (" << vFrame.dataSize << " bytes)\n";
        cout << "  - Audio: Chunk " << aFrame.id << " (" << aFrame.dataSize << " bytes)\n" << flush;
        
        // Afficher immédiatement la frame
        displayFrameInPlace(vFrame, aFrame, frameCount, NUM_FRAMES);
        
        cout << "[AFFICHEUR] Frame " << frameCount << " affichée avec succès\n\n" << flush;
        
        // Calculer le temps écoulé et attendre pour maintenir le FPS
        clock_gettime(CLOCK_MONOTONIC, &frameEnd);
        long elapsedUs = (frameEnd.tv_sec - frameStart.tv_sec) * 1000000 +
                        (frameEnd.tv_nsec - frameStart.tv_nsec) / 1000;
        
        long sleepTime = FRAME_DELAY_US - elapsedUs;
        if (sleepTime > 0) {
            usleep(sleepTime);
        }
    }
    
    // Petit délai pour voir la dernière frame
    sleep(2);
    
    // Nettoyage
    if (audioPlayerPid > 0) {
        kill(audioPlayerPid, SIGKILL);
        waitpid(audioPlayerPid, nullptr, 0);
    }
    
    if (imageViewerPid > 0) {
        kill(imageViewerPid, SIGKILL);
        waitpid(imageViewerPid, nullptr, 0);
    }
    
    cout << "\n\033[2J\033[H";  // Clear screen
    cout << "╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║            ✅ FIN DE LA LECTURE                              ║\n";
    cout << "╠══════════════════════════════════════════════════════════════╣\n";
    cout << "║  Total de frames affichées: " << frameCount << "/" << NUM_FRAMES << "                              ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    cout << "[AFFICHEUR PID " << getpid() << "] Terminé ("
        << frameCount << " frames)\n\n" << flush;
}