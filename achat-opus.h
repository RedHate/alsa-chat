/* ----------------- OPUS -----------------*/

// Opus decoder encoder init
int init_opus();

// Opus decoder encoder shutdown
void opus_shutdown();

// Encode a pcm stream to opus
int opus_encode_buffer( const opus_int16 *pcm, int frame_size, unsigned char *data, opus_int32 max_data_bytes);

// Decode an opus stream to pcm
int opus_decode_buffer(const unsigned char *data, opus_int32 len, opus_int16 *pcm, int frame_size);
