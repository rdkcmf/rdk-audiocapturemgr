#ifndef _IP_OUT_H_
#define _IP_OUT_H_
#include "audio_capture_manager.h"
#include <iostream>
#include <list>
#include <map>
#include <fstream>
#include <string>

class ip_out_client : public audio_capture_client
{
	public:
	typedef enum
	{
		EXIT = 0,
		CODE_MAX
	} control_code_t;

	private:
	std::string m_data_path;
	bool m_is_enabled;
	int m_listen_fd;
	int m_write_fd;
	int m_control_pipe[2];
	unsigned int m_num_connections;
	pthread_t m_thread;

	void process_new_connection();

	public:
	ip_out_client(q_mgr * manager);
	~ip_out_client();
	virtual int data_callback(audio_buffer *buf);
	virtual std::string get_data_path();
	virtual std::string open_output();
	virtual int close_output();
	virtual int start();
	virtual int stop();
	void worker_thread();
};

#endif //_IP_OUT_H_
