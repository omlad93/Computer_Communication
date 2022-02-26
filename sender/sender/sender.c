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
	memset(&channel_addr, 0, sizeof(channel_addr));
	set_address(&channel_addr, port, ip);
    SOCKET socket = create_socket();
    //ask for file
    printf("Plase enter file name\n");
    scanf("%s", &filename);
    //file = fopen(filename, "rb");
    while (!strcmp(filename, "quit")){
        file = fopen(filename, "rb");
        if (file == NULL){
            printf("Error in openninf file\n");
            break;
        }
        //loop over the file
        sender_index = 0;
        while(fread(encoded_msg, 1, 26, file) > 0){
            //apply hamming code
            hamming(decoded_msg, encoded_msg);
            update_buffer(encoded_msg, socket, channel_addr);
        }
        //send
        write_socket(socket, &channel_addr, SENDER_BUFFER, sender_index);

        //wait for an answer
        socket =  read_socket(socket,  &channel_addr, SENDER_BUFFER, MAX_LENGTH);

        //close socket and report number of bytes that were witten and read
        print_output();
	    closesocket(socket);
	    fclose(file);

        //open new socket and connect
	    //memset(&channel_addr, 0, sizeof(channel_addr));
	    //set_address(&channel_addr, port, ip);
        SOCKET socket = create_socket();

        //ask for new filename (if "quit" - close the socket)
        printf("Plase enter file name\n");
        scanf("%s", &filename);
        if(!strcmp(filename, "quit")){
            file = fopen(filename, "rb");
        }

    }

    //cleanup
    closesocket(socket);
	fclose(file);  
}

/* applaies hamming code to 26 bits message
    returns 31 bits message*/
/*void hamming(char decoded_msg[DECODED], char encoded_msg[ENCODED]){
    char parity[PARITY_BITS];
    parity_bits(decoded_msg, parity);
    int j = 0;
    for(int i = 0; i < ENCODED; i++){
        switch (i){
            case 1:
                encoded_msg[i] = parity[0];
                break;
            case 2:
                encoded_msg[i] = parity[1];
                break;
            case 4:
                encoded_msg[i] = parity[2];
                break;
            case 8:
                encoded_msg[i] = parity[3];
                break;
            case 16:
                encoded_msg[i] = parity[4];
                break;
            default:
                encoded_msg[i] = decoded_msg[j];
                j++;
                break;
        }
    }

}*/


/* applaies hamming code to 26 bits message
    returns 31 bits message*/
void hamming(char decoded_msg[DECODED], char encoded_msg[ENCODED]){
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

}


int print_output(){
    
}

void update_buffer(char encoded_msg[ENCODED], SOCKET socket, socketaddr addr){
    for (int i = 0; i < ENCODED; i++){
        SENDER_BUFFER[sender_index + i] = encoded_msg[i];
    }
    sender_index += ENCODED;
    if (sender_index > MAX_LENGTH - ENCODED){
        Sleep(50);
        write_socket(socket, &addr, SENDER_BUFFER, sender_index);
        sender_index = 0;
    }
 
}