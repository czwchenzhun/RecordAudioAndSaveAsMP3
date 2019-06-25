#include"audioWrapper.h"

AudioWrapper::AudioWrapper(string outputFileName)
{
	m_outputFileName = outputFileName;
	mp_fmtCtx = nullptr;
	mp_ofmt = nullptr;
	mp_audioStream = nullptr;
	mp_audioCodecCtx = nullptr;
	mp_swrCtx = nullptr;
}

AudioWrapper::~AudioWrapper()
{
	av_free(mp_fmtCtx);
	av_free(mp_ofmt);
	av_free(mp_audioCodecCtx);
	av_free(mp_swrCtx);
}

bool AudioWrapper::openOutput(AVCodecContext* inputCodecCtx)
{
	if (avformat_alloc_output_context2(&mp_fmtCtx, nullptr, nullptr, m_outputFileName.c_str()) < 0)
	{
		cout << "Alloc output context failed!" << endl;
		return false;
	}

	AVCodec* codec=avcodec_find_encoder(mp_fmtCtx->oformat->audio_codec);
	if (!codec)
	{
		cout << "Could not find encoder!" << endl;
		return false;
	}
	mp_audioStream = avformat_new_stream(mp_fmtCtx, codec);
	if (!mp_audioStream)
	{
		cout << "Could not new stream!" << endl;
		avformat_free_context(mp_fmtCtx);
		return false;
	}

	mp_audioCodecCtx = avcodec_alloc_context3(codec);
	if (!mp_audioCodecCtx)
	{
		cout << "Alloc audio codec context failed!" << endl;
		return false;
	}

	mp_audioCodecCtx->sample_fmt = codec->sample_fmts ?
		codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	mp_audioCodecCtx->bit_rate = inputCodecCtx->bit_rate;

	if (codec->supported_samplerates) {
		mp_audioCodecCtx->sample_rate = codec->supported_samplerates[0];
		for (int i = 0; codec->supported_samplerates[i]; i++) {
			if (codec->supported_samplerates[i] == inputCodecCtx->sample_rate)
			{
				mp_audioCodecCtx->sample_rate = inputCodecCtx->sample_rate;
				break;
			}
		}
	}
	mp_audioCodecCtx->channels = av_get_channel_layout_nb_channels(mp_audioCodecCtx->channel_layout);
	mp_audioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
	if (codec->channel_layouts) {
		mp_audioCodecCtx->channel_layout = codec->channel_layouts[0];
		for (int i = 0; codec->channel_layouts[i]; i++) {
			if (codec->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
			{
				mp_audioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
				break;
			}
		}
	}

	mp_audioCodecCtx->channels = av_get_channel_layout_nb_channels(mp_audioCodecCtx->channel_layout);
	mp_audioCodecCtx->time_base = { 1,mp_audioCodecCtx->sample_rate };

	mp_audioStream->time_base = mp_audioCodecCtx->time_base;
	if (avcodec_parameters_from_context(mp_audioStream->codecpar, mp_audioCodecCtx) < 0)
	{
		cout << "Set parameters failed!" << endl;
		return false;
	}

	av_dump_format(mp_fmtCtx, 0, m_outputFileName.c_str(), true);

	if (avcodec_open2(mp_audioCodecCtx, codec, nullptr) != 0)
	{
		cout << "Could not open audio codec context!" << endl;
		return false;
	}
	
	if (mp_fmtCtx->flags&AVFMT_NOFILE)
	{
		cout << "Format Context has no file!" << endl;
		return false;
	}
	if (avio_open(&mp_fmtCtx->pb, m_outputFileName.c_str(), AVIO_FLAG_WRITE)<0)
	{
		cout << "AVIO open failed!" << endl;
		avformat_free_context(mp_fmtCtx);
		return false;
	}
	return true;
}

//如果输入的帧数据格式与编码器不兼容则转换格式
void AudioWrapper::initSwrContext(AVCodecContext* inputCodecCtx)
{
	if (mp_audioCodecCtx->sample_fmt != inputCodecCtx->sample_fmt)
	{
		mp_swrCtx = swr_alloc_set_opts(nullptr, mp_audioCodecCtx->channel_layout, mp_audioCodecCtx->sample_fmt, mp_audioCodecCtx->sample_rate, \
			inputCodecCtx->channel_layout, inputCodecCtx->sample_fmt, inputCodecCtx->sample_rate, 0, nullptr);
		if(mp_swrCtx)
			swr_init(mp_swrCtx);
	}
}

bool AudioWrapper::writeHead()
{
	if (avformat_write_header(mp_fmtCtx, nullptr) < 0)
	{
		return false;
	}
	return true;
}

bool AudioWrapper::writeTrail()
{
	if (av_write_trailer(mp_fmtCtx) == 0)
	{
		return true;
	}
	return false;
}

bool AudioWrapper::writeFrame(AVFrame * inputFrame)
{
	AVFrame* frame = nullptr;
	bool flag = false;//用来记录frame是函数内申请的还是inputFrame
	if (mp_swrCtx)
	{
		flag = true;

		frame = av_frame_alloc();
		if (!frame)
		{
			return false;
		}
		// 设置Frame的参数
		frame->nb_samples = inputFrame->nb_samples;
		frame->format = mp_audioCodecCtx->sample_fmt;
		frame->channel_layout = mp_audioCodecCtx->channel_layout;

		// 申请数据内存
		
		if (av_frame_get_buffer(frame, 0)< 0)
		{
			av_frame_free(&frame);
			return false;
		}
		
		if (av_frame_make_writable(frame) < 0)
		{
			av_frame_free(&frame);
			return false;
		}
		if (swr_convert(mp_swrCtx, frame->data, inputFrame->nb_samples, (const uint8_t **)inputFrame->data, inputFrame->nb_samples) < 0)
		{
			cout << "Sample convert failed!" << endl;
			return false;
		}
	}
	else
	{
		frame = inputFrame;
	}

	if (avcodec_send_frame(mp_audioCodecCtx, frame) != 0)
	{
		if (flag)
		{
			av_frame_free(&frame);
		}
		return false;
	}

	if (flag)
	{
		av_frame_free(&frame);
	}

	AVPacket* pkt = av_packet_alloc();
	// 接收编码后的Packet

	if (avcodec_receive_packet(mp_audioCodecCtx, pkt) < 0)
	{
		av_packet_free(&pkt);
		return false;
	}

	// 写入文件
	av_packet_rescale_ts(pkt, mp_audioCodecCtx->time_base, mp_audioStream->time_base);
	pkt->stream_index = 0;
	if (av_interleaved_write_frame(mp_fmtCtx, pkt) < 0)
	{
		av_packet_free(&pkt);
		return false;
	}
	return true;
}
