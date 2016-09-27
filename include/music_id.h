#include "audio_capture_manager.h"
#include <iostream>
#include <list>
#include <map>
#include <fstream>

class music_id_client : public audio_capture_client
{
	typedef int request_id_t;
	typedef struct
	{	
		request_id_t id;
		std::string filename;
		unsigned int length;
        unsigned int time_remaining;
	}request_t;
	private:
	std::list <audio_buffer *> m_queue;
	std::list <request_t*> m_requests;
	pthread_mutex_t m_mutex;
	pthread_t m_thread;
	bool m_worker_thread_alive;
	unsigned int m_total_size;
	unsigned int m_precapture_duration_seconds;
	unsigned int m_precapture_size_bytes;
	unsigned int m_queue_upper_limit_bytes;
	unsigned int m_request_counter;

	inline void lock();
	inline void unlock();
	void trim_queue();
	int write_default_file_header(std::ofstream &file);
	int update_file_header_size(std::ofstream &file, unsigned int data_size);
	int grab_last_n_seconds(const std::string &filename, unsigned int seconds);
//	int write_queue_to_file(const std::string &filename);
	void compute_queue_size();

	public:
	music_id_client(q_mgr * manager);
	~music_id_client();
	virtual int data_callback(audio_buffer *buf);
	int grab_precaptured_sample(const std::string &filename);
	request_id_t grab_fresh_sample(const std::string &filename, unsigned int seconds);
	virtual int set_audio_properties(audio_properties_t &properties);
	int set_precapture_duration(unsigned int seconds);
	void worker_thread();
};
