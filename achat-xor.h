/* ----------------- OBFUSCATION -----------------*/

// Print the key info
void print_keys();

// XOR the keyset with a passphrase
void xor_keys(const char *password);

// Directional XOR fnc (doesn't need to be global, even if that's how you think it should be done. i scope my code.)
void xor_directional(uint8_t *buffer, size_t size, uint32_t *key, int direction);

// This adds a reversable layer of noise to our signal
void xor4x(uint8_t *buffer, size_t size);

// This adds a lot of noise to our signal. good.
uint32_t shuffle32(uint32_t block, int enc);

// Directional byte flipping function
void bytefliparray(uint8_t *buffer, uint32_t size, int enc);

