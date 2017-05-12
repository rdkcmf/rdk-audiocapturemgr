#include "audio_converter.h"

audio_converter::audio_converter(const audiocapturemgr::audio_properties_t &in_props, const audiocapturemgr::audio_properties_t &out_props) : m_in_props(in_props), m_out_props(out_props)
{
	process_conversion_params();
}


int audio_converter::process_conversion_params()
{
	int ret = -1;
	bool format_ok = false;
	bool sample_rate_ok = false;
	bool downmix = false;
	bool downsample = false;
	m_op = UNSUPPORTED_CONVERSION;

	do
	{
		if((racFormat_e16BitMono == m_out_props.format) && (racFormat_e16BitStereo == m_in_props.format))
		{
			INFO("Convert 16-bit stereo to mono.\n");
			format_ok = true;
			downmix = true;
			break;
		}

		if(m_out_props.format == m_in_props.format)
		{
			INFO("No format conversion.\n");
			format_ok = true;
			break;
		}

		if((racFormat_e16BitMono == m_out_props.format) && (
			(racFormat_e16BitMonoLeft == m_in_props.format) ||
			(racFormat_e16BitMonoRight == m_in_props.format)))
		{
			INFO("Convert Mono/mono-left/mono-right to mono.\n");
			format_ok = true;
			break;
		}
		ERROR("Unsupported format conversion: 0x%x to 0x%x.\n", m_in_props.format, m_out_props.format);
	
	} while(false);

	if(format_ok)
	{
		unsigned int in_sampling_rate, in_bits_per_sample, in_num_channels;
		unsigned int out_sampling_rate, out_bits_per_sample, out_num_channels;
		audiocapturemgr::get_individual_audio_parameters(m_in_props, in_sampling_rate, in_bits_per_sample, in_num_channels);
		audiocapturemgr::get_individual_audio_parameters(m_out_props, out_sampling_rate, out_bits_per_sample, out_num_channels);

		if(out_sampling_rate > in_sampling_rate)
		{
			ERROR("Cannot up-sample. %d to %d.\n", in_sampling_rate,  out_sampling_rate);
		}
		else if(0 != (in_sampling_rate % out_sampling_rate))
		{
			ERROR("Incompatible sampling rates: %d to %d.\n", in_sampling_rate,  out_sampling_rate);
		}
		else
		{
			sample_rate_ok = true;
			if(out_sampling_rate < in_sampling_rate)
			{
				downsample = true;
				INFO("Downsample from %d to %d\n", in_sampling_rate, out_sampling_rate);
			}
		}
	}

	if(format_ok && sample_rate_ok) 
	{
		if (downmix && downsample)
		{
			m_op = DOWNMIX_AND_DOWNSAMPLE;
		}
		else if(downmix && !downsample)
		{
			m_op = DOWNMIX;
		}
		else if (!downmix && downsample)
		{
			m_op = DOWNSAMPLE;
		}
		else
		{
			m_op = NO_CONVERSION;
		}
		ret = 0;
	}
	return ret;
}

int audio_converter::downsample_and_downmix(const std::list<audio_buffer *> &queue, int size)
{
	int ret = 0;
	
	unsigned int in_sampling_rate, in_bits_per_sample, in_num_channels;
	audiocapturemgr::get_individual_audio_parameters(m_in_props, in_sampling_rate, in_bits_per_sample, in_num_channels);

	unsigned int out_sampling_rate, out_bits_per_sample, out_num_channels;
	audiocapturemgr::get_individual_audio_parameters(m_out_props, out_sampling_rate, out_bits_per_sample, out_num_channels);

	unsigned int sample_size = in_bits_per_sample / 8;
	unsigned int frame_size = in_num_channels * sample_size; 
	unsigned int leap_value = frame_size * in_sampling_rate / out_sampling_rate;
	unsigned int write_length = (DOWNMIX_AND_DOWNSAMPLE == m_op ? sample_size : frame_size ); //Write only first sample of a frame if we're downmixing.

	int read_offset = 0;

	for(auto &entry: queue)
	{
		char * ptr = (char *)entry->m_start_ptr + read_offset;
		if(0 != (entry->m_size % frame_size))
		{
			WARN("Audio buffer not aligned with frame boundary!\n");
		}

		int buffer_size = entry->m_size;
		while (buffer_size > 0)
		{
			write_data(ptr, write_length); //TODO Check for errors
			buffer_size -= leap_value;
			ptr += leap_value;
		}

		/*If the value of buffer_size when exiting the above loop is a negative value (it can only be zero or negative), we need to skip that many bytes
		 * when processing the next buffer in the list. This is to maintain the continuity of 'frame-skipping' across buffer boundaries. read_offset will 
		 * now be a positive value that we advance the read pointer of the next buffer by.
		 * 
		 * Illustration:
		 * If we're skipping 3 samples for every 4 samples read (i. e. writing only one sample for every 4 samples read), and we ran out of data in the current
		 * buffer after skipping just one sample, the remaining 2 samples need to be skipped at the beginning of the next buffer.
		 * */
		read_offset = -1 * buffer_size;

		size -= entry->m_size;
		if(0 >= size)
		{
			break;
		}
	}
	return ret;
}

int audio_converter::downmix(const std::list<audio_buffer *> &queue, int size)
{
	int ret = 0;
	
	unsigned int in_sampling_rate, in_bits_per_sample, in_num_channels;
	audiocapturemgr::get_individual_audio_parameters(m_in_props, in_sampling_rate, in_bits_per_sample, in_num_channels);

	unsigned int sample_size = in_bits_per_sample / 8;
	unsigned int frame_size = in_num_channels * sample_size; 

	for(auto &entry: queue)
	{
		char * ptr = (char *)entry->m_start_ptr;
		if(0 != (entry->m_size % frame_size))
		{
			WARN("Audio buffer not aligned with frame boundary!\n");
		}

		unsigned int buffer_size = entry->m_size;
		while (buffer_size > 0)
		{
			write_data(ptr, sample_size); //TODO Check for errors
			buffer_size -= frame_size;
			ptr += frame_size;
		}

		size -= entry->m_size;
		if(0 >= size)
		{
			break;
		}
	}
	return ret;
}
#if 0
int audio_converter::downsample(std::list<audio_buffer *> &queue, int size)
{
	int ret = 0;
	
	unsigned int in_sampling_rate, in_bits_per_sample, in_num_channels;
	audiocapturemgr::get_individual_audio_parameters(m_in_props, in_sampling_rate, in_bits_per_sample, in_num_channels);

	unsigned int out_sampling_rate, out_bits_per_sample, out_num_channels;
	audiocapturemgr::get_individual_audio_parameters(m_out_props, out_sampling_rate, out_bits_per_sample, out_num_channels);

	unsigned int frame_size = in_num_channels * in_bits_per_sample * 8; 
	unsigned int leap_value = frame_size * in_sampling_rate / out_sampling_rate;
	int read_offset = 0;

	for(auto &entry: queue)
	{
		char * ptr = (char *)entry->m_start_ptr + read_offset;
		if(0 != (entry->m_size % frame_size))
		{
			WARN("Audio buffer not aligned with frame boundary!\n");
		}

		int buffer_size = entry->m_size;
		while (buffer_size > 0)
		{
			write_data(ptr, frame_size); //TODO Check for errors
			buffer_size -= leap_value;
			ptr += leap_value;
		}
		read_offset = -1 * buffer_size;

		size -= entry->m_size;
		if(0 >= size)
		{
			break;
		}
	}
	return ret;
}
#endif 
int audio_converter::passthrough(const std::list<audio_buffer *> &queue, int size)
{
	int ret = -1;
	for(auto &entry: queue)
	{
		ret = write_data((char *)entry->m_start_ptr, entry->m_size);
		if(0 > ret)
		{
			ERROR("Write error!\n");
			break;
		}
		size -= entry->m_size;
		if(0 > size)
		{
			break;
		}
	}
	return ret;
}

int audio_converter::convert(const std::list<audio_buffer *> &queue, unsigned int size)
{
	int ret = -1;
	INFO("Operation: 0x%x\n", m_op);
	switch(m_op)
	{
		case DOWNMIX_AND_DOWNSAMPLE:
			ret = downsample_and_downmix(queue, size);
			break;
		
		case DOWNMIX:
			ret = downmix(queue, size);
			break;

		case DOWNSAMPLE:
			ret = downsample_and_downmix(queue, size);
			break;

		case NO_CONVERSION:
			ret = passthrough(queue, size);
			break;

		default:
			ERROR("Unsupported conversion.\n");
			ret = -1;
	}
	
	return ret;
}

audio_converter_file_op::audio_converter_file_op(const audiocapturemgr::audio_properties_t &in_props, const audiocapturemgr::audio_properties_t &out_props, std::ofstream &file) :
	audio_converter(in_props, out_props), m_file(file)
{
}

int audio_converter_file_op::write_data(const char * ptr, unsigned int size)
{
	int ret = 0;
	m_file.write(ptr, size); //TODO: Add error handling
	return ret;
}
