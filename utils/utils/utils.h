// #pragma once
#ifndef UTILS
#define UTILS
// #define _CRT_SECURE_NO_WARNINGS
// #define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define __STDC_LIB_EXT1__

#include <conio.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>
#include <fileapi.h>

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

#define and &&
#define or ||
#define not(x) (!x)
#define xor (a, b)(a ^ b)
#define from_binary(s) ((int)strtol(s, NULL, 2))
#define is_check_bit_pos(i) (i == 1 || i == 2 || i == 4 || i == 8 || i == 16)
#define SENDER -1
#define RECEIVER -2
#define DECODED 26
#define ENCODED 31
#define PARITY_BITS 5
#define MAX_LENGTH 1993
#define SHORT_MESSAGE 10
#define FAIL -1

#define BITS_PER_BYTE 8
#define BIT_FLIP_R(character, i) (character ^ (1 << i))   // flip the ith bit from right in character
#define BIT_SET1_R(character, i) (character | (1 << i))   // Set the ith bit from right in character to 1
#define BIT_SET0_R(character, i) (character & ~(1 << i))  // Set the ith bit from right in character to 0
#define BIT_EVAL_R(character, i) ((character >> i) & 1)   // Get the ith bit from right in character

#define P1_MASK from_binary("1010101010101010101010101010101")
#define P2_MASK from_binary("0110011001100110011001100110011")
#define P4_MASK from_binary("0001111000011110000111100001111")
#define P8_MASK from_binary("0000000111111110000000011111111")
#define P16_MASK from_binary("0000000000000001111111111111111")

// HARD CODED SHORTCUTS
#define IP0 "0.0.0.0"
#define HC_SENDER_PORT 6342
#define HC_SENDER_IP "127.0.0.1"
#define HC_RECEIVER_PORT 6343
#define HC_RECEIVER_IP "127.0.0.1"
#define HC_INPUT "input.txt"
#define HC_OUTPUT "output.txt"

typedef struct sockaddr_in socketaddr;
typedef char* str;

/*
    print message to stderr and create newline
*/
inline void log_err(str message) {
    fprintf(stderr, "%s\n", message);
}

/*
    check condition:
        if satisfied: does nothing.
        else: log message and exit(FAIL)
*/
inline void assert(int condition, str message) {
    if (not(condition)) {
        fprintf(stderr, "Assertion Error: %s\n", message);
        exit(FAIL);
    }
}

/*
    check condition:
        if satisfied: does nothing.
        else: log message with numeric err_idx and exit(FAIL))
*/
inline void assert_num(int condition, str message, int err_idx) {
    if (not(condition)) {
        fprintf(stderr, "Assertion Error: %s [%d]\n", message, err_idx);
        exit(FAIL);
    }
}


int file_exists(LPCTSTR path);
/*
    Updated Ports & IPs after socket creation
    So Channel will have information
*/
void update_sharedata(int source, int port, str ip);

int calc_hamming_bit(int pos, char encoded_msg[ENCODED]);

// Bits Utility Functions
/*
    A Function to Compute Parity of masked integer
    Used for computing parity bits
*/
int parity(int val, int mask);

/*
    Extracts only the data bits from the encoded message
*/
void get_msg_data_bits(char encoded_data[ENCODED], char stripped_data[DECODED]);

/*
    A function that return char[idx]
    ret_val ∈ {'0','1'}
    MSB = 0, LSB =7
*/
char get_bit_from_char(char c, int idx);

/*
TODO: IMPLEMENT
*/
void get_next_bits(str data, char res[PARITY_BITS], int idx, int bit_count);

/*
    TODO: Document
*/
void get_message(str bits, char m[PARITY_BITS], int size);

/*
    Return parity bits of DECODED char sequence
    Translates binary sequence to integer
    Masks according to idx op PB
    parity_bits = [P1, P2, P4, P8, P16]
*/
void parity_bits(char data[DECODED], char parity[PARITY_BITS]);

/*
TODO: IMPLEMENT
*/
void attach(str message, char c[2], int mod, int size, int idx);

/*
    Return the value of a char with fillped bit in idx
*/
char flip(char c, int idx);

// Sockets Utility Functions

/*
    Returns a new Socket
    Makes sure the socket is valid
*/
SOCKET create_socket();

/*
    takes a socketaddr* and do three steps:
    > set it to use Internet Protocol
    > assign it to port
    > if provided with an IP: assigns it
      if didn't provid an IP: assigns a valid one
*/
void set_address(socketaddr* addr, int port, char* ip);

/*
    Binds a Socket to a socketaddr
    asserting that binding succeeded
*/
int bind_socket(SOCKET socket, socketaddr* addr);

/*
    Function fot Reading information from socket
    reads at most `size` bytes
    information is saved in `data` vairable
    returns size of actual bytes that read from socket
    asserting no error occurred
*/
int read_socket(SOCKET socket, str data, int size);

/*
    Function fot Writing information to socket
    writes at most `size` bytes
    information is written from `data` vairable
    returns size of actual bytes that read from socket
    asserting no error occurred
*/
int write_socket(SOCKET socket, str data, int size);

#endif