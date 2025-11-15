#include <dirent.h>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "medias.hpp"
using namespace std;

// Variables globales
pid_t audioPlayerPid = -1;
pid_t imageViewerPid = -1;

vector<string> getImageFiles(const char* dirPath) {
    vector<string> imageFiles;
    DIR* dir = opendir(dirPath);
    
    if (dir == nullptr) {
        cerr << "[ERROR] Impossible d'ouvrir le dossier: " << dirPath << endl;
        return imageFiles;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string filename = entry->d_name;
        
        // Filtrer les fichiers images (jpg, png, bmp)
        if (filename.find(".jpg") != string::npos ||
            filename.find(".jpeg") != string::npos ||
            filename.find(".png") != string::npos ||
            filename.find(".bmp") != string::npos) {
            
            imageFiles.push_back(filename);
        }
    }
    
    closedir(dir);
    sort(imageFiles.begin(), imageFiles.end());
    
    return imageFiles;
}

// Fonction pour lire une image
bool readImageFile(const char* filepath, VideoFrame& frame) {
    ifstream file(filepath, ios::binary);
    
    if (!file.is_open()) {
        return false;
    }
    
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);
    
    if (fileSize > MAX_FILE_SIZE) {
        file.close();
        return false;
    }
    
    file.read(frame.data, fileSize);
    frame.dataSize = fileSize;
    file.close();
    
    return true;
}

// Fonction pour découper l'audio en chunks
bool readAudioChunk(const char* filepath, int chunkIndex, int totalChunks, AudioFrame& frame) {
    ifstream file(filepath, ios::binary);
    
    if (!file.is_open()) {
        return false;
    }
    
    // Obtenir taille totale
    file.seekg(0, ios::end);
    size_t totalSize = file.tellg();
    file.seekg(0, ios::beg);
    
    // Calculer taille par chunk
    size_t chunkSize = totalSize / totalChunks;
    size_t offset = chunkIndex * chunkSize;
    
    // Dernier chunk prend le reste
    if (chunkIndex == totalChunks - 1) {
        chunkSize = totalSize - offset;
    }
    
    if (chunkSize > MAX_FILE_SIZE) {
        file.close();
        return false;
    }
    
    // Lire le chunk
    file.seekg(offset);
    file.read(frame.data, chunkSize);
    frame.dataSize = chunkSize;
    file.close();
    return true;
}

// Fonction pour afficher une frame dans le même cadre
void displayFrameInPlace(const VideoFrame& vFrame, const AudioFrame& aFrame, int frameNum, int totalFrames) {
    // Sauvegarder l'image dans un fichier fixe
    const char* displayFile = "/tmp/video_display_current.jpg";
    
    ofstream outFile(displayFile, ios::binary | ios::trunc);
    if (outFile.is_open()) {
        outFile.write(vFrame.data, vFrame.dataSize);
        outFile.close();
        
        // Pour la première frame, ouvrir le viewer
        if (frameNum == 1) {
            if (imageViewerPid > 0) {
                kill(imageViewerPid, SIGKILL);
                waitpid(imageViewerPid, nullptr, WNOHANG);
            }
            
            imageViewerPid = fork();
            if (imageViewerPid == 0) {
                // Rediriger stderr pour éviter les messages d'erreur
                int devNull = open("/dev/null", O_WRONLY);
                dup2(devNull, STDERR_FILENO);
                close(devNull);
                
                #ifdef __APPLE__
                    execlp("open", "open", displayFile, NULL);
                #elif __linux__
                    // Essayer plusieurs viewers
                    execlp("eog", "eog", displayFile, NULL);
                    execlp("feh", "feh", "-R", "0.1", displayFile, NULL);
                    execlp("display", "display", displayFile, NULL);
                #endif
                exit(0);
            }
            usleep(800000); // Attendre que le viewer s'ouvre
        } else {
            // Pour les frames suivantes, juste mettre à jour le fichier
            // La plupart des viewers rechargent automatiquement
            usleep(50000); // Petit délai pour s'assurer que le fichier est écrit
        }
        
        // Afficher info dans le terminal
        cout << "\r[▶️  LECTURE] Frame " << frameNum << "/" << totalFrames 
            << " | " << vFrame.filename 
            << " | " << vFrame.dataSize << " bytes    " << flush;
    }
    
    // Jouer l'audio correspondant
    char tempAudioFile[512];
    snprintf(tempAudioFile, sizeof(tempAudioFile), "/tmp/audio_chunk_%d.wav", aFrame.id);
    
    ofstream audioOut(tempAudioFile, ios::binary | ios::trunc);
    if (audioOut.is_open()) {
        audioOut.write(aFrame.data, aFrame.dataSize);
        audioOut.close();
        
        // Arrêter le lecteur audio précédent
        if (audioPlayerPid > 0) {
            kill(audioPlayerPid, SIGKILL);
            waitpid(audioPlayerPid, nullptr, WNOHANG);
        }
        
        audioPlayerPid = fork();
        if (audioPlayerPid == 0) {
            // Rediriger stderr
            int devNull = open("/dev/null", O_WRONLY);
            dup2(devNull, STDERR_FILENO);
            dup2(devNull, STDOUT_FILENO);
            close(devNull);
            
            #ifdef __APPLE__
                execlp("afplay", "afplay", tempAudioFile, NULL);
            #elif __linux__
                execlp("aplay", "aplay", "-q", tempAudioFile, NULL);
                execlp("paplay", "paplay", tempAudioFile, NULL);
                execlp("ffplay", "ffplay", "-nodisp", "-autoexit", "-loglevel", "quiet", tempAudioFile, NULL);
            #endif
            exit(0);
        }
    }
}

