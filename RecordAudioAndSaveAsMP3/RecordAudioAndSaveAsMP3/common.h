#pragma once
#ifndef COMMON_H
#define COMMON_H

extern "C"
{
	#include<libavcodec\avcodec.h>
	#include<libavformat\avformat.h>
	#include<libavdevice\avdevice.h>
	#include<libavutil\frame.h>
	#include<libswresample\swresample.h>

	#pragma comment(lib,"avcodec.lib")
	#pragma comment(lib,"avformat.lib")
	#pragma comment(lib,"avdevice.lib")
	#pragma comment(lib,"avutil.lib")
	#pragma comment(lib,"swresample.lib")
}

#endif // !COMMON_H


