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
inline void set_ref_audio_buffer(audio_buffer *ptr, unsigned int refcount){ ptr->m_refcount = refcount; } //needs global lock
void audio_buffer_get_global_lock();
void audio_buffer_release_global_lock();
void free_audio_buffer(audio_buffer *ptr);
