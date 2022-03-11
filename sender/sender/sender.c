#include "sender/sender.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    FILE* file;
    char* ip = argv[1];
	int port = atoi(argv[2]);
	char* filename;
    char decoded_msg[DECODED];
    char encoded_msg[ENCODED];
    //char *total_encoded_msg[];

    //create a socket
    socketaddr channel_addr;
    SOCKET socket = create_socket();
	memset(&channel_addr, 0, sizeof(channel_addr));
	set_address(&channel_addr, port, ip);
    update_sharedata(SENDER, port, ip);
    assert(connect(socket,&channel_addr,sizeof(channel_addr))!=SOCKET_ERROR,"connection falied");
    //ask for file
    printf("Please enter file name\n");
    scanf("%s", &filename);
    //file = fopen(filename, "rb"); 

    while (!strcmp(filename, "quit")){
        file = fopen(filename, "rb");
        if (file == NULL){
            printf("Error in openninf file\n");
            break;
        }

        //loop over the file
        buff_current_size = 0;
        while(fread(encoded_msg, 1, 26, file) > 0){
            //apply hamming code
            hamming(decoded_msg, encoded_msg);
            update_buffer(encoded_msg, socket, channel_addr);
        }
        //send
        write_socket(socket, &channel_addr, SENDER_BUFFER, buff_current_size);

        //wait for an answer
        socket =  read_socket(socket,  &channel_addr, SENDER_BUFFER, MAX_LENGTH);

        //close socket and report number of bytes that were witten and read
        print_output();
	    closesocket(socket);
        WSACleanup();
	    fclose(file);

        //open new socket and connect
	    //memset(&channel_addr, 0, sizeof(channel_addr));
	    //set_address(&channel_addr, port, ip);
        socket = create_socket();
        assert(connect(socket,&channel_addr,sizeof(channel_addr))!=SOCKET_ERROR,"connection falied");

        //ask for new filename (if "quit" - close the socket)
        printf("Plase enter file name\n");
        scanf("%s", &filename);
    }
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
    parity_bits(decoded_msg[DECODED],check_bits[PARITY_BITS]);
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
    
}

void update_buffer(char encoded_msg[ENCODED], SOCKET socket, socketaddr addr){
    for (int i = 0; i < ENCODED; i++){
        SENDER_BUFFER[buff_current_size + i] = encoded_msg[i];
    }
    buff_current_size += ENCODED;
    if (buff_current_size > MAX_LENGTH - ENCODED){
        Sleep(50); // ???
        write_socket(socket, &addr, SENDER_BUFFER, buff_current_size);
        buff_current_size = 0;
    }
 
}