#include "utils.h"

/*
    Updated Ports & IPs after socket creation
    So Channel will have information
*/
void update_sharedata(int source, int port, str ip) {
    SDP sdp = &sd;
    str m = (source == RECEIVER) ? "Receiver" : "Sender";
    if (source == RECEIVER) {
        sdp->receiver_port = port;
        sdp->receiver_ready = TRUE;
        sprintf(sdp->receiver_ip, ip);
        // sdp->receiver_ip = ip;
    } else {
        sdp->sender_port = port;
        sdp->sender_ready = TRUE;
        sprintf(sdp->sender_ip, ip);
    }
    assert(ip != NULL, "IP IS NULL");
    printf("%s updated Shared Data\n", m);
    sdp->open_channel = sdp->sender_ready & sdp->receiver_ready;
}

/* calculates the hamming parity bit in pos - 1 */
int calc_hamming_bit(int pos, char encoded_msg[ENCODED]) {
    int i, j, cnt;
    cnt = 0;
    i = pos - 1;
    while (i < 31) {
        for (j = i; j < (i + pos); j++) {
            if (encoded_msg[j] == 1) {
                cnt++;
            }
            i = i + 2 * pos;
        }
        if (cnt % 2 == 0) {
            return 0;
        } else {
            return 1;
        }
    }
    return -1;  // WHAT SHOULD BE HERE?
    // -1 IS FOR COMPILATION ONLY}
}

// Bits Utility Functions

/*
    A Function to Compute Parity of masked integer
    Used for computing parity bits
*/
int parity(int val, int mask) {
    int x = val & mask;
    int y = x ^ (x >> 1);
    y = y ^ (y >> 2);
    y = y ^ (y >> 4);
    y = y ^ (y >> 8);
    y = y ^ (y >> 16);
    return (y & 1) ? TRUE : FALSE;
}

/*extarcts only the data bits from the encoded message*/
void get_msg_data_bits(char encoded_data[ENCODED], char stripped_data[DECODED]) {
    int j = 0;
    for (int i = 0; i < ENCODED; i++) {
        if (not(is_check_bit_pos(i))) {
            stripped_data[j] = encoded_data[i];
            j++;
        }
    }
}

/*
    A function that return char[idx]
    ret_val ∈ {'0','1'}
    MSB = 0, LSB =7
*/
char get_bit_from_char(char c, int idx) {
    char wanted_bit;
    assert(idx <= 7, "idx > 7");
    wanted_bit = (1 << (7 - idx)) & c;
    wanted_bit = (wanted_bit) ? (char)(1) : (char)(0);
    return wanted_bit;
}

// void get_next_bits(str data, char res[PARITY_BITS], int idx, int bit_count) {
// }

/*
    TODO: Document
*/
void get_message(str bits, char m[PARITY_BITS], int size) {
    int idx;
    unsigned char c = m[0];
    for (int i = 0; i < size; i++) {
        idx = (i > 7) ? (i - 7) : i;
        c = (i > 7) ? m[1] : c;
        bits[i] = get_bit_from_char(c, idx);
    }
}

/*
    Return parity bits of DECODED char sequence
    Translates binary sequence to integer
    Masks according to idx op PB
    parity_bits = [P1, P2, P4, P8, P16]
*/
void parity_bits(char data[DECODED], char parity_bits[PARITY_BITS]) {
    int numeric_data = from_binary(data);
    parity_bits[0] = parity(numeric_data, P1_MASK);
    parity_bits[1] = parity(numeric_data, P2_MASK);
    parity_bits[2] = parity(numeric_data, P4_MASK);
    parity_bits[3] = parity(numeric_data, P8_MASK);
    parity_bits[4] = parity(numeric_data, P16_MASK);
}

/*
    Return the value of a char with fillped bit in idx
*/
char flip(char c, int idx) {
    char origin_bit = get_bit_from_char(c, idx);
    char flipped = 1 - origin_bit;
    if (flipped) {
        return c | (1 << (7 - idx));
    } else {
        unsigned char t = 254;
        for (int i = 0; i < 7 - idx; i++) {
            t = (t << 1) + 1;
        }
        return c & t;
    }
}

// Sockets Utility Functions

/*
    Returns a new Socket
    Makes sure the socket is valid
*/
SOCKET create_socket() {
    SOCKET s;
    WSADATA wsa_data;
    int su = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    assert(su == NO_ERROR, "startup error");
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert(s != INVALID_SOCKET, "Invalid Socket");
    return s;
}

/*
    takes a socketaddr* and do three steps:
    > set it to use Internet Protocol
    > assign it to port
    > if provided with an IP: assigns it
      if didn't provid an IP: assigns a valid one
*/
void set_address(socketaddr* addr, int port, str ip) {
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = (ip == NULL) ? htonl(INADDR_ANY) : inet_addr(ip);
}

/*
    Binds a Socket to a socketaddr
    asserting that binding succeeded
*/
int bind_socket(SOCKET s, socketaddr* addr) {
    int bind_condition = ((bind(s, (SOCKADDR*)addr, sizeof(struct sockaddr))) > 0);
    assert(bind_condition != SOCKET_ERROR, "Binding Failed");
    return TRUE;
}

/*
    Function fot Reading information from socket
    reads at most `size` bytes
    reads all data that can be read from socket
    information is saved in `data` vairable
    returns size of actual bytes that read from socket
    asserting no error occurred
*/
int read_socket(SOCKET s, str data, int size) {
    // do while res > 0
    int res = 0;
    char message[45];
    do {
        res += recv(s, data, size, 0);
        assert_num(res >= 0, "Read Message Failed", WSAGetLastError());
    } while (res > 0);
    sprintf(message, "read_socket(): Read %d / %d Bytes", res, size);
    log_err(message);
    return res;
}

/*
    Function fot Writing information to socket
    writes at most `size` bytes
    writes all data from buffer
    information is written from `data` vairable
    returns size of actual bytes that read from socket
    asserting no error occurred
*/
int write_socket(SOCKET s, str data, int size) {
    // finish when res == size
    int res = 0;
    char message[45];
    do {
        res += send(s, data, size, 0);
        assert_num(res >= 0, "Write Message Failed", WSAGetLastError());
    } while (res < size);
    sprintf(message, "read_socket(): Read %d / %d Bytes", res, size);
    assert_num(res == size, "write_socket wrote too much bytes", res);
    log_err(message);
    return res;
}
