
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

char MESSAGE_BUFFER[MAX_LENGTH];
char* SENDER_BUFFER;

//char RECEIVER_BUFFER[MAX_LENGTH];
int buff_current_size;
//int receiver_index;

uint32_t convert_msg_to_int(char* msg);

void convert_msg_to_string(char* encoded_msg, uint32_t encoded_msg_int);

int get_msg_size(FILE* file);

void message_hamming(char decoded_msg[DECODED], char encoded_msg[ENCODED]);

void hamming(char* decoded_msg, char* encoded_msg);

int print_output();

void update_buffer(char decoded_msg[DECODED], SOCKET socket, socketaddr addr);
