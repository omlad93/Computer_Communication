#pragma once
#include "receiver.h"

int main(int argc, char* argv[]) {
    FILE* file;
    char* ip = argv[1];
    int port = atoi(argv[2]);
    char* filename;
    char msg[MAX_LENGTH];
    char fixed_msg[MAX_LENGTH];
    int status = 0;
    int size;
    
    // create a socket
    socketaddr channel_addr;
    SOCKET socket = create_socket();
    memset(&channel_addr, 0, sizeof(channel_addr));
	set_address(&channel_addr, port, ip);
    assert(connect(socket,&channel_addr,sizeof(channel_addr))!=SOCKET_ERROR,"connection falied");

    server_stats = (stats*)calloc(1, sizeof(stats));
    received_msg_size = 0;
    // ask for file
    printf("Plase enter file name\n");
    scanf("%s", &filename);

    //file = fopen(filename, "wb");
    while (!strcmp(filename, "quit")){
        file = fopen(filename, "wb");
        if (file == NULL){
            printf("Error in openninf file\n");
            break;
        }
        //read message
        status = read_scoket(socket, &channel_addr, RECEIVER_BUF, MAX_LENGTH); 
        received_msg_size += status;
        //encode hamming message
        fix_hamming_message(msg, fixed_msg, received_msg_size);
        //write the received message to file
        update_receiver_file(file, msg);
        // write the received message to file
        
        //sends a respond 
        respond_to_sender(socket, &channel_addr);

        // print to file
        print_receiver_file();

        
        closesocket(socket);
        WSACleanup();
	    fclose(file);

        // open new socket and connect
        socket = create_socket();
        assert(connect(socket,&channel_addr,sizeof(channel_addr))!=SOCKET_ERROR,"connection falied");

        // ask for new filename (if "quit" - close the socket)
        printf("Plase enter file name\n");
        scanf("%s", &filename);

    } 

    // cleanup
    shutdown(socket,SD_BOTH);
    closesocket(socket);
	WSACleanup();

}

void update_receiver_file(FILE *file, char *msg){
    int num_of_byts;
    num_of_byts = (received_msg_size / ENCODED) * DECODED;
    server_stats->num_written += num_of_byts;
    fwrite(msg, 1, num_of_byts, file);
    received_msg_size = 0;
}

void fix_hamming_message(char msg[MAX_LENGTH], char fixed_msg[MAX_LENGTH], int msg_size){
    char substring[ENCODED];
    for(int i = 0; i < (msg_size / ENCODED); i++){
        calc_curr_substring(i, msg, substring);
        fix_hamming_substring(i, msg, fixed_msg);
        server_stats->num_received += ENCODED;    
    }

}

void calc_curr_substring(int start, char msg[MAX_LENGTH], char substring[ENCODED]){
    for(int i = 0; i < 31; i++){
        substring[i] = msg[start + i];
    }
}

void fix_hamming_substring(int start, char substring[ENCODED], char fixed_msg[MAX_LENGTH]){
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

    

}

void respond_to_sender(SOCKET socket, socketaddr *channel_addr) {
	char msg[100];
	sprintf(msg, "%d#%d#%d", server_stats->num_received, server_stats->num_written, server_stats->num_errors_fixed);
	write_socket(socket, &channel_addr, msg, 100);
}