/*
 * 
 * vchat
 * [8-Bit alsa voice chat]
 * 
 * An organically grown way of how not to create voice chat.
 * 
 * Uses libopus!
 * 
 * c99 compliant
 * 
 * **NOTE**
 * 
 *   You may encounter an issue with there being a douplicate time
 * definition in one of your alsa headers, alsa/config.h iirc. Comment
 * it out of that header. You have "time.h" in c99.
 * 
 *   Also may complain about implicit declaration of nanosleep. -It
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
#define _XOR_ 1

// Headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <alloca.h>
#include <termios.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <opus/opus.h>
// Project headers
#include "achat-termios.h"
#include "achat-opus.h"
#include "achat-cdefs.h"
#include "achat-debug.h"
#include "achat-xor.h"

// Var for shutdown
int running = 1;        // control var

// For visualizer
int debug      = 0;     // toggle with 'd' button
int color_mode = 0;     // toggle color mode of the visualizer with 'c' key

// Microphone
float mic_gain = 1.0f;  // controlled with ',' and '.' keys
time_t key_up_start;    // used to count time for mic timeout
int   key_up   = 0;     // toggle with spacebar

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

/* ----------------- ALSA -----------------*/

// ALSA handles
snd_pcm_t *capture_handle;
snd_pcm_t *playback_handle;

// Initialize ALSA for capture or playback
int init_alsa(snd_pcm_t **handle, const char *device, snd_pcm_stream_t stream, uint32_t channels) {
    
    // Local vars
    int err;
    snd_pcm_hw_params_t *params;
    
    // Init a handle for the PCM steam from a chosen device
    if ((err = snd_pcm_open(handle, device, stream, 0)) < 0) {
        fprintf(stderr, "ALSA open error: %s\n", snd_strerror(err));
        return 1;
    }
	
	// I assume this allocates memory for params, return doesn't allow error checking
	snd_pcm_hw_params_alloca(&params);
	
	// Init all channel params and error check
    if ((err = snd_pcm_hw_params_any(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_any error: %s\n", snd_strerror(err));
		return 2;
	}
	
	// (ignore that gcc throws warning on this or fix it lol)
	// set resample since we want to adjust the rate 
	uint32_t resample = 1;
    if ((err = snd_pcm_hw_params_set_rate_resample(*handle, params, *(unsigned int*)&resample)) < 0) {
        //fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate_resample error: %s\n", snd_strerror(err));
		return 3;
	}
	// Set access
    if ((err = snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_access error: %s\n", snd_strerror(err));
		return 4;
	}
	// Set format
    if ((err = snd_pcm_hw_params_set_format(*handle, params, FORMAT)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_format error: %s\n", snd_strerror(err));
		return 5;
	}
	
	// Set channels
    snd_pcm_hw_params_set_channels(*handle, params, channels);
    
	// Set rate
	uint32_t sample_rate = SAMPLE_RATE;
    if ((err = snd_pcm_hw_params_set_rate_near(*handle, params, &sample_rate, 0)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate error: %s\n", snd_strerror(err));
		return 6;
	}
	// Set hw params
    if ((err = snd_pcm_hw_params(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params error: %s\n", snd_strerror(err));
		return 7;
	}
	// Prepare for pcm
    if ((err = snd_pcm_prepare(*handle)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_prepare error: %s\n", snd_strerror(err));
		return 8;
	}

    printf("ALSA initialized at %u Hz for %d\n", sample_rate, stream);

    return 0;
    
}

// Thread for receiving audio and playing
void* receive_play_audio(void* arg) {
	// Get the socket fd
    int sockfd = *(int*)arg;
    // Create a buffer for opus data
    uint8_t opus[FRAME_SIZE];
	// Create a buffer for pcm
	short pcm[FRAME_SIZE];
    // Receive some data
    while (running) {
		// Read into buffer from socket
		ssize_t r = read(sockfd, opus, FRAME_SIZE);
		if (r > 0) {
			// Debug
			if(debug == 2) {
				printf("\033[1;36mRecv) %ld\033[0m ", r);
				debug_print_buf((uint8_t*)opus, r);
			}
#ifdef _XOR_
			// Byte flipping
			bytefliparray((uint8_t*)opus, sizeof(opus), 0);
			// Apply XOR
			xor4x((uint8_t*)opus, sizeof(opus));
#endif
			// opus decode
			opus_decode_buffer(opus, r, pcm, FRAME_SIZE);			
			// write pcm to sound device
			snd_pcm_writei(playback_handle, pcm, FRAME_SIZE);
			if(debug == 1) {
				printf("\033[1;36mPCM) %d\033[0m ", FRAME_SIZE);
				debug_print_buf((uint8_t*)pcm, FRAME_SIZE);
			}
			if(debug == 3) {
				audio_visualiser(pcm, sizeof(pcm), color_mode);
			}
		}
    }
    // Return
    return NULL;
}

// Thread for capturing and sending
void* capture_send_audio(void* arg) {
	// Get the socket fd
    int sockfd = *(int*)arg;
    // Create a buffer for pcm
    short pcm[FRAME_SIZE];
	// Create a buffer for opus data
    uint8_t opus[FRAME_SIZE];
    // Send some data
    while (running) {
		// This needs to stay reading
		// Read in buffer from capture handle
		ssize_t r = snd_pcm_readi(capture_handle, pcm, FRAME_SIZE);
		if (r > 0) {
			// Is the mic on?
			if(key_up == 1) {
				// Check the time 60 second time out
				if((time(NULL)-key_up_start) < MIC_TIMEOUT_SECONDS) {
					int c;
					// Adjust mic gain before modulation
					for (c = 0; c < sizeof(pcm); c++) {
						pcm[c] = (opus_int16)(pcm[c] * mic_gain);
					}
#ifdef _DEBUG_
					if(debug == 1) {
					// Print debug
						printf("\033[1;35mPCM) %d\033[0m ", FRAME_SIZE);
						debug_print_buf((uint8_t*)pcm, FRAME_SIZE);
					}
#endif
					// opus encode
					int bytes_encoded = opus_encode_buffer(pcm, FRAME_SIZE, opus, FRAME_SIZE);
#ifdef _XOR_
					// Apply XOR
					xor4x((uint8_t*)opus, sizeof(opus));
					// Byte flipping
					bytefliparray((uint8_t*)opus, sizeof(opus), 1);
#endif
					// Send to sock
					write(sockfd, (uint8_t*)opus, bytes_encoded);
#ifdef _DEBUG_
					// Debug
					if(debug == 2) {
					// Print debug
						printf("\033[1;35mSend) %d\033[0m ", bytes_encoded);
						debug_print_buf((uint8_t*)opus, bytes_encoded);
					}
					if(debug == 3) {
						audio_visualiser(pcm, sizeof(pcm), color_mode);
					}
#endif
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
	if(capture) {
		// Set capture device to selected capture device
		if(init_alsa(&capture_handle, argv[3], SND_PCM_STREAM_CAPTURE, CHANNELS) != 0){
			return 0;
		}
	}
	// If playback handle was initialized
	if(play) {
		// Set capture device to selected playback device
		if(init_alsa(&playback_handle, argv[4], SND_PCM_STREAM_PLAYBACK, CHANNELS) != 0){
			return 0;
		}
	}
    
    // Init opus encoder and decoder handles
    init_opus();
    
    // Init non-blocking input for spacebar on-off toggle
    enable_nonblocking_input();
    
#ifdef _XOR_
    // Mutate the included keys
    xor_keys("gofuckyourselfasshole!");
#endif
    
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
		if(play) {
			// init alsa playback and capture threads and psockets
			pthread_t recv_thread;
			pthread_create(&recv_thread, NULL, receive_play_audio, &sockfd);
			printf("recv thread started\n");
		}
		
		// If capture handle was initialized
		if(capture) {
			// init alsa playback and capture threads and psockets
			pthread_t send_thread;
			pthread_create(&send_thread, NULL, capture_send_audio, &sockfd);
			printf("send thread started\n");
		}
		
		// Loop de loop
		while (running) {
			
			// Input var
			char ch;
			// Read in byte from stdin
			int n = read(STDIN_FILENO, &ch, 1);
#ifdef _DEBUG_
			if (n > 0 && ch == 'd') {
				// Swap debug display modes
				debug ++;
				// If its greater than 2, reset
				if(debug > 3)
					debug = 0;
			}
			if (n > 0 && ch == 'c') {
				// Swap debug display modes
				color_mode ++;
				// If its greater than 2, reset
				if(color_mode > 1)
					color_mode = 0;
			}
#endif
			// If capture handle was initialized
			if(capture) {
				// Has spacebar been pressed?
				if (n > 0 && ch == ' ') {
					//Enable / disable the mic
					if(!key_up) {
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
				if (n > 0 && ch == ',') {
					mic_gain -= 0.1f;
				}
				if (n > 0 && ch == '.') {
					mic_gain += 0.1f;
				}
			}
			// Microsecond sleep
			sleep_us(1000);
		}
		
	}
	
	// Close sockets
    close(sockfd);
    opus_shutdown();
    
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
                    uint16_t buffer[FRAME_SIZE];
                    
                    // Receive the buffer from the current socket
                    int nbytes = read(i, (uint8_t*)buffer, sizeof(buffer));
                    if (nbytes > 0) {

						// Debug
						printf("\033[1;36mRecv) %d\033[0m ", nbytes);
						debug_print_buf((uint8_t*)buffer, nbytes);
						
						// Send to all active sockets
						int j;
                        for (j = 0; j <= fdmax; j++) {
							// Check the socket is set, and not the listener or the current i index itteration
                            if (FD_ISSET(j, &master) && j != listener && j != i) {
								// Send the buffer to socket
                                write(j,  (uint8_t*)buffer, nbytes);

                                // Debug
                                printf("\033[1;35mSend) %d\033[0m ", nbytes);
								debug_print_buf((uint8_t*)buffer, nbytes);

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

/* ----------------- INFO -----------------*/

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

/* ----------------- MAIN -----------------*/

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

