// #pragma once
#ifndef NOISY_CHANNEL
#define NOISY_CHANNEL
//#define _CRT_SECURE_NO_WARNINGS
//#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include "../../utils/utils/utils.h"
#include <time.h>
#include <io.h>
#include <stdio.h>
#include <sys/types.h>

#define RANDOMIZE "-r"
#define DETERMINISTIC "-d"
#define random_double() (rand() / RAND_MAX)

typedef struct Noise {
    str type;
    int n;
    int seed;
    double probability;
    int flipped;
    int transmitted;
} Noise;
typedef Noise* Noise_p;

/*
    set noise with proper parameters according to user input
*/
void generate_noise(Noise_p noise, str noise_type, int a1, int a2);

/*
    apply determinstic noise on transmitted data
    Called by apply_noise(...)
*/
void apply_deterministic(Noise_p noise, str data, int size, int verbose);

/*
    apply randomize noise on transmitted data
    Called by apply_noise(...)
*/
void apply_randomized(Noise_p noise, str data, int size, int verbose);

/*
    apply noise on incoming data, according to noise model
*/
void apply_noise(Noise_p noise, str data, int size, int verbose);

#endif