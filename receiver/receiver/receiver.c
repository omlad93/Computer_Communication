#pragma once
#include "receiver.h"



void convert_char_arr_to_mgs(char* orig_msg, char* parsed_msg, int orig_msg_size) {
    for (int i = 0; i < orig_msg_size; i++) {
        orig_msg[i] = (char)(0);
        for (int j = 0; j < 8; j++) {
            if (parsed_msg[8*i + j] == '1') {
                orig_msg[i] = BIT_SET1_R(orig_msg[i], 8-1-j);
            }
        }
    }
}

/*
    Writes the parsed message received from channel
*/
void update_receiver_file(FILE* file, char* msg,int num_of_byts) {
    //int num_of_byts;
    //num_of_byts = (receiver_stats->num_received / ENCODED) * DECODED;
    receiver_stats->num_written += num_of_byts;
    fwrite(msg, sizeof(char), num_of_byts, file);
    received_msg_size = 0;
}

/*
    Parsing the hole message
    Fixing each 26 bits
*/
void fix_hamming_message(char* msg, char* fixed_msg, int msg_size) {
    char substring[ENCODED] = {0};
    // int j;
    uint32_t int_msg, fixed_msg_int;
    for (int i = 0; i < (msg_size / ENCODED); i++) {
        calc_curr_substring(i*ENCODED, msg, substring);
        int_msg = convert_msg_to_int(substring);
        fixed_msg_int = fix_hamming_substring(int_msg);
        receiver_stats->num_received += ENCODED;
        add_stripped_substring_to_buffer(fixed_msg, fixed_msg_int, i * DECODED);
    }
}

/* Updates fixed message buffer with stripped 26 bits */
void add_stripped_substring_to_buffer(char* fixed_msg, int fixed_msg_int, int start) {
    int index = 0;
    for (int i = 0; i < ENCODED; i++) {
        if (i != 0 && i != 1 && i != 3 && i != 7 && i != 15) {
            if ((fixed_msg_int >> i & 1)) {
                fixed_msg[start + index] = '1';
            }
            else {
                fixed_msg[start + index] = '0';
            }
            index++;
        }
    }
}

uint32_t convert_string_to_int(char* str, int str_size) {
    uint32_t str_int = 0;
    int indx = 0;
    for (int i = 0; i < str_size; i++) {
        if (str[i] == '1') {
            (str_int) |= (1 << (i));
        }
    }
    return str_int;
}

uint32_t convert_msg_to_int(char* msg) {
    uint32_t msg_int = 0;
    int indx = 0;
    for (int i = 0; i < ENCODED; i++) {
        if (msg[i] == '1') {
            (msg_int) |= (1 << (i));
        }
    }
    return msg_int;
}

/*
    Extracting the next 32 bits
*/
void calc_curr_substring(int start, char* msg, char substring[ENCODED]) {
    for (int i = 0; i < ENCODED; i++) {
        substring[i] = msg[start + i];
    }
}

/* Fixes a single chunk of message */
uint32_t fix_hamming_substring(uint32_t int_msg) {
    int err = 0;
    int err_index = -1;
    uint32_t masked = 0;
    uint32_t parity_bit = 0;
    uint32_t stripped = int_msg;
    // calc 15 parity bit
    parity_bit = parity(int_msg, 0x7FFF8000);
    if (parity_bit) {  // error was detected
        err = 1;
        err_index += (int)pow(2, 4);
    }
    // calc 7 parity bit
    parity_bit = parity(int_msg, 0x7F807F80);
    if (parity_bit) {  // error was detected
        err = 1;
        err_index += (int)pow(2, 3);
    }
    // calc 3 parity bit
    parity_bit = parity(int_msg, 0x38787878);
    if (parity_bit) {  // error was detected
        err = 1;
        err_index += (int)pow(2, 2);
    }
    // calc 1 parity bit
    parity_bit = parity(int_msg, 0x66666666);
    if (parity_bit) {  // error was detected
        err = 1;
        err_index += (int)pow(2, 1);
    }
    // calc 0 parity bit
    parity_bit = parity(int_msg, 0x55555555);
    if (parity_bit) {
        // error was detected
        err = 1;
        err_index += (int)pow(2, 0);
    }
    if (err) {
        printf("\tERROR was detected\n");
        receiver_stats->num_errors_fixed++;
        stripped ^= (1ULL << (err_index));
    }
    return stripped;
}

void respond_to_sender(SOCKET socket) {
    char msg[100];
    sprintf_s(msg, sizeof(msg), "%d %d %d", receiver_stats->num_received, receiver_stats->num_written, receiver_stats->num_errors_fixed);
    write_socket(socket, msg, 100);
}

void print_receiver_output() {
    fprintf(stderr, "received: %d bytes\n", receiver_stats->num_received);
    fprintf(stderr, "wrote: %d bytes\n", receiver_stats->num_written);
    fprintf(stderr, "detected & corrected %d errors\n", receiver_stats->num_errors_fixed);
}

int main(int argc, char* argv[]) {
    FILE* file;
    char* ip = argv[1];
    int port = atoi(argv[2]);
    int message_size_int, encoded_message_size_int, fixed_msg_int;
    char filename[MAX_LENGTH];
    char msg[MAX_LENGTH];
    char message_size_str[SHORT_MESSAGE];
    char* fixed_msg;  // NEED TO CHANGE THE SIZE
    char* origin_message;
    int status = 0;
    if (argc == 4) {
        if (not(strcmp(argv[3], "-debug"))) { //DEBUG MODE
            log_err("working on debug mode (Fixed Port & Local IP)");
            port = HC_RECEIVER_PORT;
            ip = IP0;
        }
    }
    // create a socket
    socketaddr channel_addr;
    SOCKET socket = create_socket();
    memset(&channel_addr, 0, sizeof(channel_addr));
    set_address(&channel_addr, HC_RECEIVER_PORT, HC_RECEIVER_IP);
    // update_sharedata(RECEIVER, port, ip);
    assert_num(connect(socket, (SOCKADDR*)&channel_addr, sizeof(struct sockaddr)) != SOCKET_ERROR, "connection falied", WSAGetLastError());

    receiver_stats = (stats*)calloc(1, sizeof(stats));
    received_msg_size = 0;
    // ask for file
    printf("RECEIVER\n");
    printf("Please enter file name\n");
    assert(scanf("%s\n", filename) != 0, "Scanning Failed");

    //strcpy(filename, HC_OUTPUT);

    while (strcmp(filename, "quit") != 0) {
        assert(fopen_s(&file, filename, "w") == 0, "Error in opening file\n");
        printf("\tWriting file: %s According to Data Received from the Noisy-Channel \n", filename);

        // read message size
        status = read_socket(socket, message_size_str, SHORT_MESSAGE);
        // NEED TO SEND ACK ???
        encoded_message_size_int = atoi(message_size_str); //expanded encoded message size
        message_size_int = (encoded_message_size_int / ENCODED) * DECODED; //expanded decoded message size
        RECEIVER_BUF = (char*)malloc(encoded_message_size_int * sizeof(char));
        fixed_msg = (char*)malloc(message_size_int * sizeof(char));
        origin_message = (char*)malloc((message_size_int/8 + 1) * sizeof(char)); // orifinal message 

        status = read_socket(socket, RECEIVER_BUF, encoded_message_size_int);
        /* FOR DEBUG ONLY */
        printf("\tRECEIVER_BUF = %s \n", RECEIVER_BUF);
        // encode hamming message
        fix_hamming_message(RECEIVER_BUF, fixed_msg, encoded_message_size_int);
        convert_char_arr_to_mgs(origin_message, fixed_msg, message_size_int / 8);
        origin_message[message_size_int / 8] = '\0';
        printf("\tdecoded_msg = %s \n", fixed_msg);
        //fixed_msg_int = convert_string_to_int(fixed_msg, message_size_int);
        //itoa(fixed_msg_int, msg, 10);
        printf("\tfixed hamming\n");
        // write the received message to file
        update_receiver_file(file, origin_message, message_size_int / 8);
        printf("\tWrote file\n");
        //  write the received message to file
        printf("\tGot message from socket (Channel) [%dB]\n", encoded_message_size_int);
        // sends a respond ???
        // respond_to_sender(socket);

        // print to file
        // print_receiver_file();

        closesocket(socket);
        WSACleanup();
        fclose(file);
        free(fixed_msg);
        free(RECEIVER_BUF);

        // open new socket and connect
        socket = create_socket();
        assert(connect(socket, (SOCKADDR*)&channel_addr, sizeof(channel_addr)) != SOCKET_ERROR, "connection falied");

        // ask for new filename (if "quit" - close the socket)
        printf("Please enter file name\n");
        assert(scanf("%s\n", filename) != 0, "Scanning Failed");
    }
    // cleanup
    shutdown(socket, SD_BOTH);
    closesocket(socket);
    WSACleanup();
    print_receiver_output();
    return 0;
}
