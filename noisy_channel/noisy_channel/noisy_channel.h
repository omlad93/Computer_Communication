// #pragma once
#ifndef NOISY_CHANNEL
#define NOISY_CHANNEL
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include "../../utils/utils/utils.h"

#define RANDOMIZE "-r"
#define DETERMINISTIC "-d"
#define random_double() (double)(rand() / RAND_MAX)

char CHANNEL_BUFFER[MAX_LENGTH];
int TRANSFER_COUNTER = 0;
int FLIPPED_COUNTER = 0;
typedef struct Noise {
    str type;
    int n;
    int seed;
    double probability;
    int flipped;
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
void apply_deterministic(Noise_p noise, str data, int size);

/*
    apply randomize noise on transmitted data
    Called by apply_noise(...)
*/
void apply_randomized(Noise_p noise, str data, int size);

/*
    apply noise on incoming data, according to noise model
*/
int apply_noise(Noise_p noise, str data, int size);

#endif