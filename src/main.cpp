#include <iostream>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include "process.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║   SYSTÈME VIDÉO IPC - LECTURE TEMPS RÉEL                     ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <dossier_images> <fichier_audio>\n";
        cout << "Exemple: " << argv[0] << " ./frames ./audio.wav\n\n";
        return 1;
    }
    
    const char* videoDir = argv[1];
    const char* audioFile = argv[2];
    
    cout << "[CONFIG] Dossier vidéo: " << videoDir << "\n";
    cout << "[CONFIG] Fichier audio: " << audioFile << "\n\n" << flush;
    
    // Créer mémoire partagée
    SharedData* shared = (SharedData*)mmap(NULL, sizeof(SharedData),
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_ANONYMOUS,
                                        -1, 0);
    
    if (shared == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }
    
    shared->init();
    shared->config.init(videoDir, audioFile);
    
    cout << "[MAIN PID " << getpid() << "] Mémoire partagée initialisée\n" 
        << flush;
    
    // Créer les processus
    pid_t pid_emetteur = fork();
    if (pid_emetteur == 0) {
        processEmetteur(shared);
        exit(0);
    }
    if (pid_emetteur < 0) {
        cout << "Erreur fork émetteur\n";
        exit(1);
    }
    
    pid_t pid_recepteur = fork();
    if (pid_recepteur == 0) {
        processRecepteur(shared);
        exit(0);
    }
    if (pid_recepteur < 0) {
        cout << "Erreur fork récepteur\n";
        exit(1);
    }
    
    pid_t pid_afficheur = fork();
    if (pid_afficheur == 0) {
        processAfficheur(shared);
        exit(0);
    }
    if (pid_afficheur < 0) {
        cout << "Erreur fork afficheur\n";
        exit(1);
    }
    
    // Parent: afficher les PIDs
    cout << "[MAIN PID " << getpid() << "] 3 processus créés:\n";
    cout << "  - Émetteur: PID " << pid_emetteur << "\n";
    cout << "  - Récepteur: PID " << pid_recepteur << "\n";
    cout << "  - Afficheur: PID " << pid_afficheur << "\n\n" << flush;
    
    // Attendre tous les processus
    waitpid(pid_emetteur, nullptr, 0);
    cout << "[MAIN] Émetteur terminé\n" << flush;
    
    waitpid(pid_recepteur, nullptr, 0);
    cout << "[MAIN] Récepteur terminé\n" << flush;
    
    waitpid(pid_afficheur, nullptr, 0);
    cout << "[MAIN] Afficheur terminé\n" << flush;
    
    // Nettoyer
    shared->destroy();
    munmap(shared, sizeof(SharedData));
    
    cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║              SYSTÈME TERMINÉ AVEC SUCCÈS                     ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}