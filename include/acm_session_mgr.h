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
#include "audio_capture_manager.h"
#include "music_id.h"
#include "ip_out.h"
#include "audiocapturemgr_iarm.h"
#include <vector>
#include <list>

typedef struct
{
	int session_id;
	audio_capture_client * client;
	q_mgr * source;
	bool enable;
	audiocapturemgr::iarmbus_output_type_t output_type;
}acm_session_t;

class acm_session_mgr
{
	private:
	std::list <acm_session_t *> m_sessions;
	std::vector <q_mgr *> m_sources;
	pthread_mutex_t m_mutex;
	int m_session_counter;

	public:
	acm_session_mgr();
	~acm_session_mgr();
	static acm_session_mgr * get_instance();

	int activate();
	int deactivate();
	int get_sample_handler(void * arg);
	int generic_handler(void * arg);
	int open_handler(void * arg);
	int close_handler(void * arg);
	int get_default_audio_props_handler(void * arg);
	int get_audio_props_handler(void * arg);
	int get_output_props_handler(void * arg);
	int set_audio_props_handler(void * arg);
	int set_output_props_handler(void * arg);
	int start_handler(void * arg);
	int stop_handler(void * arg);
	void set_filename_prefix(std::string &prefix);

	private:
	q_mgr * get_source(int source);
	void lock();
	void unlock();
	acm_session_t * get_session(int session_id);
};
