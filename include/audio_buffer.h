class audio_buffer
{
	public:
		unsigned char * m_start_ptr;
		unsigned int m_size;
		unsigned int m_clip_length;
		unsigned int m_refcount;

		audio_buffer(const unsigned char *in_ptr, unsigned int in_size, unsigned int clip_length, unsigned int refcount);
		~audio_buffer();
};

audio_buffer * create_new_audio_buffer(const unsigned char *in_ptr, unsigned int in_size, unsigned int clip_length, unsigned int refcount);
void unref_audio_buffer(audio_buffer *ptr);
void free_audio_buffer(audio_buffer *ptr);
