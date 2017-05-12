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
#ifndef _AUDIO_CAPTURE_MANAGER_H_
#define _AUDIO_CAPTURE_MANAGER_H_
#include <pthread.h>
#include <vector>
#include <semaphore.h>
#include <string>
#include "audio_buffer.h"
#include "basic_types.h"
#include "rmf_error.h"
#include "media-utils/audioCapture/rmfAudioCapture.h"

namespace audiocapturemgr
{
	typedef struct
	{
		racFormat format;
		racFreq sampling_frequency;
		size_t fifo_size;
		size_t threshold;
		unsigned int delay_compensation_ms;
	}audio_properties_t;
	void get_individual_audio_parameters(const audio_properties_t &audio_props, unsigned int &sampling_rate, unsigned int &bits_per_sample, unsigned int &num_channels);
	unsigned int calculate_data_rate(const audio_properties_t &audio_props);
	std::string get_suffix(unsigned int ticker);
}

class audio_capture_client;
class q_mgr
{
	private:
		std::vector <audio_buffer *> *m_current_incoming_q;
		std::vector <audio_buffer *> *m_current_outgoing_q;
		std::vector <audio_capture_client *> m_clients;
		audiocapturemgr::audio_properties_t m_audio_properties;
		unsigned int m_bytes_per_second;
		unsigned int m_total_size;
		unsigned int m_num_clients;
		pthread_mutex_t m_q_mutex;
		pthread_mutex_t m_client_mutex;
		sem_t m_sem;
		pthread_t m_thread;
		bool m_processing_thread_alive;
		bool m_notify_new_data;
		bool m_started;
		RMF_AudioCaptureHandle m_device_handle;
		unsigned int m_max_queue_size;

	private:
		inline void lock(pthread_mutex_t &mutex);
		inline void unlock(pthread_mutex_t &mutex);
		inline void notify_data_ready();
		void swap_queues(); //caller must lock before invoking this.
		void flush_queue(std::vector <audio_buffer *> *q);
		void flush_system();
		void process_data();
		void update_buffer_references();

	public:
		q_mgr();
		~q_mgr();

		int set_audio_properties(audiocapturemgr::audio_properties_t &in_properties);
		void get_audio_properties(audiocapturemgr::audio_properties_t &out_properties);
		void get_default_audio_properties(audiocapturemgr::audio_properties_t &out_properties);
		unsigned int get_data_rate();
		void add_data(unsigned char *buf, unsigned int size);
		void data_processor_thread();
		int register_client(audio_capture_client *client);
		int unregister_client(audio_capture_client *client);
		int start();
		int stop();

		static rmf_Error data_callback(void *context, void *buf, unsigned int size);
};

class audio_capture_client
{

	private:
		unsigned int m_priority;
		pthread_mutex_t m_mutex;

	protected:
		q_mgr * m_manager;
		void release_buffer(audio_buffer *ptr);
		void lock();
		void unlock();

	public:
		virtual int data_callback(audio_buffer *buf);
		audio_capture_client(q_mgr * manager);
		virtual ~audio_capture_client();
		unsigned int get_priority() {return m_priority;}
		void set_manager(q_mgr *manager);
		virtual int set_audio_properties(audiocapturemgr::audio_properties_t &properties);
		virtual void get_audio_properties(audiocapturemgr::audio_properties_t &properties);
		void get_default_audio_properties(audiocapturemgr::audio_properties_t &properties);
		virtual void notify_event(audio_capture_events_t event){}
		virtual int start();
		virtual int stop();
};
#endif // _AUDIO_CAPTURE_MANAGER_h_
