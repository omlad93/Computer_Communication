// #pragma once
#ifndef NOISY_CHANNEL
#define NOISY_CHANNEL
#define _CRT_RAND_S


#include <io.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <winsock2.h>

#include "../../utils/utils/utils.h"

#define LOW_16 0x0000FFFF
#define HIGH_16 0xFFFF0000

#define RANDOMIZE "-r"
#define DETERMINISTIC "-d"
#define random_float() ((float)(rand()) / (float)(RAND_MAX))

typedef struct Noise {
    str type;
    int n;
    int seed;
    double probability;
    int flipped;
    int draws;
    int transmitted;
} Noise;
typedef Noise* Noise_p;

/*
    load 2 floats using two parts of 32 bits uint
    uses a single draw for both.
*/
float load_randoms_16b(float rnd[2]);

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