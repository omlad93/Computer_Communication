#include "utils.h"

// Bits Utility Functions
char get_bit_from_char(char c, int idx) {
    char wanted_bit;
    assert(idx <= 7, "idx > 7");
    idx = 7 - idx;
    wanted_bit = idx & c;
    wanted_bit = (wanted_bit) ? (char)(1) : (char)(0);
    return wanted_bit;
}

void get_next_bits(str data, char res[PARITY_BITS], int idx, int bit_count);

void get_message(str bits, char m[PARITY_BITS], int size) {
    int idx;
    unsigned char c = m[0];
    for (int i = 0; i < size; i++) {
        idx = (i > 7) ? (i - 7) : i;
        c = (i > 7) ? m[1] : c;
        bits[i] = get_bit_from_char(c, idx);
    }
}

void parity_bits(char data[DECODED], char parity_bits[PARITY_BITS]) {
    char p1, p2, p4, p8, p16;
    int numeric_data = from_binary(data);
    parity_bits[0] = parity(numeric_data, P1_MASK);
    parity_bits[1] = parity(numeric_data, P2_MASK);
    parity_bits[2] = parity(numeric_data, P4_MASK);
    parity_bits[3] = parity(numeric_data, P8_MASK);
    parity_bits[4] = parity(numeric_data, P16_MASK);
}

void attach(str message, char c[2], int mod, int size, int idx);

char flip(char c, int idx) {
    char origin_bit = get_bit_from_char(c, idx);
    char flipped = 1 - origin_bit;
    if (flipped) {
        return c | (1 << (7 - idx));
    } else {
        char t = 254;
        for (int i = 0; i < 7 - idx; i++) {
            t = (t << 1) + 1;
        }
        return c & t;
    }
}

// Sockets Utility Functions

SOCKET create_socket() {
    SOCKET s;
    WORD version_req = MAKEWORD(1, 1);
    WSADATA data;
    WSAStartup(version_req, &data);
    s = socket(AF_INET, SOCK_DGRAM, 0);
    assert(s != INVALID_SOCKET, "Invalid Socket");
    return s;
}

void set_address(sockaddr* addr, int port, str ip) {
    addr->sin_family = AF_INET;
    addr->sin_port = port;
    addr->sin_addr = (ip != NULL) ? inet_addr(ip) : htonl(INADDR_ANY);
}

int bind_socket(SOCKET s, socketaddr* addr) {
    WORD version_req = MAKEWORD(1, 1);
    WSADATA data;
    WSAStartup(version_req, &data);
    assert(((bind(s, addr, sizeof(*addr))) < 0), "Bind < 0");
    return True;
}

int read_socket(SOCKET s, socketaddr* addr, str data, int size) {
    int res;
    int l = sizeof(addr);
    WORD version_req = MAKEWORD(1, 1);
    WSADATA data;
    WSAStartup(version_req, &data);
    res = recvfrom(s, data, size, 0, addr, &l);
    assert_num(res >= 0, "Read Message Failed! -", WSAGetLastError());
    return res;
}

int write_socket(SOCKET s, socketaddr* addr, str data, int size) {
    WORD version_req = MAKEWORD(1, 1);
    WSADATA data;
    WSAStartup(version_req, &data);
    addr->sin_family = AF_INET;
    int res = sendto(s, data, size, 0, addr, sizeof(struct sockaddr));
    assert_num(res >= 0, "Write Message Failed! -", WSAGetLastError());
    return res;
}

int server_loop(SOCKET s, socketaddr* addr, str data, int size) {
    int status, keyboard;
    char user_buffer[MAX_LENGTH] = {0};
    fd_set fs;
    int i = 0, len = 0;
    struct timeval time_val;
    char c;
    time_val.tv_sec = 0;
    time_val.tv_usec = 100;
    while (True) {
        FD_ZEROS(&fs);
        FD_SET(s, &fs);
        status = select(1, &fs, NULL, NULL, &time_val);
        assert(status >= 0, 'Selection Failed [Server]');
        if (FD_ISSET(s, &fs)) {
            log_err("Reading Message From Sender");
            len = socket_read(s, addr, data, size);
            log_err("Message was %d Bits long");
            return len;
        } else {
            log_err("enter file name:");
            scanf("%s", &user_buffer);
            log_err(user_buffer);
            if strcmp (user_buffer, "quit") {
                log_err("Recognized 'quit': aborting connection.");
                return -1;
            }
        }
    }
}
