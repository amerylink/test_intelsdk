// test_sdk.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "../intel_medias/intel_medias/intel_medias.h"
#pragma comment(lib, "../bin/intel_medias.lib")


int _tmain(int argc, _TCHAR* argv[])
{
	//sInputParams_Dec dec_input;
	//dec_input.bUseHWLib = true;
	//dec_input.memType = enmMemType_System;
	////dec_input.videohWnd = m_hPlayWnd;
	//dec_input.codecId = MFX_CODEC_AVC;//H264;
	//dec_input.nWidth = 1920;
	//dec_input.nHeight = 1080;
	//dec_input.nAsyncDepth = 2;
	sInputParams dec_input;
	handle_t decoder = intelmedia_decoder_open();
	intelmedia_decoder_init(decoder, dec_input);



	getchar();
	return 0;
}

