#ifndef GLOBALS_H
#define GLOBALS_H

const int SIZE_OF_POPULATION = 100;

// log da STN: registra só 1 em cada STN_LOGGER_INTERVAL gerações, para os
// STN_LOGGER_NUM_VECTORS vetores de peso próprios da STN -- não os
// SIZE_OF_POPULATION vetores internos do MOEA/D
// Runtime-configurable (CLI args) instead of compile-time constants, so P
// and the sampling interval can be swept without recompiling -- see
// moead.cpp/nsga2.cpp for the defaults and CLI parsing.
extern int STN_LOGGER_INTERVAL;
extern int STN_LOGGER_NUM_VECTORS;

#endif 
