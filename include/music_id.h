/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#ifndef _MUSIC_ID_H_
#define _MUSIC_ID_H_
#include "audio_capture_manager.h"
#include "audio_converter.h"
#include "socket_adaptor.h"
#include <iostream>
#include <list>
#include <map>
#include <fstream>
#include <string>
#include <thread>

typedef void (*request_complete_callback_t)(void *data, std::string &file, int result);
class music_id_client : public audio_capture_client
{
	public:
	typedef enum
	{
		FILE_OUTPUT = 0,
		SOCKET_OUTPUT
	} preferred_delivery_method_t;

	private:
	typedef int request_id_t;
	typedef struct
	{	
		request_id_t id;
		std::string filename;
		unsigned int length;
        unsigned int time_remaining;
		request_complete_callback_t callback;
		void * callback_data;
	}request_t;

	std::list <audio_buffer *> m_queue;
	std::list <request_t*> m_requests;
	std::list <audio_converter_memory_sink *> m_outbox;
	std::thread m_worker_thread;
	bool m_worker_thread_alive;
	unsigned int m_total_size;
	unsigned int m_precapture_duration_seconds;
	unsigned int m_precapture_size_bytes;
	unsigned int m_queue_upper_limit_bytes;
	unsigned int m_request_counter;
	bool m_enable_wav_header_output;
	audiocapturemgr::audio_properties_t m_output_properties;
	bool m_convert_output;
	preferred_delivery_method_t m_delivery_method;
	socket_adaptor * m_sock_adaptor;
	const std::string m_sock_path;

	void trim_queue();
	int write_default_file_header(std::ofstream &file);
	int update_file_header_size(std::ofstream &file, unsigned int data_size);
	int grab_last_n_seconds(const std::string &filename, unsigned int seconds);
	int grab_last_n_seconds(unsigned int seconds); //For socket mode output
	void compute_queue_size();

	public:
	music_id_client(q_mgr * manager, preferred_delivery_method_t mode);
	~music_id_client();
	virtual int data_callback(audio_buffer *buf);
	int grab_precaptured_sample(const std::string &filename = nullptr);
	request_id_t grab_fresh_sample(unsigned int seconds, const std::string &filename = nullptr, request_complete_callback_t cb = nullptr, void * cb_data = nullptr);
	virtual int set_audio_properties(audiocapturemgr::audio_properties_t &properties);
	virtual void get_audio_properties(audiocapturemgr::audio_properties_t &properties);
	int set_precapture_duration(unsigned int seconds);
	void worker_thread();
	unsigned int get_max_supported_duration();
	unsigned int enable_wav_header(bool isEnabled);
	void enable_output_conversion(bool isEnabled) { m_convert_output = isEnabled; }
	void send_clip_via_socket();
	const std::string & get_sock_path() { return m_sock_path; }
};

#endif //_MUSIC_ID_H_
