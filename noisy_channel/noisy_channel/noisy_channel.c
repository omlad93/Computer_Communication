#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif  // ! _CRT_SECURE_NO_WARNINGS
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif  // ! _WINSOCK_DEPRECATED_NO_WARNINGS
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
        printf("\tGenerated Noise: Random(%f%%) with seed=%d\n", 100 * noise->probability, noise->seed);
    } else {
        noise->type = DETERMINISTIC;
        noise->n = a1;
        printf("\tGenerated Noise: Deterministic(%d)\n", noise->n);
    }
    noise->flipped = 0;
    noise->transmitted = 0;
}

/*
    apply determinstic noise on transmitted data
    Called by apply_noise(...)
*/
void apply_deterministic(Noise_p noise, str data, int size, int verbose) {
    int next_noised_bit = noise->n - 1;
    if (verbose) {
        printf("\tApplying Noise on a %d bits message: Excpecting %d flips\n", size, (int)size / noise->n);
    }
    for (int i = 0; i < size; i++) {
        if (i == next_noised_bit) {
            data[i] = BYTE_FLIP(data[i]);
            next_noised_bit += noise->n;
            noise->flipped++;
            if (verbose) {
                printf("\t%d) flipped bit %d from %c to %c\n", noise->flipped, i, BYTE_FLIP(data[i]), data[i]);
            }
        }
    }
}

/*
    apply randomize noise on transmitted data
    Called by apply_noise(...)
*/
void apply_randomized(Noise_p noise, str data, int size, int verbose) {
    int flipping;  // should flip
    float rnd;
    if (verbose) {
        printf("\tApplying Noise on a %d bits message: Excpecting ~%d flips\n", size, (int)(size * noise->probability));
    }
    for (int i = 0; i < size; i++) {
        rnd = (float)(rand()) / (float)(RAND_MAX);
        flipping = rnd <= noise->probability;
        if (flipping) {
            data[i] = BYTE_FLIP(data[i]);
            noise->flipped++;
            if (verbose) {
                printf("\t%d) flipped bit %d from %c to %c\n", noise->flipped, i, BYTE_FLIP(data[i]), data[i]);
            }
        }
    }
}

/*
    apply noise on incoming data, according to noise model
*/
void apply_noise(Noise_p noise, str data, int size, int verbose) {
    noise->flipped = 0;
    if (not(strcmp(noise->type, RANDOMIZE))) {
        apply_randomized(noise, data, size, verbose);
    } else {
        apply_deterministic(noise, data, size, verbose);
    }
    noise->transmitted = size;
}

/*
set up sockets (assign ports, bind,listten)
if in hard_coded mode use HC_PORT_<CLIENT> instead of free ports.
*/
void initial_setup(SOCKET listen_socket_sender, socketaddr* sender_sa_p, SOCKET listen_socket_receiver, socketaddr* receiver_sa_p, int hard_coded) {
    int sender_port = hard_coded ? HC_SENDER_PORT : 0;
    int receiver_port = hard_coded ? HC_RECEIVER_PORT : 0;
    set_address(sender_sa_p, sender_port, IP0);
    set_address(receiver_sa_p, receiver_port, IP0);
    bind_socket(listen_socket_sender, sender_sa_p);
    bind_socket(listen_socket_receiver, receiver_sa_p);
    int addlen = sizeof(SOCKADDR);
    assert_num(getsockname(listen_socket_sender, (SOCKADDR*)sender_sa_p, &addlen) == 0,
               "Couldn't get Port for Sender", WSAGetLastError());
    printf("Socket for Sender:   IP='%s' [local] , Port=%d\n", inet_ntoa(sender_sa_p->sin_addr), sender_sa_p->sin_port);
    assert_num(getsockname(listen_socket_receiver, (SOCKADDR*)receiver_sa_p, &addlen) == 0,
               "Couldn't get Port for Receiver", WSAGetLastError());
    printf("Socket for Receiver: IP='%s' [local] , Port=%d\n", inet_ntoa(sender_sa_p->sin_addr), receiver_sa_p->sin_port);
    assert_num(listen(listen_socket_sender, SOMAXCONN) == 0, "listening to sender Returned non-zero", WSAGetLastError());
    assert_num(listen(listen_socket_receiver, SOMAXCONN) == 0, "listening to receiver Returned non-zero", WSAGetLastError());
}

void channel_selection(SOCKET sender_socket, SOCKET receiver_socket, fd_set* sender_fds_p, fd_set* receiver_fds_p) {
    assert(sender_socket != INVALID_SOCKET, "sender socket is invalid");
    assert(receiver_socket != INVALID_SOCKET, "receiver socket is invalid");
    // log_err("\tchannel has accepted both sockets");
    FD_ZERO(sender_fds_p);
    FD_ZERO(receiver_fds_p);
    FD_SET(sender_socket, sender_fds_p);
    FD_SET(receiver_socket, receiver_fds_p);
    assert_num(select(2, sender_fds_p, receiver_fds_p, NULL, NULL) != SOCKET_ERROR, "Selection Error", WSAGetLastError());
}

int main(int argc, char* argv[]) {
    int a1, a2, message_size_int, size = 0, write_size = 0;
    int counter = 0;
    str noise_type, channel_buffer;
    char user_command[4] = {"yes"};
    char message_size_str[MAX_LENGTH];
    double ratio;
    assert_num((argc <= 5) & (argc >= 3), "Noisy Channel Got Unexpected Numer og Arguments", argc);
    int debug_mode = FALSE;
    if ((argc == 5) or ((argc == 4) and (not(strcmp(argv[1], "-d"))))) {  // DEBUG MODE
        if (not(strcmp(argv[argc - 1], "-debug"))) {
            log_err("working on debug mode (Fixed Ports & Verbose)");
            debug_mode = TRUE;
        }
    }
    noise_type = argv[1];
    a1 = atoi(argv[2]);
    a2 = (not(strcmp(argv[1], RANDOMIZE))) ? atoi(argv[3]) : 0;
    Noise_p noise = (Noise_p)(calloc(1, sizeof(Noise)));
    fd_set sender_fds, receiver_fds;
    socketaddr receiver_sa, sender_sa;
    SOCKET listen_socket_sender = create_socket();
    SOCKET listen_socket_receiver = create_socket();
    SOCKET sender_socket, receiver_socket;
    // INIT
    initial_setup(listen_socket_sender, &sender_sa, listen_socket_receiver, &receiver_sa, debug_mode);
    generate_noise(noise, noise_type, a1, a2);
    while (strcmp(user_command, "no") != 0) {
        // Initiation
        sender_socket = accept(listen_socket_sender, NULL, NULL);
        receiver_socket = accept(listen_socket_receiver, NULL, NULL);
        channel_selection(sender_socket, receiver_socket, &sender_fds, &receiver_fds);

        // Read From Sender:
        size = read_socket(sender_socket, message_size_str, SHORT_MESSAGE);
        message_size_int = atoi(message_size_str);
        channel_buffer = (str)calloc(message_size_int, sizeof(char));
        size = read_socket(sender_socket, channel_buffer, message_size_int);  // ADD SIZE PARSING
        printf("\tReceived message from socket (Sender) [%dB]\n", message_size_int / BITS_PER_BYTE);

        //  Apply Noise
        apply_noise(noise, channel_buffer, size, debug_mode);

        // Write to Receiver:
        write_size = write_socket(receiver_socket, message_size_str, SHORT_MESSAGE);   // ADD SIZE PARSING
        write_size = write_socket(receiver_socket, channel_buffer, message_size_int);  // ADD SIZE PARSING
        printf("\tSent message to socket (Receiver) [%dB]\n", message_size_int / BITS_PER_BYTE);

        closesocket(receiver_socket);
        closesocket(sender_socket);
        ratio = 100 * ((float)noise->flipped / (float)(noise->transmitted));
        printf("\tTransffered %d bits [%dB], Applied Noise on %d bits (%f%% from all bits)\n",
               noise->transmitted, (int)(noise->transmitted / BITS_PER_BYTE), noise->flipped, ratio);

        do {
            log_err("continue?");
            assert(scanf("%s", user_command) != 0, "Scanning Failed");
            if (not(strcmp(user_command, "yes"))) break;
            if (not(strcmp(user_command, "no"))) break;
        } while (TRUE);
    }

    free(noise);
    closesocket(listen_socket_sender);
    closesocket(listen_socket_receiver);
    WSACleanup();
    return 0;
}