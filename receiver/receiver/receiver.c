#pragma once
#include "receiver.h"

int main(int argc, char* argv[]) {
    FILE* file;
    char* ip = argv[1];
    int port = atoi(argv[2]);
    char* filename;
    char* data_read;
    int status = 0;
    int size;
    SOCKET socket;
    // create a socket
    socketaddr channel_addr, receiver_addr;
    received_msg_size = 0;
    memset(&channel_addr, 0, sizeof(channel_addr));
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    set_address(&channel_addr, port, ip);
    socket = create_socket();
    bind_socket(socket, &receiver_addr);
    // ask for file
    printf("Plase enter file name\n");
    scanf("%s", &filename);
    file = fopen(filename, "wb");
    do {
        // read message
        while (1) {
            status = server_loop(socket, &channel_addr, data_read, MAX_LENGTH);  // what sould be the size?
            if (status == -1) {
                break;
            } else {
                received_msg_size += status;

                // encode hamming message

                // write the received message to file
            }
        }

        // sends a respond

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
        if (!strcmp(filename, "quit")) {
            file = fopen(filename, "rb");
        }

    } while (!strcmp(filename, "quit"));

    // cleanup
    closesocket(socket);
    fclose(file);
}