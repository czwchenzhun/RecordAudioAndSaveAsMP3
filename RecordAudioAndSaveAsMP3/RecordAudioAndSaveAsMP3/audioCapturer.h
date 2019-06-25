#pragma once
#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include"common.h"
#include"fmt.h"

class AudioCapturer
{
public:
	AudioCapturer(AVInputFormat* ifmt, string audioInputUrl);
	~AudioCapturer();
	bool openInput();
	AVPacket* capturePacket();
	AVFrame* captureFrame();
public:
	AVInputFormat* mp_ifmt;
	string m_audioInputUrl;
	AVFormatContext* mp_fmtCtx;
	int m_audioIndex;
	AVCodecContext* mp_audioCodecCtx;
};

#endif