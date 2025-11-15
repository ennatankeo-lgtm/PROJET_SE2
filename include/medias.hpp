#ifndef MEDIAS_H
#define MEDIAS_H

#include "structures.hpp"
#include<vector>
#include<string>

// Fonction pour lire les images du dossier
std::vector<std::string> getImageFiles(const char* dirPath) ;
// Fonction pour lire une image
bool readImageFile(const char* filepath, VideoFrame& frame) ;
// Fonction pour découper l'audio en chunks
bool readAudioChunk(const char* filepath, int chunkIndex, int totalChunks, AudioFrame& frame) ;
void displayFrameInPlace(const VideoFrame& vFrame , const AudioFrame& aFrame, int frameNum, int totalFrames);

// Variables globales pour l'affichage
extern pid_t audioPlayerPid;
extern pid_t imageViewerPid;

#endif