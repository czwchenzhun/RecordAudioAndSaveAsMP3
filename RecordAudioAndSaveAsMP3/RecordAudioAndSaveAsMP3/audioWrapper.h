#pragma once
#ifndef AUDIOWRAPPER
#define AUDIOWRAPPER

#include"common.h"
#include<iostream>
using namespace std;

class AudioWrapper
{
public:
	AudioWrapper(string outputFileName);
	~AudioWrapper();
	bool openOutput(AVCodecContext* inputCodecCtx);
	void initSwrContext(AVCodecContext* inputCodecCtx);
	bool writeHead();
	bool writeTrail();
	bool writeFrame(AVFrame* frame);
public:
	string m_outputFileName;
	AVFormatContext* mp_fmtCtx;
	AVOutputFormat* mp_ofmt;
	AVStream* mp_audioStream;
	AVCodecContext* mp_audioCodecCtx;

	SwrContext* mp_swrCtx;
};

#endif // !AUDIOWRAPPER

