#ifndef PROCESS_H
#define PROCESS_H

#include "queues.hpp"


// Déclarations des processus
void processEmetteur(SharedData* shared);
void processRecepteur(SharedData* shared);
void processAfficheur(SharedData* shared);

#endif