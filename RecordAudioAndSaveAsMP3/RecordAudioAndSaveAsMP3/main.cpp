#include"audioCapturer.h"
#include"audioWrapper.h"
#include<csignal>

static bool run = true;

void signalHandler(int sig)
{
	if (sig == SIGINT)
	{
		run = false;
	}
}

void func(int& strlen_last, char* buffer);

void main()
{
	av_register_all();
	avdevice_register_all();
	AVInputFormat *ifmt = av_find_input_format("dshow");
	//string audioInputUrl = R"(audio=@device_cm_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\wave_{0A9B2B3D-93B2-4F45-8063-F08ABE650585})";
	string audioInputUrl=R"(audio=@device_sw_{33D9A762-90C8-11D0-BD43-00A0C911CE86}\{8E146464-DB61-4309-AFA1-3578E927E935})";
	AudioCapturer ac(ifmt, audioInputUrl);
	AudioWrapper aw("out.mp3");
	if (!ac.openInput())
	{
		goto end;
	}
	if (!aw.openOutput(ac.mp_audioCodecCtx))
	{
		goto end;
	}
	aw.initSwrContext(ac.mp_audioCodecCtx);

	AVFrame* frame = nullptr;
	aw.writeHead();

	int counter = 0;

	signal(SIGINT, signalHandler);
	cout << "Press ctrl+c to terminate!" << endl;

	float sample_rate = ac.mp_audioCodecCtx->sample_rate;
	float sample_time = 0;

	char buffer[10] = {'\0'};
	int  strlen_last = 3;
	cout << "Use time:..." << endl;
	while(run)
	{
		frame = ac.captureFrame();
		if (frame)
		{
			frame->pts = counter;
			counter++;
			if (aw.writeFrame(frame))
			{
				sample_time += (float)frame->nb_samples / sample_rate;
				sprintf(buffer,"%.2f", sample_time);
				func(strlen_last, buffer);
			}
			av_frame_free(&frame);
		}
	}
	aw.writeTrail();

end:
	cout << "\a\nexit" << endl;
	getchar();
}

void func(int& strlen_last,char* buffer)
{
	for (int i = 0; i < strlen_last; i++)
	{
		cout << '\b';
	}
	cout << buffer;
	strlen_last = strlen(buffer);
}