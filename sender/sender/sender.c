#include "sender.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    FILE* file;
    int message_size_int, encoded_message_size_int;
    char* ip = argv[1];
	int port = atoi(argv[2]);
    char filename[MAX_LENGTH];
    char size_encoded_message[SHORT_MESSAGE];
    char decoded_msg[DECODED];
    char encoded_msg[ENCODED];
    //create a socket
    socketaddr channel_addr;
    SOCKET socket = create_socket();
	memset(&channel_addr, 0, sizeof(channel_addr));
	set_address(&channel_addr, HC_SENDER_PORT, HC_SENDER_IP);
    assert_num(connect(socket, (SOCKADDR*) &channel_addr,sizeof(struct sockaddr))!=SOCKET_ERROR,"connection falied", WSAGetLastError());

    //ask for file
    printf("SENDER\n");
    printf("Please enter file name\n");

    //scanf_s("%s", filename, (unsigned int)sizeof(filename));
    //file = fopen(filename, "rb"); 
    strcpy(filename, HC_INPUT);
    

    while (strcmp(filename, "quit") != 0){

        assert(fopen_s(&file, filename, "r")==0,"Error in openning file\n");
        printf("\tSending file: %s to Receiver Through the Noisy-Channel \n", filename);

        // Send to Channel: Size + Message
        message_size_int = get_msg_size(file);
        encoded_message_size_int = (message_size_int / DECODED) * ENCODED;
        SENDER_BUFFER = (char*)malloc(encoded_message_size_int * sizeof(char));
        rewind(file);
        itoa(encoded_message_size_int, size_encoded_message, 10);
        write_socket(socket, size_encoded_message, SHORT_MESSAGE); // Sending Message Size
        // NEED TO WHAIT FOR ACK ???

        //loop over the file
        buff_current_size = 0;
        while(fread(decoded_msg, 1, DECODED, file) > 0){ // reads 26 bytes from the file
        //    //apply hamming code
            message_hamming(decoded_msg, encoded_msg);
            update_buffer(encoded_msg, socket, channel_addr);
            /* FOR DEGUG ONLY */
            printf("\tdecoded msg : %s\n", decoded_msg);
            printf("\tencoded msg : %s\n", encoded_msg);
        }
        write_socket(socket, SENDER_BUFFER, encoded_message_size_int); // Sending Actual Message
        printf("\tSender Read File: buffer size is %d\n",buff_current_size);
        printf("\tSent message to socket (Channel) [%dB]\n", encoded_message_size_int);
        
        //close socket and report number of bytes that were witten and read
        print_output();
	    closesocket(socket);
        free(SENDER_BUFFER);
        WSACleanup();
	    fclose(file);

        //open new socket and connect
        socket = create_socket();
        assert_num(connect(socket, (SOCKADDR*)&channel_addr, sizeof(struct sockaddr)) != SOCKET_ERROR, "connection falied", WSAGetLastError());
        //ask for new filename (if "quit" - close the socket)
        printf("Plase enter file name\n");
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

int get_msg_size(FILE* file) {
    int num = 0;
    while (fread(MESSAGE_BUFFER, 1, DECODED, file) > 0) { // reads 26 bytes from the file
        num++;
    }
    return num * DECODED;
}

/* applaies hamming code on 26 bytes of the message
    stores the result in encoded message*/
void message_hamming(char decoded_msg[DECODED], char encoded_msg[ENCODED]) {

    uint32_t encoded_msg_int, decoded_msg_int;
    uint32_t masked = 0;
    uint32_t parity_bit = 0;

    decoded_msg_int = convert_msg_to_int(decoded_msg);
    /* FOR DEBUG ONLY */
    printf("\tdecoded_msg_int : %d\n", decoded_msg_int);
    encoded_msg_int = decoded_msg_int;
    // calc 0 parity bit
    parity_bit = parity(decoded_msg_int, 0x55555555);
    if (parity_bit) {
        (encoded_msg_int) |= (1 << (0));
    }
    // calc 1 parity bit
    parity_bit = parity(decoded_msg_int, 0x66666666);
    if (parity_bit) {
        (encoded_msg_int) |= (1 << (1));
    }
    // calc 3 parity bit
    parity_bit = parity(decoded_msg_int, 0x38787878);
    if (parity_bit) {
        (encoded_msg_int) |= (1 << (3));
    }
    // calc 7 parity bit
    parity_bit = parity(decoded_msg_int, 0x7F807F80);
    if (parity_bit) {
        (encoded_msg_int) |= (1 << (7));
    }
    // calc 15 parity bit
    parity_bit = parity(decoded_msg_int, 0x7FFF8000);
    if (parity_bit) {
        (encoded_msg_int) |= (1 << (15));
    }
    /* FOR DEBUG ONLY */
    printf("\tencoded_msg_int : %d\n", encoded_msg_int);

    convert_msg_to_string(encoded_msg, encoded_msg_int);
}

/* Gets an int and stores it as a string in a buffer */
void convert_msg_to_string(char* encoded_msg, uint32_t encoded_msg_int) {
    for (int i = 0; i < ENCODED; i++) {
        if ((encoded_msg_int >> i & 1)) {
            encoded_msg[i] = '1';
        }
        else {
            encoded_msg[i] = '0';
        }
    }
}

/* Gets a string and stores it to int */
uint32_t convert_msg_to_int(char *msg) {
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

/* applaies hamming code to 26 bits message
    returns 31 bits message
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
*/
// TODO
int print_output(){
    return 0;
}

/* Updates sender buffer with next chunk of encoded 31 bits */
void update_buffer(char encoded_msg[ENCODED], SOCKET socket, socketaddr addr){
    for (int i = 0; i < ENCODED; i++){
        SENDER_BUFFER[buff_current_size + i] = encoded_msg[i];
    }
    buff_current_size += ENCODED;
    if (buff_current_size > MAX_LENGTH - ENCODED){ //should not get here because we did dynamic buffer allocation
        Sleep(50); // ???
        write_socket(socket, SENDER_BUFFER, buff_current_size);
        buff_current_size = 0;
    }
 
}