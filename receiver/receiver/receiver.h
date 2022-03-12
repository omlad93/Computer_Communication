#pragma once
#include <conio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>
#include "../../utils/utils/utils.h"

//#define _CRT_SECURE_NO_WARNINGS
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once


typedef struct statistics {
    int num_received;
    int num_written;
    int num_errors_fixed;

} stats;

stats *receiver_stats;

int received_msg_size;

char RECEIVER_BUF[MAX_LENGTH];

void update_receiver_file(FILE *file, char *msg);

void fix_hamming_message(char msg[MAX_LENGTH], char fixed_msg[MAX_LENGTH], int msg_size);

void fix_hamming_substring(int start, char msg[MAX_LENGTH], char fixed_msg[MAX_LENGTH]);

void calc_curr_substring(int start, char msg[MAX_LENGTH], char substring[ENCODED]);

void print_receiver_output();

void respond_to_sender(SOCKET socket);