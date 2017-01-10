#ifndef _AUDIOCAPTUREMGR_IARM_H_
#define _AUDIOCAPTUREMGR_IARM_H_

#include "audio_capture_manager.h"

#define IARMBUS_AUDIOCAPTUREMGR_NAME "audiocapturemgr"
/*Original API list*/
#define IARMBUS_AUDIOCAPTUREMGR_ENABLE "enableCapture"
#define IARMBUS_AUDIOCAPTUREMGR_REQUEST_SAMPLE "requestSample"

/*New API list*/
#define IARMBUS_AUDIOCAPTUREMGR_OPEN "open"
#define IARMBUS_AUDIOCAPTUREMGR_CLOSE "close"
#define IARMBUS_AUDIOCAPTUREMGR_START "start"
#define IARMBUS_AUDIOCAPTUREMGR_STOP "stop"
#define IARMBUS_AUDIOCAPTUREMGR_GET_DEFAULT_AUDIO_PROPS "getDefaultAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_GET_AUDIO_PROPS "getAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_GET_OUTPUT_PROPS "getOutputProperties"
#define IARMBUS_AUDIOCAPTUREMGR_SET_AUDIO_PROPERTIES "setAudioProperties"
#define IARMBUS_AUDIOCAPTUREMGR_SET_OUTPUT_PROPERTIES "setOutputProperties"

/*End API list*/

#define AUDIOCAPTUREMGR_FILENAME_PREFIX "audio_sample"
#define AUDIOCAPTUREMGR_FILE_PATH "/opt/"
namespace audiocapturemgr
{
	typedef enum
	{
		BUFFERED_FILE_OUTPUT = 0,
		REALTIME_SOCKET,
		MAX_SUPPORTED_OUTPUT_TYPES
	}iarmbus_output_type_t;

	typedef int session_id_t;
	typedef enum
	{
		DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY = 0,
		IARMBUS_MAX_ACM_EVENT
	}iarmbus_events_t;

	typedef enum
	{
		ACM_RESULT_SUCCESS = 0,
		ACM_RESULT_UNSUPPORTED_API,
		ACM_RESULT_STREAM_UNAVAILABLE,
		ACM_RESULT_DURATION_OUT_OF_BOUNDS,
		ACM_RESULT_BAD_SESSION_ID,
		ACM_RESULT_INVALID_ARGUMENTS,
		ACM_RESULT_GENERAL_FAILURE,
		ACM_RESULT_PRECAPTURE_DURATION_TOO_LONG = 254,
		ACM_RESULT_PRECAPTURE_NOT_SUPPORTED = 255
	}iarmbus_audiocapturemgr_result_t;

	typedef struct
	{
		unsigned int result;
		float max_duration;
	}iarmbus_enable_payload_t;


	typedef struct
	{
		float duration;
		bool is_precapture;
	}iarmbus_request_payload_t;

	typedef struct
	{
		char dataLocator[64];
	}iarmbus_notification_payload_t;

	#define MAX_OUTPUT_PATH_LEN 256
	typedef struct
	{
		union
		{
			char file_path[MAX_OUTPUT_PATH_LEN]; //get unix domain socket name (ip out) 
			unsigned int buffer_duration; //set precapture duration (music id)
			unsigned int max_buffer_duration; //get max supported buffer duration (music id)
		}output;
	}iarmbus_delivery_props_t;

	typedef struct
	{
		int source;//0 for primary, increasing by 1 for each new source.
		iarmbus_output_type_t output_type;
	}iarmbus_open_args;

	typedef struct
	{
		session_id_t session_id;
		int result;
		union
		{
			iarmbus_open_args arg_open;
			audio_properties_t arg_audio_properties;
			iarmbus_request_payload_t arg_sample_request;
			iarmbus_delivery_props_t arg_output_props;
		}details;
	}iarmbus_acm_arg_t;
}
#endif //_AUDIOCAPTUREMGR_IARM_H_
