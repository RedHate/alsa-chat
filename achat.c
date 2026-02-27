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

// Alsa related definitions
#define SAMPLE_RATE 48000 //libopus expects this, it will resample for you if you tell it to.
#define CHANNELS    1     //single channel
#define FRAME_SIZE  960   //libopus expects this or a multiple or division of it
#define FORMAT      SND_PCM_FORMAT_S16_LE //libopus expects 16bit audio

// Var for shutdown
int running = 1;

// For visualizer
int debug = 0;
int color_mode = 0;

// Microphone time out
#define MIC_TIMEOUT_SECONDS 60
float mic_gain = 1.0f;
int key_up = 0;
time_t key_up_start;

/* ----------------- OBFUSCATION -----------------*/

#ifdef _XOR_
/* --------- experiemental crap --------- */
/*
 * THIS IS NOT CRYPTO. This is obfuscation. 
 */

// XOR related definitions
#define XOR_KEY_LEN      96
#define XOR_KEY_ARRAY    4
#define XOR_FORWARD      0
#define XOR_BACKWARD     1

uint32_t keys[XOR_KEY_ARRAY][XOR_KEY_LEN] = {
	{
		0x5C681CA3, 
		0xCFAF8B12, 
		0x6304C2ED, 
		0x1583F0EF, 
		0xFB53E0EC, 
		0x05FE5D58, 
		0x11FB13FD, 
		0x22031F4E, 
		0x1730B6CB, 
		0xEBAF1097, 
		0x1D7B3868, 
		0x3B44C4E2, 
		0x984D20BC, 
		0x17772E7F, 
		0x0DB08431, 
		0xE4ECD63F, 
		0xDAF40788, 
		0xDBE843E0, 
		0xDC3C000B, 
		0x59A2F6E1, 
		0x62DAF66D, 
		0x1916A088, 
		0xE8910E40, 
		0xC1B912EE, 
		0x9F16112B, 
		0xB4AA85BA, 
		0x4F485550, 
		0xDEEEC9B0, 
		0xDAF40788, 
		0xDBE843E0, 
		0xDC3C000B, 
		0x59A2F6E1, 
		0xBB3EC976, 
		0x96658EA6, 
		0x3B559747, 
		0xC726038E, 
		0xE52A03B7, 
		0xD6D04797, 
		0xF3AA1947, 
		0x8675FEA8, 
		0x4978724A, 
		0x07AE229B, 
		0x0770D2DA, 
		0xAB485423, 
		0xEEB7EBD1, 
		0x288B6556, 
		0x3F693E79, 
		0x4CD6D4F8, 
		0x9F0F36BF, 
		0xFDB7631E, 
		0x29CD6ABB, 
		0xEE0B82EE, 
		0x49011AC8, 
		0xB7EACAC3, 
		0x4AECE49E, 
		0x6C389A97, 
		0x6485E497, 
		0xBE376DD9, 
		0xE2D25E86, 
		0x5D5E6589, 
		0x072FF55F, 
		0xA06B1E37, 
		0x93E8E6E1, 
		0x50DCA452, 
		0x1C81F1F6, 
		0x96D5DC30, 
		0xAE2E2214, 
		0x691EDB80, 
		0x1C2069A0, 
		0xDC4AB6E8, 
		0x2555A276, 
		0xD5E710CA, 
		0x2FEC9B20, 
		0xF7661A99, 
		0xF027F7D7, 
		0x1B455615, 
		0x7BBE0877, 
		0x7E32FEC2, 
		0x4D883B75, 
		0xF55EE023, 
		0xFFA58331, 
		0xB03C29FD, 
		0x40433EF8, 
		0xACD36D62, 
		0xD02A7AAB, 
		0x3DCBDB3C, 
		0x8022AD9E, 
		0x7162214C, 
		0xB48D77B7, 
		0xFDAF0E5A, 
		0xFA454643, 
		0xC00CF78C, 
		0x8D045407, 
		0x464964DC, 
		0x24DAC8FE, 
		0xC250368D, 
	},
	{
		0x2DC86E2F, 
		0xF111A75A, 
		0xEF17C959, 
		0x438C3353, 
		0x161C635C, 
		0x12072838, 
		0x56175C09, 
		0xAA0F6276, 
		0x4BF26DE7, 
		0x451352CB, 
		0x6897D9C8, 
		0xD657D76A, 
		0xF979A24C, 
		0x4AAB9783, 
		0x3B7294F5, 
		0x775F8EC3, 
		0xB12419A8, 
		0xB4494C60, 
		0xACCC0027, 
		0x3F292D65, 
		0xE9B12FD9, 
		0x7D4C22A8, 
		0x4AD53740, 
		0xC75D5956, 
		0xE34E5587, 
		0x66009352, 
		0x72690010, 
		0xA555EF70, 
		0xB12419A8, 
		0xB4494C60, 
		0xACCC0027, 
		0x3F292D65, 
		0x57C5ECAE, 
		0xCFF3B43E, 
		0xD603CA5B, 
		0xDBBE0DB6, 
		0x71820D6B, 
		0x8D9159CB, 
		0x3D027C5B, 
		0x9FA20408, 
		0x6C99BB62, 
		0x1916A8F7, 
		0x1AB399B2, 
		0x066904AF, 
		0x54684495, 
		0x8AA6F00E, 
		0xC2CDC79D, 
		0x7F8D8718, 
		0xE333EC43, 
		0x0B6AEF66, 
		0x8EF8C057, 
		0x56258956, 
		0x6D0571E8, 
		0x6841E1CF, 
		0x615F76E6, 
		0xDCDAF0CB, 
		0xF69276CB, 
		0x46EADABD, 
		0x699B249E, 
		0x2827F3AD, 
		0x1B902023, 
		0x21C766EB, 
		0xDC4B7D65, 
		0x13AE351A, 
		0x6E86362E, 
		0xCD82ACF0, 
		0x1696AA44, 
		0xCD65B580, 
		0x6CA1CF20, 
		0xAD606D48, 
		0xB0032BAE, 
		0x827B53E2, 
		0x905EF7A0, 
		0x2AFE70FD, 
		0x30B8288B, 
		0x76500E41, 
		0x954629AB, 
		0x86F905CA, 
		0x7BA8D6A1, 
		0x202560AF, 
		0x01338FF5, 
		0x70CC8E09, 
		0x414FC518, 
		0x1F9ED8EA, 
		0x90839007, 
		0xCAE4B7CC, 
		0x80A81BE6, 
		0xB4EAA47C, 
		0x66B8A96B, 
		0x0B133732, 
		0x13505F4F, 
		0xC03F29BC, 
		0xB915041B, 
		0x5F6CF7AC, 
		0xB7B1EB06, 
		0xCB10ECB9, 
	},
	{
		0xF14E8CDF, 
		0xE00BD2FA, 
		0x1E6B5CC9, 
		0x7B4F0663, 
		0x77DA569C, 
		0x3275ABB8, 
		0x37629C99, 
		0x0AF94516, 
		0xF4D4B397, 
		0x14267E7B, 
		0xE1EA4548, 
		0xB32AA1CA, 
		0x6EE3868C, 
		0xE012EFB3, 
		0x8C5BDBA5, 
		0x02A762F3, 
		0xA3658328, 
		0xF0DD8A60, 
		0x600C0257, 
		0xCDBBFB35, 
		0x72A3D249, 
		0xA98E0828, 
		0xE7864340, 
		0xB288CC36, 
		0xD7AB0DF7, 
		0x0609A672, 
		0x54F90110, 
		0xF00B1870, 
		0xA3658328, 
		0xF0DD8A60, 
		0x600C0257, 
		0xCDBBFB35, 
		0x2B9B264E, 
		0x30C8F7DE, 
		0xB63F6FEB, 
		0x605ED6D6, 
		0x69A2DBDB, 
		0x5484C57B, 
		0xED25B9EB, 
		0x65824488, 
		0xA5020D42, 
		0x887C2787, 
		0xB18A0292, 
		0x60F94E5F, 
		0x12EC0DC5, 
		0x20C9F0EE, 
		0xEE11BE4D, 
		0x8755F698, 
		0xD00D2873, 
		0xBDC41906, 
		0x6174C527, 
		0x347D1C36, 
		0xBD526F68, 
		0xEC5FFD3F, 
		0x74A81886, 
		0x1175FC7B, 
		0x9FB51A7B, 
		0x2847716D, 
		0xF0296D7E, 
		0xAA58C97D, 
		0xA2922213, 
		0x3DB1085B, 
		0x18FCAB35, 
		0x294D64BA, 
		0x86E554CE, 
		0x15A863F0, 
		0x7FFC0E04, 
		0x1B3EED80, 
		0xA6BD3D20, 
		0x7B66B9C8, 
		0xB031914E, 
		0xA5CE6DC2, 
		0x95B18DA0, 
		0x85197F2D, 
		0x3B3AA03B, 
		0x1350EA51, 
		0xC124B31B, 
		0xE969596A, 
		0xC125BCB1, 
		0x22736A5F, 
		0x120B70A5, 
		0x7C046E99, 
		0x55B39498, 
		0xE673564A, 
		0x98BA9077, 
		0x64AFCB0C, 
		0x8A29A586, 
		0xFA40E3BC, 
		0x0D323FDB, 
		0xBA204412, 
		0x2655ABBF, 
		0xC3CDB27C, 
		0x284545AB, 
		0xA9A38D6C, 
		0xCCAF5B66, 
		0x7A1E2729, 
	},
	{
		0xBFC253DF, 
		0xEBD928FA, 
		0x753795C9, 
		0x34496563, 
		0xAD8CCA9C, 
		0x47DE13B8, 
		0x55FE0599, 
		0xF3BC5316, 
		0x20672497, 
		0x3258057B, 
		0x0BAF0D48, 
		0x998B6BCA, 
		0x8D650A8C, 
		0xF2FD5CB3, 
		0xD7807EA5, 
		0xA5C591F3, 
		0xC6E6AB28, 
		0x2D57EA60, 
		0x6C0E5557, 
		0x7640CE35, 
		0xD1719B49, 
		0x27862028, 
		0x61C50340, 
		0x3A44FA36, 
		0x7CA6FAF7, 
		0x0FAFD472, 
		0xADF81110, 
		0xFB136870, 
		0xC6E6AB28, 
		0x2D57EA60, 
		0x6C0E5557, 
		0x7640CE35, 
		0xB0BD684E, 
		0xF83F29DE, 
		0x895084EB, 
		0x3E8800D6, 
		0xCB7900DB, 
		0xD041BE7B, 
		0xC89C52EB, 
		0xE7C6CC88, 
		0xA70F4F42, 
		0xF45BA087, 
		0x3B889092, 
		0x99B7115F, 
		0xFEE1C8C5, 
		0xE9391EEE, 
		0xFFAFF34D, 
		0xD2A36E98, 
		0xDD255B73, 
		0x79DD1F06, 
		0x15B1E227, 
		0x49612A36, 
		0xEF3D0768, 
		0xB3A2C23F, 
		0xDCB09E86, 
		0x6489877B, 
		0x2AAF617B, 
		0x6F361C6D, 
		0xD944137E, 
		0xF291B47D, 
		0x30B03113, 
		0x8CB9535B, 
		0xE4579E35, 
		0x6429DEBA, 
		0x63B19ACE, 
		0xBDCB93F0, 
		0x83F20A04, 
		0x25D36D80, 
		0x1B801D20, 
		0x1DDF71C8, 
		0x81A0DF4E, 
		0x6BA3AFC2, 
		0x243C2DA0, 
		0x9C66522D, 
		0x019A9B3B, 
		0x43BABB51, 
		0xE597A81B, 
		0x8030336A, 
		0xE4990DB1, 
		0x5119355F, 
		0x197BD5A5, 
		0x786AF799, 
		0xE6270C98, 
		0x95251C4A, 
		0x222AE777, 
		0xCB64C70C, 
		0xA38C2386, 
		0xBAA35FBC, 
		0x3F0DE4DB, 
		0x9A645612, 
		0x73FE14BF, 
		0x0E7FCE7C, 
		0x6D00EEAB, 
		0x0A2EE16C, 
		0x63F43D66, 
		0x64390E29, 
	},
};

// Print the key info
void print_keys() {
	// Loop through the keys
	int i, j;
	for (i = 0; i < XOR_KEY_ARRAY; i++) {
		// Loop through key length
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// print the bytes
			printf("0x%08X \n", (uint32_t)keys[i][j]);
		}
		// print a new line for each key
		printf("\n");
	}
}

// XOR the keyset with a passphrase
void xor_keys(uint32_t keys[XOR_KEY_ARRAY][XOR_KEY_LEN], const char *password) {
	// Set key position to 0
	int keypos = 0;
	// Loop through the keys
	int i, j;
	// positive
	for (i = 0; i < XOR_KEY_ARRAY-1; i++) {
		// Loop through selected key
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// XOR the key
			keys[i][j] = (uint32_t)((uint8_t)password[keypos] ^ (uint32_t)keys[i][j]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == strlen(password)) keypos = 0;
		}
	}
	// negative
	for (i = XOR_KEY_ARRAY-1; i > 0; i--) {
		// Loop through selected key
		for (j = 0; j < XOR_KEY_LEN; j++) {
			// XOR the key
			keys[i][j] = (uint32_t)((uint8_t)password[keypos] ^ (uint32_t)keys[i][j]);
			// Move the key position
			keypos++;
			// If the key position is greater than the key length reset it
			if (keypos == strlen(password)) keypos = 0;
		}
	}
}

// Directional XOR fnc (doesn't need to be global, even if that's how you think it should be done. i scope my code.)
void xor_directional(uint8_t *buffer, size_t size, uint32_t *key, int direction) {
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
	for (i = 0; i < XOR_KEY_ARRAY-1; i++) {
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
	for (i = XOR_KEY_ARRAY-1; i > 0; i--) {
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

// This adds a lot of noise to our signal. good.
uint32_t shuffle32(uint32_t block, int enc){
	uint32_t a = (block >> 0)  & 0xFF,
			 b = (block >> 8)  & 0xFF,
			 c = (block >> 16) & 0xFF,
			 d = (block >> 24) & 0xFF;
	if(enc)
		return (c<<0)|(a<<8)|(d<<16)|(b<<24);
	else
		return (b<<0)|(d<<8)|(a<<16)|(c<<24);
}

// Directional byte flipping function
void bytefliparray(uint8_t *buffer, uint32_t size, int enc){
	int c;
	for(c=0;c<16;c++){
		int i;
		for(i=0;i<size;i+=4){
			// Shuffle and re-assign 4 bytes of the array
			*((uint32_t*)&buffer[i]) = shuffle32(*((uint32_t*)&buffer[i]), enc);
		}
	}
}

/* --------- end of experiemental crap --------- */
#endif


/* ----------------- TERM -----------------*/

// Local Var
struct termios term;

// Restore original terminal properties
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

/* ----------------- DEBUG -----------------*/

void debug_print_buf(uint8_t *buffer, uint32_t size) {
	int c;
	for(c=0;c<size;c++) {
		printf("%02hX,", *(uint8_t*)&buffer[c]);
	}
	printf("\n");
}

void audio_visualiser(short *buffer, size_t size, int color_mode) {
	
	/*
	 * This is honestly one of the coolest things I have ever made
	 * such a simple thing and looks so neat
	 */
	
	// Visualiser array
	char visualiser_array[32][32]={
		"+",
		"++",
		"+++",
		"++++",
		"+++++",
		"++++++",
		"+++++++",
		"++++++++",
		"+++++++++",
		"++++++++++",
		"+++++++++++",
		"++++++++++++",
		"+++++++++++++",
		"++++++++++++++",
		"+++++++++++++++",
		"++++++++++++++++",
		"+++++++++++++++++",
		"++++++++++++++++++",
		"+++++++++++++++++++",
		"++++++++++++++++++++",
		"+++++++++++++++++++++",
		"++++++++++++++++++++++",
		"+++++++++++++++++++++++",
		"++++++++++++++++++++++++",
		"+++++++++++++++++++++++++",
		"++++++++++++++++++++++++++",
		"+++++++++++++++++++++++++++",
		"++++++++++++++++++++++++++++",
		"+++++++++++++++++++++++++++++",
		"++++++++++++++++++++++++++++++",
		"+++++++++++++++++++++++++++++++",
		"++++++++++++++++++++++++++++++++",
	};
	
	// Terminal color strings
	char color_strings[8][10]={
		"\033[1;37m",//WHITE
		"\033[1;34m",//BLUE
		"\033[1;36m",//CYAN
		"\033[1;32m",//GREEN
		"\033[1;33m",//YELLOW
		"\033[1;31m",//RED
		"\033[1;35m",//MAGENTA
		"\033[1;37m",//WHITE1
		//"\033[0m", //NOCOLOR
	};
	
	// Main visualiser loop
	int c;
	for(c=0;c<sizeof(buffer);c++) {
		// Type cast and reduce to 8 bit
		char byte = (char)(buffer[c]/255);
		// Check upper and lower signal boundaries
		if((byte >= 0) && (byte <= 127)) {
				// Print the pretty stuff
				printf("[%04d]%s                     | %s\033[0m\n", *(char*)&byte, (color_mode)? color_strings[6] : color_strings[((int)(byte/8))/2],   visualiser_array[(int)(byte/8)]);
		}
		// Check upper and lower signal boundaries
		if((byte <= 0) && (byte >= -127)) {
				// Print the pretty stuff
				printf("[%04d]%s%20s |\033[0m\n", *(char*)&byte, (color_mode)? color_strings[2] : color_strings[((int)(~byte/6))/2], visualiser_array[(int)(~byte/8)]);
		}
	}
			
}

/* ----------------- OPUS -----------------*/

// Opus handles
OpusDecoder *decoder;
OpusEncoder *encoder;

// Opus decoder encoder init
int init_opus() {
    
    int err;
    
	// ENCODER
    // Create Opus encoder
    encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_AUDIO, &err);
    if (err != OPUS_OK) {
        fprintf(stderr, "Failed to create encoder: %s\n", opus_strerror(err));
        return 1;
    }

    // Set the lowest acceptable bitrate for high compression
    opus_encoder_ctl(encoder, OPUS_SET_VBR(1));
	opus_encoder_ctl(encoder, OPUS_SET_VBR_CONSTRAINT(1));
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(8000)); // mono voice
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10)); // optional, max compression efficiency
	opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_APPLICATION_VOIP));

	// DECODER
	// Create Opus decoder
	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
	if (err != OPUS_OK) {
		fprintf(stderr, "Failed to create decoder: %s\n", opus_strerror(err));
		return 1;
	}


    return 0;
}

// Opus decoder encoder shutdown
void opus_shutdown(){
	opus_encoder_destroy(encoder);
	opus_decoder_destroy(decoder);
}

// Encode a pcm stream to opus
int opus_encode_buffer( const opus_int16 *pcm, int frame_size, unsigned char *data, opus_int32 max_data_bytes){
    
    //unsigned char opus_data[4000]; // buffer for encoded data
    int bytes_encoded = opus_encode(encoder, pcm, frame_size, data, max_data_bytes);
    if (bytes_encoded < 0) {
        fprintf(stderr, "Encode failed: %s\n", opus_strerror(bytes_encoded));
        return 1;
    }

    //printf("Encoded %d bytes\n", bytes_encoded);
    return bytes_encoded;
    
}

// Decode an opus stream to pcm
int opus_decode_buffer(const unsigned char *data, opus_int32 len, opus_int16 *pcm, int frame_size){

	int bytes_deccoded = opus_decode(decoder, data, len, pcm, frame_size, 0);
	if (bytes_deccoded < 0) {
		fprintf(stderr, "Decode failed: %s\n", opus_strerror(bytes_deccoded));
		return 1;
	}
	
    //printf("Decoded %d bytes\n", bytes_deccoded);
    return bytes_deccoded;
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
		init_alsa(&capture_handle, argv[3], SND_PCM_STREAM_CAPTURE, CHANNELS);
	}
	// If playback handle was initialized
	if(play) {
		// Set capture device to selected playback device
		init_alsa(&playback_handle, argv[4], SND_PCM_STREAM_PLAYBACK, CHANNELS);
	}
    
    // Init opus encoder and decoder handles
    init_opus();
    
    // Init non-blocking input for spacebar on-off toggle
    enable_nonblocking_input();
    
#ifdef _XOR_
    // Mutate the included keys
    xor_keys(keys, "gofuckyourselfasshole!");
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

