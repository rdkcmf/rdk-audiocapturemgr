#include "music_id.h"
class acm_iarm_interface
{
	private:
	bool m_is_active;
	bool m_enable_audio_input;
	music_id_client * m_client;

	public:
	acm_iarm_interface();
	~acm_iarm_interface();
	static acm_iarm_interface * get_instance();

	int activate(music_id_client * client);
	int deactivate();
	int enable_capture_handler(void * arg);
	int get_sample_handler(void * arg);
	void set_filename_prefix(std::string &prefix);
};
