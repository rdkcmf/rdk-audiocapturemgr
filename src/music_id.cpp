#include "music_id.h"
#include <unistd.h>
#include <stdint.h>


const unsigned int DEFAULT_PRECAPTURE_DURATION_SEC = 6;

static void * music_id_thread_launcher(void * data)
{
    music_id_client * ptr = (music_id_client *) data;
    ptr->worker_thread();
    return NULL;
}
music_id_client::music_id_client(q_mgr * manager) : audio_capture_client(manager), m_worker_thread_alive(true), m_total_size(0), 
	m_queue_upper_limit_bytes(0), m_request_counter(0), m_enable_wav_header_output(false)
{
	DEBUG("Creating instance.\n");
	pthread_mutexattr_t mutex_attribute;
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_init(&mutex_attribute));
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_ERRORCHECK));
	REPORT_IF_UNEQUAL(0, pthread_mutex_init(&m_mutex, &mutex_attribute));
	set_precapture_duration(DEFAULT_PRECAPTURE_DURATION_SEC);
	REPORT_IF_UNEQUAL(0, pthread_create(&m_thread, NULL, music_id_thread_launcher, (void *) this));
}

music_id_client::~music_id_client()
{
    DEBUG("Deleting instance.\n");
    m_worker_thread_alive = false;
    REPORT_IF_UNEQUAL(0, pthread_join(m_thread, NULL));
	
	/*Flush all queues.*/
	INFO("Flushing request queue. Size is %d\n", m_requests.size());
	std::list<request_t *>::iterator req_iter;
	for(req_iter = m_requests.begin(); req_iter != m_requests.end(); req_iter++)
	{
		delete (*req_iter);
	}
	m_requests.clear();
	
	INFO("Flushing buffers.\n");
	std::list<audio_buffer *>::iterator buf_iter;
	for(buf_iter = m_queue.begin(); buf_iter != m_queue.end(); buf_iter++)
	{
		release_buffer(*buf_iter);
	}
	m_queue.clear();

    REPORT_IF_UNEQUAL(0, pthread_mutex_destroy(&m_mutex));
}

inline void music_id_client::lock()
{
    REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
}
inline void music_id_client::unlock()
{
    REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
}

int music_id_client::data_callback(audio_buffer *buf)
{
	lock();
	m_queue.push_back(buf);
	m_total_size += buf->m_size;
	unlock();
	return 0;
}

int music_id_client::set_precapture_duration(unsigned int seconds)
{
    lock();
	m_precapture_duration_seconds = seconds;
	m_precapture_size_bytes = seconds * m_manager->get_data_rate();
	if(m_queue_upper_limit_bytes < m_precapture_size_bytes)
	{
		m_queue_upper_limit_bytes = m_precapture_size_bytes;
	}
    compute_queue_size();
    trim_queue();
    unlock();
	return 0;
}

int music_id_client::set_audio_properties(audio_properties_t &properties)
{
	int ret;
	ret = audio_capture_client::set_audio_properties(properties);
	if(0 == ret) 
	{
		/* Populate bit rate fields.*/
		m_precapture_size_bytes = m_precapture_duration_seconds * m_manager->get_data_rate();
	}
	return ret;
}
void music_id_client::trim_queue() //Recommend using lock.
{
	int excess_bytes = m_total_size - m_queue_upper_limit_bytes;
	DEBUG("excess_bytes = %d\n", excess_bytes);
	while(0 < excess_bytes)
	{
		/* Lose buffers until losing any more would take us below the precapture threshold.*/
		unsigned int current_buffer_size = m_queue.front()->m_size;
		if((unsigned int)excess_bytes >= current_buffer_size)
		{
			excess_bytes -= current_buffer_size; 
			m_total_size -= current_buffer_size;
			release_buffer((m_queue.front()));
			m_queue.pop_front();
		}
		else
		{
			break;
		}
	}
}

int music_id_client::grab_precaptured_sample(const std::string &filename)
{
    int ret;
    lock();
	ret = grab_last_n_seconds(filename, m_precapture_duration_seconds);
    unlock();
    return ret;
}

int music_id_client::grab_last_n_seconds(const std::string &filename, unsigned int seconds)
{
	int ret = 0;
	int data_dump_size = seconds * m_manager->get_data_rate();

	if(0 != m_queue.size())
	{
		std::ofstream file(filename.c_str(), std::ios::binary);
		if(file.is_open())
		{
			if(m_enable_wav_header_output)
			{
				write_default_file_header(file);
			}
			std::list <audio_buffer *>::iterator iter;
			for(iter = m_queue.begin(); iter != m_queue.end(); iter++)
			{
				file.write((char *)(*iter)->m_start_ptr, (*iter)->m_size);
				data_dump_size -= (*iter)->m_size;
				if(0 > data_dump_size)
				{
					break;
				}
			}
			if(m_enable_wav_header_output)
			{
				unsigned int payload_size = static_cast<unsigned int>(file.tellp()) - 44;
				update_file_header_size(file, payload_size);
			}
			INFO("Precaptured sample written to %s.\n", filename.c_str());
		}
		else
		{
			ERROR("Could not open file %s.\n", filename.c_str());
			ret = -1;
		}
	}
	else
	{
		ERROR("Error! Precaptured queue is empty.\n");
		ret = -1;
	}
	return ret;
}

music_id_client::request_id_t music_id_client::grab_fresh_sample(const std::string &filename, unsigned int seconds, request_complete_callback_t cb , void * cb_data)
{
	request_id_t id = -1;
	lock();
	request_t *req = new request_t;
	req->id = m_request_counter++;
	req->filename = filename;
	req->length = seconds;
	req->time_remaining = seconds;
	req->callback = cb;
	req->callback_data = cb_data;
	m_requests.push_back(req);
	compute_queue_size();
	unlock();
	return id;
}

void music_id_client::compute_queue_size() //needs lock
{
	std::list<request_t *>::iterator iter;
	unsigned int max_length = m_precapture_duration_seconds;
	for(iter = m_requests.begin(); iter != m_requests.end(); iter++)
	{
		if(max_length < (*iter)->length)
		{
			max_length = (*iter)->length;
		}
	}
    INFO("New max length for queue: %d\n", max_length);
	m_queue_upper_limit_bytes = max_length * m_manager->get_data_rate();
}

void music_id_client::worker_thread()
{
	INFO("Enter.\n");
	while(m_worker_thread_alive)
	{
		DEBUG("Tick.\n");
		lock();
		std::list<request_t *>::iterator iter = m_requests.begin();
		while(iter != m_requests.end())
		{
			request_t *request = *iter;
			if(0 == request->time_remaining--)
			{
				INFO("Request %d is up.\n", request->id);
				int ret = grab_last_n_seconds(request->filename, request->length);
				if(0 != ret) 
				{
					ERROR("Failed to fulfil request %d.\n", (*iter)->id);
				}
				if(request->callback)
				{
					(request->callback)(request->callback_data, request->filename, ret);
				}
				delete request;
				iter = m_requests.erase(iter);
                compute_queue_size();
			}
			else
			{
				iter++;
			}
		}
        trim_queue();
		unlock();
		sleep(1);
	}
	INFO("Exit.\n");
}

static void write_32byte_little_endian(uint32_t data, std::ofstream &file)
{
    uint8_t buf[4];
    buf[0] = (uint8_t)(0xFF & data);
    buf[1] = (uint8_t)(0xFF & (data >> 8));
    buf[2] = (uint8_t)(0xFF & (data >> 16));
    buf[3] = (uint8_t)(0xFF & (data >> 24));
    file.write((const char *)&buf, 4);
}

#if 0
static void write_32data_big_endian(uint32_t data, std::ofstream &file)
{
    uint8_t buf[4];
    buf[3] = (uint8_t)(0xFF & data);
    buf[2] = (uint8_t)(0xFF & (data >> 8));
    buf[1] = (uint8_t)(0xFF & (data >> 16));
    buf[0] = (uint8_t)(0xFF & (data >> 24));
    file.write((const char *)&buf, 4);
}
#endif
static void write_16byte_little_endian(uint16_t data, std::ofstream &file)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(0xFF & data);
    buf[1] = (uint8_t)(0xFF & (data >> 8));
    file.write((const char *)&buf, 2);
}

#if 0
int music_id_client::write_queue_to_file(const std::string &filename) // needs lock
{
    int ret = 0;

    if(0 != m_queue.size())
    {
        std::ofstream file(filename.c_str(), std::ios::binary);
        if(file.is_open())
        {
            write_file_header(file, m_total_size);       
            std::list <audio_buffer *>::iterator iter;
            for(iter = m_queue.begin(); iter != m_queue.end(); iter++) 
            {
                file.write((char *)(*iter)->m_start_ptr, (*iter)->m_size);
			}
            INFO("Sample written to %s.\n", filename.c_str());
        }
        else
        {
            ERROR("Could not open file %s.\n", filename.c_str());
            ret = -1;
        }
    }
    else
    {
        ERROR("Error! queue is empty.\n");
        ret = -1;
    }
    return ret;
}
#endif

int music_id_client::write_default_file_header(std::ofstream &file)
{
	/* Write file header chunk.*/
	file.write("RIFF", 4);
	uint32_t chunkSize = 0; //Will be populated later.
	write_32byte_little_endian(chunkSize, file);

	file.write("WAVE", 4);

	/* Write fmt sub-chunk */
	file.write("fmt ", 4);
	write_32byte_little_endian(16, file);//SubChunk1Size
	write_16byte_little_endian(1, file);//Audo format PCM

	audio_properties_t properties;
	get_audio_properties(properties);
	write_16byte_little_endian((uint16_t)properties.num_channels, file);
	write_32byte_little_endian(properties.sample_rate, file);
	write_32byte_little_endian(m_manager->get_data_rate(), file);
	write_16byte_little_endian((uint16_t)(properties.num_channels * properties.bits_per_sample / 8), file); //Block align
	write_16byte_little_endian((uint16_t)properties.bits_per_sample, file);

	/* Write data sub-chunk header*/
	file.write("data", 4);
	write_32byte_little_endian(chunkSize, file); //Will be populated later.
	return 0;
}
int music_id_client::update_file_header_size(std::ofstream &file, unsigned int data_size)
{
	INFO("Finalizing file header. Payload size: %dkB.\n", (data_size/1024));

	/* Update file header chunk size.*/
	uint32_t chunkSize = 36 + data_size;
    file.seekp(4);
	write_32byte_little_endian(chunkSize, file);

	/* Update data subchunk size.*/
    file.seekp(40);
	write_32byte_little_endian(data_size, file);
	return 0;
}

static const unsigned int MAX_PRECAPTURE_LENGTH_SEC = 120;
unsigned int music_id_client::get_max_supported_duration()
{
	//TODO: If necessary, make this a run-time decision based on the current data rate of audio.
	return MAX_PRECAPTURE_LENGTH_SEC;
}

unsigned int music_id_client::enable_wav_header(bool isEnabled)
{
	m_enable_wav_header_output = isEnabled;
	return 0;
}
