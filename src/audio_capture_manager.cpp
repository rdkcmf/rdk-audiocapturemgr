#include <stdlib.h>
#include <string.h>
#include "audio_capture_manager.h"
#include <algorithm>
#include <unistd.h>
#include "rmfAudioCapture.h"

const unsigned int QUEUE_CHECK_LOOP_TIME_US = 1000 * 50; //50ms

static void * q_mgr_thread_launcher(void * data)
{
	q_mgr * ptr = (q_mgr *) data;
	ptr->data_processor_thread();
	return NULL;
}

rmf_Error q_mgr::data_callback(void *context, void *buf, unsigned int size)
{
	((q_mgr *)context)->add_data((unsigned char*)buf, size);
	return RMF_SUCCESS;
}


inline void q_mgr::lock(pthread_mutex_t &mutex)
{
	REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&mutex));
}
inline void q_mgr::unlock(pthread_mutex_t &mutex)
{
	REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&mutex));
}
inline void q_mgr::notify_data_ready()
{
	DEBUG("Enter.\n");
	if(m_notify_new_data)
	{
		DEBUG("Posting semaphore.\n");
		REPORT_IF_UNEQUAL(0, sem_post(&m_sem));
		m_notify_new_data = false;
	}
}
void q_mgr::swap_queues() //caller must lock before invoking this.
{
	if(!m_current_outgoing_q->empty())
	{
		WARN("Outgoing queue wasn't empty. Flushing it now.\n");
		flush_queue(m_current_outgoing_q);
	}
	std::vector <audio_buffer *> * temp = m_current_incoming_q;
	m_current_incoming_q = m_current_outgoing_q;
	m_current_outgoing_q = temp;
}

void q_mgr::flush_queue(std::vector <audio_buffer *> *q)
{
	DEBUG("Flushing queue.\n");
	std::vector <audio_buffer *>::iterator iter;
	for(iter = q->begin(); iter != q->end(); iter++)
	{
		free_audio_buffer(*iter);	
	}
	q->clear();
}

q_mgr::q_mgr(audio_properties_t &in_properties) : m_total_size(0), m_num_clients(0), m_notify_new_data(false), m_device_handle(NULL)
{
	DEBUG("Creating instance.\n");
	pthread_mutexattr_t mutex_attribute;
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_init(&mutex_attribute));
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_ERRORCHECK));
	REPORT_IF_UNEQUAL(0, pthread_mutex_init(&m_q_mutex, &mutex_attribute));
	REPORT_IF_UNEQUAL(0, pthread_mutex_init(&m_client_mutex, &mutex_attribute));
	REPORT_IF_UNEQUAL(0, sem_init(&m_sem, 0, 0));
	m_current_incoming_q = new std::vector <audio_buffer *>;
	m_current_outgoing_q = new std::vector <audio_buffer *>; 
	set_audio_properties(in_properties);
	REPORT_IF_UNEQUAL(0, pthread_create(&m_thread, NULL, q_mgr_thread_launcher, (void *) this));
}
q_mgr::~q_mgr()
{
	DEBUG("Deleting instance.\n");
    if(NULL != m_device_handle)
    {
        stop();
    }
	m_processing_thread_alive = false;
	REPORT_IF_UNEQUAL(0, sem_post(&m_sem));
	REPORT_IF_UNEQUAL(0, pthread_join(m_thread, NULL));
	REPORT_IF_UNEQUAL(0, sem_destroy(&m_sem));
	REPORT_IF_UNEQUAL(0, pthread_mutex_destroy(&m_q_mutex));
	REPORT_IF_UNEQUAL(0, pthread_mutex_destroy(&m_client_mutex));
	flush_queue(m_current_incoming_q);
	flush_queue(m_current_outgoing_q);
	delete m_current_incoming_q;
	delete m_current_outgoing_q;
}

int q_mgr::set_audio_properties(audio_properties_t &in_properties)
{
	DEBUG("Enter.\n");
	lock(m_q_mutex);
	memcpy(&m_audio_properties, &in_properties, sizeof(m_audio_properties));
	m_bytes_per_second = m_audio_properties.bits_per_sample * m_audio_properties.sample_rate * m_audio_properties.num_channels / 8;
	lock(m_client_mutex);
	flush_system();
	unlock(m_client_mutex);
	unlock(m_q_mutex);

	lock(m_client_mutex);
	std::vector<audio_capture_client *>::iterator iter;
	for(iter = m_clients.begin(); iter != m_clients.end(); iter++)
	{
		(*iter)->notify_event(AUDIO_SETTINGS_CHANGE_EVENT);
	}
	unlock(m_client_mutex);
	return 0;
}

void q_mgr::get_audio_properties(audio_properties_t &out_properties)
{
	memcpy(&out_properties, &m_audio_properties, sizeof(out_properties));
}

unsigned int q_mgr::get_data_rate()
{
	return m_bytes_per_second;
}

void q_mgr::add_data(unsigned char *buf, unsigned int size)
{
	DEBUG("Adding data.\n");
	lock(m_q_mutex);
	audio_buffer * temp = create_new_audio_buffer(buf, size, 0, m_num_clients);

	m_current_incoming_q->push_back(temp);
	m_total_size += size;
	//TODO: guard against accumulating audio_buffers if there is no consumption.
	notify_data_ready();
	unlock(m_q_mutex);
}
void q_mgr::data_processor_thread()
{
	DEBUG("Launching.\n");
	m_processing_thread_alive = true;
	while(m_processing_thread_alive)
	{
		/*
		 * 1. If there are audio_buffers remaining in outbound queue, process them.
		 * 2. If no audio_buffers remain in outbound queue, wait for semaphore.
		 * 3. When semaphore is posted, if data is available in the other queue, swap queues.
		 * */
		DEBUG("Enter processing loop.\n");
		while(!m_current_outgoing_q->empty())
		{
			process_data();	
		}
		DEBUG("No data in outgoing queue.\n");
		/* No more data to be consumed. Gotta check the other queue */
		lock(m_q_mutex);
		if(!m_current_incoming_q->empty())
		{	
			DEBUG("Incoming queue has buffers. Swapping them.\n");
			swap_queues();
			unlock(m_q_mutex);
		}
		else
		{
			/*Block until data is available.*/
			m_notify_new_data = true;
			unlock(m_q_mutex);
			DEBUG("Incoming queue is empty as well. Waiting until a buffer arrives.\n");
			if(0 != sem_wait(&m_sem))
			{
				ERROR("Critical semaphore error. Exiting\n");
				return;
			}
			/* Just in case the semaphore was posted to shut down this thred... */
			if(!m_processing_thread_alive)
			{
				break;
			}

			lock(m_q_mutex);
			DEBUG("New data available in incoming queue. Swapping them.\n");
			swap_queues();
			unlock(m_q_mutex);
		}

	}
	DEBUG("Exiting.\n");
}

void q_mgr::update_buffer_references()
{
	std::vector <audio_buffer *>::iterator buffer_iter;
	audio_buffer_get_global_lock();
	for(buffer_iter = m_current_outgoing_q->begin(); buffer_iter != m_current_outgoing_q->end(); buffer_iter++)
	{
		set_ref_audio_buffer(*buffer_iter, m_num_clients);	
	}
	audio_buffer_release_global_lock();
}

void q_mgr::process_data()
{
	DEBUG("Processing %d buffers of data.\n", m_current_outgoing_q->size());

	lock(m_client_mutex);
	
	update_buffer_references();
	
	std::vector <audio_buffer *>::iterator buffer_iter;
	for(buffer_iter = m_current_outgoing_q->begin(); buffer_iter != m_current_outgoing_q->end(); buffer_iter++)
	{
		std::vector <audio_capture_client *>::iterator client_iter;
		for(client_iter = m_clients.begin(); client_iter != m_clients.end(); client_iter++)
		{
			(*client_iter)->data_callback(*buffer_iter);	
		}
	}
	unlock(m_client_mutex);
	m_current_outgoing_q->clear();
}

int q_mgr::register_client(audio_capture_client * client)
{
	DEBUG("Enter.\n");
	lock(m_client_mutex);
	m_clients.push_back(client);
	m_num_clients = m_clients.size();
	if(1 == m_num_clients)
	{
		start();
	}
	unlock(m_client_mutex);
	INFO("Total clients: %d.\n", m_num_clients);
	return 0;
}
int q_mgr::unregister_client(audio_capture_client * client)
{
	DEBUG("Enter.\n");
	lock(m_client_mutex);
	m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), client), m_clients.end());
	m_num_clients = m_clients.size();
	if(0 == m_num_clients)
	{
		stop();
	}
	unlock(m_client_mutex);
	INFO("Total clients: %d.\n", m_num_clients);
	return 0;
}

void q_mgr::flush_system()
{
	/*
	 * Flush everything in incoming queue.
	 * Wait until outbound queue is exhausted.
	 * Return.
	 * */
	flush_queue(m_current_incoming_q);
	while(!m_current_outgoing_q->empty())
	{
		usleep(QUEUE_CHECK_LOOP_TIME_US);
	}
	INFO("Exit.\n");
}

static void log_settings(RMF_AudioCapture_Settings &settings)
{
	INFO("Format 0x%x, sampling freq: 0x%x, FIFO size: %d, threshold: %d\n",
		settings.format, settings.samplingFreq, settings.fifoSize, settings.threshold);
}
static const size_t DEFAUL_FIFO_SIZE = 64 * 1024;
static const size_t	DEFAULT_THRESHOLD = 8 * 1024;

int q_mgr::start()
{
	if(NULL != m_device_handle)
	{
		WARN("Looks like device is already open.\n");
		return 0;
	}
	int ret = RMF_AudioCapture_Open(&m_device_handle);
	INFO("open() result is 0x%x\n", ret);
	
	RMF_AudioCapture_Settings settings;
	RMF_AudioCapture_GetDefaultSettings(&settings);
	
	settings.cbBufferReady = &q_mgr::data_callback;
	settings.cbBufferReadyParm = (void *)this;
	settings.fifoSize = DEFAUL_FIFO_SIZE; 
	settings.threshold = DEFAULT_THRESHOLD;

	log_settings(settings);
    INFO("Calcualted data rate is %d bytes per second.\n", m_bytes_per_second);
	
	ret = RMF_AudioCapture_Start(m_device_handle, &settings);
	INFO("start() result is 0x%x\n", ret);
	return ret;
}

int q_mgr::stop()
{
	if(NULL == m_device_handle)
	{
		WARN("Looks like device is already closed.\n");
		return 0;
	}
	int ret = RMF_AudioCapture_Stop(m_device_handle);
	INFO("stop() result is 0x%x\n", ret);
	ret = RMF_AudioCapture_Close(m_device_handle);
	INFO("close() result is 0x%x\n", ret);
	m_device_handle = NULL;
	return ret;
}

void audio_capture_client::release_buffer(audio_buffer *ptr)
{ 
	unref_audio_buffer(ptr);
} 

int audio_capture_client::data_callback(audio_buffer *buf)
{ 
	release_buffer(buf);
	return 0;
} 
audio_capture_client::audio_capture_client(q_mgr * manager): m_priority(0), m_manager(manager)
{ 
} 
audio_capture_client::~audio_capture_client()
{	
} 

void audio_capture_client::set_manager(q_mgr *mgr)
{
	m_manager = mgr;
}

int audio_capture_client::set_audio_properties(audio_properties_t &properties)
{
	return m_manager->set_audio_properties(properties);
}
void audio_capture_client::get_audio_properties(audio_properties_t &properties)
{
	m_manager->get_audio_properties(properties);
}

int audio_capture_client::start()
{
	return m_manager->register_client(this);
}

int audio_capture_client::stop()
{
	return m_manager->unregister_client(this);
}
