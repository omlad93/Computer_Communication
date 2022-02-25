#pragma once
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

void generate_noise(Noise_p noise, str noise_type, int a1, int a2);

void apply_deterministic(Noise_p noise, str data, int size);

void apply_randomized(Noise_p noise, str data, int size);

int apply_noise(Noise_p noise, str data, int size);
