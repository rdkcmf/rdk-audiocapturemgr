#ifndef _AUDIO_CONVERTER_H_
#define _AUDIO_CONVERTER_H_

#include <fstream>
#include <list>
#include "audio_capture_manager.h"

class audio_converter
{
	public:
		typedef enum
		{
			NO_CONVERSION = 0,
			DOWNMIX,
			DOWNSAMPLE,
			DOWNMIX_AND_DOWNSAMPLE,
			UNSUPPORTED_CONVERSION,
		} conversion_ops_t;

	private:
	const audiocapturemgr::audio_properties_t &m_in_props;
	const audiocapturemgr::audio_properties_t &m_out_props;
	conversion_ops_t m_op;
	bool m_downmix;
	bool m_downsample;

	virtual	int write_data(const char * ptr, unsigned int size){}
	int process_conversion_params();
	int downmix(const std::list<audio_buffer *> &queue, int size); 
	int downsample_and_downmix(const std::list<audio_buffer *> &queue, int size);
	int passthrough(const std::list<audio_buffer *> &queue, int size);


	public:
	audio_converter(const audiocapturemgr::audio_properties_t &in_props,const audiocapturemgr::audio_properties_t &out_props);
	virtual ~audio_converter() {}
	int convert(const std::list<audio_buffer *> &queue, unsigned int size);
	int convert(const audio_buffer * buffer) {} //TODO
};

class audio_converter_file_op : public audio_converter
{
	private:
	std::ofstream &m_file;
	virtual int write_data(const char * ptr, unsigned int size) override;

	public:
	audio_converter_file_op(const audiocapturemgr::audio_properties_t &in_props, const audiocapturemgr::audio_properties_t &out_props, std::ofstream &file);
	~audio_converter_file_op() {}
};

#endif //_AUDIO_CONVERTER_H_
