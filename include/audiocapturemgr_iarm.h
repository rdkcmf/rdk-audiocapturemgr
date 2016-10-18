#ifndef _AUDIOCAPTUREMGR_IARM_H_
#define _AUDIOCAPTUREMGR_IARM_H_

#define IARMBUS_AUDIOCAPTUREMGR_NAME "audiocapturemgr"
#define IARMBUS_AUDIOCAPTUREMGR_ENABLE "enableCapture"
#define IARMBUS_AUDIOCAPTUREMGR_REQUEST_SAMPLE "requestSample"
#define AUDIOCAPTUREMGR_FILENAME_PREFIX "audio_sample"
#define AUDIOCAPTUREMGR_FILE_PATH "/opt/"
namespace audiocapturemgr
{
	typedef enum
	{
		DATA_CAPTURE_IARM_EVENT_AUDIO_CLIP_READY = 0,
		IARMBUS_MAX_ACM_EVENT
	} iarmbus_events_t;

	typedef enum
	{
		ACM_RESULT_SUCCESS = 0,
		ACM_RESULT_UNSUPPORTED_API,
		ACM_RESULT_STREAM_UNAVAILABLE,
		ACM_RESULT_DURATION_OUT_OF_BOUNDS,
		ACM_RESULT_GENERAL_FAILURE,
		ACM_RESULT_PRECAPTURE_DURATION_TOO_LONG = 254,
		ACM_RESULT_PRECAPTURE_NOT_SUPPORTED = 255
	} iarmbus_audiocapturemgr_result_t;

	typedef struct
	{
		unsigned int result;
		float max_duration;
	} iarmbus_enable_payload_t;


	typedef struct
	{
		iarmbus_audiocapturemgr_result_t result;
		float duration;
		bool is_precapture;
		unsigned int stream;
	} iarmbus_request_payload_t;

	typedef struct
	{
		char dataLocator[64];
	} iarmbus_notification_payload_t;

}
#endif //_AUDIOCAPTUREMGR_IARM_H_
