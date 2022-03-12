#include "sender.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    FILE* file;
    char* ip = argv[1];
	int port = atoi(argv[2]);
    char filename[MAX_LENGTH];
    char decoded_msg[DECODED];
    char encoded_msg[ENCODED];
    //char *total_encoded_msg[];

    //create a socket
    socketaddr channel_addr;
    SOCKET socket = create_socket();
	memset(&channel_addr, 0, sizeof(channel_addr));
	set_address(&channel_addr, HC_SENDER_PORT, HC_SENDER_IP);
    // update_sharedata(SENDER, port, ip);
    printf("Sender\n\tConnecting\n");
    assert_num(connect(socket, (SOCKADDR*) &channel_addr,sizeof(struct sockaddr))!=SOCKET_ERROR,"connection falied", WSAGetLastError());
    //ask for file
    printf("\tPlease enter file name\n");
    //scanf_s("%s", filename, (unsigned int)sizeof(filename));
    //file = fopen(filename, "rb"); 
    strcpy(filename, HC_INPUT);
    printf("got file: %s\n", filename);

    while (strcmp(filename, "quit") != 0){
        assert(fopen_s(&file, filename, "rb")==0,"Error in openning file\n");
 
        //loop over the file
        buff_current_size = 0;
        while(fread(encoded_msg, 1, 26, file) > 0){
            //apply hamming code
            hamming(decoded_msg, encoded_msg);
            update_buffer(encoded_msg, socket, channel_addr);
            log_err("\treading file");
        }
        printf("\tSender Read File: buffer size is %d\n",buff_current_size);
        
        //send
        write_socket(socket, SENDER_BUFFER, buff_current_size);

        //wait for an answer
        //socket =  read_socket(socket, SENDER_BUFFER, MAX_LENGTH);

        //close socket and report number of bytes that were witten and read
        print_output();
	    closesocket(socket);
        WSACleanup();
	    fclose(file);

        //open new socket and connect
	    //memset(&channel_addr, 0, sizeof(channel_addr));
	    //set_address(&channel_addr, port, ip);
        socket = create_socket();
        assert_num(connect(socket, (SOCKADDR*)&channel_addr, sizeof(struct sockaddr)) != SOCKET_ERROR, "connection falied", WSAGetLastError());
        //ask for new filename (if "quit" - close the socket)
        printf("\tPlase enter file name\n");
        assert(scanf("%s", filename) != 0, "Scanning Failed");
    }
    log_err("\tFinished");
    //cleanup
    shutdown(socket,SD_BOTH);
    closesocket(socket);
    WSACleanup();
    // print output ???
    return 0;
}


/* applaies hamming code to 26 bits message
    returns 31 bits message*/
/*void hamming(char decoded_msg[DECODED], char encoded_msg[ENCODED]){
    int j,k;
    j = 0;
    k = 0; 
    for(int i = 0; i < 31; i++){
        //check if i is an hamming bit position
        if(i == ((int)pow(2,k)-1)){ 
            encoded_msg[i] = 0;
            k++;
        }
        //copy the origin msg bit
        else{
            encoded_msg[i] = decoded_msg[j];
            j++;
        }
    }

    //calculate hamming bits
    for(int i = 0; i < 5; i++){
        int pos = (int)pow(2,i);
        int hamming_bit = calc_hamming_bit(pos, encoded_msg);
        encoded_msg[pos - 1] = hamming_bit;
    }

}*/

/* applaies hamming code to 26 bits message
    returns 31 bits message*/
void hamming(char decoded_msg[DECODED], char encoded_msg[ENCODED]){
    char check_bits[PARITY_BITS]; 
    parity_bits(decoded_msg,check_bits);
    int j = 0;
    for(int i = 0; i < ENCODED; i++){
        switch (i)
        {
        case 1:
            encoded_msg[i] = check_bits[0];
            break;
        case 2:
            encoded_msg[i] = check_bits[1];
            break;
        case 4:
            encoded_msg[i] = check_bits[2];
            break;
        case 8:
            encoded_msg[i] = check_bits[3];
            break;
        case 16:
            encoded_msg[i] = check_bits[4];
            break;           
        default:
            encoded_msg[i] = decoded_msg[j];
            j++;
            break;
        }
    }
}

// TODO
int print_output(){
    return 0;
}

void update_buffer(char encoded_msg[ENCODED], SOCKET socket, socketaddr addr){
    for (int i = 0; i < ENCODED; i++){
        SENDER_BUFFER[buff_current_size + i] = encoded_msg[i];
    }
    buff_current_size += ENCODED;
    if (buff_current_size > MAX_LENGTH - ENCODED){
        Sleep(50); // ???
        write_socket(socket, SENDER_BUFFER, buff_current_size);
        buff_current_size = 0;
    }
 
}