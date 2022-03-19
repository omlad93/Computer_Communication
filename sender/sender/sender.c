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
    char expanded_decoded_message[DECODED * 8];
    int start = 0;
    if (argc == 4){
        if(not(strcmp(argv[3], "-debug"))) { //DEBUG MODE
            log_err("working on debug mode (Fixed Port & Local IP)");
            port = HC_SENDER_PORT;
            ip = IP0;
        }
    }
    // create a socket
    socketaddr channel_addr;
    SOCKET socket = create_socket();
    memset(&channel_addr, 0, sizeof(channel_addr));
    set_address(&channel_addr, HC_SENDER_PORT, HC_SENDER_IP);
    assert_num(connect(socket, (SOCKADDR*)&channel_addr, sizeof(struct sockaddr)) != SOCKET_ERROR, "connection falied", WSAGetLastError());

    // ask for file
    printf("SENDER\n");
    printf("Please enter file name\n");

    assert(scanf("%s\n", filename) != 0, "Scanning Failed");
    //strcpy(filename, HC_INPUT);

    while (strcmp(filename, "quit") != 0) {
        assert(fopen_s(&file, filename, "r") == 0, "Error in openning file\n");
        printf("\tSending file: %s to Receiver Through the Noisy-Channel \n", filename);

        // Send to Channel: Size + Message
        message_size_int = (get_msg_size(file)) * 8; // number of bits in the message
        encoded_message_size_int = (message_size_int / DECODED) * ENCODED;
        EXPANDED_MESSAGE = (char*)malloc(message_size_int * sizeof(char)); // decoded expanded buffer
        SENDER_BUFFER = (char*)malloc(encoded_message_size_int * sizeof(char)); // decoded expanded buffer
        rewind(file);
        itoa(encoded_message_size_int, size_encoded_message, 10);
        write_socket(socket, size_encoded_message, SHORT_MESSAGE);  // Sending Message Size
        printf("\tSending message size\n");
        // NEED TO WHAIT FOR ACK ???

        // loop over the file
        buff_current_size = 0;
        while (fread(decoded_msg, 1, DECODED, file) > 0) {  // reads 26 bytes from the file
            convert_msg_to_char_arr(decoded_msg, expanded_decoded_message, DECODED);
            /* FOR DEBUG ONLY */
           // printf("\expanded_decoded_message = %s \n", expanded_decoded_message);
            //printf("\tMesaage Converted\n");
            update_expanded_message_buffer(start*DECODED*8, expanded_decoded_message);
            start++;
            //    //apply hamming code
            //message_hamming(expanded_decoded_message, encoded_msg);
            //update_buffer(encoded_msg, socket, channel_addr);
            /* FOR DEGUG ONLY */
            //printf("\tdecoded msg : %s\n", decoded_msg);
            //printf("\tencoded msg : %s\n", encoded_msg);
        }
        //start = 0;
        /* FOR DEBUG ONLY */
        printf("\expanded_decoded_message = %s \n", EXPANDED_MESSAGE);
        for (int i = 0; i < (message_size_int / DECODED); i++) {
            copy_n_chars(EXPANDED_MESSAGE, decoded_msg, i * DECODED, DECODED);
            message_hamming(decoded_msg, encoded_msg);
            update_buffer(encoded_msg, socket, channel_addr);
            //start++;
        }
        write_socket(socket, SENDER_BUFFER, encoded_message_size_int);  // Sending Actual Message
        /* FOR DEBUG ONLY */
        //printf("\tSENDER_BUFFER = %s \n", SENDER_BUFFER);
        //printf("\tSender Read File: buffer size is %d\n", buff_current_size);
        //printf("\tSent message to socket (Channel) [%dB]\n", encoded_message_size_int);

        // close socket and report number of bytes that were witten and read
        print_output();
        closesocket(socket);
        free(SENDER_BUFFER);
        free(EXPANDED_MESSAGE);
        WSACleanup();
        fclose(file);

        // open new socket and connect
        socket = create_socket();
        assert_num(connect(socket, (SOCKADDR*)&channel_addr, sizeof(struct sockaddr)) != SOCKET_ERROR, "connection falied", WSAGetLastError());
        // ask for new filename (if "quit" - close the socket)
        printf("Please enter file name\n");
        assert(scanf("%s", filename) != 0, "Scanning Failed");
    }
    log_err("\tFinished");
    // cleanup
    shutdown(socket, SD_BOTH);
    closesocket(socket);
    WSACleanup();
    // print output ???
    return 0;
}

void convert_msg_to_char_arr(char* orig_msg, char* parsed_msg, int orig_msg_size) {
    int val;
    for (int i = 0; i < orig_msg_size; i++) {
        for (int j = 0; j < 8; j++) {
            val = BIT_EVAL_R(orig_msg[i], j);
            if (val == 1) {
                parsed_msg[8 * i + (8 - 1 - j)] = '1';
            }
            else {
                parsed_msg[8 * i + (8 - 1 - j)] = '0';
            }
             
        }
    }
}

void copy_n_chars(char* source, char* dest, int start, int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = source[start + i];
    }
}

void update_expanded_message_buffer(int start, char* decoded_msg) {
    for (int i = 0; i < DECODED*8; i++) {
        EXPANDED_MESSAGE[start + i] = decoded_msg[i];
    }
}

int get_msg_size(FILE* file) {
    int num = 0;
    while (fread(MESSAGE_BUFFER, 1, DECODED, file) > 0) {  // reads 26 bytes from the file
        num++;
    }
    return num * DECODED;
}

/* applaies hamming code on 26 bytes of the message
    stores the result in encoded message*/
void message_hamming(char* decoded_msg, char* encoded_msg) {
    uint32_t encoded_msg_int, decoded_msg_int;
    uint32_t masked = 0;
    uint32_t parity_bit = 0;

    decoded_msg_int = convert_msg_to_int(decoded_msg);
    /* FOR DEBUG ONLY */
    //printf("\tdecoded_msg_int : %d\n", decoded_msg_int);
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

    convert_msg_to_string(encoded_msg, encoded_msg_int);
}

/* Gets an int and stores it as a string in a buffer */
void convert_msg_to_string(char* encoded_msg, uint32_t encoded_msg_int) {
    for (int i = 0; i < ENCODED; i++) {
        if ((encoded_msg_int >> i & 1)) {
            encoded_msg[i] = '1';
        } else {
            encoded_msg[i] = '0';
        }
    }
}

/* Gets a string and stores it to int */
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

// TODO
int print_output() {
    return 0;
}

/* Updates sender buffer with next chunk of encoded 31 bits */
void update_buffer(char encoded_msg[ENCODED], SOCKET socket, socketaddr addr) {
    for (int i = 0; i < ENCODED; i++) {
        SENDER_BUFFER[buff_current_size + i] = encoded_msg[i];
    }
    buff_current_size += ENCODED;
    if (buff_current_size > MAX_LENGTH - ENCODED) {  // should not get here because we did dynamic buffer allocation
        Sleep(50);                                   // ???
        write_socket(socket, SENDER_BUFFER, buff_current_size);
        buff_current_size = 0;
    }
}
