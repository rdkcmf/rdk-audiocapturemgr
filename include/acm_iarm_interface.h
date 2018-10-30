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
