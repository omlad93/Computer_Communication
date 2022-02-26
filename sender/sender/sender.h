
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

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

char SENDER_BUFFER[MAX_LENGTH];
//char RECEIVER_BUFFER[MAX_LENGTH];
int sender_index;
//int receiver_index;


void hamming(char* decoded_msg, char* encoded_msg);

int print_output();

void update_buffer(char decoded_msg[DECODED], SOCKET socket, socketaddr addr);
