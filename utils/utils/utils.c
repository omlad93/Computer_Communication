#include "utils.h"

/*
    Updated Ports & IPs after socket creation
    So Channel will have information
*/
void update_sharedata(int source, int port, str ip) {
    SDP sdp = &sd;
    if (first_init_sd) {
        first_init_sd = TRUE;
        memset(sdp, 0, sizeof(sd));
    }
    str m = (source == RECEIVER) ? "Receiver" : "Sender";
    if (source == RECEIVER) {
        sdp->receiver_port = port;
        sdp->receiver_ready = TRUE;
        // sdp->receiver_ip = ip;
    } else {
        sdp->sender_port = port;
        sdp->sender_ready = TRUE;
        // sdp->sender_ip = ip;
    }
    assert(ip != NULL, "IP IS NULL");
    printf("%s updated Shared Data\n", m);
    sdp->open_channel = sdp->sender_ready & sdp->receiver_ready;
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

void get_next_bits(str data, char res[PARITY_BITS], int idx, int bit_count) {
}

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

void attach(str message, char c[2], int mod, int size, int idx);

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
    WORD version_req = MAKEWORD(1, 1);
    WSADATA data;
    WSAStartup(version_req, &data);
    s = socket(AF_INET, SOCK_DGRAM, 0);
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
    addr->sin_port = port;
    addr->sin_addr.s_addr = (ip == NULL) ? htonl(INADDR_ANY) : inet_addr(ip);
}

/*
    Binds a Socket to a socketaddr
    asserting that binding succeeded
*/
int bind_socket(SOCKET s, socketaddr* addr) {
    WORD version_req = MAKEWORD(1, 1);
    WSADATA wsa_data;
    WSAStartup(version_req, &wsa_data);
    int bind_condition = ((bind(s, (struct sockaddr*)addr, sizeof(*addr))) > 0);
    assert(bind_condition, "Binding Failed");
    return TRUE;
}

/*
    Function fot Reading information from socket
    reads at most `size` bytes
    information is saved in `data` vairable
    returns size of actual bytes that read from socket
    asserting no error occurred
*/
int read_socket(SOCKET s, socketaddr* addr, str data, int size) {
    int res;
    int l = sizeof(addr);
    WORD version_req = MAKEWORD(1, 1);
    WSADATA wsa_data;
    WSAStartup(version_req, &wsa_data);
    res = recvfrom(s, data, size, 0, (struct sockaddr*)addr, &l);
    assert_num(res >= 0, "Read Message Failed", WSAGetLastError());
    return res;
}

/*
    Function fot Writing information to socket
    writes at most `size` bytes
    information is written from `data` vairable
    returns size of actual bytes that read from socket
    asserting no error occurred
*/
int write_socket(SOCKET s, socketaddr* addr, str data, int size) {
    WSADATA wsa_data;
    WORD version_req = MAKEWORD(1, 1);
    WSAStartup(version_req, &wsa_data);
    addr->sin_family = AF_INET;
    int res = sendto(s, data, size, 0, (struct sockaddr*)addr, sizeof(struct sockaddr));
    assert_num(res >= 0, "Write Message Failed", WSAGetLastError());
    return res;
}

/*
    A loop of waiting for information on socket
    when there is information read it to `data` variable
    return value is length of data read
    can be called again for new file
    if called again can be terminated nicely using `quit` as filename
    Can be moved to server.c !
*/
int server_loop(SOCKET s, socketaddr* addr, str data, int size) {
    int status;
    char user_buffer[MAX_LENGTH] = {0};
    fd_set fs;
    int len = 0;
    struct timeval time_val;
    time_val.tv_sec = 0;
    time_val.tv_usec = 100;
    while (TRUE) {
        FD_ZERO(&fs);
        FD_SET(s, &fs);
        status = select(1, &fs, NULL, NULL, &time_val);
        assert(status >= 0, "Selection Failed [Server]");
        if (FD_ISSET(s, &fs)) {
            log_err("Reading Message From Sender");
            len += read_socket(s, addr, data, size);
            log_err("Message was %d Bits long");
            return len;
        } else {

            /*log_err("enter file name:");
            scanf_s("%s", user_buffer, MAX_LENGTH);
            log_err(user_buffer);
            if (strcmp(user_buffer, "quit")) {
                log_err("Recognized 'quit': aborting connection.");
                return FAIL;
            }*/
            return FAIL;
        }
        // else {
        //     log_err("enter file name:");
        //     scanf_s("%s", user_buffer, MAX_LENGTH);
        //     log_err(user_buffer);
        //     if (strcmp(user_buffer, "quit")) {
        //         log_err("Recognized 'quit': aborting connection.");
        //         return FAIL;
        //     }
        // }
    }
}