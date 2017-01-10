#include "acm_session_mgr.h"
#include "audiocapturemgr_iarm.h"
#include <string>
#include <string.h>
#include <sstream>
#include "libIARM.h"
#include "libIBus.h"


using namespace audiocapturemgr;

static const unsigned int MAX_SUPPORTED_SOURCES = 1; //Primary only at the moment
static acm_session_mgr g_singleton;

static unsigned int ticker = 0;
std::string audio_filename_prefix = AUDIOCAPTUREMGR_FILENAME_PREFIX;
std::string audio_file_path = AUDIOCAPTUREMGR_FILE_PATH;

static void request_callback(void * data, std::string &file, int result)
{
	if(0 != result)
	{
		ERROR("Failed to grab sample.\n");
	}
	else
	{
		iarmbus_notification_payload_t payload;
		if(file.size() < sizeof(payload.dataLocator))
		{
			memcpy(payload.dataLocator, file.c_str(), file.size());
			payload.dataLocator[file.size()] = '\0';
			int ret = IARM_Bus_BroadcastEvent(IARMBUS_AUDIOCAPTUREMGR_NAME, DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY, 
				&payload, sizeof(payload));
			REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
		}
		else
		{
			WARN("Incoming filename is too big for payload buffer.\n");
		}
	}
}

static IARM_Result_t request_sample(void * arg)
{
	g_singleton.get_sample_handler(arg);
	return IARM_RESULT_SUCCESS;
}

static IARM_Result_t open(void * arg)
{
	g_singleton.open_handler(arg);
	return IARM_RESULT_SUCCESS;
}
static IARM_Result_t close(void * arg)
{
	g_singleton.close_handler(arg);
	return IARM_RESULT_SUCCESS;
}
static IARM_Result_t start(void * arg)
{
	g_singleton.start_handler(arg);
	return IARM_RESULT_SUCCESS;
}
static IARM_Result_t stop(void * arg)
{
	g_singleton.stop_handler(arg);
	return IARM_RESULT_SUCCESS;
}
static IARM_Result_t get_audio_props(void * arg)
{
	g_singleton.get_audio_props_handler(arg);
	return IARM_RESULT_SUCCESS;
}
static IARM_Result_t get_default_audio_props(void * arg)
{
	g_singleton.get_default_audio_props_handler(arg);
	return IARM_RESULT_SUCCESS;
}
static IARM_Result_t get_output_props(void * arg)
{
	g_singleton.get_output_props_handler(arg);
	return IARM_RESULT_SUCCESS;
}
static IARM_Result_t set_output_props(void * arg)
{
	g_singleton.set_output_props_handler(arg);
	return IARM_RESULT_SUCCESS;
}
static IARM_Result_t set_audio_props(void * arg)
{
	g_singleton.set_audio_props_handler(arg);
	return IARM_RESULT_SUCCESS;
}
acm_session_mgr::acm_session_mgr() : m_session_counter(0)
{
	INFO("Enter\n");

	for(unsigned int i = 0; i < MAX_SUPPORTED_SOURCES; i++)
	{
		m_sources.push_back((q_mgr *)NULL);
	}

	/*Add primary audio to the list of sources*/
	m_sources[0] = new q_mgr;

	pthread_mutexattr_t mutex_attribute;
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_init(&mutex_attribute));
	REPORT_IF_UNEQUAL(0, pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_ERRORCHECK));
	REPORT_IF_UNEQUAL(0, pthread_mutex_init(&m_mutex, &mutex_attribute));
}

acm_session_mgr::~acm_session_mgr()
{
	INFO("Enter\n");
	REPORT_IF_UNEQUAL(0, pthread_mutex_destroy(&m_mutex));
}

void acm_session_mgr::lock()
{
    REPORT_IF_UNEQUAL(0, pthread_mutex_lock(&m_mutex));
}

void acm_session_mgr::unlock()
{
    REPORT_IF_UNEQUAL(0, pthread_mutex_unlock(&m_mutex));
}

acm_session_mgr * acm_session_mgr::get_instance()
{
	return &g_singleton;
}

int acm_session_mgr::activate()
{
	int ret;
	INFO("Enter\n");

	//TODO: add early exit for each of the failures below
	ret = IARM_Bus_Init(IARMBUS_AUDIOCAPTUREMGR_NAME);
	REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);

	ret = IARM_Bus_Connect();
	REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);

	ret =  IARM_Bus_RegisterEvent(IARMBUS_MAX_ACM_EVENT);
	REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);

	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_REQUEST_SAMPLE, request_sample); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_OPEN, open); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_CLOSE, close); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_START, start); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_STOP, stop); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_GET_DEFAULT_AUDIO_PROPS, get_default_audio_props); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_GET_AUDIO_PROPS, get_audio_props); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_GET_OUTPUT_PROPS, get_output_props); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_SET_AUDIO_PROPERTIES, set_audio_props); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_RegisterCall(IARMBUS_AUDIOCAPTUREMGR_SET_OUTPUT_PROPERTIES, set_output_props); REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	return ret;
}
int acm_session_mgr::deactivate()
{
	int ret;
	INFO("Enter\n");
	ret = IARM_Bus_Disconnect();
	REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	ret = IARM_Bus_Term();
	REPORT_IF_UNEQUAL(IARM_RESULT_SUCCESS, ret);
	return ret;
}

int acm_session_mgr::open_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("source: 0x%x, output type: 0x%x\n", param->details.arg_open.source, param->details.arg_open.output_type);
	if( (MAX_SUPPORTED_SOURCES <= param->details.arg_open.source) || (MAX_SUPPORTED_OUTPUT_TYPES <= param->details.arg_open.output_type))
	{
		ERROR("Unsupported paramters.\n");
		param->result = ACM_RESULT_INVALID_ARGUMENTS;
		return param->result;
	}

	acm_session_t *new_session = new acm_session_t;
	new_session->source = m_sources[param->details.arg_open.source];
	switch(param->details.arg_open.output_type)
	{
		case BUFFERED_FILE_OUTPUT:
			new_session->client = new music_id_client(new_session->source);
			param->result = 0;
			break;

		case REALTIME_SOCKET:
			new_session->client = new ip_out_client(new_session->source);
			param->result = 0;
			break;

		default:
			ERROR("Unrecognized output type.\n");
	}
	new_session->enable = false;
	new_session->output_type = param->details.arg_open.output_type;

	acm_session_t *duplicate_session = NULL;
	
	lock();
	new_session->session_id = m_session_counter++;
	/*Special handling to deal with Receiver restarts. We accept only one instance of
	 * music id client per source type. If a new open request is made, this indicates the app
	 * that called for it has probably lost track (Receiver restart, maybe). Reuse the previous
	 * session*/
	if(BUFFERED_FILE_OUTPUT == new_session->output_type)
	{
		std::list <acm_session_t *>::iterator iter;
		for(iter = m_sessions.begin(); iter != m_sessions.end(); iter++)
		{
			if((BUFFERED_FILE_OUTPUT == (*iter)->output_type) && (new_session->source == (*iter)->source))
			{
				duplicate_session = *iter;
				m_sessions.erase(iter);
				break;
			}
		}
	}
	m_sessions.push_back(new_session);
	unlock();

	if(duplicate_session)
	{
		WARN("Detroying existing music id session (id %d) for the same source.\n", duplicate_session->session_id);
		duplicate_session->client->stop();
		delete duplicate_session->client;
		delete duplicate_session;
	}
	param->result = 0;
	INFO("Created session 0x%x\n", new_session->session_id);
	param->session_id = new_session->session_id;

	return param->result;
}

int acm_session_mgr::close_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);

	acm_session_t * ptr = NULL; 
	
	lock();
	std::list <acm_session_t *>::iterator iter;
	for(iter = m_sessions.begin(); iter != m_sessions.end(); iter++)
	{
		if(param->session_id == (*iter)->session_id)
		{
			ptr = *iter;
			m_sessions.erase(iter);
			break;
		}
	}
	unlock();
		
	if(ptr)
	{
		if(ptr->enable)
		{
			ptr->client->stop();
			ptr->enable = false;
			INFO("Stopped session 0x%x.\n", param->session_id);
		}
		delete ptr->client;
		delete ptr;
		param->result = 0;
		INFO("Session destroyed.\n");
	}
	else
	{
		WARN("Session not found!\n");
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}
	return param->result;
}

acm_session_t * acm_session_mgr::get_session(int session_id)
{
	acm_session_t * ptr = NULL;
	lock();
	std::list<acm_session_t *>::iterator iter;
	for(iter = m_sessions.begin(); iter != m_sessions.end(); iter++)
	{
		if(session_id == (*iter)->session_id)
		{
			ptr = *iter;
			break;
		}
	}
	unlock();
	return ptr;
}

int acm_session_mgr::start_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);

	acm_session_t * ptr = get_session(param->session_id);
	if(ptr)
	{
		if(!ptr->enable)
		{
			ptr->client->start();
			ptr->enable = true;
			INFO("Started session 0x%x.\n", param->session_id);
		}
		param->result = 0;
	}
	else
	{
		ERROR("Session not found!\n")
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}

	return param->result;
}

int acm_session_mgr::stop_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);

	acm_session_t * ptr = get_session(param->session_id);
	if(ptr)
	{
		if(ptr->enable)
		{
			ptr->client->stop();
			ptr->enable = false;
			INFO("Stopped session 0x%x.\n", param->session_id);
		}
		param->result = 0;
	}
	else
	{
		ERROR("Session not found!\n")
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}

	return param->result;
}

int acm_session_mgr::get_default_audio_props_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);
	acm_session_t * ptr = get_session(param->session_id);
	if(ptr)
	{
		audio_properties_t props;
		ptr->client->get_default_audio_properties(props);
		param->details.arg_audio_properties = props;
		param->result = 0;
	}
	else
	{
		ERROR("Session not found!\n")
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}
	return param->result;
}


int acm_session_mgr::get_audio_props_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);
	acm_session_t * ptr = get_session(param->session_id);
	if(ptr)
	{
		audio_properties_t props;
		ptr->client->get_audio_properties(props);
		param->details.arg_audio_properties = props;
		param->result = 0;
	}
	else
	{
		ERROR("Session not found!\n")
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}
	return param->result;
}
int acm_session_mgr::get_output_props_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);
	acm_session_t * ptr = get_session(param->session_id);
	if(ptr)
	{
		if(REALTIME_SOCKET == ptr->output_type)
		{
			ip_out_client * client = static_cast <ip_out_client *> (ptr->client);
			std::string sock_path = client->get_data_path();
			if(sock_path.empty())
			{
				param->result = ACM_RESULT_GENERAL_FAILURE;
			}
			else
			{
				strncpy(param->details.arg_output_props.output.file_path, sock_path.c_str(), sizeof(param->details.arg_output_props.output.file_path));
				param->result = 0;
			}
		}
		else if(BUFFERED_FILE_OUTPUT == ptr->output_type)
		{
			music_id_client * client = static_cast <music_id_client *> (ptr->client);
			param->details.arg_output_props.output.max_buffer_duration = client->get_max_supported_duration();
			param->result = 0;
		}
	}
	else
	{
		ERROR("Session not found!\n")
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}
	return param->result;
}

int acm_session_mgr::set_audio_props_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);
	acm_session_t * ptr = get_session(param->session_id);
	if(ptr)
	{
		param->result = ptr->client->set_audio_properties(param->details.arg_audio_properties);
	}
	else
	{
		ERROR("Session not found!\n")
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}
	return param->result;
}


int acm_session_mgr::set_output_props_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);
	acm_session_t * ptr = get_session(param->session_id);
	if(ptr)
	{
		if(BUFFERED_FILE_OUTPUT == ptr->output_type)
		{
			music_id_client * client = static_cast <music_id_client *> (ptr->client);
			unsigned int duration = param->details.arg_output_props.output.buffer_duration; 
			if(duration < client->get_max_supported_duration())
			{
				param->result = client->set_precapture_duration(duration);
			}
			else
			{
				ERROR("Duration out of bounds!\n");
				param->result = ACM_RESULT_DURATION_OUT_OF_BOUNDS;
			}
		}
		else
		{
			WARN("Not implemented for this type of output.\n");
			param->result = ACM_RESULT_UNSUPPORTED_API;
		}
	}
	else
	{
		ERROR("Session not found!\n")
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}
	return param->result;
}

int acm_session_mgr::get_sample_handler(void * arg)
{
	iarmbus_acm_arg_t *param = static_cast <iarmbus_acm_arg_t *> (arg);
	INFO("session_id 0x%x\n", param->session_id);
	acm_session_t * ptr = get_session(param->session_id);
	if(ptr)
	{
		if(BUFFERED_FILE_OUTPUT == ptr->output_type)
		{
			music_id_client * client = static_cast <music_id_client *> (ptr->client);

			if(false == ptr->enable)
			{
				ERROR("Audio capture is currently disabled!\n");
				param->result = ACM_RESULT_DURATION_OUT_OF_BOUNDS;
			}
			else
			{
				int ret = 0;
				param->result = ACM_RESULT_SUCCESS;
				std::string filename = audio_file_path + audio_filename_prefix + get_suffix(ticker++);

				if(param->details.arg_sample_request.is_precapture)
				{
					ret = client->grab_precaptured_sample(filename);
					if(0 == ret)
					{
						/* Precapture is immediate. Notify now.*/
						request_callback(NULL, filename, 0);
					}
				}
				else
				{
					unsigned int duration = param->details.arg_sample_request.duration;	
					if(duration <= client->get_max_supported_duration())
					{
						ret = client->grab_fresh_sample(filename, duration, &request_callback, NULL);
					}
					else
					{
						ERROR("Duration out of bounds!\n");
						param->result = ACM_RESULT_DURATION_OUT_OF_BOUNDS;
					}
				}
				/*Note: tricky condition check. ret is also zero if it's an out-of-bounds error, 
				 * but in that case, result is already updated.*/
				if(0 != ret)
				{
					param->result = ACM_RESULT_GENERAL_FAILURE;
				}
				
			}
		}
		else
		{
			WARN("Not supported with this output type.\n");
			param->result = ACM_RESULT_UNSUPPORTED_API;
		}
	}
	else
	{
		ERROR("Session not found!\n")
		param->result = ACM_RESULT_BAD_SESSION_ID;
	}
	return param->result;
}

void acm_session_mgr::set_filename_prefix(std::string &prefix)
{
	audio_filename_prefix = prefix;	
}
