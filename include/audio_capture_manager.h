#ifndef _AUDIO_CAPTURE_MANAGER_H_
#define _AUDIO_CAPTURE_MANAGER_H_
#include <pthread.h>
#include <vector>
#include <semaphore.h>
#include "audio_buffer.h"
#include "basic_types.h"
#include "rmf_error.h"
#include "rmfAudioCapture.h"

class audio_capture_client;
class q_mgr
{
	private:
	std::vector <audio_buffer *> *m_current_incoming_q;
	std::vector <audio_buffer *> *m_current_outgoing_q;
	std::vector <audio_capture_client *> m_clients;
	audio_properties_t m_audio_properties;
	unsigned int m_bytes_per_second;
	unsigned int m_total_size;
	unsigned int m_num_clients;
	pthread_mutex_t m_mutex;
	sem_t m_sem;
	pthread_t m_thread;
	bool m_processing_thread_alive;
	bool m_notify_new_data;
	RMF_AudioCaptureHandle m_device_handle;

	private:
	inline void lock();
	inline void unlock();
	inline void notify_data_ready();
	void swap_queues(); //caller must lock before invoking this.
	void flush_queue(std::vector <audio_buffer *> *q);
	void flush_system();
	void process_data();

	public:
	q_mgr(audio_properties_t &in_properties);
	~q_mgr();

	int set_audio_properties(audio_properties_t &in_properties);
	void get_audio_properties(audio_properties_t &out_properties);
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

    protected:
	q_mgr * m_manager;
    void release_buffer(audio_buffer *ptr);

    public:
    virtual int data_callback(audio_buffer *buf);
    audio_capture_client(q_mgr * manager);
    ~audio_capture_client();
	unsigned int get_priority() {return m_priority;}
	void set_manager(q_mgr *manager);
	virtual int set_audio_properties(audio_properties_t &properties);
	void get_audio_properties(audio_properties_t &properties);
	virtual void notify_event(audio_capture_events_t event){}
};
#endif // _AUDIO_CAPTURE_MANAGER_h_
