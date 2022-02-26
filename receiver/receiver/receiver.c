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
    SOCKET socket;
    // create a socket
    socketaddr channel_addr, receiver_addr;
    server_stats = (stats*)calloc(1, sizeof(stats));
    received_msg_size = 0;
    memset(&channel_addr, 0, sizeof(channel_addr));
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    set_address(&channel_addr, port, ip);
    socket = create_socket();
    bind_socket(socket, &receiver_addr);
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
        while(TRUE){
            status = server_loop(socket, &channel_addr, RECEIVER_BUF, MAX_LENGTH); 
            if (status == -1){
                break;
            } else {
                received_msg_size += status;

                //encode hamming message
                fix_hamming_message(msg, fixed_msg, received_msg_size);

                //write the received message to file
                update_receiver_file(file, msg);

                // write the received message to file
            }
        }

        //sends a respond 
        respond_to_sender(socket, &channel_addr);

        closesocket(socket);
        fclose(file);

        // print to file
        print_receiver_file();

        // open new socket and connect
        memset(&channel_addr, 0, sizeof(channel_addr));
        memset(&receiver_addr, 0, sizeof(receiver_addr));
        set_address(&channel_addr, port, ip);
        socket = create_socket();
        bind_socket(socket, &receiver_addr);

        // ask for new filename (if "quit" - close the socket)
        printf("Plase enter file name\n");
        scanf("%s", &filename);

    } 

    // cleanup
    closesocket(socket);
	fclose(file);

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