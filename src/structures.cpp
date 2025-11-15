#include <cstring>
#include "structures.hpp"

void MediaConfig::init(const char* vDir, const char* aFile) {
        strncpy(videoDir, vDir, sizeof(videoDir) - 1);
        strncpy(audioFile, aFile, sizeof(audioFile) - 1);
    }