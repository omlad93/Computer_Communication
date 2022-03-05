#pragma once
#include <conio.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>

#define not(x) (!x)
#define xor (a, b)(a ^ b)
#define from_binary(s) ((int)strtol(s, NULL, 2))
#define is_check_bit_pos(i)(i == 1 || i == 2 || i == 4 || i == 8 || i == 16)
#define SENDER -1
#define RECEIVER -2
#define DECODED 26
#define ENCODED 31
#define PARITY_BITS 5
#define MAX_LENGTH 1993
#define FAIL -1

#define P1_MASK from_binary("1010101010101010101010101010101")
#define P2_MASK from_binary("0110011001100110011001100110011")
#define P4_MASK from_binary("0001111000011110000111100001111")
#define P8_MASK from_binary("0000000111111110000000011111111")
#define P16_MASK from_binary("0000000000000001111111111111111")


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

typedef struct sharedData {
    int receiver_port;
    int receiver_ready;
    char receiver_ip[15];
    int sender_port;
    int sender_ready;
    char sender_ip[15];
    int open_channel;
} SharedData;
typedef SharedData* SDP;
int first_init = TRUE;
SharedData sd;
// SDP sdp = &sd;
// memset(sdp, 0, sizeof(dp));

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
    Extarcts only the data bits from the encoded message
*/
void get_msg_data_bits(char encoded_data[ENCODED], char sripped_data[DECODED]);

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
int read_socket(SOCKET socket, socketaddr* addr, str data, int size);

/*
    Function fot Writing information to socket
    writes at most `size` bytes
    information is written from `data` vairable
    returns size of actual bytes that read from socket
    asserting no error occurred
*/
int write_socket(SOCKET socket, socketaddr* addr, str data, int size);

/*
    A loop of waiting for information on socket
    when there is information read it to `data` variable
    return value is length of data read
    can be called again for new file
    if called again can be terminated nicely using `quit` as filename
    Can be moved to server.c !
*/
int server_loop(SOCKET socket, socketaddr* addr, str data, int size);
