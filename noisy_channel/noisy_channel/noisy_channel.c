#include "noisy_channel.h"

/*
    set noise with proper parameters according to user input
*/
void generate_noise(Noise_p noise, str noise_type, int a1, int a2) {
    int is_random = not(strcmp(noise_type, RANDOMIZE));
    int is_deterministic = not(strcmp(noise_type, DETERMINISTIC));
    int cond = is_random ^ is_deterministic;
    assert(cond, "Failed to Choose Noise Type");
    if (is_random) {
        noise->type = RANDOMIZE;
        srand(a2);
        noise->seed = a2;
        noise->probability = a1 / pow(2.0, 16.0);
        printf("\t Generated Noise: Random with p=%f%%\n", 100*noise->probability);
    } else {
        noise->type = DETERMINISTIC;
        noise->n = a1;
        printf("\t Generated Noise: Deterministic with n=%d\n", noise->n);

    }
    noise->flipped = 0;
}

/*
    apply determinstic noise on transmitted data
    Called by apply_noise(...)
*/
void apply_deterministic(Noise_p noise, str data, int size, int verbose) {
    int next_noised_bit = noise->n - 1;
    int bit_to_flip;  // idx of bit to flip (from r)
    if (verbose) {
        printf("Applying Noise on a %d bytes [%d bits] message: Excpecting ~%d flips\n",size,size*BITS_PER_BITE, (int)size*BITS_PER_BITE/noise->n);
    }
    for (int i = 0; i < size; i++) {
        if ((int)next_noised_bit / BITS_PER_BITE == i) {
            bit_to_flip = BITS_PER_BITE - (next_noised_bit % BITS_PER_BITE);
            data[i] = BIT_FLIP_R(data[i], bit_to_flip);
            next_noised_bit += noise->n;
            noise->flipped++;
            if (verbose) {
                printf("%d) flipped bit %d\n",noise->flipped, i * BITS_PER_BITE + BITS_PER_BITE - bit_to_flip);
            }
        }
    }
}

/*
    apply randomize noise on transmitted data
    Called by apply_noise(...)
*/
void apply_randomized(Noise_p noise, str data, int size, int verbose) {
    int flipping, bit_to_flip;  // should flip, idx of bit to flip (from r);
    float rnd;
    srand(time(NULL));
    if (verbose) {
        printf("Applying Noise on a %d bytes [%d bits] message: Excpecting ~%d flips\n", size, size * BITS_PER_BITE, (int)(size * BITS_PER_BITE * noise->probability));
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < BITS_PER_BITE; j++) {
            rnd = (float)(rand())/(float)(RAND_MAX);
            flipping = rnd <= noise->probability;
            if (flipping) {
                bit_to_flip = BITS_PER_BITE - j;
                data[i] = BIT_FLIP_R(data[i], bit_to_flip);
                noise->flipped = flipping ? noise->flipped + 1 : noise->flipped;
                if (verbose) {
                    printf("%d) flipped bit %d (randomly choosed %f)\n",noise->flipped, i * BITS_PER_BITE + j,rnd);
                }
            }
        }
    }
}

/*
    apply noise on incoming data, according to noise model
*/
void apply_noise(Noise_p noise, str data, int size, int verbose) {
    if (not(strcmp(noise->type, RANDOMIZE))) {
        apply_randomized(noise, data, size,verbose);
    } else {
        apply_deterministic(noise, data, size, verbose);
    }
}

int main(int argc, char* argv[]) {
    /* Questions:
     * who supplies IP for server and sender?
     * Who supplies self port?
     */
    int a1, a2, selection, message_size_int, size = 0, write_size = 0;
    int counter = 0;
    str noise_type, channel_buffer;
    char user_command[4] = {"yes"};
    char message_size_str[MAX_LENGTH];

    assert_num((argc <= 4) & (argc > 1), "Noisy Channel Got Unexpected Numer og Arguments", argc);
    noise_type = argv[1];
    a1 = atoi(argv[2]);
    a2 = (argc == 4) ? atoi(argv[3]) : 0;
    Noise_p noise = (Noise_p)(calloc(1, sizeof(Noise)));
    generate_noise(noise, noise_type, a1, a2);
    SDP sdp = &sd;
    fd_set sender_fds, receiver_fds;
    socketaddr receiver_sa, sender_sa;

    // INIT

    // Channel <-> Sender
    while (not(sdp->open_channel)) {
        update_sharedata(SENDER, HC_SENDER_PORT, HC_SENDER_IP);
        update_sharedata(RECEIVER, HC_RECEIVER_PORT, HC_RECEIVER_IP);
        break;
    }
    SOCKET listen_socket_sender = create_socket();
    SOCKET listen_socket_receiver = create_socket();
    SOCKET sender_socket, receiver_socket;

    set_address(&sender_sa, sdp->sender_port, sdp->sender_ip);
    set_address(&receiver_sa, sdp->receiver_port, sdp->receiver_ip);
    bind_socket(listen_socket_sender, &sender_sa);
    bind_socket(listen_socket_receiver, &receiver_sa);
    assert_num(listen(listen_socket_sender, SOMAXCONN) == 0, "listening to sender Returned non-zero", WSAGetLastError());
    assert_num(listen(listen_socket_receiver, SOMAXCONN) == 0, "listening to recieverReturned non-zero", WSAGetLastError());
    int size_blah = sizeof(socketaddr);

    while (strcmp(user_command, "no") != 0) {
        // INIT
        // log_err("\tchannel is waiting for sockets :)");
        sender_socket = accept(listen_socket_sender, NULL, NULL);
        receiver_socket = accept(listen_socket_receiver, NULL, NULL);
        assert(sender_socket != INVALID_SOCKET, "sender socket is invalid");
        assert(receiver_socket != INVALID_SOCKET, "receiver socket is invalid");
        // log_err("\tchannel has accepted both sockets");
        FD_ZERO(&sender_fds);
        FD_ZERO(&receiver_fds);
        FD_SET(sender_socket, &sender_fds);
        FD_SET(receiver_socket, &receiver_fds);
        assert_num(selection = select(2, &sender_fds, &receiver_fds, NULL, NULL) != SOCKET_ERROR, "Selection Error", WSAGetLastError());
        log_err("\tSelection Complete");

        // Read From Sender:
        size = read_socket(sender_socket, message_size_str, SHORT_MESSAGE);
        message_size_int = atoi(message_size_str);
        channel_buffer = (str)calloc(message_size_int, sizeof(char));
        size = read_socket(sender_socket, channel_buffer, message_size_int);  // ADD SIZE PARSING
        printf("\tReceived message from socket (Sender) [%dB]\n", message_size_int);

        //printf("\n\tBefore:\n%s\n",channel_buffer);
        // Appling Noise
        apply_noise(noise, channel_buffer, size,TRUE);
        //printf("\n\tAfter:\n%s\n", channel_buffer);


        // Writing to Receiver:
        write_size = write_socket(receiver_socket, message_size_str, SHORT_MESSAGE);   // ADD SIZE PARSING
        write_size = write_socket(receiver_socket, channel_buffer, message_size_int);  // ADD SIZE PARSING
        printf("\tSent message to socket (Receiver) [%dB]\n", message_size_int);

        closesocket(receiver_socket);
        closesocket(sender_socket);
        do {
            log_err("countine?");
            assert(scanf("%s", user_command) != 0, "Scanning Failed");
            if (not(strcmp(user_command, "yes"))) break;
            if (not(strcmp(user_command, "no"))) break;
        } while (TRUE);
    }
    free(noise);
    closesocket(listen_socket_sender);
    closesocket(listen_socket_receiver);
    WSACleanup();
    printf("Read %d Bytes and Wrote %d Bytes, Applied Noise on %d bits\n", size, write_size, noise->flipped);
    return 0;
}