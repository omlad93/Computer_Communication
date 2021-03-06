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
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif 

typedef struct statistics {
    int num_received;
    int num_written;
    int num_errors_fixed;

} stats;

stats *receiver_stats;

int received_msg_size;

char* ORIGIN_MESSAGE;
char* EXPANDED_MESSAGE;
char* RECEIVER_BUF;

void update_receiver_file(FILE* file, char* msg, int num_of_byts);

void fix_hamming_message(char msg[MAX_LENGTH], char fixed_msg[MAX_LENGTH], int msg_size);

uint32_t fix_hamming_substring(uint32_t int_msg);

void add_stripped_substring_to_buffer(char fixed_msg[MAX_LENGTH], int fixed_msg_int, int start);

void calc_curr_substring(int start, char msg[MAX_LENGTH], char substring[ENCODED]);

void print_receiver_output();

uint32_t convert_msg_to_int(char* msg);

uint32_t convert_string_to_int(char* str, int str_size);

void respond_to_sender(SOCKET socket);

void convert_char_arr_to_mgs(char* orig_msg, char* parsed_msg, int orig_msg_size);