#pragma once
#include <conio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>

#include "utils.h"

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once


typedef struct statistics {
    int num_received;
    int num_written;
    int num_errors_fixed;

} stats;

stats *server_stats;

int received_msg_size;

char RECEIVER_BUF[MAX_LENGTH];

void update_receiver_file(FILE *file, char *msg);

void fix_hamming_message(char* parsed_msg, int msg_size);
