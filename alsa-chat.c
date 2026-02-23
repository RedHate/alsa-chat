/*
 * 
 * vchat
 * [8-Bit alsa voice chat]
 * 
 * An organically grown way of how not to create voice chat.
 * 
 * TCP, 8Bit, 31kbps, Stream masking with xor, Spacebar toggle
 * 60 second key up time out
 * 
 * c99 compliant
 * 
 * 
 * **NOTE**
 * 
 *   you may encounter an issue with there being a douplicate time
 * definition in one of your alsa headers, alsa/config.h iirc. Comment
 * it out of that header. You have "time.h" in c99.
 * 
 *   Also may complain about implicit declaration of nano sleep. -It
 * works on my end so test it before replacing it with some crap like 
 * sleep();
 * 
 * 
 * Use plug devices! 
 * 			
 * 		Like so: plug:hw:1 rather than hw:1
 * 
 * 
 *                               -Ultros was here in 2026
 * 
 */

// Uncomment to enable debugging // comment to turn off
#define _DEBUG_ 1

// Headers
#include <alloca.h>
#include <termios.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

// Alsa related definitions
#define SAMPLE_RATE 4000
#define CHANNELS    1
#define FRAME_SIZE  8
#define FORMAT      SND_PCM_FORMAT_S8
#define TYPE		char

// Microphone time out
#define MIC_TIMEOUT_SECONDS 60
int key_up = 0;
time_t key_up_start;
float mic_gain = 1.0f;

// Var for shutdown
int running = 1;

// Global ALSA handles
snd_pcm_t *capture_handle;
snd_pcm_t *playback_handle;

// XOR related definitions
#define XOR_KEY_LEN  20
#define XOR_FORWARD  0
#define XOR_BACKWARD 1
#define KEY_ARRAY    8

// XOR keys
uint8_t keys[KEY_ARRAY][XOR_KEY_LEN] = { 
	{ 0xec, 0x1d, 0x53, 0xdf, 0xc3, 0x23, 0x28, 0xfa, 0xe0, 0xfe, 0x95, 0xc9, 0x51, 0x2a, 0x65, 0x63, 0xe6, 0xb4, 0xf9, 0x3a },
	{ 0x67, 0x10, 0xca, 0x9c, 0x54, 0x66, 0x13, 0xb8, 0x50, 0x67, 0x05, 0x99, 0xa0, 0xaa, 0x53, 0x16, 0xec, 0x5e, 0xcc, 0x3c },
	{ 0x04, 0xf0, 0x24, 0x97, 0x37, 0x23, 0x05, 0x7b, 0x06, 0xe7, 0x0d, 0x48, 0xf2, 0x41, 0x6b, 0xca, 0x55, 0xad, 0x9f, 0xf6 },
	{ 0x87, 0xe9, 0x0a, 0x8c, 0xae, 0x4e, 0x5c, 0xb3, 0xa9, 0x25, 0x7e, 0xa5, 0x34, 0x36, 0x91, 0xf3, 0xdd, 0x1e, 0x1c, 0xc8 },
	{ 0x6d, 0xce, 0xab, 0x28, 0xc7, 0x37, 0xea, 0x60, 0x39, 0x59, 0x55, 0x57, 0xb8, 0x75, 0xce, 0x35, 0x0a, 0x9e, 0x21, 0x46 },
	{ 0x4a, 0x38, 0x9b, 0x49, 0x07, 0xae, 0x20, 0x28, 0x62, 0x85, 0x03, 0x40, 0xc0, 0x72, 0xfa, 0x36, 0x05, 0xe6, 0x97, 0xeb },
	{ 0x86, 0x51, 0xfa, 0xf7, 0xdb, 0xdd, 0xd4, 0x72, 0xbc, 0xe8, 0x11, 0x10, 0x93, 0x63, 0x68, 0x70, 0x3c, 0xb0, 0xec, 0x69 },
	{ 0x6d, 0xce, 0xab, 0x28, 0xc7, 0x37, 0xea, 0x60, 0x39, 0x59, 0x55, 0x57, 0xb8, 0x75, 0xce, 0x35, 0x0a, 0x9e, 0x21, 0x46 },
};

// Print the key info
void print_keys() {
	// Loop through the keys
	int i, j;
	for (i = 0; i < KEY_ARRAY-1; i++) {
		// Loop through key length
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// print the bytes
			printf("0x%02hX, ", (unsigned char)keys[i][j]);
		}
		// print a new line for each key
		printf("\n");
	}
}

// XOR the keyset with a passphrase
void xor_keys(uint8_t keys[KEY_ARRAY][XOR_KEY_LEN], const char *password){
	// Set key position to 0
	int keypos = 0;
	// Loop through the keys
	int i, j;
	// positive
	for (i = 0; i < KEY_ARRAY-1; i++) {
		// Loop through selected key
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// XOR the key
			keys[i][j] = (uint8_t)((uint8_t)password[keypos] ^ (uint8_t)keys[i][j]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == strlen(password)) keypos = 0;
		}
	}
	// negative
	for (i = KEY_ARRAY-1; i > 0; i--) {
		// Loop through selected key
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// XOR the key
			keys[i][j] = (uint8_t)((uint8_t)password[keypos] ^ (uint8_t)keys[i][j]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == strlen(password)) keypos = 0;
		}
	}
}

// Directional XOR fnc (doesn't need to be global, even if that's how you think it should be done. i scope my code.)
void xor_directional(uint8_t *buffer, size_t size, uint8_t *key, int direction) {
	// Used for xor position
	int keypos = 0;
	// XOR forward / backward
	if (direction == XOR_FORWARD) {
		// Lazy XOR
		uint32_t i;
		for (i = 0; i < size; i++) {
			// XOR shift the data according to the key and its relative read position
			buffer[i] = (uint8_t)((uint8_t)key[keypos] ^ (uint8_t)buffer[i]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == XOR_KEY_LEN) keypos = 0;
		}
	}
	else if (direction == XOR_BACKWARD) {
		// Lazy XOR
		uint32_t i;
		for (i = size; i > 0; i--) {
			// XOR shift the data according to the key and its relative read position
			buffer[i] = (uint8_t)((uint8_t)key[keypos] ^ (uint8_t)buffer[i]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == XOR_KEY_LEN) keypos = 0;
		}
	}
}

// This adds a reversable layer of noise to our signal
void xor4x(uint8_t *buffer, size_t size) {
	// Set the direction to 0
	int direction = 0;
	// loop through keys
	int i;
	// positive
	for (i = 0; i < KEY_ARRAY-1; i++) {
		// odds
		if (i & 1)
			direction = XOR_FORWARD;
		// evens
		if (!(i & 1))
			direction = XOR_BACKWARD;
		//xor it in a direction
		xor_directional(buffer, size, keys[i], direction);
	}
	// Negative
	for (i = KEY_ARRAY-1; i > 0; i--) {
		// odds
		if (i & 1)
			direction = XOR_FORWARD;
		// evens
		if (!(i & 1))
			direction = XOR_BACKWARD;
		//xor it in a direction
		xor_directional(buffer, size, keys[i], direction);
	}
}

// This adds a lot of noise to our signal. good. also reversable.
void byte_swap(uint8_t *in, uint32_t size){
	
	// set up a temporary buffer
	uint8_t t_buf[size];
	
	int i;
	for(i=0; i<size; i+=2){
		t_buf[i+0] = in[i+1];
		t_buf[i+1] = in[i+0];;
	}
	
	// put to original buffer
	for(i = 0; i < size; i++){
		in[i] = t_buf[i];
	}
}

// This may give an implicit declatation warning, but it seems to be functional try 1000000 as param
void sleep_us(long microseconds) {
	// Local var
	struct timespec ts;
	// Set time granularity to the microsecond
	ts.tv_sec = microseconds / 1000000;
	ts.tv_nsec = (microseconds % 1000000) * 1000;
	// Sleep
	nanosleep(&ts, NULL);
}

// Local Var
struct termios term;

void restore_terminal(void) {
	tcsetattr(STDIN_FILENO,TCSANOW,&term);
}

// Set terminal to non-blocking input
void enable_nonblocking_input() {
    // Get attributes
    tcgetattr(STDIN_FILENO, &term);
    // Set exit
    atexit(restore_terminal);
    // Set Flags
    term.c_lflag &= ~(ICANON | ECHO); // raw input
    // Set attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    // Set stdin to non blocking
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

// Initialize ALSA for capture or playback
int init_alsa(snd_pcm_t **handle, const char *device, snd_pcm_stream_t stream, uint32_t channels) {
    
    // Local vars
    int err;
    snd_pcm_hw_params_t *params;
    
    // Init a handle for the PCM steam from a chosen device
    if ((err = snd_pcm_open(handle, device, stream, 0)) < 0) {
        fprintf(stderr, "ALSA open error: %s\n", snd_strerror(err));
        return err;
    }
	
	// I assume this allocates memory for params, return doesn't allow error checking
	snd_pcm_hw_params_alloca(&params);
	
	// Init all channel params and error check
    if ((err = snd_pcm_hw_params_any(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_any error: %s\n", snd_strerror(err));
		return err;
	}
	
	// (ignore that gcc throws warning on this or fix it lol)
	// set resample since we want to adjust the rate 
	uint32_t resample = 1;
    if ((err = snd_pcm_hw_params_set_rate_resample(*handle, params, *(unsigned int*)&resample)) < 0) {
        //fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate_resample error: %s\n", snd_strerror(err));
		return err;
	}
	// Set access
    if ((err = snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_access error: %s\n", snd_strerror(err));
		return err;
	}
	// Set format
    if ((err = snd_pcm_hw_params_set_format(*handle, params, FORMAT)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_format error: %s\n", snd_strerror(err));
		return err;
	}
	
	// Set channels
    snd_pcm_hw_params_set_channels(*handle, params, channels);
    
	// Set rate
	uint32_t sample_rate = SAMPLE_RATE;
    if ((err = snd_pcm_hw_params_set_rate_near(*handle, params, &sample_rate, 0)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate error: %s\n", snd_strerror(err));
		return err;
	}
	// Set hw params
    if ((err = snd_pcm_hw_params(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params error: %s\n", snd_strerror(err));
		return err;
	}
	// Prepare for pcm
    if ((err = snd_pcm_prepare(*handle)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_prepare error: %s\n", snd_strerror(err));
		return err;
	}

    printf("ALSA initialized at %u Hz for %d\n", sample_rate, stream);

    return 0;
    
}

// Thread for receiving audio and playing
void* receive_play_audio(void* arg) {
	// Get the socket fd
    int sockfd = *(int*)arg;
    // Create a buffer for audio
    TYPE buffer[FRAME_SIZE*CHANNELS];
    // Receive some data
    while (running) {
		// Read into buffer from socket
		ssize_t r = recv(sockfd, buffer, sizeof(buffer), 0);
		if (r > 0){
			// Swap neighbouring bytes
			byte_swap((uint8_t*)buffer, sizeof(buffer));
			// Apply XOR
			xor4x((uint8_t*)buffer, sizeof(buffer));
			// 8 bit hi-low cutoff
			/*
			int c;
			for(c=0;c<sizeof(buffer);c++){
				if((((uint8_t)buffer[c] <= 0xFF) && ((uint8_t)buffer[c] >= 0xFC)) || 
				   (((uint8_t)buffer[c] <= 0x04) && ((uint8_t)buffer[c] >= 0x00))) buffer[c] = 0x00;
			}
			*/
			// Write buffer to playback handle
			snd_pcm_writei(playback_handle, buffer, r);
#ifdef _DEBUG_
			// Debug
			printf("\033[1;36mRecv) %ld\033[0m ", r);
			int c;
			for(c=0;c<sizeof(buffer);c++){
				printf("%02hX,", (unsigned char)buffer[c]);
			}
			printf("\n");
#endif
		}
    }
    // Return
    return NULL;
    
}

// Thread for capturing and sending
void* capture_send_audio(void* arg) {
	
	// Get the socket fd
    int sockfd = *(int*)arg;
    // Create a buffer for audio
    TYPE buffer[FRAME_SIZE*CHANNELS];
    
    // Send some data
    while (running) {
		// This needs to stay reading
		// Read in buffer from capture handle
		ssize_t r = snd_pcm_readi(capture_handle, buffer, sizeof(buffer));
		if (r > 0){
			// Is the mic on?
			if(key_up == 1){
				if((time(NULL)-key_up_start) < MIC_TIMEOUT_SECONDS){
					int c;
					// Adjust mic gain before modulation
					for (c = 0; c < sizeof(buffer); c++) {
						buffer[c] = (TYPE)(buffer[c] * mic_gain);
					}
#ifdef _DEBUG_
					// Debug
					printf("\033[1;35mSend) %ld\033[0m ", r);
					for(c=0;c<sizeof(buffer);c++){
						printf("%02hX,",  (unsigned char)buffer[c]);
					}
					printf("\n");
#endif
					// Apply XOR
					xor4x((uint8_t*)buffer, sizeof(buffer));
					// Swap neighbouring bytes
					byte_swap((uint8_t*)buffer, sizeof(buffer));
					// Send to sock
					r = send(sockfd, buffer, sizeof(buffer),0);
					
				}
				else{
					key_up = 0;
					printf("\033[1;31m[Mic off]\033[0m\n");
				}
			}
		}
    }
    // Return
    return NULL;
}

// Start chat 
int client(int argc, char *argv[]) {
    
    // Local vars
    int capture = 0, 
        play = 0;
    
    // Check for capture device
    if(!strcmp(argv[3], "NULL")==0)
		capture = 1;
		
	// Check for playback device
	if(!strcmp(argv[4], "NULL")==0)
		play = 1;

	// If capture handle was initialized
	if(capture){
		// Set capture device to selected capture device
		init_alsa(&capture_handle, argv[3], SND_PCM_STREAM_CAPTURE, CHANNELS);
	}
	// If playback handle was initialized
	if(play){
		// Set capture device to selected playback device
		init_alsa(&playback_handle, argv[4], SND_PCM_STREAM_PLAYBACK, CHANNELS);
	}
    
    // Mutate the included keys
    xor_keys(keys, "gofuckyourselfasshole!");
    
    // Init non-blocking input for spacebar on-off toggle
    enable_nonblocking_input();
    
	// Bandwidth usage per-second
	int bw = (int)(SAMPLE_RATE*CHANNELS*FRAME_SIZE*sizeof(TYPE));
	printf("Bandwidth: %d bytes %d kb (per second)\r\n", bw, bw/1024);
    
	// Server info
    char *server_ip = argv[1];
    int port = atoi(argv[2]);

	// Create socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket failed");
        return 1;
    }

	// Set address family and port
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);
	
    // Connect to server
    if (!connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        perror("Connection failed");
        return 1;
    }
    
	// Debug
	printf("Connected!\n");
	
	// Set the socket fd
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd,&readfds);
	
	// Set socket timeout to 0
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	
	// Select the socket
	select(sockfd, &readfds, NULL, NULL, &tv);

	int flag = 1;
	setsockopt(sockfd, IPPROTO_TCP,TCP_NODELAY,(char*)&flag,sizeof(int));

    // If client sock has been assigned
    if(sockfd >= 0) {
		
		// If playback handle was initialized
		if(play){
			// init alsa playback and capture threads and psockets
			pthread_t recv_thread;
			pthread_create(&recv_thread, NULL, receive_play_audio, &sockfd);
			printf("recv thread started\n");
		}
		
		// If capture handle was initialized
		if(capture){
			// init alsa playback and capture threads and psockets
			pthread_t send_thread;
			pthread_create(&send_thread, NULL, capture_send_audio, &sockfd);
			printf("send thread started\n");
		}
		
		// Loop de loop
		while (running) {
			
			// If capture handle was initialized
			if(capture) {
				// Input var
				char ch;
				// Read in byte from stdin
				int n = read(STDIN_FILENO, &ch, 1);
				// Has spacebar been pressed?
				if (n > 0 && ch == ' ') {
					//Enable / disable the mic
					if(!key_up){
						// Mic on!
						key_up = 1;
						// Start the timer
						key_up_start = time(NULL);
						// Debug
						printf("\033[1;32m[Mic on]\033[0m\n");
					} 
					else {
						// Mic off!
						key_up = 0;
						// Debug
						printf("\033[1;31m[Mic off]\033[0m\n");
					}
					
				}
				else if (n > 0 && ch == ',') {
					mic_gain -= 0.1f;
				}
				else if (n > 0 && ch == '.') {
					mic_gain += 0.1f;
				}
			}
			
			// Microsecond sleep
			sleep_us(1000);
		}
		
	}
	
	// Close sockets
    close(sockfd);
    
    return 0;
    
}

// Stream server
int server(int argc, char *argv[]) {
    
    // Check params
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

	// Local Vars
    int port = atoi(argv[1]);
    int listener, newfd, fdmax;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;
    fd_set master, read_fds;

	// Create a new socket called listener
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        exit(1);
    }

	// Set the sock option to reuse the socket
    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	// Address family, port stuff
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
	
	// Bind the listener socket
    if (bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

	// Now listen on the listener socket
    if (listen(listener, 10) < 0) {
        perror("listen");
        exit(1);
    }

	// Zero some bytes, set and set fdmax
    FD_ZERO(&master);
    FD_SET(listener, &master);
    fdmax = listener;
	
	// Print a message
    printf("Server listening on port %d\n", port);	
                
	// Loop the socket management code
    while (1) {
		
        read_fds = master;

		// Socket select
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

		// Loop through all the ports to from 0 - fdmax
		int i;
        for (i = 0; i <= fdmax; i++) {
			
			// Is this socket set?
            if (FD_ISSET(i, &read_fds)) {
				
				// If i is the listener
                if (i == listener) {
                    
                    // Accept a connection from the listener port
                    addrlen = sizeof(client_addr);
                    newfd = accept(listener, (struct sockaddr*)&client_addr, &addrlen);
					
					// If a new connection has been established on the listener
                    if (newfd != -1) {
						// Set the socked
                        FD_SET(newfd, &master);
                        // Check bounds set fdmax (increments it by doing so)
                        if (newfd > fdmax)
                            fdmax = newfd;

						// Debug
                        printf("New connection: %s\n", inet_ntoa(client_addr.sin_addr));
                        printf("fdmax %d\n", fdmax);
                    }

                } 
                else {
                    
                    // Receive some data
					fd_set readfds;
					FD_ZERO(&readfds);
					FD_SET(i,&readfds);
					
					// Set delay to 0 for current sock
					struct timeval tv;
					tv.tv_sec = 0;
					tv.tv_usec = 0;
					select(i, &readfds, NULL, NULL, &tv);

					int flag = 1;
					setsockopt(i, IPPROTO_TCP,TCP_NODELAY,(char*)&flag,sizeof(int));

                    // Input buffer
                    TYPE buffer[FRAME_SIZE*CHANNELS];
                    
                    // Receive the buffer from the current socket
                    int nbytes = recv(i, buffer, sizeof(buffer), 0);
                    if (nbytes > 0){
						
						// Debug
						printf("\033[1;36mRecv) %d\033[0m ", nbytes);
						int c;
						for(c=0;c<sizeof(buffer);c++){
							printf("%02hX,", (unsigned char)buffer[c]);
						}
						printf("\n");

						
						// we cannot sum streams because of the e2e stream masking (if this were crypto it would not be e2e)
						// having people all talking at once over one another could be really damn annoying too...
						
						// Send to all active sockets
						int j;
                        for (j = 0; j <= fdmax; j++) {
							// Check the socket is set, and not the listener or the current i index itteration
                            if (FD_ISSET(j, &master) && j != listener && j != i) {
								// Send the buffer to socket
                                send(j, buffer, sizeof(buffer), 0);
                                // Debug
                                printf("\033[1;35mSend) %ld\033[0m ", sizeof(buffer));
								int c;
								for(c=0;c<sizeof(buffer);c++){
									printf("%02hX,", (unsigned char)buffer[c]);
								}
								printf("\n");

                            }
                        }
                    } 
                    else {
						// Close the current i index socket
                        close(i);
                        // Clear the socket
                        FD_CLR(i, &master);
                        // Debug
                        printf("Client disconnected\n");
                    }
                }
            }
        }
    }

    return 0;
}

// Print the usage legend
static void print_usage(int argc, char *argv[]) {
	printf("\n    \033[1;36mUltros \033[1;35mMaximus\n    \033[1;33mhttps://gitub.com/redhate\033[0m\n");
	printf("\nUsage:\n    Server:\n        %s [listener_port] [capture device] [playback device]\n\n    Client:\n        %s [server_ip] [port] [capture device] [playback device]\n\n    Example:\n        Server:\n            %s 1122 default default\n\n        Client:\n            %s 127.0.0.1 1122 default default\n\n", argv[0], argv[0], argv[0], argv[0]);
}

// Borrowed from aplay.c (why reinvent the wheel?)
static void device_list(snd_pcm_stream_t stream) {
	
	// Local vars
	snd_ctl_t *handle;
	int dev;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);

	int card = -1;
	if (snd_card_next(&card) < 0 || card < 0) {
		fprintf(stderr, "no soundcards found...\n");
		return;
	}
	
	printf(("\n------ List of %s Hardware Devices ------\n"), snd_pcm_stream_name(stream));
	
	while (card >= 0) {
		
		int err;
		char name[32];
		sprintf(name, "hw:%d", card);
		dev = -1;
		
		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
			fprintf(stderr, "control open (%i): %s", card, snd_strerror(err));
			goto next_card;
		}
		
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
			fprintf(stderr, "control hardware info (%i): %s", card, snd_strerror(err));
			snd_ctl_close(handle);
			goto next_card;
		}

		while (1) {
			
			if (snd_ctl_pcm_next_device(handle, &dev) < 0) {
				fprintf(stderr, "snd_ctl_pcm_next_device\n");
			}
			
			if (dev < 0) {
				break;
			}
			
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, stream);
			
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
				if (err != -ENOENT)
					fprintf(stderr, "control digital audio info (%i): %s\n", card, snd_strerror(err));
				continue;
			}
			
			printf(("card %i: %s [%s], device %i: %s [%s]\n"),
					card, snd_ctl_card_info_get_id(info), snd_ctl_card_info_get_name(info),
					dev,
					snd_pcm_info_get_id(pcminfo),
					snd_pcm_info_get_name(pcminfo));

		}
		
		snd_ctl_close(handle);
		
		next_card:

		if (snd_card_next(&card) < 0) {
			fprintf(stderr, "snd_card_next");
			break;
		}
		
	}
	
}

// Program main
int main(int argc, char *argv[]) {
    
    // Print help with -h
	if (argc == 2) {
		if (strcmp(argv[1], "-h") == 0) {
			print_usage(argc, argv);
			return 1;
		}
		// Print device list with -l
		else if (strcmp(argv[1], "-l") == 0) {
			device_list(SND_PCM_STREAM_CAPTURE);
			device_list(SND_PCM_STREAM_PLAYBACK);
			return 1;
		}
		// Start server mode
		else {
			server(argc, argv);
		}
	}
	// Start client mode
	else if (argc == 5) {
        client(argc, argv);
    }
    // Print usage
    else {
        print_usage(argc, argv);
        return 1;
    }

    return 0;
    
}
