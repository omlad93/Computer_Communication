#pragma once
#include "receiver.h"

/*
    Writes the parsed message received from channel
*/
void update_receiver_file(FILE* file, char* msg) {
    int num_of_byts;
    num_of_byts = (received_msg_size / ENCODED) * DECODED;
    receiver_stats->num_written += num_of_byts;
    fwrite(msg, 1, num_of_byts, file);
    received_msg_size = 0;
}

/*
    Parsing the hole message
    Fixing each 26 bits
*/
void fix_hamming_message(char msg[MAX_LENGTH], char fixed_msg[MAX_LENGTH], int msg_size) {
    char substring[ENCODED];
    for (int i = 0; i < (msg_size / ENCODED); i++) {
        calc_curr_substring(i, msg, substring);
        fix_hamming_substring(i, msg, fixed_msg);
        receiver_stats->num_received += ENCODED;
    }
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
*/
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
    sprintf_s(msg,sizeof(msg), "%d %d %d", receiver_stats->num_received, receiver_stats->num_written, receiver_stats->num_errors_fixed);
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
    char* filename = calloc(MAX_LENGTH, sizeof(char));
    char msg[MAX_LENGTH];
    char fixed_msg[MAX_LENGTH];
    int status = 0;
    int err;

    // create a socket
    socketaddr channel_addr;
    SOCKET socket = create_socket();
    memset(&channel_addr, 0, sizeof(channel_addr));
    set_address(&channel_addr, port, ip);
    //update_sharedata(RECEIVER, port, ip);
    printf("Reciever\n\tConnecting");
    assert_num(connect(socket, (SOCKADDR*) &channel_addr, sizeof(struct sockaddr)) != SOCKET_ERROR, "connection falied",WSAGetLastError());

    receiver_stats = (stats*)calloc(1, sizeof(stats));
    received_msg_size = 0;
    // ask for file
    printf("\tPlase enter file name\n");
    scanf_s("%s", filename, (unsigned int)sizeof(filename));

    // file = fopen(filename, "wb");
    while (!strcmp(filename, "quit")) {
        err = fopen_s(&file, filename, "wb");
        if (err == 0) {
            printf("Error in openninf file\n");
            break;
        }
        // read message
        status = read_socket(socket, RECEIVER_BUF, MAX_LENGTH);
        received_msg_size += status;
        // encode hamming message
        fix_hamming_message(msg, fixed_msg, received_msg_size);
        // write the received message to file
        update_receiver_file(file, msg);
        // write the received message to file

        // sends a respond
        respond_to_sender(socket);

        // print to file
        //print_receiver_file();

        closesocket(socket);
        WSACleanup();
        fclose(file);

        // open new socket and connect
        socket = create_socket();
        assert(connect(socket, (SOCKADDR*) &channel_addr, sizeof(channel_addr)) != SOCKET_ERROR, "connection falied");

        // ask for new filename (if "quit" - close the socket)
        printf("\tPlase enter file name\n");
        scanf_s("%s", filename, (unsigned int)sizeof(filename));
    }

    // cleanup
    shutdown(socket, SD_BOTH);
    closesocket(socket);
    WSACleanup();
    print_receiver_output();
    return 0;
}
