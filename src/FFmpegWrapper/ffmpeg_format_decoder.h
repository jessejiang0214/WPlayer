#pragma once

#include "stdafx.h"
#include "ffmpeg.h"
#include "ffmpeg_format_wrapper.h"

class FF_Format_Stream_Decoder
{
	int RefCount = 1;
	AVFormatContext* pFormatContext = NULL;
	FF_Format_Stream_Wrapper* pMyWrapper = NULL;

	AVCodecContext* pCodecContext = NULL;
	AVCodec* pCodec = NULL;
	AVStream* pStream = NULL;
public:
	FF_Format_Stream_Decoder(AVFormatContext* pContext);
	FF_Format_Stream_Decoder(FF_Format_Stream_Wrapper* pWrapper);
	~FF_Format_Stream_Decoder();
public:
	operator AVCodecContext*() const;
	operator AVCodec*() const;
	operator AVStream*() const;
public:
	int Ref();
	int Unref();
public:
	bool InitializeDecoder(int index,int threads = -1);
};