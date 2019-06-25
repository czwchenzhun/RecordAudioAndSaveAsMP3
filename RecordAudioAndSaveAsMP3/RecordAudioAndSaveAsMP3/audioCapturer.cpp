#include"audioCapturer.h"

AudioCapturer::AudioCapturer(AVInputFormat * ifmt, string audioInputUrl)
{
	mp_ifmt = ifmt;
	m_audioInputUrl = audioInputUrl;
	mp_fmtCtx = nullptr;
	mp_audioCodecCtx = nullptr;
}

AudioCapturer::~AudioCapturer()
{
	av_free(mp_fmtCtx);
	av_free(mp_audioCodecCtx);
}

bool AudioCapturer::openInput()
{
	if (!mp_ifmt)
		return false;
	mp_fmtCtx = avformat_alloc_context();
	if (!mp_fmtCtx)
	{
		cout << "Alloc AVFormatContext failed!" << endl;
		return false;
	}
	if (avformat_open_input(&mp_fmtCtx, ANSIToUTF8(m_audioInputUrl).c_str(), mp_ifmt, NULL) != 0)
	{
		cout << "Could not open input stream！" << endl;
	}
	
	m_audioIndex = -1;
	for (int i = 0; i < mp_fmtCtx->nb_streams; i++)
	{
		if (mp_fmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			m_audioIndex = i;
			break;
		}
	}
	if (m_audioIndex == -1)
	{
		cout << "Could not find audio stream!" << endl;
		return false;
	}

	mp_audioCodecCtx = mp_fmtCtx->streams[m_audioIndex]->codec;

	AVCodec* audioCodec = nullptr;
	audioCodec = avcodec_find_decoder(mp_audioCodecCtx->codec_id);
	if (!audioCodec)
	{
		cout << "Audio codec not found!" <<endl;
		return false;
	}

	if (avcodec_open2(mp_audioCodecCtx, audioCodec, NULL) < 0)
	{
		cout << "Could not open audio codec context!" << endl;
		return false;
	}
	//执行完上一步后发现，mp_audioCodecCtx->channel_layout为0但mp_audioCodecCtx->channels为2
	//由于channel_layout在包装器中需要使用，这里找到正确的channel_layout
	if (!mp_audioCodecCtx->channel_layout)
	{
		mp_audioCodecCtx->channel_layout=av_get_channel_layout("stereo");
	}

	return true;
}

AVPacket * AudioCapturer::capturePacket()
{
	AVPacket* packet = av_packet_alloc();
	if (av_read_frame(mp_fmtCtx, packet) < 0)
	{
		av_packet_free(&packet);
		return nullptr;
	}
	else
	{
		return packet;
	}
}

AVFrame * AudioCapturer::captureFrame()
{
	AVPacket* packet = capturePacket();
	if (!packet)
	{
		return nullptr;
	}
	AVFrame* frame = av_frame_alloc();
	int got_frame;
	if (avcodec_decode_audio4(mp_audioCodecCtx, frame, &got_frame, packet) < 0)
	{
		cout << "Decode audio error!" << endl;
		av_frame_free(&frame);
		return nullptr;
	}
	if (!got_frame)
	{
		av_frame_free(&frame);
		return nullptr;
	}
	return frame;
}
