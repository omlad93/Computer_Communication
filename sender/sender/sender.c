#include "sender/sender.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    FILE* file;
    char* ip = argv[1];
	int port = atoi(argv[2]);
	char* filename;
    char *decoded_msg[DECODED];
    char *encoded_msg[ENCODED];
    //char *total_encoded_msg[];
    do {
        //create a socket
        struct sockaddr_in channel_addr;
	    memset(&channel_addr, 0, sizeof(channel_addr));
	    set_address(&channel_addr, port, ip);
        SOCKET socket = create_socket();
        //ask for file
        printf("Plase enter file name\n");
        scanf("%s", &filename);
        file = fopen(filename, "rb");

        //loop over the file
        while(fread(encoded_msg, 1, 11, file) > 0){
            //apply hamming code
            hamming(decoded_msg, encoded_msg);
            update_buffer(decoded_msg, socket, &channel_addr);
        }
        //send
        //write_socket(socket, &channel_addr, send_buf_cur_ind[0], &channel_addr);

        //wait for an answer
        //socket = read_socket(socket, &channel_addr, REC_BUF, LARGE_NUM);

        //close socket and report number of bytes that were witten and read
        print_output();
	    closesocket(socket);
	    fclose(file);

        //open new socket and connect
	    memset(&channel_addr, 0, sizeof(channel_addr));
	    set_address(&channel_addr, port, ip);
        SOCKET socket = create_socket();

        //ask for new filename (if "quit" - close the socket)
        printf("Plase enter file name\n");
        scanf("%s", &filename);
        if(!strcmp(filename, "quit")){
            file = fopen(filename, "rb");
        }

    } while (!strcmp(filename, "quit"));   
}

void hamming(char* decoded_msg, char* encoded_msg){

}