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
    } else {
        noise->type = DETERMINISTIC;
        noise->n = a1;
    }
    noise->flipped = 0;
}

/*
    apply determinstic noise on transmitted data
    Called by apply_noise(...)
*/
void apply_deterministic(Noise_p noise, str data, int size) {
    for (int i = 0; i < size; i++) {
        if (i == noise->n - 1) {
            data[i] = flip(data[i], i);
            noise->flipped++;
        }
    }
}

/*
    apply randomize noise on transmitted data
    Called by apply_noise(...)
*/
void apply_randomized(Noise_p noise, str data, int size) {
    int flipping;
    for (int i = 0; i < size; i++) {
        flipping = random_double() < noise->probability;
        data[i] = (flipping) ? flip(data[i], i) : data[i];
        noise->flipped = flipping ? noise->flipped + 1 : noise->flipped;
    }
}

/*
    apply noise on incoming data, according to noise model
*/
int apply_noise(Noise_p noise, str data, int size) {
    if (not(strcmp(noise->type, RANDOMIZE))) {
        apply_randomized(noise, data, size);
    } else {
        apply_deterministic(noise, data, size);
    }
    return noise->flipped;
}

int main(int argc, char* argv[]) {
    /* Questions:
     * who supplies IP for server and sender?
     * Who supplies self port?
     *
     */
    int a1, a2, selection, size=0, write_size=0;
    int counter = 0;
    str noise_type;
    assert_num((argc <= 4) & (argc > 1), "Noisy Channel Got Unexpected Numer og Arguments", argc);
    noise_type = argv[1];
    a1 = atoi(argv[2]);
    if (argc == 4) {
        a2 = atoi(argv[3]);
    } else {
        a2 = 0;
    }
    // a2 = (argc == 3) ? atoi(argv[3]) : 0;
    Noise_p noise = (Noise_p)(calloc(1, sizeof(Noise)));
    generate_noise(noise, noise_type, a1, a2);
    SDP sdp = &sd;
    fd_set sender_fds, receiver_fds;
    socketaddr receiver_sa, sender_sa;

    // INIT

    // Channel <-> Sender
    while (not(sdp->open_channel)) {
        sleep(50);
    }
    SOCKET sender_socket = create_socket();
    SOCKET receiver_socket = create_socket();
    set_address(&sender_sa, sdp->sender_port, sdp->sender_ip);
    set_address(&receiver_sa, sdp->receiver_port, sdp->receiver_ip);
    bind_socket(sender_socket, &sender_sa);
    bind_socket(receiver_socket, &receiver_sa);
    listen(sender_socket,0);
    listen(receiver_socket,0);

    // Channel <-> Server
    
    while (TRUE) {
        accept(sender_socket,&sender_sa,sizeof(sender_sa));
        accept(receiver_socket,&receiver_sa,sizeof(receiver_sa));
        FD_ZERO(&sender_fds);
        FD_ZERO(&receiver_fds);
        FD_SET(sender_socket, &sender_fds);
        FD_SET(receiver_socket, &receiver_fds);
        selection = select(2, &sender_fds, &receiver_fds, NULL, NULL);
        assert(selection >= 0, "Selection Failed [Channel]");
        // if (FD_ISSET(server_socket, &fs)) {
        //     log_err("Channel Finished Reading From Socket");
        //     break;  // DONE
        
        size = read_socket(sender_socket, &sender_sa, CHANNEL_BUFFER, MAX_LENGTH);
        // FLIPPED_COUNTER += apply_noise(noise, CHANNEL_BUFFER, size);
        write_size = write_socket(receiver_socket, &receiver_sa, CHANNEL_BUFFER, size);

        size = read_socket(receiver_socket, &receiver_sa, CHANNEL_BUFFER, MAX_LENGTH);  // read the message from the server
        write_size = write_socket(sender_socket, &sender_sa, CHANNEL_BUFFER, size);
        if (not(WSAEventSelect(sender_socket,WSACreateEvent(),FD_CLOSE) ||  WSAEventSelect(receiver_socket,WSACreateEvent(),FD_CLOSE))){
            break;
        }
        // break if shutdown()
        
    }
    free(noise);
    closesocket(receiver_socket);
    closesocket(sender_socket);
    WSACleanup();
    printf("%d, %d", size, write_size);
    // print_channel_output(server_ip, sender_ip);
    return 0;
}