#pragma once
#include "receiver.h"

/*
    Writes the parsed message received from channel
*/
void update_receiver_file(FILE* file, char* msg) {
    int num_of_byts;
    num_of_byts = (receiver_stats->num_received / ENCODED) * DECODED;
    receiver_stats->num_written += num_of_byts;
    fwrite(msg, sizeof(char), num_of_byts, file);
    received_msg_size = 0;
}

/*
    Parsing the hole message
    Fixing each 26 bits
*/
void fix_hamming_message(char msg[MAX_LENGTH], char* fixed_msg, int msg_size) {
    char substring[ENCODED] = { 0 };
    //int j;
    uint32_t int_msg, fixed_msg_int;
    for (int i = 0; i < (msg_size / ENCODED); i++) {
        calc_curr_substring(i, msg, substring);
        int_msg = convert_msg_to_int(substring);
        fixed_msg_int = fix_hamming_substring(int_msg);
        receiver_stats->num_received += ENCODED;
        add_stripped_substring_to_buffer(fixed_msg, fixed_msg_int, i*ENCODED);
    }
}

void add_stripped_substring_to_buffer(char* fixed_msg, int fixed_msg_int, int start){
    for (int i = 0; i < ENCODED; i++) {
        if (i != 0 && i != 1 && i != 3 && i != 7 && i != 15) {
            if ((fixed_msg_int >> i & 1)) {
                fixed_msg[start + i] = '1';
            }
            else {
                fixed_msg[start + i] = '0';
            }
        }
    }


}
uint32_t convert_msg_to_int(char* msg) {
    uint32_t msg_int = 0;
    int indx = 0;
    for (int i = 0; i < ENCODED; i++) {
        if (i != 0 && i != 1 && i != 3 && i != 7 && i != 15) {
            if (msg[indx] == '1') {
                (msg_int) |= (1 << (i));
            }
            indx++;
        }
    }
    return msg_int;
}

/*
    Extracting the next 32 bits
*/
void calc_curr_substring(int start, char msg[MAX_LENGTH], char substring[ENCODED]) {
    for (int i = 0; i < 31; i++) {
        substring[i] = msg[start + i];
    }
}
/*
    Fixing each 26 bits of message

void fix_hamming_substring(int start, char substring[ENCODED], char fixed_msg[MAX_LENGTH]) {
    char error_pos[5];
    int error_pos_int, j = 0;
    // exctract data bits
    char data[DECODED];
    char check_bits[PARITY_BITS];
    get_msg_data_bits(substring, data);
    // calc check bitas
    parity_bits(data, check_bits);
    // calc xor to get the error position
    error_pos[0] = substring[1] ^ check_bits[0];
    error_pos[1] = substring[2] ^ check_bits[1];
    error_pos[2] = substring[4] ^ check_bits[2];
    error_pos[3] = substring[8] ^ check_bits[3];
    error_pos[4] = substring[16] ^ check_bits[4];
    error_pos_int = (int)((error_pos[0]) | (error_pos[1] << 1) | (error_pos[2] << 2) | (error_pos[3] << 3) | (error_pos[4] << 4));
    if (error_pos_int != 0) {
        receiver_stats->num_errors_fixed++;
        substring[error_pos_int - 1] = 1 - substring[error_pos_int - 1];
    }
    for (int i = start; i < DECODED; i++) {
        if (not(is_check_bit_pos(j))) {
            fixed_msg[i] = substring[j];
        }
        j++;
    }
}*/

uint32_t fix_hamming_substring(uint32_t int_msg) {
    int err = 0;
    int err_index = -1;
    uint32_t masked = 0;
    uint32_t parity_bit = 0;
    uint32_t stripped = int_msg;
    // calc 15 parity bit
    parity_bit = parity(int_msg, 0x7FFF8000);
    if (parity_bit) { //error was detected
        err = 1;
        err_index += (int)pow(2, 4);
    }
    // calc 7 parity bit
    parity_bit = parity(int_msg, 0x7F807F80);
    if (parity_bit) { //error was detected
        err = 1;
        err_index += (int)pow(2, 3);
    }    
    // calc 3 parity bit
    parity_bit = parity(int_msg, 0x38787878);
    if (parity_bit) { //error was detected
        err = 1;
        err_index += (int)pow(2, 2);
    }    
    // calc 1 parity bit
    parity_bit = parity(int_msg, 0x66666666);
    if (parity_bit) { //error was detected
        err = 1;
        err_index += (int)pow(2, 1);
    }    
    // calc 0 parity bit
    parity_bit = parity(int_msg, 0x55555555);
    if (parity_bit) { 
        //error was detected
        err = 1;
        err_index += (int)pow(2, 0);
    }
    if (err) {
        receiver_stats->num_errors_fixed++;
        stripped ^= (1ULL << (err_index));
    }
    return stripped;
}

/*void fix_hamming_substring(int start, char substring[ENCODED], char fixed_msg[MAX_LENGTH]){
    int error_pos = 0;
    for(int i = 0; i < 5; i++){
        int pos = (int)pow(2, i);
        int hamming_bit = calc_hamming_bit(pos, substring);
        if (hamming_bit != 0){
            error_pos += pos;
        }
        if(error_pos != 1){
            fixed_msg[i+error_pos] = ~fixed_msg[i+error_pos];
        }
    }



}*/

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
    int message_size_int;
    char filename[MAX_LENGTH];
    // char msg[MAX_LENGTH];
    char message_size_str[SHORT_MESSAGE];
    char* fixed_msg; // NEED TO CHANGE THE SIZE
    int status = 0;

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
    printf("Plase enter file name\n");
    // scanf_s("%s", filename, (unsigned int)sizeof(filename));
    // assert(filename != NULL, "FileName");

    strcpy(filename, HC_OUTPUT);

    // file = fopen(filename, "wb");

    while (strcmp(filename, "quit") != 0) {
        assert(fopen_s(&file, filename, "w") == 0, "Error in opening file\n");
        printf("\tWriting file: %s According to Data Received from the Noisy-Channel \n", filename);

        // read message size
        status = read_socket(socket, message_size_str, SHORT_MESSAGE);
        // NEED TO SEND ACK ???
        message_size_int = atoi(message_size_str);
        RECEIVER_BUF = (char*)malloc(message_size_int * sizeof(char));
        fixed_msg = (char*)malloc(message_size_int * sizeof(char));

        status = read_socket(socket, RECEIVER_BUF, message_size_int);
        // encode hamming message
        fix_hamming_message(RECEIVER_BUF, fixed_msg, message_size_int);
        printf("\tfixed hamming\n");
        // write the received message to file
        update_receiver_file(file, RECEIVER_BUF);
        printf("\tWrote file\n");
        //  write the received message to file
        printf("\tGot message from socket (Channel) [%dB]\n", message_size_int);
        // sends a respond
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
    printf("I'm Out!\n");
    // cleanup
    shutdown(socket, SD_BOTH);
    closesocket(socket);
    WSACleanup();
    print_receiver_output();
    return 0;
}
