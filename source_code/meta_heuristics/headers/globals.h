#ifndef GLOBALS_H
#define GLOBALS_H

const int SIZE_OF_POPULATION = 100;

// log da STN: registra só 1 em cada STN_LOGGER_INTERVAL gerações, para os
// STN_LOGGER_NUM_VECTORS vetores de peso próprios da STN -- não os
// SIZE_OF_POPULATION vetores internos do MOEA/D
const int STN_LOGGER_INTERVAL = 50;
const int STN_LOGGER_NUM_VECTORS = 10;

#endif 
