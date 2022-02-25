#pragma once
#include <conio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define True 1
#define False 0
#define not(x) (!x)
#define xor (a, b)(a ^ b)
#define from_binary(s) ((int)strtol(s, NULL, 2))
#define DECODED 26
#define ENCODED 31
#define PARITY_BITS 5
#define MAX_LENGTH 1993

#define P1_MASK from_binary("1010101010101010101010101010101")
#define P2_MASK from_binary("0110011001100110011001100110011")
#define P4_MASK from_binary("0001111000011110000111100001111")
#define P8_MASK from_binary("0000000111111110000000011111111")
#define P16_MASK from_binary("0000000000000001111111111111111")
#define parity(x, mask) (__builtin_parity(x & mask))
// #define log_err(x) printf(stderr, "%s\n".format(x))
typedef struct sockaddr_in socketaddr;
typedef char* str;

void log_err(str message) {
    fprintf(stderr, "%s\n", message);
}
void assert(int condition, str message) {
    if (not(condition)) {
        fprintf(stderr, "Assertion Error: %s\n", message);
        exit(-1);
    }
}
void assert_num(int condition, str message, int err_idx) {
    if (not(condition)) {
        fprintf(stderr, "Assertion Error: %s [%d]\n", message, err_idx);
        exit(-1);
    }
}

// Indexing is considering MSB as 0

// Bits Utility Functions

char get_bit_from_char(char c, int idx);

void get_next_bits(str data, char res[PARITY_BITS], int idx, int bit_count);

void get_message(str bits, char m[PARITY_BITS], int size);

void parity_bits(char data[DECODED], char parity[PARITY_BITS]);

void attach(str message, char c[2], int mod, int size, int idx);

char flip(char c, int idx);

// Sockets Utility Functions

SOCKET create_socket();

void set_address(socketaddr* addr, int port, char* ip);

int bind_socket(SOCKET socket, socketaddr* addr);

int read_socket(SOCKET socket, socketaddr* addr, str data, int size);

int write_socket(SOCKET socket, socketaddr* addr, str data, int size);

int server_loop(SOCKET socket, socketaddr* addr, str data, int size);
